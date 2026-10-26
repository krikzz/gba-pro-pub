/* 
 * File:   everdrive.h
 * Author: Igor
 *
 * Created on January 10, 2025, 5:20 PM
 */

#ifndef EVERDRIVE_H
#define	EVERDRIVE_H

#define STATUS_KEY      0x5A
#define PROTOCOL_ID     0x02


#define ERR_UNXP_STAT           0x40
#define ERR_NULL_PATH           0x41
#define ERR_FIFO_TOUT           0x5C
//****************************************************************************** everdrive control registers

typedef struct {
    vu16 FIFODATA; //   R/W  onboard mcu fifo io port
    vu16 FIFOSTAT; //   R    Fifo status
    vu16 SYSSTAT; //    R    mcu/sdc/fpga status
    vu16 TIMER; //      R    system timer, 32bit, 1ms resolution
    vu16 reserved[3];
    vu16 CMODE; //      W    cart modes(menu, game, sst handler)
} EdReg;

#define FIFOSTAT_MCURXF (1 << 6) //fifo flags. mcu can read
#define FIFOSTAT_CPURXF (1 << 7) //fifo flags. system cpu can read

#define SYSSTAT_BUSY    (1 << 0) //sdc busy, mcu busy, mcu fifo not empty
#define SYSSTAT_CMDOK   (1 << 1) //mcu finished command execution
#define SYSSTAT_FPGOK   (1 << 2) //fpga reboot complete
#define SYSSTAT_STROBE  (1 << 3) //inverted after every read

//0: regs en
//1: gmode
//2: sst ram en
#define CMOD_MENU       (0xA8 | 0x05)//ram1, regs unlocked
#define CMOD_GAME       (0xA8 | 0x02)//ram0, regs locked 
#define CMOD_TEST       (0xA8 | 0x03)//ram0, regs unlocked


//#define SOL_SENS_MAX    255
//#define SOL_SENS_DEF    32

#define EDREG_BASE      (ADDR_GBA_ROM + 0x80)
#define EDREG           ((EdReg *) EDREG_BASE)

typedef enum {
    BRM_TYPE_OFF = 0x00,
    BRM_TYPE_EEP,
    BRM_TYPE_SRAM,
    BRM_TYPE_FLA64,
    BRM_TYPE_FLA128
} BramType;

typedef enum {
    BRM_SIZE_TOTAL = 0x20000,
    BRM_SIZE_SRAM = 0x8000,
    BRM_SIZE_FL64 = 0x10000,
    BRM_SIZE_FL128 = 0x20000,
    BRM_SIZE_EEP = 8192,
} BramSize;


#define CTRL_BOOT_ON    (1 << 0) //enable boot rom
#define CTRL_GPAK_RST   (1 << 2) //reset gamepak hardware

#define GPAK_HW_RTC     (1 << 0)
#define GPAK_HW_SOL     (1 << 1)
#define GPAK_HW_GYR     (1 << 2)
#define GPAK_HW_TIL     (1 << 3)

#define SUB_STATUS_DISK 0x00
#define SUB_STATUS_USB  0x01
#define SUB_STATUS_CUE  0x02
#define SUB_STATUS_BOOT 0x03
#define SUB_STATUS_UPD  0x04

#define BMOD_GBA_CSTART 0x0000//cold start
#define BMOD_GBA_REBOOT 0x4252//'RB' reboot request
#define BMOD_GBA_SST    0x5453//'ST' in-game menu mode
#define BMOD_GBA_RETURN 0x5452//'RT' return from game
#define BMOD_GBA_VDUMP  0x4456//'VD' vram dump
#define BMOD_GBA_GSTART 0x5347//'GS' game start


#define EPO_TYPE_LINK           0x10 //link (usb or console)
#define EPO_TYPE_LINK_ACK       0x11 //link (usb or console 1k blocks)
#define EPO_TYPE_FS             0x12 //file
#define EPO_TYPE_FCI            0x13 //ed mem
#define EPO_TYPE_FLA            0x14 //mcu flash
#define EPO_TYPE_RAM            0x15 //ram buffer
#define EPO_TYPE_NOP            0x16

