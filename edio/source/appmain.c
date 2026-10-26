
#include "appmain.h"

const int __gba_multiboot;

u8 *tst_file_rd = (u8 *) "edgba/themes/default.bgr";
u8 *tst_file_wr = (u8 *) "edgba/sysdata/tst-file.dat";

void exitToMenu() {

    ed_bootmod_set(BMOD_GBA_RETURN);

    //Before calling ed_set_cmode, make sure the program isn't running from the cartridge's memory. 
    //At this point, the memory bank will switch to the one containing the menu, 
    //so ed_set_cmode can only be called when the program is running from the console's internal memory.
    ed_set_cmode(CMOD_MENU);
    asm("swi 0x000000");
}

u32 max_ed(u32 v1, u32 v2) {
    if (v1 > v2)return v1;
    return v2;
}

u32 min_ed(u32 v1, u32 v2) {
    if (v1 < v2)return v1;
    return v2;
}

void waitAB() {

    printf_lcd("\n");
    printf_lcd("press [A] or [B] to continue\n");

    while (1) {

        u16 joy = sysJoyWait();

        if (joy == JOY_A || joy == JOY_B) {
            break;
        }
    }
}

void printtError(u8 resp) {

    //console_clear();
    printf_lcd("error: 0x%02X\n", resp);
    waitAB();
}

void printHex(u8 *src, u16 len) {

    for (int i = 0; i < len; i++) {
        if (i != 0 && i % 8 == 0) {
            printf_lcd("\n");
        }

        printf_lcd("%02X", *src++);
    }

    printf_lcd("\n");
}

u8 fileToRam(void) {

    u8 resp;
    gClearScreen();
    printf_lcd("file to ram:\n[%s]...\n", tst_file_rd);

    u8 buff[1024];

    resp = ed_fs_file_open(tst_file_rd, FA_READ);
    if (resp)return resp;

    u32 size = min_ed(ed_fs_file_available(), sizeof (buff) - 1);
    buff[size] = 0;

    resp = ed_fs_file_read(buff, size);
    if (resp)return resp;

    printf_lcd("ok.file content:\n\n");
    printHex(buff, 64);
    waitAB();

    return 0;
}

u8 fileToRom() {

    u8 resp;
    u32 offset = 0x100000;
    gClearScreen();
    printf_lcd("file to rom:\n[%s]...\n", tst_file_rd);

    resp = ed_fs_file_open(tst_file_rd, FA_READ);
    if (resp)return resp;

    u32 size = min_ed(ed_fs_file_available(), 1024);

    resp = ed_fs_file_read_fci(ADDR_FCI_ROM + offset, size);
    if (resp)return resp;

    u8 *buff = (u8 *) (ADDR_GBA_ROM + offset);

    printf_lcd("ok.file content:\n\n");
    printHex(buff, 64);
    waitAB();

    return 0;
}

u8 ramToFile(void) {

    u8 resp;

    gClearScreen();
    printf_lcd("ram to file:\n[%s]...\n", tst_file_wr);


    //skip 256 bytes because regs mapped at 0x80+
    u8 *buff = (u8 *) (ADDR_GBA_ROM + 256);

    resp = ed_fs_file_open(tst_file_wr, FA_READ | FA_WRITE | FA_CREATE_ALWAYS);
    if (resp)return resp;

    resp = ed_fs_file_write(buff, 4096);
    if (resp)return resp;

    resp = ed_fs_file_close();
    if (resp)return resp;


    printf_lcd("rom header was copied to:\n[%s]\n", tst_file_wr);
    waitAB();

    return 0;
}

u8 romToFile(void) {

    u8 resp;

    gClearScreen();
    printf_lcd("rom to tile:\n[%s]...\n", tst_file_wr);


    resp = ed_fs_file_open(tst_file_wr, FA_READ | FA_WRITE | FA_CREATE_ALWAYS);
    if (resp)return resp;

    resp = ed_fs_file_write_fci(ADDR_FCI_ROM + 256, 4096);
    if (resp)return resp;

    resp = ed_fs_file_close();
    if (resp)return resp;


    printf_lcd("rom header was copied to:\n[%s]\n", tst_file_wr);
    waitAB();

    return 0;
}

void printfToUsb() {

    gClearScreen();
    printf_lcd("run [edlink.exe usbrd --print]\n\n");
    printf_lcd("press [A] to send message\n");
    printf_lcd("press [B] to return\n");

    while (1) {

        u16 joy = sysJoyWait();

        if (joy == JOY_B) {
            break;
        }

        if (joy == JOY_A) {
            printf_usb("hello gba world!\n");
        }
    }
}

void ramToUsb(void) {

    //send current rom header via usb
    gClearScreen();
    printf_lcd("run [edlink.exe usbrd --file  rx.bin --len 2048]\n");

    waitAB();

    //skip 256 bytes because regs mapped at 0x80+
    u8 *buff = (u8 *) (ADDR_GBA_ROM + 256);

    ed_usb_wr(buff, 2048);
}

