/* 
 * File:   printf.h
 * Author: Igor
 *
 * Created on June 2, 2026, 1:27 PM
 */

#ifndef PRINTF_H
#define	PRINTF_H

u16 sprintf_ed(char *buf, const char *fmt, ...);
void printf_usb(const char *fmt, ...);
void printf_lcd(const char *fmt, ...);

#endif	/* PRINTF_H */

