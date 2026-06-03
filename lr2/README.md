# ЛР2 — LFSR (CMake)

Приложение для шифрования и расшифрования данных регистром сдвига с линейной обратной связью (LFSR) методом XOR.

## Сборка

```bash
cmake -B build -S .
cmake --build build --config Release
```

Исполняемый файл: `build/Release/TI2.exe` (MSVC) или `build/TI2.exe` (Ninja/MinGW).

## Запуск

```bash
./build/Release/TI2.exe
```

Ключ — 38 бит (символы `0` и `1`). Формула обратной связи: позиции 1, 5, 6, 38 (как в веб-версии lab2).