void usbPrint() {

    gClearScreen();
    printf_lcd("run [edlink.exe fifowr --file message.txt]\n");

    u8 buff[128];
    u16 joy_lock = 1;

    printf_lcd("\npress [A] or [B] key to exit\n");

    while (1) {

        // Warning: FIFO size is only 2048 bytes.
        // Do not send new data until the previous data is fully received.
        int len = ed_fifo_rda(buff, sizeof (buff));

        if (len > 0) {
            printf_lcd("%.*s", len, buff);
        }

        u16 joy = sysJoyRead();

        if (joy == 0) {
            joy_lock = 0;
        }

        if ((joy == JOY_A || joy == JOY_B) && !joy_lock) {
            break;
        }
    }
}

u8 fileList() {

    u8 resp;
    u8 buff[1024];
    FileInfo inf = {.file_name = buff};
    u16 dir_size;

    resp = ed_fs_dir_load((u8 *) "", DIR_OPT_SORTED);
    if (resp)return resp;

    ed_fs_dir_get_size(&dir_size);
    if (resp)return resp;

    gClearScreen();

    for (int i = 0; i < dir_size && i < SCREEN_H - 4; i++) {

        //Request records. 
        //Faster in bulk, but the total unread data size must not exceed the FIFO size (2048 bytes).
        //Quickly read all records, then process them, or read them one by one.
        ed_fs_dir_get_recs(i, 1, sizeof (buff) - 1);
        resp = ed_fs_rx_next_rec(&inf);
        if (resp)return resp;

        if (inf.is_dir) {
            printf_lcd("[FIL]");
        } else {
            printf_lcd("[DIR]");
        }

        printf_lcd(" %.24s\n", inf.file_name);
    }


    waitAB();

    return 0;
}

void rtcPrint() {

    RtcTime rtc = {0};

    gClearScreen();
    ed_rtc_get(&rtc);

    printf_lcd("rtc: %02X.%02X.20%02X %02X:%02X:%02X\n",
            rtc.dom, rtc.mon, rtc.yar, rtc.hur, rtc.min, rtc.sec);
    waitAB();

}

void sysPath() {

    u8 path[1024 + 1];
    gClearScreen();

    ed_dev_rom_path(path, 0); // path to the current rom
    printf_lcd("rom: %s\n", path);

    waitAB();
}

int main() {

    //u8 resp;
    sysInit();
    ed_set_cmode(CMOD_TEST); //turn on regs and stay in game mode.
    ed_fifo_flush();

    int selector = 0;

    enum {
        MENU_EXIT_TO_MENU, //return to the main menu
        MENU_PRINTF_TO_USB, //dbg printf using usb (gba to pc)
        MENU_USB_PRINT, //tx message from pc to gba
        MENU_FILE_TO_RAM, //read file via fifon (slow)
        MENU_FILE_TO_ROM, //read file to cart memory (fast)
        MENU_RAM_TO_FILE, //write file via fifo (slow)
        MENU_ROM_TO_FILE, //write file from cart memory (fast)
        MENU_RAM_TO_USB, //write to usb via fifo
        MENU_FILE_LIST,
        MENU_RTC_PRINT,
        MENU_SYS_PATH,
        MENU_SIZE
    };


    char *menu[MENU_SIZE];
    menu[MENU_EXIT_TO_MENU] = "exit to menu";
    menu[MENU_PRINTF_TO_USB] = "printf to usb";
    menu[MENU_USB_PRINT] = "usb read and print";
    menu[MENU_FILE_TO_RAM] = "file to mem (ram)";
    menu[MENU_FILE_TO_ROM] = "file to mem (rom)";
    menu[MENU_RAM_TO_FILE] = "mem to file (ram)";
    menu[MENU_ROM_TO_FILE] = "mem to file (rom)";
    menu[MENU_RAM_TO_USB] = "mem to usb (ram)";
    menu[MENU_FILE_LIST] = "file list";
    menu[MENU_RTC_PRINT] = "rtc print";
    menu[MENU_SYS_PATH] = "system path";


    while (1) {

        gClearScreen();


        for (int i = 0; i < MENU_SIZE; i++) {
            printf_lcd("%c%s\n", selector == i ? '>' : ' ', menu[i]);
        }


        u16 joy = sysJoyWait();

        if (joy == JOY_U) {
            selector--;
            if (selector < 0) {
                selector = MENU_SIZE - 1;
            }
        }

        if (joy == JOY_D) {
            selector = (selector + 1) % MENU_SIZE;
        }

        if (joy != JOY_A) {
            continue;
        }

        u8 resp = 0;

        switch (selector) {

            case MENU_EXIT_TO_MENU:
                exitToMenu();
                break;

            case MENU_PRINTF_TO_USB:
                printfToUsb();
                break;

            case MENU_FILE_TO_RAM:
                resp = fileToRam();
                break;

            case MENU_FILE_TO_ROM:
                resp = fileToRom();
                break;

            case MENU_RAM_TO_FILE:
                resp = ramToFile();
                break;

            case MENU_ROM_TO_FILE:
                resp = romToFile();
                break;

            case MENU_RAM_TO_USB:
                ramToUsb();
                break;

            case MENU_USB_PRINT:
                usbPrint();
                break;

            case MENU_FILE_LIST:
                resp = fileList();
                break;

            case MENU_RTC_PRINT:
                rtcPrint();
                break;

            case MENU_SYS_PATH:
                sysPath();
                break;

        }

        if (resp) {
            printtError(resp);
        }
    }


}

