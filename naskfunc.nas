; naskfunc
; TAB=4

[BITS 32]

; オブジェクトファイルのための情報
    GLOBAL io_hlt   ; このプログラムに含まれる関数名

; 以下は実際の関数
[SECTION .text] ; オブジェクトファイルではこれを書いてからプログラムを書く

io_hlt:
    HLT
    RET