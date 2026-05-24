#include "RsaLogic.h"

#include "MathUtils.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

bool parse_uint32(const std::wstring &text, uint32_t &out)
{
    if (text.empty())
        return false;
    try {
        unsigned long v = std::stoul(text);
        if (v > UINT32_MAX)
            return false;
        out = static_cast<uint32_t>(v);
        return true;
    } catch (...) {
        return false;
    }
}

static RsaResult fail(const std::wstring &msg, RsaFocusField focus = RsaFocusField::None)
{
    return {false, msg, focus, {}};
}

RsaResult validate_encrypt_params(uint32_t p, uint32_t q, uint32_t Ks)
{
    if (!is_prime(static_cast<int32_t>(p)))
        return fail(L"p должно быть простым числом!", RsaFocusField::P);
    if (!is_prime(static_cast<int32_t>(q)))
        return fail(L"q должно быть простым числом!", RsaFocusField::Q);
    if (p == q)
        return fail(L"p и q должны быть разными числами!");

    uint32_t r = p * q;
    uint32_t phi = (p - 1) * (q - 1);

    if (r <= 256 || r >= 65536)
        return fail(L"r должен находиться в пределах от 256 до 65536. Подберите другие p и q.");

    if (Ks <= 1 || Ks >= phi)
        return fail(L"Kз должен находиться в пределах от 1 до ф(r)", RsaFocusField::Ks);

    int32_t x, y;
    if (extended_evklid(static_cast<int32_t>(Ks), static_cast<int32_t>(phi), x, y) != 1)
        return fail(L"Значения Kз и φ(r) должны быть взаимно простыми", RsaFocusField::Ks);

    return {true, L"", RsaFocusField::None, {}};
}

RsaResult validate_decrypt_params(uint32_t r, uint32_t Ks)
{
    if (r <= 256 || r >= 65536)
        return fail(L"Модуль r должен быть в диапазоне (256, 65536)", RsaFocusField::R);
    if (Ks <= 1)
        return fail(L"Закрытый ключ Kз должен быть > 1!", RsaFocusField::Ks);
    return {true, L"", RsaFocusField::None, {}};
}

RsaResult compute_keys(uint32_t p, uint32_t q, uint32_t Ks)
{
    RsaResult check = validate_encrypt_params(p, q, Ks);
    if (!check.ok)
        return check;

    uint32_t r = p * q;
    uint32_t phi = (p - 1) * (q - 1);
    int32_t Ko = mod_inverse(static_cast<int32_t>(Ks), static_cast<int32_t>(phi));
    if (Ko < 0)
        return fail(L"Не удалось вычислить Ko. Проверьте Kз.", RsaFocusField::Ks);

    RsaResult ok;
    ok.ok = true;
    ok.r = r;
    ok.phi = phi;
    ok.Ko = Ko;
    return ok;
}

RsaResult encrypt_file(const std::wstring &inputPath, const std::wstring &outputPath,
                       uint32_t p, uint32_t q, uint32_t Ks)
{
    RsaResult check = validate_encrypt_params(p, q, Ks);
    if (!check.ok)
        return check;

    uint32_t r = p * q;
    uint32_t phi = (p - 1) * (q - 1);
    int32_t Ko = mod_inverse(static_cast<int32_t>(Ks), static_cast<int32_t>(phi));
    if (Ko < 0)
        return fail(L"Не удалось вычислить Ko.", RsaFocusField::Ks);

    std::ifstream fin(std::filesystem::path(inputPath), std::ios::binary);
    if (!fin.is_open())
        return fail(L"Не удалось открыть входной файл!");

    std::ofstream fout(std::filesystem::path(outputPath), std::ios::binary);
    if (!fout.is_open())
        return fail(L"Не удалось создать выходной файл!");

    std::vector<uint8_t> data((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
    fin.close();

    RsaResult result;
    result.ok = true;

    for (size_t i = 0; i < data.size(); ++i) {
        uint8_t m = data[i];
        uint32_t c = mod_pow(static_cast<uint32_t>(m), static_cast<uint32_t>(Ko), r);
        uint16_t block = static_cast<uint16_t>(c);
        fout.write(reinterpret_cast<const char *>(&block), sizeof(block));

        if (i < 100)
            result.log.push_back(std::to_wstring(m) + L" -> " + std::to_wstring(c));
    }

    if (data.size() > 100)
        result.log.push_back(L"...");
    result.log.push_back(L"Шифрование завершено. Файл сохранен.");
    fout.close();
    return result;
}

RsaResult decrypt_file(const std::wstring &inputPath, const std::wstring &outputPath,
                       uint32_t r, uint32_t Ks)
{
    RsaResult check = validate_decrypt_params(r, Ks);
    if (!check.ok)
        return check;

    std::ifstream fin(std::filesystem::path(inputPath), std::ios::binary);
    if (!fin.is_open())
        return fail(L"Не удалось открыть зашифрованный файл!");

    std::ofstream fout(std::filesystem::path(outputPath), std::ios::binary);
    if (!fout.is_open())
        return fail(L"Не удалось создать выходной файл!");

    RsaResult result;
    result.ok = true;

    uint16_t block = 0;
    int count = 0;
    while (fin.read(reinterpret_cast<char *>(&block), sizeof(block))) {
        uint32_t c = static_cast<uint32_t>(block);
        uint32_t m = mod_pow(c, Ks, r);
        uint8_t byte_val = static_cast<uint8_t>(m & 0xFF);
        fout.write(reinterpret_cast<const char *>(&byte_val), 1);

        if (count < 100)
            result.log.push_back(std::to_wstring(c) + L" -> " + std::to_wstring(byte_val));
        ++count;
    }

    if (count > 100)
        result.log.push_back(L"...");
    result.log.push_back(L"Расшифрование завершено.");
    fin.close();
    fout.close();
    return result;
}
