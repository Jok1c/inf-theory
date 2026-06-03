#include "LfsrLogic.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <filesystem>
#include <fstream>
#include <iterator>

namespace {

constexpr int kFormula[] = {3, 28};

static bool is_binary_key_complete(const std::wstring &key)
{
    if (key.size() != static_cast<size_t>(kRegisterLength))
        return false;
    for (wchar_t ch : key) {
        if (ch != L'0' && ch != L'1')
            return false;
    }
    return true;
}

} // namespace

LfsrKeyValidation validate_register_key(const std::wstring &key)
{
    if (key.empty())
        return {true, L""};
    if (key.size() < static_cast<size_t>(kRegisterLength)) {
        return {false, L"Введите ровно " + std::to_wstring(kRegisterLength) + L" символов (сейчас " +
                         std::to_wstring(key.size()) + L")."};
    }
    return {true, L""};
}

std::wstring filter_binary_key(const std::wstring &key)
{
    std::wstring filtered;
    filtered.reserve(key.size());
    for (wchar_t ch : key) {
        if (ch == L'0' || ch == L'1')
            filtered.push_back(ch);
    }
    if (filtered.size() > static_cast<size_t>(kRegisterLength))
        filtered.resize(kRegisterLength);
    return filtered;
}

std::wstring bytes_to_bits_string(const std::vector<uint8_t> &bytes, size_t totalLength)
{
    if (bytes.empty())
        return L"";

    const size_t actualTotal = totalLength > 0 ? totalLength : bytes.size();
    const size_t lengthToProcess = std::min(bytes.size(), kDisplayLimit);

    std::wstring bits;
    for (size_t i = 0; i < lengthToProcess; ++i) {
        for (int bit = 7; bit >= 0; --bit)
            bits += ((bytes[i] >> bit) & 1) ? L'1' : L'0';
        bits += L' ';
    }

    if (actualTotal > kDisplayLimit) {
        bits += L"\r\n\r\n... [Показаны первые ";
        bits += std::to_wstring(kDisplayLimit);
        bits += L" байт из ";
        bits += std::to_wstring(actualTotal);
        bits += L"]";
    }
    return bits;
}

std::vector<uint8_t> wide_to_utf8(const std::wstring &text)
{
    if (text.empty())
        return {};

    const int size =
        WideCharToMultiByte(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr);
    if (size <= 0)
        return {};

    std::vector<uint8_t> out(static_cast<size_t>(size));
    WideCharToMultiByte(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()),
                        reinterpret_cast<char *>(out.data()), size, nullptr, nullptr);
    return out;
}

LfsrEncryptResult lfsr_xor_crypt(const std::vector<uint8_t> &source, const std::wstring &keyBits)
{
    if (source.empty())
        return {false, L"Сначала введите текст или выберите файл.", {}, {}};
    if (!is_binary_key_complete(keyBits)) {
        return {false,
                L"Ключ должен быть ровно " + std::to_wstring(kRegisterLength) + L" символов.",
                {},
                {}};
    }

    const size_t len = source.size();
    std::vector<uint8_t> result(len);
    std::vector<int> state(static_cast<size_t>(kRegisterLength));
    for (int i = 0; i < kRegisterLength; ++i)
        state[static_cast<size_t>(i)] = keyBits[static_cast<size_t>(i)] == L'1' ? 1 : 0;

    const size_t keyPreviewLength = std::min(len, kDisplayLimit);
    std::vector<uint8_t> keyPreview(keyPreviewLength);

    for (size_t i = 0; i < len; ++i) {
        uint8_t generatedKeyByte = 0;
        for (int b = 0; b < 8; ++b) {
            int xorVal = 0;
            for (int tap : kFormula)
                xorVal ^= state[state.size() - static_cast<size_t>(tap)];

            generatedKeyByte |= static_cast<uint8_t>(state[0] << (7 - b));
            state.erase(state.begin());
            state.push_back(xorVal);
        }

        if (i < keyPreviewLength)
            keyPreview[i] = generatedKeyByte;
        result[i] = source[i] ^ generatedKeyByte;
    }

    return {true, L"", std::move(result), std::move(keyPreview)};
}

bool write_binary_file(const std::wstring &path, const std::vector<uint8_t> &data)
{
    std::ofstream out(std::filesystem::path(path), std::ios::binary);
    if (!out.is_open())
        return false;
    if (!data.empty())
        out.write(reinterpret_cast<const char *>(data.data()), static_cast<std::streamsize>(data.size()));
    return static_cast<bool>(out);
}

bool read_binary_file(const std::wstring &path, std::vector<uint8_t> &data, std::wstring &error)
{
    std::ifstream in(std::filesystem::path(path), std::ios::binary);
    if (!in.is_open()) {
        error = L"Не удалось открыть файл.";
        return false;
    }
    data.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    return true;
}

std::wstring suggest_output_filename(const std::wstring &inputPath, bool fromText)
{
    if (fromText)
        return L"encrypted_text.bin";

    if (inputPath.size() >= 4 && inputPath.compare(inputPath.size() - 4, 4, L".enc") == 0)
        return inputPath.substr(0, inputPath.size() - 4);
    return inputPath + L".enc";
}
