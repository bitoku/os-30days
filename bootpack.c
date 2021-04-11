void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);

void HariMain(void)
{
    int i;
    char *p; // BYTE [..]

    init_palette();

    p = (char *)0xa0000;

    for (i = 0; i <= 0xaffff; i++) {
        p[i] = i & 0x0f;
    }

    for (;;) {
        io_hlt();
    }
}

void init_palette(void) {
    static unsigned char table_rgb[16 * 3] = {
            0x00, 0x00, 0x00, // black
            0xff, 0x00, 0x00, // bright red
            0x00, 0xff, 0x00, // bright green
            0xff, 0xff, 0x00, // bright yellow
            0x00, 0x00, 0xff, // bright blue
            0xff, 0x00, 0xff, // bright purple
            0x00, 0xff, 0xff, // bright cyan
            0xff, 0xff, 0xff, // white
            0x84, 0x00, 0x00, // dark red
            0x00, 0x84, 0x00, // dark green
            0x84, 0x84, 0x00, // dark yellow
            0x00, 0x00, 0x84, // dark blue
            0x84, 0x00, 0x84, // dark purple
            0x00, 0x84, 0x84, // dark cyan
            0x84, 0x84, 0x84, // dark grey
    };
    set_palette(0, 15, table_rgb);
    return;
}

void set_palette(int start, int end, unsigned char *rgb) {
    int i, eflags;
    eflags = io_load_eflags();
    io_cli();
    io_out8(0x03c8, start);
    for (i = start; i <= end; i++) {
        io_out8(0x03c9, rgb[0] / 4);
        io_out8(0x03c9, rgb[1] / 4);
        io_out8(0x03c9, rgb[2] / 4);
        rgb += 3;
    }
    io_store_eflags(eflags);
    return;
}