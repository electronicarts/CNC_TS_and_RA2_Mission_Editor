// Standalone Win32 test: compile with v143 /std:c++17 /EHsc /utf-8.
#include "../CsfText.h"
#include <fstream>
#include <iostream>
#include <iterator>

void Check(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

int main(int argc, char** argv)
{
    try
    {
        const std::wstring broken = L"\u00D2\u00E0\u00ED\u00EA \u00D0\u00E8\u00ED\u00EE";
        Check(CsfText::DecodeLegacyRussian(broken) == L"Танк Рино", "Legacy Russian decoding");
        Check(CsfText::DecodeLegacyRussian(L"Їжак, Європа, Ґрунт") == L"Їжак, Європа, Ґрунт", "Native Ukrainian preservation");
        Check(CsfText::DecodeLegacyRussian(L"Танк Рино") == L"Танк Рино", "Native Russian preservation");
        Check(CsfText::DecodeLegacyRussian(L"").empty(), "Empty string");
        Check(!CsfText::IsLegacyRussian({{"Name:TANK", broken}}), "Small sample must not trigger repair");
        std::vector<CsfText::Entry> russian, spanish, unicode, english;
        for (int i = 0; i < 100; ++i)
        {
            russian.push_back({"Name:TANK", broken});
            spanish.push_back({"Name:TANK", L"España: cañón, avión, infantería, vehículo, rápido"});
            unicode.push_back({"Name:TANK", L"Танк Рино, Україна, піхота"});
            english.push_back({"Name:TANK", L"Rhino Heavy Tank"});
        }
        Check(CsfText::IsLegacyRussian(russian), "Legacy corpus detection");
        Check(!CsfText::IsLegacyRussian(spanish), "Spanish accents must not trigger repair");
        Check(!CsfText::IsLegacyRussian(unicode), "Native Unicode must not trigger repair");
        Check(!CsfText::IsLegacyRussian(english), "English must not trigger repair");

        // A label with no value followed by a WRTS value and its extra data.
        std::vector<unsigned char> data;
        auto number = [&](uint32_t n) { for (int i = 0; i < 4; ++i) data.push_back((n >> (8 * i)) & 255); };
        auto tag = [&](const char* s) { data.insert(data.end(), s, s + 4); };
        tag(" FSC"); number(3); number(2); number(1); number(0); number(0);
        tag(" LBL"); number(0); number(4); tag("ZERO");
        tag(" LBL"); number(1); number(4); tag("TEST");
        tag("WRTS"); number(1); data.push_back(0xBE); data.push_back(0xFF); number(3);
        data.insert(data.end(), {1, 2, 3});
        const auto parsed = CsfText::Read(data.data(), data.size());
        Check(parsed.size() == 1 && parsed[0].label == "TEST" && parsed[0].text == L"A", "CSF label/extra data parsing");
        for (size_t size = 0; size < data.size(); ++size)
        {
            bool rejected = false;
            try { CsfText::Read(data.data(), size); } catch (const std::runtime_error&) { rejected = true; }
            Check(rejected, "Truncated CSF must fail safely");
        }
        data[0] = 0;
        bool rejected = false;
        try { CsfText::Read(data.data(), data.size()); } catch (const std::runtime_error&) { rejected = true; }
        Check(rejected, "Invalid signature must fail safely");
        if (argc > 1)
        {
            std::ifstream file(argv[1], std::ios::binary);
            Check(file.good(), "Could not open real CSF");
            const std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(file)), {});
            const auto actual = CsfText::Read(bytes.data(), bytes.size());
            Check(CsfText::IsLegacyRussian(actual), "Installed Russian CSF detection");
            size_t repaired = 0;
            for (const auto& entry : actual)
            {
                const auto decoded = CsfText::DecodeLegacyRussian(entry.text);
                if (decoded != entry.text) ++repaired;
                Check(CsfText::DecodeLegacyRussian(decoded) == decoded, "Decoding must be idempotent");
            }
            Check(repaired > 100, "Real names must be repaired");
            std::cout << "Real CSF: " << actual.size() << " labels, " << repaired << " repaired\n";
        }
        std::cout << "CSF encoding and bounds tests passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
