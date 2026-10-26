
#include "sys.h"
#include "everdrive.h"


#define JOY_PORT *((volatile u16*)0x4000130)
#define SCREEN_BUFF 0x6000000


extern u8 font[];
u16 *g_scr_ptr;
u16 g_current_pal;


void gLoadFont(u16 *dst, u16 bg);
void gSetPaletteColor(u8 pal_idx, u8 col_idx, u16 val);

void sysInit() {

    *(u32*) 0x4000000 = 0x100; // mode3, bg2 on 
    *(u16 *) 0x4000008 = 0 | (1 << 2); //BG0 PRIORITY AND tile data address
    *(u16 *) 0x4000010 = 0; //scroll x
    *(u16 *) 0x4000012 = 0; //scroll y
    g_current_pal = 0;

    sysFillRam((void *) 0x6000000, 0, 96 * 1024);
    gLoadFont((u16 *) 0x6004400, 0);
    gLoadFont((u16 *) 0x6006400, 1);
    sysFillRam((u16 *) 0x6004020, 0x1111, 32);

    sysInitColors();
    gClearScreen();
}

void sysInitColors() {

    gSetPaletteColor(0, 0, RGB16(8, 8, 8));
    gSetPaletteColor(0, 15, RGB16(31, 31, 31));
    gSetPaletteColor(1, 0, RGB16(0, 0, 0));
    gSetPaletteColor(0, 1, 0);
    gSetPaletteColor(1, 15, RGB16(31, 31, 31));

}

void sysFillRam(void *addr, u16 val, u32 len) {

    DMA_SRC = (u32) & val;
    DMA_DST = (u32) addr;
    DMA_LEN = len / 2;
    DMA_CTR = 0x8100;
}

void gSetPaletteColor(u8 pal_idx, u8 col_idx, u16 val) {

    u16 *pal_ptr = (u16 *) 0x05000000;
    pal_ptr[col_idx + pal_idx * 16] = val;
}

void gLoadFont(u16 *dst, u16 bg) {

    u16 x;
    u16 y;
    u16 i;
    u16 font_val;
    bg <<= 12;

    for (i = 0; i < 96; i++) {

        for (y = 0; y < 8; y++) {

            font_val = font[i * 8 + y];

            for (x = 0; x < 8; x++) {

                dst[x / 4] >>= 4;
                dst[x / 4] |= (font_val & 128) == 0 ? bg : 0xf000;
                font_val <<= 1;
            }
            dst += 2;
        }

    }
}

u16 x_ctr;

void gClearScreen() {

    sysFillRam((void *) SCREEN_BUFF, 0, PLAN_SIZE);
    gSetXY(0, 0);
}

u16 gGetY() {

    return (((u32) g_scr_ptr) - SCREEN_BUFF) / (PLAN_W * 2);
}

void gPutString(u8 *str, u16 len) {

    while (len--) {

        if (*str == '\n') {
            gSetXY(0, gGetY() + 1);
        } else {

            if (x_ctr >= SCREEN_W) {
                gSetXY(0, gGetY() + 1);
            }

            *g_scr_ptr++ = *str | g_current_pal;
            x_ctr++;
        }

        str++;
    }
}

void gSetXY(u8 x, u8 y) {
    x_ctr = x;
    g_scr_ptr = (u16 *) (SCREEN_BUFF + (x + y * PLAN_W) * 2);
}

void gVsync() {

    while ((GBA_DISPSTAT & 1) == 1);
    while ((GBA_DISPSTAT & 1) == 0);
}

u16 sysJoyRead() {

    u16 joy;

    static u8 joy_ctr;


    gVsync();
    joy = (JOY_PORT ^ 0xffff) & ~0xfc00;


    if (joy != 0 && (joy & (JOY_L | JOY_R | JOY_U | JOY_D)) != 0) {
        joy_ctr++;
    } else {
        joy_ctr = 0;
    }
    if (joy_ctr > JOY_DELAY) {
        joy_ctr -= 4;
        joy = 0;
    }


    return joy;
}

u16 sysJoyWait() {

    u16 joy = 0;

    while (sysJoyRead() != 0);
    while (joy == 0) {
        joy = sysJoyRead();
    }
    return joy;
}
