; hello-os
; TAB=4

CYLS EQU 10     ; どこまで読み込むか

    ORG 0x7c00

    JMP entry
    DB  0x90
    DB  "HELLOIPL"
    DW  512
    DB  1
    DW  1
    DB  2
    DW  224
    DW  2880
    DB  0xf0
    DW  9
    DW  18
    DW  2
    DD  0
    DD  2880
    DB  0, 0, 0x29
    DD  0xffffffff
    DB  "HELLO-OS   "
    DB  "FAT12   "
    TIMES 18 DB 0

entry:
    MOV AX, 0       ; レジスタ初期化
    MOV SS, AX
    MOV SP, 0x7c00
    MOV DS, AX

    MOV AX, 0x0820
    MOV ES, AX
    MOV CH, 0       ; シリンダ0
    MOV DH, 0       ; ヘッド0
    MOV CL, 2       ; セクタ2

readloop:
    MOV SI, 0
retry:
    MOV AH, 0x02    ; AH=0x02 : ディスク読み込み
    MOV AL, 1       ; 1セクタ
    MOV BX, 0
    MOV DL, 0x00    ; Aドライブ
    INT 0x13        ; ディスクBIOS呼び出し
    JNC next        ; エラーが起きなければnextへ
    ADD SI, 1       ; SIに1を足す
    CMP SI, 5       ; SIと5を比較
    JAE error       ; SI >= 5ならエラー
    MOV AH, 0x00
    MOV DL, 0x00    ; Aドライブ
    INT 0x13        ; ドライブのリセット
    JMP retry

next:
    MOV AX, ES      ; アドレスを0x200進める
    ADD AX, 0x0020
    MOV ES, AX      ; ADD ES, 0x020という命令がないのでこうしている
    ADD CL, 1       ; CLに1を足す
    CMP CL, 18      ; CLと18を比較
    JBE readloop    ; CL <= 18だったらreadloop
    MOV CL, 1
    ADD DH, 1
    CMP DH, 2
    JB  readloop    ; DH < 2 だったらreadloop
    MOV DH, 0
    ADD CH, 1
    CMP CH, CYLS
    JB  readloop    ; CH < CYLS だったらreadloop

    MOV [0x0ff0], CH; IPLがどこまで読んだかメモ
    JMP 0xc200

error:
    MOV SI, msg

putloop:
    MOV AL, [SI]
    ADD SI, 1
    CMP AL, 0
    JE  fin
    MOV AH, 0x0e
    MOV BX, 15
    INT 0x10
    JMP putloop

fin:
    HLT
    JMP fin

msg:
    DB  0x0a, 0x0a
    DB  "load error"
    DB  0x0a
    DB  0

    TIMES 0x1fe-($-$$) DB 0 ; pad 0 to 0x001fe

    DB  0x55, 0xaa

