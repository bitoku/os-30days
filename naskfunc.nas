; naskfunc
; TAB=4

[BITS 32]

; オブジェクトファイルのための情報
    GLOBAL io_hlt, write_mem8   ; このプログラムに含まれる関数名

; 以下は実際の関数
[SECTION .text] ; オブジェクトファイルではこれを書いてからプログラムを書く

io_hlt: ; void io_hlt(void);
    HLT
    RET

write_mem8:    ; void write_mem8(int addr, int data);
    MOV ECX, [ESP+4]    ; [ESP+4]にaddrが入っているのでECXに読み込む
    MOV AL, [ESP+8]     ; [ESP+8]にdataが入っているのでALに読み込む
    MOV [ECX], AL
    RET