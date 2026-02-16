# Инструкция по отправке в GitHub

Из-за проблем с кириллицей в путях PowerShell, выполните команды вручную:

## Вариант 1: Использовать скрипт

Запустите в PowerShell:
```powershell
.\push-to-github.ps1
```

## Вариант 2: Выполнить команды вручную

1. **Скопируйте файлы в папку inf-theory:**
   - index.html
   - styles.css
   - package.json
   - tsconfig.json
   - README.md
   - .gitignore
   - папка src
   - папка dist

2. **Откройте терминал и перейдите в папку inf-theory:**
   ```bash
   cd inf-theory
   ```

3. **Выполните git команды:**
   ```bash
   git add .
   git commit -m "Initial commit: система шифрования файлов"
   git remote add origin https://github.com/Jok1c/inf-theory.git
   git branch -M main
   git push -u origin main
   ```

Если remote уже существует, используйте:
```bash
git remote set-url origin https://github.com/Jok1c/inf-theory.git
```

Если нужно настроить git user (если еще не настроено):
```bash
git config user.email "your-email@example.com"
git config user.name "Your Name"
```
