// Класс для шифрования методом децимации (умножения)
class DecimationCipher {
    private russianAlphabet: string = 'абвгдеёжзийклмнопрстуфхцчшщъыьэюя';
    private alphabetSize: number;
    private key: number;

    constructor(key: string) {
        this.alphabetSize = this.russianAlphabet.length;
        this.key = this.validateKey(key);
    }

    // Проверка: целое число 1..32 и взаимная простота с размером алфавита (33)
    private validateKey(key: string): number {
        const maxKey = this.alphabetSize - 1;
        const k = parseInt(key.trim(), 10);
        if (Number.isNaN(k) || k < 1 || k > maxKey) {
            throw new Error(
                `Ключ для децимаций должен быть целым числом от 1 до ${maxKey} (введено: "${key.trim()}")`
            );
        }
        if (this.gcd(k, this.alphabetSize) !== 1) {
            throw new Error(`Ключ ${k} не взаимно прост с ${this.alphabetSize}`);
        }
        return k;
    }

    // Вычисление НОД алгоритмом Евклида
    private gcd(a: number, b: number): number {
        while (b !== 0) {
            const temp = b;
            b = a % b;
            a = temp;
        }
        return a;
    }

    // Расширенный алгоритм Евклида для нахождения обратного элемента
    private extendedGcd(a: number, b: number): { gcd: number; x: number; y: number } {
        if (a === 0) {
            return { gcd: b, x: 0, y: 1 };
        }
        const result = this.extendedGcd(b % a, a);
        const x = result.y - Math.floor(b / a) * result.x;
        const y = result.x;
        return { gcd: result.gcd, x, y };
    }

    // Вычисление обратного элемента по модулю
    private modInverse(a: number, m: number): number {
        const result = this.extendedGcd(a % m, m);
        return ((result.x % m) + m) % m;
    }

    // Шифрование: каждая буква умножается на ключ по модулю
    encrypt(text: string): string {
        if (!text) return '';
        return text
            .toLowerCase()
            .split('')
            .map(char => {
                const pos = this.russianAlphabet.indexOf(char);
                if (pos === -1) return char;
                const encryptedPos = (pos * this.key) % this.alphabetSize;
                return this.russianAlphabet[encryptedPos];
            })
            .join('');
    }

    // Расшифрование: каждая буква умножается на обратный ключ по модулю
    decrypt(text: string): string {
        if (!text) return '';
        const keyInverse = this.modInverse(this.key, this.alphabetSize);
        return text
            .toLowerCase()
            .split('')
            .map(char => {
                const pos = this.russianAlphabet.indexOf(char);
                if (pos === -1) return char;
                const decryptedPos = (pos * keyInverse) % this.alphabetSize;
                return this.russianAlphabet[decryptedPos];
            })
            .join('');
    }
}

// Класс для шифрования методом Виженера с самогенерирующимся ключом
class VigenereCipher {
    private russianAlphabet: string = 'абвгдеёжзийклмнопрстуфхцчшщъыьэюя';
    private alphabetSize: number;
    private initialKey: string;

    constructor(key: string) {
        this.alphabetSize = this.russianAlphabet.length;
        this.initialKey = key.toLowerCase().split('').filter(c => this.russianAlphabet.includes(c)).join('');
    }

    // Генерация ключевой буквы: сначала начальный ключ, затем исходный текст
    private getKeyChar(keySequence: string[], originalTextChars: string[]): string {
        if (originalTextChars.length < this.initialKey.length) {
            return keySequence[originalTextChars.length];
        } else {
            const indexInOriginal = originalTextChars.length - this.initialKey.length;
            return originalTextChars[indexInOriginal];
        }
    }

    // Шифрование: сначала используется начальный ключ, затем буквы исходного текста
    encrypt(text: string): string {
        if (!text) return '';
        const keySequence: string[] = this.initialKey.split('');
        const result: string[] = [];
        const originalTextChars: string[] = [];
        
        for (let i = 0; i < text.length; i++) {
            const char = text[i].toLowerCase();
            const pos = this.russianAlphabet.indexOf(char);
            if (pos === -1) {
                result.push(char);
                continue;
            }
            
            const keyChar = this.getKeyChar(keySequence, originalTextChars);
            const keyPos = this.russianAlphabet.indexOf(keyChar);
            const encryptedPos = (pos + keyPos) % this.alphabetSize;
            const encryptedChar = this.russianAlphabet[encryptedPos];
            
            result.push(encryptedChar);
            originalTextChars.push(char);
        }
        
        return result.join('');
    }

    // Расшифрование: автоключ строится только по расшифрованным буквам (не-буквы игнорируются)
    decrypt(text: string): string {
        if (!text) return '';
        const keySequence: string[] = this.initialKey.split('');
        const result: string[] = [];
        const decryptedTextChars: string[] = [];

        for (let i = 0; i < text.length; i++) {
            const char = text[i].toLowerCase();
            const pos = this.russianAlphabet.indexOf(char);
            if (pos === -1) {
                result.push(char);
                continue;
            }

            const keyChar = this.getKeyChar(keySequence, decryptedTextChars);
            const keyPos = this.russianAlphabet.indexOf(keyChar);
            const decryptedPos = (pos - keyPos + this.alphabetSize) % this.alphabetSize;
            const decryptedChar = this.russianAlphabet[decryptedPos];

            result.push(decryptedChar);
            decryptedTextChars.push(decryptedChar);
        }

        return result.join('');
    }
}

