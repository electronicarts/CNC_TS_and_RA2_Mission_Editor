"""Validate the shipped translations using the editor's actual INI rules.

Run with Python 3, from any directory:
    python FinalAlert2YR-src/MissionEditor/tests/test_localization.py

No generated files, third-party packages, or running editor are needed.  The
parser intentionally does not use configparser: CIniFile::InsertFile treats a
semicolon as a comment even inside quotes, recognizes brackets before '=', and
preserves key/value whitespace when bNoSpaces is false (the language default).
"""

from collections import Counter
from dataclasses import dataclass, field
from pathlib import Path
import re
import unittest


EDITOR = Path(__file__).resolve().parents[1]
LANGUAGE_DATA = EDITOR / "data" / "FinalAlert2"
LANGUAGES = ("Russian", "Ukrainian", "Spanish")
SUFFIXES = ("StringsRA2", "Strings", "TranslationsRA2", "Translations")


@dataclass
class IniDocument:
    sections: dict[str, dict[str, str]] = field(default_factory=dict)
    duplicates: list[str] = field(default_factory=list)
    declarations: list[str] = field(default_factory=list)
    commented_values: list[str] = field(default_factory=list)


def parse_editor_ini(text: str, *, no_spaces: bool = False) -> IniDocument:
    """Emulate IniFile.cpp::InsertFile, including silent key overwrites."""
    document = IniDocument()
    section = ""
    # getline uses '\n'; splitlines would incorrectly treat other characters as
    # record delimiters and would hide malformed literal CRs inside values.
    for number, raw in enumerate(text.split("\n"), 1):
        line = re.split(r"[\r\n;]", raw, maxsplit=1)[0]
        opening, closing, equals = line.find("["), line.find("]"), line.find("=")
        if (
            opening != -1
            and closing != -1
            and opening < closing
            and (equals == -1 or equals > opening)
        ):
            section = line[opening + 1 : closing]
            document.declarations.append(section)
        elif equals != -1 and section:
            key, value = line[:equals], line[equals + 1 :]
            if no_spaces:
                key, value = key.strip(), value.strip()
            values = document.sections.setdefault(section, {})
            if key in values:
                document.duplicates.append(f"line {number}: [{section}] {key}")
            values[key] = value
            # Comments on their own lines are welcome.  After a translation
            # assignment they can silently remove the end of a user message.
            if ";" in raw:
                document.commented_values.append(f"line {number}: [{section}] {key}")
    return document


def language_key(text: str) -> str:
    """Match functions.cpp::LanguageKey (normalization precedes lookup)."""
    text = text.strip()
    for old, new in (
        ("\n", r"\n"),
        ("\r", r"\r"),
        ("\t", r"\t"),
        ("=", r"\x3d"),
        ("[", r"\x5b"),
        ("]", r"\x5d"),
        (";", r"\x3b"),
    ):
        text = text.replace(old, new)
    return text


def language_value(text: str) -> str:
    """Match LanguageValue's literal control-character decoding."""
    return text.replace(r"\n", "\n").replace(r"\r", "\r").replace(r"\t", "\t")


def required_tokens(text: str) -> Counter:
    """Tokens which must survive translation, allowing grammatical reordering."""
    decoded = language_value(text)
    tokens = Counter(re.findall(r"%[1-9]|\||[\t\r\n]", decoded))
    wildcards = re.findall(r"\*\.[A-Za-z0-9]+", decoded)
    # Save-filter positions/counts are significant; explanatory prose can
    # repeat an extension when that reads more naturally in the translation.
    tokens.update(wildcards if "|" in decoded else set(wildcards))
    return tokens


RC_TEXT = re.compile(r'"((?:[^"\\]|\\.|"")*)"')
CAPTION_DIRECTIVES = {
    "CAPTION", "LTEXT", "RTEXT", "CTEXT", "PUSHBUTTON", "DEFPUSHBUTTON",
    "GROUPBOX", "CONTROL", "MENUITEM", "POPUP", "CHECKBOX", "AUTOCHECKBOX",
    "RADIOBUTTON", "AUTORADIOBUTTON", "STATE3", "AUTO3STATE", "USERBUTTON",
}
# These STRINGTABLE entries identify the build/author, rather than UI language.
BUILD_METADATA_IDS = {"IDS_BUILTBY", "IDS_VERSION", "IDS_LINEARVERSION"}


