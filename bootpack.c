#include "bootpack.h"

//10進数からASCIIコードに変換
int dec2asc (char *str, int dec) {
    int len = 0, len_buf; //桁数
    int buf[10];
    int minus = 0;
    if (dec < 0) {
        minus = 1;
        dec = -dec;
    }
    while (1) { //10で割れた回数（つまり桁数）をlenに、各桁をbufに格納
        buf[len++] = dec % 10 + 0x30;
        if (dec < 10) break;
        dec /= 10;
    }
    if (minus) {
        buf[len++] = '-';
    }
    len_buf = len;
    while (len) {
        *(str++) = buf[--len];
    }
    return len_buf;
}

//16進数からASCIIコードに変換
int hex2asc (char *str, int dec) { //10で割れた回数（つまり桁数）をlenに、各桁をbufに格納
    int len = 0, len_buf; //桁数
    int buf[10];
    while (1) {
        buf[len++] = dec % 16;
        if (dec < 16) break;
        dec /= 16;
    }
    len_buf = len;
    while (len) {
        len --;
        *(str++) = (buf[len]<10)?(buf[len] + 0x30):(buf[len] - 10 + 0x61);
    }
    return len_buf;
}

void sprintf (char *str, char *fmt, ...) {
    va_list list;
    int len;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wvarargs"
    va_start (list, 2);
#pragma GCC diagnostic pop

    while (*fmt) {
        if(*fmt=='%') {
            fmt++;
            switch(*fmt){
                case 'd':
                    len = dec2asc(str, va_arg (list, int));
                    break;
                case 'x':
                    len = hex2asc(str, va_arg (list, int));
                    break;
            }
            str += len; fmt++;
        } else {
            *(str++) = *(fmt++);
        }
    }
    *str = 0x00; //最後にNULLを追加
    va_end (list);
}

#define EFLAGS_AC_BIT		0x00040000
#define CR0_CACHE_DISABLE	0x60000000

unsigned int memtest_sub(unsigned int start, unsigned int end) {
    unsigned int i, *p, old, pat0 = 0xaa55aa55, pat1 = 0x55aa55aa;
    for (i = start; i <= end; i += 0x1000) {
        p = (unsigned int *) (i + 0xffc);
        old = *p;           // 弄る前の値をoldに保存
        *p = pat0;          // 試しに書いてみる
        *p ^= 0xffffffff;   // それを反転する
        if (*p != pat1) {  // 反転結果になったか
            not_memory:
            *p = old;
            break;
        }
        *p ^= 0xffffffff;
        if (*p != pat0) {
            goto not_memory;
        }
        *p = old;       // いじった値をもとに戻す
    }
    return i;
}

unsigned int memtest(unsigned int start, unsigned int end) {
    char flg486 = 0;
    unsigned int eflg, cr0, i;

    // 386か486以降なのかを確認
    eflg = io_load_eflags();
    eflg |= EFLAGS_AC_BIT; // AC-bit = 1
    io_store_eflags(eflg);
    eflg = io_load_eflags();
    if ((eflg & EFLAGS_AC_BIT) != 0) {
        flg486 = 1;
    }
    eflg &= ~EFLAGS_AC_BIT; // AC-bit = 0;
    io_store_eflags(eflg);

    if (flg486) {
        cr0 = load_cr0();
        cr0 |= CR0_CACHE_DISABLE; // キャッシュ禁止
        store_cr0(cr0);
    }

    i = memtest_sub(start, end);

    if (flg486) {
        cr0 = load_cr0();
        cr0 &= ~CR0_CACHE_DISABLE; // キャッシュ許可
        store_cr0(cr0);
    }

    return i;
}
unsigned int memtest_sub(unsigned int start, unsigned int end);


void HariMain(void)
{
    struct BOOTINFO *binfo = (struct BOOTINFO *) 0x0ff0;
    char s[256];
    unsigned char mcursor[256], keybuf[32], mousebuf[128];
    int mx, my, i;
    struct MOUSE_DEC mdec;

    init_gdtidt();
    init_pic();
    io_sti();
    fifo8_init(&keyfifo, 32, keybuf);
    fifo8_init(&mousefifo, 128, mousebuf);
    io_out8(PIC0_IMR, 0xf9);  // PIC1とキーボードを許可
    io_out8(PIC1_IMR, 0xef);  // マウスを許可

    init_keyboard();

    init_palette();
    init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
    init_mouse_cursor8(mcursor, COL8_008484);
    mx = (binfo->scrnx - 16) / 2;
    my = (binfo->scrny - 28 - 16) / 2;
    init_mouse_cursor8(mcursor, COL8_008484);
    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
    sprintf(s, "(%d, %d)", mx, my);
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

    enable_mouse(&mdec);

    i = memtest(0x00400000, 0xbfffffff) / (1024 * 1024);
    sprintf(s, "memory %dMB", i);
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 32, COL8_FFFFFF, s);

    for (;;) {
        io_cli();
        if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
            io_stihlt();
        } else {
            if (fifo8_status(&keyfifo) != 0) {
                i = fifo8_get(&keyfifo);
                io_sti();
                sprintf(s, "%x", i);
                boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
                putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
            }
            if (fifo8_status(&mousefifo) != 0) {
                i = fifo8_get(&mousefifo);
                io_sti();
                if (mouse_decode(&mdec, i) != 0) {
                    sprintf(s, "[lcr] %d %d", mdec.x, mdec.y);
                    if ((mdec.btn & 0x01) != 0) {
                        s[1] = 'L';
                    }
                    if ((mdec.btn & 0x02) != 0) {
                        s[3] = 'R';
                    }
                    if ((mdec.btn & 0x04) != 0) {
                        s[2] = 'C';
                    }
                    boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 240, 31);
                    putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
                    // マウスカーソルの移動
                    boxfill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx + 15, my + 15);
                    mx += mdec.x;
                    my += mdec.y;
                    if (mx < 0) {
                        mx = 0;
                    }
                    if (my < 0) {
                        my = 0;
                    }
                    if (mx > binfo->scrnx - 16) {
                        mx = binfo->scrnx - 16;
                    }
                    if (my > binfo->scrny - 16) {
                        my = binfo->scrny - 16;
                    }
                    sprintf(s, "(%d, %d)", mx, my);
                    boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 79, 15);
                    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
                    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
                }
            }
        }
    }
}
