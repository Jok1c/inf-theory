class DecimationCipher {
    private russianAlphabet: string = 'абвгдеёжзийклмнопрстуфхцчшщъыьэюя';
    private alphabetSize: number;
    private key: number;

    constructor(key: string) {
        this.alphabetSize = this.russianAlphabet.length;
        this.key = this.validateKey(key);
    }

    private validateKey(key: string): number {
        const k = parseInt(key, 10);
        if (this.gcd(k, this.alphabetSize) !== 1) {
            throw new Error(`Ключ ${k} не взаимно прост с ${this.alphabetSize}`);
        }
        return k;
    }

    private gcd(a: number, b: number): number {
        while (b !== 0) {
            const temp = b;
            b = a % b;
            a = temp;
        }
        return a;
    }

    private extendedGcd(a: number, b: number): { gcd: number; x: number; y: number } {
        if (a === 0) {
            return { gcd: b, x: 0, y: 1 };
        }
        const result = this.extendedGcd(b % a, a);
        const x = result.y - Math.floor(b / a) * result.x;
        const y = result.x;
        return { gcd: result.gcd, x, y };
    }

    private modInverse(a: number, m: number): number {
        const result = this.extendedGcd(a % m, m);
        return ((result.x % m) + m) % m;
    }

    private filterText(text: string): string {
        return text
            .toLowerCase()
            .split('')
            .filter(c => this.russianAlphabet.includes(c))
            .join('');
    }

    encrypt(text: string): string {
        const filteredText = this.filterText(text);
        if (!filteredText) return '';
        return filteredText
            .split('')
            .map(char => {
                const pos = this.russianAlphabet.indexOf(char);
                const encryptedPos = (pos * this.key) % this.alphabetSize;
                return this.russianAlphabet[encryptedPos];
            })
            .join('');
    }

    decrypt(text: string): string {
        const filteredText = this.filterText(text);
        if (!filteredText) return '';
        const keyInverse = this.modInverse(this.key, this.alphabetSize);
        return filteredText
            .split('')
            .map(char => {
                const pos = this.russianAlphabet.indexOf(char);
                const decryptedPos = (pos * keyInverse) % this.alphabetSize;
                return this.russianAlphabet[decryptedPos];
            })
            .join('');
    }
}

class VigenereCipher {
    private russianAlphabet: string = 'абвгдеёжзийклмнопрстуфхцчшщъыьэюя';
    private alphabetSize: number;
    private initialKey: string;

    constructor(key: string) {
        this.alphabetSize = this.russianAlphabet.length;
        this.initialKey = this.filterKey(key);
    }

    private filterKey(key: string): string {
        return key
            .toLowerCase()
            .split('')
            .filter(c => this.russianAlphabet.includes(c))
            .join('');
    }

    private filterText(text: string): string {
        return text
            .toLowerCase()
            .split('')
            .filter(c => this.russianAlphabet.includes(c))
            .join('');
    }

    encrypt(text: string): string {
        const filteredText = this.filterText(text);
        if (!filteredText) return '';
        const keySequence: string[] = this.initialKey.split('');
        const encrypted: string[] = [];
        for (let i = 0; i < filteredText.length; i++) {
            let keyChar: string;
            let k = i;
            if (k > this.initialKey.length - 1) {
                k = k - (this.initialKey.length - 1);
                keyChar = filteredText[k - 1];
            } else {
                keyChar = keySequence[k];
            }
            const charPos = this.russianAlphabet.indexOf(filteredText[i]);
            const keyPos = this.russianAlphabet.indexOf(keyChar);
            const encryptedPos = (charPos + keyPos) % this.alphabetSize;
            const encryptedChar = this.russianAlphabet[encryptedPos];
            encrypted.push(encryptedChar);
        }
        return encrypted.join('');
    }

    decrypt(text: string): string {
        const filteredText = this.filterText(text);
        if (!filteredText) return '';
        const keySequence: string[] = this.initialKey.split('');
        const decrypted: string[] = [];
        for (let i = 0; i < filteredText.length; i++) {
            let keyChar: string;
            if (i >= this.initialKey.length) {
                const k = i - this.initialKey.length + 1;
                keyChar = decrypted[k - 1];
            } else {
                keyChar = keySequence[i];
            }
            const charPos = this.russianAlphabet.indexOf(filteredText[i]);
            const keyPos = this.russianAlphabet.indexOf(keyChar);
            const decryptedPos = (charPos - keyPos + this.alphabetSize) % this.alphabetSize;
            const decryptedChar = this.russianAlphabet[decryptedPos];
            decrypted.push(decryptedChar);
        }
        return decrypted.join('');
    }
}

type CipherMethod = 'decimation' | 'vigenere';
type Operation = 'encrypt' | 'decrypt';

class CipherApp {
    private methodSelect!: HTMLSelectElement;
    private keyInput!: HTMLInputElement;
    private fileInput!: HTMLInputElement;
    private encryptFileBtn!: HTMLButtonElement;
    private decryptFileBtn!: HTMLButtonElement;

    constructor() {
        this.initializeElements();
        this.attachEventListeners();
    }

    private initializeElements(): void {
        this.methodSelect = document.getElementById('method') as HTMLSelectElement;
        this.keyInput = document.getElementById('key') as HTMLInputElement;
        this.fileInput = document.getElementById('fileInput') as HTMLInputElement;
        this.encryptFileBtn = document.getElementById('encryptFileBtn') as HTMLButtonElement;
        this.decryptFileBtn = document.getElementById('decryptFileBtn') as HTMLButtonElement;
    }

    private attachEventListeners(): void {
        this.encryptFileBtn.addEventListener('click', () => this.handleFileOperation('encrypt'));
        this.decryptFileBtn.addEventListener('click', () => this.handleFileOperation('decrypt'));
    }

    private getCipher(method: CipherMethod, key: string): DecimationCipher | VigenereCipher {
        return method === 'decimation' ? new DecimationCipher(key) : new VigenereCipher(key);
    }

    private async handleFileOperation(operation: Operation): Promise<void> {
        const file = this.fileInput.files![0];
        const method = this.methodSelect.value as CipherMethod;
        const key = this.keyInput.value.trim();
        const content = await this.readFile(file);
        const cipher = this.getCipher(method, key);
        const result = operation === 'encrypt' ? cipher.encrypt(content) : cipher.decrypt(content);
        this.downloadFile(result, `${operation === 'encrypt' ? 'encrypted' : 'decrypted'}_${file.name}`);
    }

    private readFile(file: File): Promise<string> {
        return new Promise((resolve) => {
            const reader = new FileReader();
            reader.onload = (e) => resolve(e.target!.result as string);
            reader.readAsText(file, 'utf-8');
        });
    }

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

document.addEventListener('DOMContentLoaded', () => {
    new CipherApp();
});
