
#include "appmain.h"
#include <stdarg.h>

#define PRINTF_BUFF_SIZE    256

typedef struct {
    char *buff;
    u16 ptr;
    u16 size;
} PrinfBuff;

static PrinfBuff *pb;

static void out_char(char c) {

    if (pb->ptr < pb->size - 1) {
        pb->buff[pb->ptr++] = c;
    }
}

static u16 u32_to_str(u32 val, u32 base, char *buf, u8 upper) {

    const char *hex = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    char tmp[32];
    u16 len = 0;

    if (val == 0) {
        buf[0] = '0';
        return 1;
    }

    while (val) {
        tmp[len++] = hex[val % base];
        val /= base;
    }

    for (u16 i = 0; i < len; i++) {
        buf[i] = tmp[len - 1 - i];
    }

    return len;
}

static void out_padded(const char *s, u16 len, u16 width, char pad) {

    while (width > len) {
        out_char(pad);
        width--;
    }

    while (len--) {
        out_char(*s++);
    }
}

// Main formatter. Writes at most size-1 chars + terminating zero, returns string length.

static u16 vsprintf_buff(char *buf, u16 size, const char *fmt, va_list ap) {

    PrinfBuff pb_obj;
    PrinfBuff *pb_prev = pb; // keeps the formatter reentrant (call from IRQ)

    pb = &pb_obj;
    pb->buff = buf;
    pb->ptr = 0;
    pb->size = size;

    while (*fmt) {

        if (*fmt != '%') {
            out_char(*fmt++);
            continue;
        }

        fmt++;

        char pad = ' ';
        u16 width = 0;

        if (*fmt == '0') {
            pad = '0';
            fmt++;
        }

        if (*fmt == '*') {
            s32 w = va_arg(ap, s32);
            width = w < 0 ? 0 : (u16) w;
            fmt++;
        } else {
            while (*fmt >= '0' && *fmt <= '9') {
                width = width * 10 + (*fmt - '0');
                fmt++;
            }
        }

        u16 prec = 0xFFFF; // max string length, 0xFFFF = no limit

        if (*fmt == '.') {
            fmt++;
            prec = 0;
            if (*fmt == '*') {
                s32 p = va_arg(ap, s32);
                prec = p < 0 ? 0xFFFF : (u16) p; // negative = no limit
                fmt++;
            } else {
                while (*fmt >= '0' && *fmt <= '9') {
                    prec = prec * 10 + (*fmt - '0');
                    fmt++;
                }
            }
        }

        char num[32];
        u16 len;

        switch (*fmt) {
            case 's':
            {
                const char *s = va_arg(ap, const char *);
                if (!s) {
                    s = "(null)";
                }

                len = 0;
                while (len < prec && s[len]) {
                    len++;
                }

                out_padded(s, len, width, pad);
                break;
            }

            case 'c':
            {
                char c = (char) va_arg(ap, int);
                out_padded(&c, 1, width, pad);
                break;
            }

            case 'd':
            {
                s32 v = va_arg(ap, s32);

                if (v < 0) {
                    out_char('-');
                    v = -v;
                    if (width) {
                        width--;
                    }
                }

                len = u32_to_str((u32) v, 10, num, 0);
                out_padded(num, len, width, pad);
                break;
            }

            case 'u':
                len = u32_to_str(va_arg(ap, u32), 10, num, 0);
                out_padded(num, len, width, pad);
                break;

            case 'x':
                len = u32_to_str(va_arg(ap, u32), 16, num, 0);
                out_padded(num, len, width, pad);
                break;

            case 'X':
                len = u32_to_str(va_arg(ap, u32), 16, num, 1);
                out_padded(num, len, width, pad);
                break;

            case '%':
                out_char('%');
                break;

            default:
                out_char('%');
                out_char(*fmt);
                break;
        }

        if (*fmt) {
            fmt++;
        }
    }

    pb->buff[pb->ptr] = 0;

    u16 len = pb->ptr;
    pb = pb_prev;

    return len;
}

u16 vsprintf_ed(char *buf, const char *fmt, va_list ap) {

    return vsprintf_buff(buf, 0xFFFF, fmt, ap);
}

u16 sprintf_ed(char *buf, const char *fmt, ...) {

    va_list ap;
    va_start(ap, fmt);

    u16 len = vsprintf_ed(buf, fmt, ap);

    va_end(ap);

    return len;
}

void printf_usb(const char *fmt, ...) {

    char buff[PRINTF_BUFF_SIZE];
    va_list ap;
    va_start(ap, fmt);

    u16 len = vsprintf_buff(buff, sizeof (buff), fmt, ap);

    va_end(ap);

    ed_usb_wr(buff, len);
}

void printf_lcd(const char *fmt, ...) {

    char buff[PRINTF_BUFF_SIZE];

    va_list ap;
    va_start(ap, fmt);

    u16 len = vsprintf_buff(buff, sizeof (buff), fmt, ap);

    va_end(ap);

    gPutString(buff, len);
}