type CipherMethod = 'decimation' | 'vigenere';
type Operation = 'encrypt' | 'decrypt';

// Главный класс приложения для работы с шифрованием
class CipherApp {
    private methodSelect!: HTMLSelectElement;
    private keyInput!: HTMLInputElement;
    private textInput!: HTMLTextAreaElement;
    private fileInput!: HTMLInputElement;
    private encryptTextBtn!: HTMLButtonElement;
    private decryptTextBtn!: HTMLButtonElement;
    private encryptFileBtn!: HTMLButtonElement;
    private decryptFileBtn!: HTMLButtonElement;
    private resultSection!: HTMLElement;
    private resultOutput!: HTMLTextAreaElement;
    private copyResultBtn!: HTMLButtonElement;

    // Инициализация элементов интерфейса и привязка обработчиков событий
    constructor() {
        this.methodSelect = document.getElementById('method') as HTMLSelectElement;
        this.keyInput = document.getElementById('key') as HTMLInputElement;
        this.textInput = document.getElementById('textInput') as HTMLTextAreaElement;
        this.fileInput = document.getElementById('fileInput') as HTMLInputElement;
        this.encryptTextBtn = document.getElementById('encryptTextBtn') as HTMLButtonElement;
        this.decryptTextBtn = document.getElementById('decryptTextBtn') as HTMLButtonElement;
        this.encryptFileBtn = document.getElementById('encryptFileBtn') as HTMLButtonElement;
        this.decryptFileBtn = document.getElementById('decryptFileBtn') as HTMLButtonElement;
        this.resultSection = document.getElementById('resultSection') as HTMLElement;
        this.resultOutput = document.getElementById('resultOutput') as HTMLTextAreaElement;
        this.copyResultBtn = document.getElementById('copyResultBtn') as HTMLButtonElement;
        
        this.encryptTextBtn.addEventListener('click', () => this.handleTextOperation('encrypt'));
        this.decryptTextBtn.addEventListener('click', () => this.handleTextOperation('decrypt'));
        this.encryptFileBtn.addEventListener('click', () => this.handleFileOperation('encrypt'));
        this.decryptFileBtn.addEventListener('click', () => this.handleFileOperation('decrypt'));
        this.copyResultBtn.addEventListener('click', () => this.copyResult());
    }

    // Создание объекта шифра в зависимости от выбранного метода
    private getCipher(method: CipherMethod, key: string): DecimationCipher | VigenereCipher {
        try {
            return method === 'decimation' ? new DecimationCipher(key) : new VigenereCipher(key);
        } catch (error) {
            alert((error as Error).message);
            throw error;
        }
    }

    // Обработка операции шифрования/расшифрования текста из текстового поля
    private handleTextOperation(operation: Operation): void {
        const text = this.textInput.value.trim();
        if (!text) {
            alert('Введите текст');
            return;
        }

        const method = this.methodSelect.value as CipherMethod;
        const key = this.keyInput.value.trim();
        
        if (!key) {
            alert('Введите ключ');
            return;
        }

        try {
            const cipher = this.getCipher(method, key);
            const result = operation === 'encrypt' ? cipher.encrypt(text) : cipher.decrypt(text);
            this.showResult(result);
        } catch (error) {
        }
    }

    // Обработка операции шифрования/расшифрования файла
    private async handleFileOperation(operation: Operation): Promise<void> {
        const file = this.fileInput.files![0];
        if (!file) {
            alert('Выберите файл');
            return;
        }

        const method = this.methodSelect.value as CipherMethod;
        const key = this.keyInput.value.trim();
        
        if (!key) {
            alert('Введите ключ');
            return;
        }

        try {
            const content = await this.readFile(file);
            const cipher = this.getCipher(method, key);
            const result = operation === 'encrypt' ? cipher.encrypt(content) : cipher.decrypt(content);
            this.downloadFile(result, `${operation === 'encrypt' ? 'encrypted' : 'decrypted'}_${file.name}`);
        } catch (error) {
        }
    }

    // Отображение результата операции в интерфейсе
    private showResult(result: string): void {
        this.resultOutput.value = result;
        this.resultSection.style.display = 'block';
    }

    // Копирование результата в буфер обмена
    private async copyResult(): Promise<void> {
        try {
            await navigator.clipboard.writeText(this.resultOutput.value);
            const originalText = this.copyResultBtn.textContent;
            this.copyResultBtn.textContent = 'Скопировано!';
            setTimeout(() => {
                if (this.copyResultBtn) {
                    this.copyResultBtn.textContent = originalText;
                }
            }, 2000);
        } catch (error) {
        }
    }

    // Асинхронное чтение содержимого файла
    private readFile(file: File): Promise<string> {
        return new Promise((resolve) => {
            const reader = new FileReader();
            reader.onload = (e) => resolve(e.target!.result as string);
            reader.readAsText(file, 'utf-8');
        });
    }

    // Скачивание файла с результатом операции
    private downloadFile(content: string, filename: string): void {
        const blob = new Blob([content], { type: 'text/plain;charset=utf-8' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = filename;
        a.style.display = 'none';
        document.body.appendChild(a);
        a.click();
        setTimeout(() => {
            document.body.removeChild(a);
            URL.revokeObjectURL(url);
        }, 100);
    }
}

// Инициализация приложения после загрузки DOM
document.addEventListener('DOMContentLoaded', () => {
    new CipherApp();
});