/*
#define HAN_TYPE_BRM    0x01
#define HAN_TYPE_RTC    0x02
#define HAN_TYPE_SOL    0x04
#define HAN_TYPE_IMU    0x08
#define HAN_TYPE_PWR    0x10

#define IMU_FLIP_TILX   0x01
#define IMU_FLIP_TILY   0x02
#define IMU_FLIP_GYRO   0x04*/
//****************************************************************************** addr and size
#define ADDR_FCI_RAM0   0x0000000       //32MB PSRAM
#define ADDR_FCI_RAM1   0x4000000       //8MB PSRAM
#define ADDR_FCI_IRM    0x8100000       //1KB on-chip ram
#define ADDR_FCI_SYS    0x8800000       //system registers

#define ADDR_OFF_MENU   0x000000        //menu code
#define ADDR_OFF_BMOD   0x0000BE        //boot mode flag
#define ADDR_OFF_DBUF   0x100000        //data buffer gp (system psram)
#define ADDR_OFF_SST    0x200000        //data buffer sst(system psram)
#define ADDR_OFF_BRM    0x7E0000        //bram (system psram)

#define ADDR_FCI_ROM    (ADDR_FCI_RAM0 + 0x000000)       //32MB PSRAM
#define ADDR_FCI_MENU   (ADDR_FCI_RAM1 + ADDR_OFF_MENU)
#define ADDR_FCI_BMOD   (ADDR_FCI_MENU + ADDR_OFF_BMOD)  //2B boot mode
#define ADDR_FCI_FLAGS  (ADDR_FCI_MENU + SIZE_MENU-256)  //flags (system psram)
#define ADDR_FCI_DBUF   (ADDR_FCI_RAM1 + ADDR_OFF_DBUF)  //1M IO buffer (system psram)
#define ADDR_FCI_SST    (ADDR_FCI_RAM1 + ADDR_OFF_SST)   //1M IO buffer (system psram)
#define ADDR_FCI_BRM    (ADDR_FCI_RAM1 + ADDR_OFF_BRM)   //128KB battery ram 

#define ADDR_FCI_CFG    (ADDR_FCI_SYS + 0x00000)         //system config
#define ADDR_FCI_FIFO   (ADDR_FCI_SYS + 0x10000)         //mcu fifo
#define ADDR_FCI_SDIO   (ADDR_FCI_SYS + 0x30000)         //sdio
#define ADDR_FCI_CART   (ADDR_FCI_SYS + 0x40000)         //cart area (mostly for rtc)
#define ADDR_FCI_MICC   (ADDR_FCI_SYS + 0x50000)         //sst handler micro code 


#define ADDR_GBA_ROM    0x8000000
#define ADDR_GBA_SWINF  (ADDR_GBA_ROM + 0xD0)
#define ADDR_GBA_HOOK   (ADDR_GBA_ROM + 0x04)
#define ADDR_GBA_BMOD   (ADDR_GBA_ROM + ADDR_OFF_BMOD)
#define ADDR_GBA_DBUF   (ADDR_GBA_ROM + ADDR_OFF_DBUF)
#define ADDR_GBA_SST    (ADDR_GBA_ROM + ADDR_OFF_SST)
#define ADDR_GBA_MICC   (ADDR_GBA_ROM + 0x800000)
#define ADDR_GBA_SRM    0xE000000

#define ADDR_FLA_ICOR   0x00000         //mcu firmware update

#define SIZE_ROM        0x2000000       //rom memory size     
#define SIZE_IRM        0x0000400       //on-chip ram for sdio operations
#define SIZE_MICC       4096            //on-chip ram for sst and cheats microcode
#define SIZE_MICC_SST   1536            //sst handler
#define SIZE_MICC_CC    (SIZE_MICC - SIZE_MICC_SST)//cheats code
#define SIZE_DBUF       0x100000        //data buffer in system psram
#define SIZE_SST        0x100000        //data buffer in system psram
#define SIZE_MENU       0x40000

#define ACK_BLOCK_SIZE  1024
#define RST_SRC_POR     0x00 //power on (mcu)

