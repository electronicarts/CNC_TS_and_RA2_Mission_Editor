#pragma once

#include <windows.h>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

// CSF stores complemented UTF-16LE. Some older Russian releases instead
// widened Windows-1251 bytes to U+00xx for their custom game font.
namespace CsfText
{
struct Entry
{
    std::string label;
    std::wstring text;
};

inline std::vector<Entry> Read(const unsigned char* data, size_t size)
{
    size_t pos = 0;
    auto take = [&](size_t count) {
        if (count > size - pos)
            throw std::runtime_error("Truncated CSF string file");
        const auto* result = data + pos;
        pos += count;
        return result;
    };
    auto number = [&]() {
        uint32_t value;
        std::memcpy(&value, take(4), 4);
        return value;
    };
    if (std::memcmp(take(4), " FSC", 4) != 0)
        throw std::runtime_error("Invalid CSF signature");
    number(); // version
    const auto labels = number();
    number(); // string count
    number(); // reserved
    number(); // language (unreliable in legacy translations)
    std::vector<Entry> entries;
    for (uint32_t i = 0; i < labels; ++i)
    {
        if (std::memcmp(take(4), " LBL", 4) != 0)
            throw std::runtime_error("Invalid CSF label");
        const auto values = number();
        const auto labelSize = number();
        const auto* label = take(labelSize);
        Entry entry{ std::string(reinterpret_cast<const char*>(label), labelSize), {} };
        for (uint32_t j = 0; j < values; ++j)
        {
            const auto* tag = take(4);
            const bool extra = std::memcmp(tag, "WRTS", 4) == 0;
            if (!extra && std::memcmp(tag, " RTS", 4) != 0)
                throw std::runtime_error("Invalid CSF string");
            const auto length = number();
            if (length > (size - pos) / 2)
                throw std::runtime_error("Invalid CSF string length");
            const auto* text = take(static_cast<size_t>(length) * 2);
            if (j == 0)
            {
                entry.text.reserve(length);
                for (uint32_t k = 0; k < length; ++k)
                    entry.text.push_back(static_cast<wchar_t>(~(text[k * 2] | (text[k * 2 + 1] << 8)) & 0xffff));
            }
            if (extra)
            {
                const auto extraSize = number();
                take(extraSize);
            }
        }
        if (values)
            entries.push_back(std::move(entry));
    }
    return entries;
}

inline bool IsLegacyRussian(const std::vector<Entry>& entries)
{
    size_t western = 0, ascii = 0, cyrillic = 0;
    for (const auto& entry : entries)
        for (const wchar_t ch : entry.text)
        {
            if (ch >= 0xc0 && ch <= 0xff) ++western;
            if ((ch >= L'A' && ch <= L'Z') || (ch >= L'a' && ch <= L'z')) ++ascii;
            if (ch >= 0x400 && ch <= 0x4ff) ++cyrillic;
        }
    // Decide from the whole file, never a single accented Spanish/German name.
    return western >= 64 && western > ascii && western > cyrillic * 4;
}

inline std::wstring DecodeLegacyRussian(const std::wstring& text)
{
    std::string bytes;
    bytes.reserve(text.size());
    for (const wchar_t ch : text)
    {
        // Keep already-Unicode strings untouched, including mixed scripts.
        if (ch > 0xff) return text;
        bytes.push_back(static_cast<char>(ch));
    }
    if (bytes.empty()) return text;
    const int count = MultiByteToWideChar(1251, 0, bytes.data(), static_cast<int>(bytes.size()), nullptr, 0);
    if (!count) return text;
    std::wstring result(count, L'\0');
    MultiByteToWideChar(1251, 0, bytes.data(), static_cast<int>(bytes.size()), result.data(), count);
    return result;
}
}
