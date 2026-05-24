#pragma once

#include <cstdint>
#include <string>
#include <vector>

enum class RsaFocusField { None, P, Q, Ks, R };

struct RsaEncryptParams {
    uint32_t p = 0;
    uint32_t q = 0;
    uint32_t Ks = 0;
    uint32_t r = 0;
    uint32_t phi = 0;
    int32_t Ko = 0;
};

struct RsaResult {
    bool ok = false;
    std::wstring error;
    RsaFocusField focus = RsaFocusField::None;
    std::vector<std::wstring> log;
    uint32_t r = 0;
    uint32_t phi = 0;
    int32_t Ko = 0;
};

bool parse_uint32(const std::wstring &text, uint32_t &out);

RsaResult validate_encrypt_params(uint32_t p, uint32_t q, uint32_t Ks);
RsaResult validate_decrypt_params(uint32_t r, uint32_t Ks);
RsaResult compute_keys(uint32_t p, uint32_t q, uint32_t Ks);
RsaResult encrypt_file(const std::wstring &inputPath, const std::wstring &outputPath,
                       uint32_t p, uint32_t q, uint32_t Ks);
RsaResult decrypt_file(const std::wstring &inputPath, const std::wstring &outputPath,
                       uint32_t r, uint32_t Ks);