#define MAX_IOCOR_SIZE  (0x38000+16)
//****************************************************************************** file mode
#define	FA_READ		 0x01
#define	FA_WRITE	 0x02
#define	FA_OPEN_EXISTING 0x00
#define	FA_CREATE_NEW	 0x04
#define	FA_CREATE_ALWAYS 0x08
#define	FA_OPEN_ALWAYS	 0x10
#define	FA_OPEN_APPEND	 0x30

#define	AT_RDO          0x01    // Read only
#define	AT_HID          0x02	// Hidden
#define	AT_SYS          0x04	// System
#define AT_DIR          0x10	// Directory
#define AT_ARC          0x20	// Archive

#define DIR_OPT_SORTED  0x01
#define DIR_OPT_HIDESYS 0x02
#define DIR_OPT_SEEKCUE 0x04
#define DIR_OPT_FILTCUE 0x08
#define DIR_OPT_FILTROM 0x10
#define DIR_OPT_FILTRBF 0x20
//******************************************************************************

typedef struct {
    u32 size;
    u16 date;
    u16 time;
    u8 is_dir; //structure size hardcoded in ed_rx_file_info
    u8 *file_name;
} FileInfo;

typedef struct {
    u8 bram_type : 3;
    u8 reserved : 1;
    u8 sens_til : 1;
    u8 sens_gyr : 1;
    u8 sens_sol : 1;
    u8 rtc : 1;
} GpakHW;

typedef struct {
    u8 rom_size;
    GpakHW gpak_hw;
    u8 reserved;
    u8 ctrl;
} CartCfg;

typedef struct {//do not use enum types!
    u32 rom_size;
    u32 brm_size;
    u32 rom_type;
    u32 brm_type;
    u32 rtc_en;
    u32 til_en;
    u32 gyr_en;
    u32 sol_en;
    u32 sol_sens;
    u32 til_flip;
    u32 mode_b;
    u32 reserved[5];
} DevCfg;

typedef struct {
    u8 fla_id[8];
    u8 cpu_id[12];
    u32 serial_g;
    u32 serial_l;
    u32 boot_ctr;
    u32 game_ctr;
    u16 asm_date;
    u16 asm_time;
    u16 sw_date;
    u16 sw_time;
    u16 boot_date;
    u16 boot_time;
    u16 hv_ver;
    u8 device_id;
    u8 reserved1; //rapp_status;
    u8 rst_src;
    u8 boot_status;
    u8 bat_dry;
    u8 disk_status;
    u8 reserved2; //pwr_sys;
    u8 reserved3; //pwr_usb;
    u8 fla_cap; //flash size in power of two
    u8 reserved[1];
    u16 efu_date;
    u16 efu_time;
} SysInfoIO;

typedef struct {
    u16 v50;
    u16 v25;
    u16 v12;
    u16 vbt;
} Vdc;

typedef struct {
    u16 vector_f6;
    u16 vector_f8;
    u16 vector_fa;
    u8 codesize;
    u8 code[128 - 7];
} CCRamCodes;

typedef struct {
    u8 addr[3];
    u8 val;
} CCRomSlot;

typedef struct {
    CCRomSlot slot[16];
} CCRomCodes;

typedef struct {
    u8 yar;
    u8 mon;
    u8 dom;
    u8 hur;
    u8 min;
    u8 sec;
} RtcTime;

typedef u8 EpoType;

typedef struct {
    u32 src_addr;
    u32 dst_addr;
    u32 len;
    EpoType src_type;
    EpoType dst_type;
    u8 reserved[2];
} EpoXfer;

typedef enum {
    DEV_PATH_ROM,
    DEV_PATH_BRM,
    DEV_PATH_RTC
} DevPathType;

typedef struct {
    u8 han_brm;
    u8 han_rtc;
    u8 han_sol;
    u8 han_imu;
    u8 cache_brm;
    u8 cache_rtc;
    u8 reserved[10];
} HanStatus;