def resource_captions(text: str) -> dict[str, str]:
    """Read actual RC captions, menu labels and STRINGTABLE tooltip strings.

    All conditional branches are checked, including legacy TS captions. Only
    the first quoted field is a caption; a CONTROL's class name is not one.
    RC doubled quotes become quotes, and \n/\r/\t become runtime characters.
    """
    captions = {}
    in_stringtable = False
    for line in text.splitlines():
        stripped = line.strip()
        if stripped == "STRINGTABLE":
            in_stringtable = True
            continue
        if in_stringtable and stripped == "END":
            in_stringtable = False
            continue
        directive = stripped.split(None, 1)[0] if stripped else ""
        if directive not in CAPTION_DIRECTIVES and not (
            in_stringtable and directive.startswith(("ID_", "IDS_"))
        ):
            continue
        if directive in BUILD_METADATA_IDS:
            continue
        match = RC_TEXT.search(line)
        if not match:
            continue
        caption = language_value(match.group(1).replace('""', '"')).replace(r'\"', '"')
        # Empty, numeric and arrow-only labels need no linguistic translation.
        if not any(character.isalpha() for character in caption):
            continue
        key = language_key(caption)
        captions.setdefault(key, caption.strip())
    return captions


def read_legacy_language() -> IniDocument:
    data = (LANGUAGE_DATA / "FALanguage.ini").read_bytes()
    try:
        text = data.decode("utf-8")
    except UnicodeDecodeError:
        text = data.decode("cp1252")
    return parse_editor_ini(text)


class ParserRegressionTests(unittest.TestCase):
    def test_real_parser_comment_bracket_and_whitespace_behavior(self):
        parsed = parse_editor_ini(
            '[Main]\n key = value ;discarded\nQuoted="a;b"\n'
            'Text=value [brackets]\nDESC [quite long]=lost caption\n'
            'After=wrong section\n[Main]\n key =replacement\n'
        )
        self.assertEqual(parsed.sections["Main"][" key "], "replacement")
        self.assertEqual(parsed.sections["Main"]["Quoted"], '"a')
        self.assertEqual(parsed.sections["Main"]["Text"], "value [brackets]")
        self.assertEqual(parsed.sections["quite long"], {"After": "wrong section"})
        self.assertEqual(len(parsed.duplicates), 1)
        self.assertNotIn(" key", parsed.sections["Main"])
        trimmed = parse_editor_ini("[S]\n a = b \n", no_spaces=True)
        self.assertEqual(trimmed.sections["S"], {"a": "b"})

    def test_lookup_escaping_keeps_dangerous_captions_in_their_section(self):
        caption = " \tAdd = [item]; details\nnow "
        key = language_key(caption)
        self.assertEqual(key, r"Add \x3d \x5bitem\x5d\x3b details\nnow")
        parsed = parse_editor_ini(f"[Captions]\n{key}=Translated\nNext=Still here\n")
        self.assertEqual(parsed.sections["Captions"][key], "Translated")
        self.assertEqual(parsed.sections["Captions"]["Next"], "Still here")

    def test_resource_quotes_menus_and_controls(self):
        captions = resource_captions(
            'LTEXT "The ""useable"" size",IDC_STATIC,0,0,10,10\n'
            'MENUITEM "Copy\\tCtrl+C",ID_COPY\n'
            'CONTROL "Add = Item",IDC_ADD,"Button",0,0,0,10,10\n'
            'LTEXT "42",IDC_NUMBER,0,0,10,10\n'
        )
        self.assertEqual(set(captions), {'The "useable" size', r"Copy\tCtrl+C", r"Add \x3d Item"})


class TranslationRegressionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.base = read_legacy_language()
        cls.captions = resource_captions((EDITOR / "MissionEditor.rc").read_text(encoding="utf-8-sig"))
        cls.documents = {}
        cls.raw_fragments = {}
        for language in LANGUAGES:
            # Strict UTF-8 catches a re-save in the local Windows code page.
            raw = (LANGUAGE_DATA / "Translations" / f"{language}.ini").read_bytes().decode("utf-8")
            cls.raw_fragments[language] = raw
            cls.documents[language] = parse_editor_ini(raw)

    def test_fragments_have_valid_sections_and_no_silent_data_loss(self):
        for language, document in self.documents.items():
            with self.subTest(language=language):
                expected = {language + "Header", *(language + "-" + suffix for suffix in SUFFIXES)}
                self.assertEqual(set(document.declarations), expected)
                self.assertEqual(set(document.sections), expected)
                self.assertFalse(document.duplicates, "Duplicate INI keys:\n" + "\n".join(document.duplicates))
                self.assertFalse(document.commented_values, "Semicolons truncate translation values:\n" + "\n".join(document.commented_values))
                self.assertNotIn("\ufffd", self.raw_fragments[language])
                self.assertFalse(self.raw_fragments[language].startswith("\ufeff"), "INI parser does not strip a BOM")
                header = document.sections[language + "Header"]
                self.assertTrue(header.get("Name"))
                self.assertEqual(header.get("Version"), "1")
                self.assertTrue(header.get("ExtensionName"))
                for suffix in ("Translations", "TranslationsRA2"):
                    for key in document.sections[language + "-" + suffix]:
                        self.assertEqual(key, language_key(key), f"Non-normalized translation key: {key!r}")
                        self.assertNotRegex(key, r"\\(?![nrt]|x(?:3d|3b|5b|5d))", f"Unrecognized key escape: {key!r}")
                for section, values in document.sections.items():
                    for key, value in values.items():
                        self.assertNotRegex(value, r"\\(?![nrt])", f"LanguageValue cannot decode escape in [{section}] {key}")

    def test_all_english_and_german_keys_are_present(self):
        for language, document in self.documents.items():
            for suffix in SUFFIXES:
                expected = set(self.base.sections["English-" + suffix])
                if suffix.startswith("Translations"):
                    expected.update(self.base.sections["German-" + suffix])
                    expected = {language_key(key) for key in expected}
                with self.subTest(language=language, section=suffix):
                    missing = sorted(expected - document.sections[language + "-" + suffix].keys())
                    self.assertFalse(missing, "Missing source keys:\n" + "\n".join(missing))

    def test_all_meaningful_resource_captions_are_present(self):
        self.assertGreater(len(self.captions), 400, "Resource scanner unexpectedly lost most captions")
        for language, document in self.documents.items():
            available = document.sections[language + "-Translations"]
            with self.subTest(language=language):
                missing = sorted(self.captions.keys() - available.keys())
                self.assertFalse(missing, "Untranslated RC captions:\n" + "\n".join(missing))

    def test_source_placeholders_filters_and_escapes_are_preserved(self):
        for language, document in self.documents.items():
            for suffix in SUFFIXES:
                source = dict(self.base.sections["English-" + suffix])
                if suffix.startswith("Translations"):
                    # English values are the primary source; German-only keys
                    # supply additional placeholders and shortcut sequences.
                    source = {**self.base.sections["German-" + suffix], **source}
                translated = document.sections[language + "-" + suffix]
                for key, value in source.items():
                    lookup = language_key(key) if suffix.startswith("Translations") else key
                    if lookup not in translated:
                        continue  # Coverage test reports missing entries.
                    with self.subTest(language=language, section=suffix, key=key):
                        self.assertEqual(required_tokens(translated[lookup]), required_tokens(value))
                        self.assertEqual(bool(translated[lookup].strip()), bool(value.strip()))

    def test_resource_shortcuts_and_literal_escapes_are_preserved(self):
        for language, document in self.documents.items():
            translated = document.sections[language + "-Translations"]
            for key, caption in self.captions.items():
                if key not in translated:
                    continue
                with self.subTest(language=language, caption=key):
                    actual = language_value(translated[key])
                    self.assertTrue(actual.strip(), "Empty caption translation")
                    self.assertEqual(required_tokens(actual), required_tokens(caption))
                    if "\t" in caption:
                        self.assertEqual(actual.split("\t")[1:], caption.split("\t")[1:], "Shortcut changed")
                    self.assertEqual(actual.count("&&"), caption.count("&&"), "Literal ampersand changed")
                    self.assertEqual(actual.replace("&&", "").count("&"), caption.replace("&&", "").count("&"), "Accelerator marker lost")


if __name__ == "__main__":
    unittest.main(verbosity=2)
