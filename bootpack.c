#include "bootpack.h"

//10進数からASCIIコードに変換
int dec2asc (char *str, int dec) {
    int len = 0, len_buf; //桁数
    int buf[10];
    while (1) { //10で割れた回数（つまり桁数）をlenに、各桁をbufに格納
        buf[len++] = dec % 10;
        if (dec < 10) break;
        dec /= 10;
    }
    len_buf = len;
    while (len) {
        *(str++) = buf[--len] + 0x30;
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
        *(str++) = (buf[len]<10)?(buf[len] + 0x30):(buf[len] - 9 + 0x60);
    }
    return len_buf;
}

void sprintf (char *str, char *fmt, ...) {
    va_list list;
    int i, len;
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

void HariMain(void)
{
    struct BOOTINFO *binfo = (struct BOOTINFO *) 0x0ff0;
    extern char hankaku[4096];
    char s[256], mcursor[256];
    int mx, my, i;

    init_gdtidt();
    init_pic();
    io_sti();

    init_palette();
    init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
    init_mouse_cursor8(mcursor, COL8_008484);
    mx = (binfo->scrnx - 16) / 2;
    my = (binfo->scrny - 28 - 16) / 2;
    init_mouse_cursor8(mcursor, COL8_008484);
    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
    sprintf(s, "(%d, %d)", mx, my);
    putfonts8_asc(binfo->vram, binfo->scrnx, 8, 8, COL8_FFFFFF, s);

    io_out8(PIC0_IMR, 0xf9);  // PIC1とキーボードを許可
    io_out8(PIC1_IMR, 0xef);  // マウスを許可

    for (;;) {
        io_cli();
        if (keybuf.flag == 0) {
            io_stihlt();
        } else {
            i = keybuf.data;
            keybuf.flag = 0;
            io_sti();
            sprintf(s, "%x", i);
            boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
            putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
        }
    }
}