void ed_init(u8 reset_cfg);
void ed_fifo_flush();
u8 ed_fifo_busy();
void ed_fifo_rd(void *dst, u16 len);
u32 ed_fifo_rda(void *dst, u32 len);
u8 ed_fifo_rd_ack(void *dst, u16 len);
void ed_fifo_wr(void *src, u16 len);
u8 ed_fifo_wr_ack(void *src, u16 len);
u8 ed_check_status();
void ed_tx_string(u8 *string);
void ed_rx_string(u8 *string);
u8 ed_fs_rx_next_rec(FileInfo *inf);
u32 ed_get_ticks();
void ed_set_ctrl(u8 ctrl);
void ed_app_start(CartCfg *cfg);


void ed_sys_inf(SysInfoIO *inf);
u8 ed_cmd_nresp(u8 stat_req);
void ed_sys_get_vdc(Vdc *vdc);
void ed_dev_game_ctr();
u8 ed_sys_fpga_init(u8 *path);
void ed_sys_rst_efu();
u8 ed_sys_efu_unpack();
u8 ed_sys_efu_install(u8 *path);
void ed_rtc_get(RtcTime *time);
void ed_rtc_set(RtcTime *time);
void ed_usb_wr(void *data, u16 len);
u8 ed_dev_rom_path(u8 *path, u8 path_type);
s32 ed_rtcc_curval();

/*
u8 ed_cmd_han_set(u8 han_mask);
u8 ed_cmd_han_status(u8 han);
void ed_cmd_abup_bsize(u32 bram_size);
void ed_cmd_abup_cpmem();
void ed_cmd_solar_sens(u32 sens);
void ed_cmd_imu_flip(u32 flip);*/

void ed_cmd_tx(u8 cmd);
void ed_scmd_tx(u8 cmd, u8 scmd);
void ed_cmd_status(void *status);
u8 ed_fs_disk_init();
u8 ed_fs_disk_goidle();
u8 ed_fs_file_open(u8 *path, u8 mode);
u8 ed_fs_file_close();
u8 ed_fs_file_read(void *dst, u32 len);
u8 ed_fs_file_read_fci(u32 addr, u32 len);
u8 ed_fs_file_write(void *src, u32 len);
u8 ed_fs_file_write_fci(u32 addr, u32 len);
u8 ed_fs_file_copy(u8 *src, u8 *dst, u8 dst_mode);
u32 ed_fs_file_available();
u8 ed_fs_file_set_ptr(u32 addr);
u8 ed_fs_file_info(u8 *path, FileInfo *inf);
u8 ed_fs_file_del(u8 *path);
u8 ed_fs_file_crc(u32 len, u32 *crc_base);
u8 ed_fs_file_seek_pat(u8 *pat, u8 psize, u32 fsize, u32 *paddr);
u8 ed_fs_dir_make(u8 *path);
u8 ed_fs_file_test(u8 *path);
u8 ed_fs_dir_test(u8 *path);

u8 ed_fs_dir_load(u8 *path, u8 dir_opt);
void ed_fs_dir_get_size(u16 *size);
void ed_fs_dir_seek_idx(u16 *idx);
void ed_fs_dir_get_recs(u16 start_idx, u16 amount, u16 max_name_len);

void ed_fci_set(u8 val, u32 addr, u32 len);
u8 ed_fci_test(u8 val, u32 addr, u32 len);
void ed_fci_rd(u32 addr, void *dst, u32 len);
void ed_fci_wr(u32 addr, void *src, u32 len);
void ed_fci_crc(u32 addr, u32 len, u32 *crc_base);
void ed_cmd_epo_xfer(EpoXfer *xfer);


u32 ed_inbank_addr(void *ptr, u8 bank);
void ed_set_config(CartCfg *cfg);
void ed_set_cmode(u16 mode);
u16 ed_bootmod_get();
void ed_bootmod_set(u16 mode);
void ed_wait_status(u8 status, u8 ack);
u8 ed_boot_status();

u8 ed_dev_rst_cfg();
u8 ed_dev_stop();
u8 ed_dev_start();
u8 ed_dev_set_path(u8 *path, DevPathType path_type);
u8 ed_dev_set_cfg(DevCfg *cfg);
u8 ed_dev_pause();
u8 ed_dev_resume(u8 rst_gpak);
void ed_dev_han_stat(HanStatus *stat);
u8 ed_dev_han_cpmem();

#endif	/* EVERDRIVE_H */

