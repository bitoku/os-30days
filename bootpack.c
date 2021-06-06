#include "bootpack.h"

//10進数からASCIIコードに変換
int dec2asc (char *str, unsigned int dec) {
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
int hex2asc (char *str, unsigned int dec) { //10で割れた回数（つまり桁数）をlenに、各桁をbufに格納
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
                    len = dec2asc(str, va_arg (list, unsigned int));
                    break;
                case 'x':
                    len = hex2asc(str, va_arg (list, unsigned int));
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

void wait_KBC_sendready(void) {
    for (;;) {
        if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
            break;
        }
    }
    return;
}

void init_keyboard(void) {
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, KBC_MODE);
    return;
}

void enable_mouse(void) {
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
    return; // うまくいくとACK(0xfa)が送信される
}

void HariMain(void)
{
    struct BOOTINFO *binfo = (struct BOOTINFO *) 0x0ff0;
    char s[256];
    unsigned char mcursor[256], keybuf[32], mousebuf[128];
    int mx, my, i;
    char mouse_dbuf[3], mouse_phase;

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

    enable_mouse();
    mouse_phase = 0;

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
                if (mouse_phase == 0) {
                    if (i == 0xfa) {
                        mouse_phase = 1;
                    }
                } else if (mouse_phase == 1) {
                    mouse_dbuf[0] = i;
                    mouse_phase = 2;
                } else if (mouse_phase == 2) {
                    mouse_dbuf[1] = i;
                    mouse_phase = 3;
                } else if (mouse_phase == 3) {
                    mouse_dbuf[2] = i;
                    mouse_phase = 1;
                    // データが3バイト揃ったら表示
                    sprintf(s, "%x %x %x", mouse_dbuf[0], mouse_dbuf[1], mouse_dbuf[2]);
                    boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32 + 8 * 8 - 1, 31);
                    putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
                }
            }
        }
    }
}
