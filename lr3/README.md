# ЛР2 — RSA (CMake)

Приложение для шифрования и расшифрования файлов по алгоритму RSA.

## Сборка

```bash
cmake -B build -S .
cmake --build build --config Release
```

Исполняемый файл: `build/Release/TI3.exe` (MSVC) или `build/TI3.exe` (Ninja/MinGW).

## Запуск

```bash
./build/Release/TI3.exe
```

Старый проект C++Builder (`.cbproj`, VCL) сохранён для справки; для сборки используйте CMake.
