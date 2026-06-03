#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

constexpr int kRegisterLength = 28;
constexpr size_t kDisplayLimit = 5000;

struct LfsrKeyValidation {
    bool ok = true;
    std::wstring error;
};

struct LfsrEncryptResult {
    bool ok = false;
    std::wstring error;
    std::vector<uint8_t> result;
    std::vector<uint8_t> keyPreview;
};

LfsrKeyValidation validate_register_key(const std::wstring &key);
std::wstring filter_binary_key(const std::wstring &key);
std::wstring bytes_to_bits_string(const std::vector<uint8_t> &bytes, size_t totalLength = 0);
std::vector<uint8_t> wide_to_utf8(const std::wstring &text);
LfsrEncryptResult lfsr_xor_crypt(const std::vector<uint8_t> &source, const std::wstring &keyBits);
bool write_binary_file(const std::wstring &path, const std::vector<uint8_t> &data);
bool read_binary_file(const std::wstring &path, std::vector<uint8_t> &data, std::wstring &error);
std::wstring suggest_output_filename(const std::wstring &inputPath, bool fromText);
