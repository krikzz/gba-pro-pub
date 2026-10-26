
#include "appmain.h"

#define CMD_STATUS      0x10
#define CMD_GET_MODE    0x11
#define CMD_RST_MCU     0x12
#define CMD_GET_VDC     0x13
#define CMD_RTC_GET     0x14
#define CMD_RTC_SET     0x15
//#define CMD_FLA_RD      0x16
//#define CMD_FLA_WR      0x17
//#define CMD_FLA_WR_SDC  0x18
//#define CMD_FCI_RD      0x19
//#define CMD_FCI_WR      0x1A
#define CMD_FCI_SET     0x1B
#define CMD_FCI_TST     0x1C
#define CMD_FCI_CRC     0x1D
#define CMD_FPG_INIT    0x1E
//#define CMD_FPG_USB     0x1E
//#define CMD_FPG_SDC     0x1F
//#define CMD_FPG_FLA     0x20
#define CMD_RTC_CAL     0x21
#define CMD_USB_WR      0x22
//#define CMD_FIFO_WR     0x23
//#define CMD_UART_WR     0x24
#define CMD_RST_EFU     0x25
#define CMD_SYS_INF     0x26
#define CMD_GAME_CTR    0x27
//#define CMD_UPD_EXEC    0x28
//#define CMD_HOST_RST    0x29
//**********************0x2A CMD_USB_STATUS
//#define CMD_RAPP_SET    0x2B
#define CMD_SUB_STATUS  0x2C
//**********************0x2D CMD_BRAM_SAVE
#define CMD_EFU_UNPACK  0x2E//install menu to sd 
#define CMD_EFU_INSTALL  0x2F//write efu to flash
//**********************0x30 CMD_CALC_FILT
#define CMD_ROM_PATH    0x31
//#define CMD_EFU_UNFILE  0x32//unpack file
#define CMD_EPO         0x33

//#define CMD_CD_MOUNT    0x41
//#define CMD_BRAM_SAVE   0x42//force bram moving to file if it was modified
//#define CMD_CALC_FILT   0x43

//#define CMD_HANDLERS    0x44//mcu handlers

//#define CMD_SST         0x44
//#define CMD_ABUP        0x45//auto backup control
//#define CMD_SOLAR       0x46

#define CMD_DISK_INIT   0xC0
#define CMD_DISK_RD     0xC1
#define CMD_DISK_WR     0xC2
#define CMD_F_DIR_OPN   0xC3
#define CMD_F_DIR_RD    0xC4
#define CMD_F_DIR_LD    0xC5
#define CMD_F_DIR_SIZE  0xC6
#define CMD_F_DIR_PATH  0xC7
#define CMD_F_DIR_GET   0xC8
#define CMD_F_FOPN      0xC9
//#define CMD_F_FRD       0xCA
//#define CMD_F_FRD_FCI   0xCB
//#define CMD_F_FWR       0xCC
//#define CMD_F_FWR_FCI   0xCD
#define CMD_F_FCLOSE    0xCE
#define CMD_F_FPTR      0xCF
#define CMD_F_FINFO     0xD0
#define CMD_F_FCRC      0xD1
#define CMD_F_DIR_MK    0xD2
#define CMD_F_DEL       0xD3
#define CMD_F_SEEK_IDX  0xD4
#define CMD_F_AVB       0xD5
#define CMD_F_FCP       0xD6
//#define CMD_F_FMV       0xD7
#define CMD_F_SEEK_PAT  0xD8 //seek data pattern
#define CMD_F_DTEST     0xD9 //check if dir exists
#define CMD_F_FTEST     0xDA //check if file exists
#define CMD_DISK_GOIDLE 0xDF

#define RTCC_SET_TIME   0
#define RTCC_CAL_START  1
#define RTCC_CAL_END    2
#define RTCC_GET_CURCAL 3
#define RTCC_GET_ESTCAL 4
#define RTCC_GET_DEVIAT 5

#define EPO_SCMD_XFER   0x10
#define EPO_SCMD_MSR    0x11 //memory security register (one time)
#define EPO_MSR_WPOFF   0x27101983
//****************************************************************************** CMD_HANDLER scmd
/*
#define HAN_SCMD_SETHAN 0x00
#define HAN_SCMD_STATUS 0x01
#define HAN_SCMD_SST    0x10
#define HAN_SCMD_ABUP   0x20
#define HAN_SCMD_SOL    0x30
#define HAN_SCMD_IMU    0x40
 */

#define SST_SCMD_SMODE  (HAN_SCMD_SST  | 0x00)//set sys mode
#define SST_SCMD_GMODE  (HAN_SCMD_SST  | 0x01)//set game mode

#define ABUP_SCMD_BSIZE (HAN_SCMD_ABUP | 0x00)//set bram size
#define ABUP_SCMD_CPMEM (HAN_SCMD_ABUP | 0x01)//force backup (in any case)

#define SOL_SCMD_SENS   (HAN_SCMD_SOL  | 0x00)//solar sensor sensivity

#define IMU_SCMD_FLIP   (HAN_SCMD_IMU  | 0x00)//flip imu sensor (tilt only for now)
//******************************************************************************
#define CMD_DEV                 0x86
#define DEV_SCMD_RST_CFG        0x10//finish pend tasks, pause han, reset fpg 
#define DEV_SCMD_STOP           0x11//finish pend tasks, stop han, cache to file, reset fpg
#define DEV_SCMD_START          0x12//init fpg, start handlers
#define DEV_SCMD_PAUSE          0x13//finish pend tasks, pause han
#define DEV_SCMD_RESUME         0x14//resume handlers
#define DEV_SCMD_SET_CFG        0x20
#define DEV_SCMD_SET_PATH       0x21
#define DEV_SCMD_GET_PATH       0x22
//#define DEV_SCMD_WR_SWAP        0x23
#define DEV_SCMD_HAN_STAT       0x24
#define DEV_SCMD_HAN_CPMEM      0x25//copy bram to cache
//******************************************************************************
void ed_wait_mcu();
void ed_run_dma();
u8 ed_mcu_is_busy();
void ed_wait_mcu();
u8 ed_fifo_rd_skip(u16 len);
u32 ed_min(u32 v1, u32 v2);
void ed_rx_file_info(FileInfo *inf);
//****************************************************************************** public
u16 cmode;

void ed_init(u8 reset_cfg) {

    ed_set_cmode(CMOD_MENU);

    if (reset_cfg) {
        ed_dev_rst_cfg();
    } else {
        ed_fifo_flush();
    }
}

u8 ed_fifo_busy() {

    return (EDREG->FIFOSTAT & FIFOSTAT_CPURXF) ? 1 : 0;
}

u8 ed_fifo_rd_to(void *dst, u16 len) {

    u8 *dst8 = (u8 *) dst;

    while (len--) {

        u32 to_ctr = 0;

        while ((EDREG->FIFOSTAT & FIFOSTAT_CPURXF)) {

            //in case of error mcu will skip data tx and will be in idle state
            if (ed_mcu_is_busy()) {
                to_ctr = 0;
            } else {
                to_ctr++;
            }

            if (to_ctr > 1000) {
                return ERR_FIFO_TOUT;
            }
        }
        *dst8++ = EDREG->FIFODATA;
    }

    return 0;
}

void ed_fifo_rd(void *dst, u16 len) {

    u8 *dst8 = dst;

    while (len--) {

        while ((EDREG->FIFOSTAT & FIFOSTAT_CPURXF)) {
            asm("nop");
        }
        *dst8++ = EDREG->FIFODATA;
    }
}

u32 ed_fifo_rda(void *dst, u32 len) {


    u32 rdx = 0;

    while (!ed_fifo_busy() && rdx < len) {
        ed_fifo_rd(dst, 1);
        dst++;
        rdx++;
    }

    return rdx;
}

u8 ed_fifo_rd_ack(void *dst, u16 len) {

    u8 resp = 0;
    u8 ack = 0;

    while (len) {

        u32 block = ed_min(ACK_BLOCK_SIZE, len);

        ed_fifo_wr(&ack, 1);
        resp = ed_fifo_rd_to(dst, block);
        if (resp)break;

        dst += block;
        len -= block;
    }

    if (resp == ERR_FIFO_TOUT) {
        resp = ed_check_status();
        if (resp == 0) {
            resp = ERR_FIFO_TOUT;
        }
    }

    return resp;
}

void ed_fifo_wr(void *src, u16 len) {

    if (len == 0) {
        return;
    }

    u8 *src8 = (u8 *) src;

    while (len--) {
        EDREG->FIFODATA = *src8++;
    }
}

u8 ed_fifo_wr_ack(void *src, u16 len) {

    u8 resp = 0;
    u8 ack = 0;

    while (len) {

        u32 block = ed_min(ACK_BLOCK_SIZE, len);

        resp = ed_fifo_rd_to(&ack, 1);
        if (resp)break;

        ed_fifo_wr(src, block);

        src += block;
        len -= block;
    }

    if (resp == ERR_FIFO_TOUT) {
        resp = ed_check_status();
        if (resp == 0) {
            resp = ERR_FIFO_TOUT;
        }
    }

    return resp;
}

u8 ed_check_status() {

    u8 resp[4];

    ed_cmd_status(resp);

    if (resp[0] != STATUS_KEY) {
        return ERR_UNXP_STAT;
    }

    return resp[3];
}

void ed_tx_string(u8 *string) {

    u16 str_len = 0;
    u8 *ptr = string;

    while (*ptr++ != 0)str_len++;

    ed_fifo_wr(&str_len, 2);
    ed_fifo_wr(string, str_len);
}

void ed_rx_string(u8 *string) {

    u16 str_len;

    ed_fifo_rd(&str_len, 2);

    if (string == 0) {
        ed_fifo_rd_skip(str_len);
        return;
    }

    string[str_len] = 0;

    ed_fifo_rd(string, str_len);
}

u32 ed_get_ticks() {

    EDREG->TIMER = 0;
    return EDREG->TIMER |
            (EDREG->TIMER << 8) |
            (EDREG->TIMER << 16) |
            (EDREG->TIMER << 24);
}

void ed_sys_inf(SysInfoIO *inf) {
    ed_cmd_tx(CMD_SYS_INF);
    ed_fifo_rd(inf, sizeof (SysInfoIO));
}

u8 ed_cmd_nresp(u8 resp) {

    u8 nresp;
    ed_cmd_tx(CMD_SUB_STATUS);
    ed_fifo_wr(&resp, 1);
    ed_fifo_rd(&nresp, 1);
    return nresp;
}

void ed_sys_get_vdc(Vdc *vdc) {

    ed_cmd_tx(CMD_GET_VDC);
    ed_fifo_rd(vdc, sizeof (Vdc));

}

u8 ed_sys_fpga_init(u8 *path) {

    u8 resp;
    u8 epo_type = EPO_TYPE_FS;
    u32 len;

    resp = ed_fs_file_open(path, FA_READ);
    if (resp)return resp;

    len = ed_fs_file_available();

    ed_cmd_tx(CMD_FPG_INIT);
    ed_fifo_wr(&len, 4);

    //ed_halt(STATUS_FPG_OK); //fpg ok flag will be reset only after fpga reconfig
    ed_wait_status(SYSSTAT_FPGOK, epo_type);

    ed_set_config(0);
    ed_fifo_flush();

    return ed_check_status();
}

/*
void ed_sys_rst_efu() {

    u8 ack = 0;
    ed_cmd_fci_set(0, ADDR_FCI_MENU + 0xA0, 8);
    ed_cmd_tx(CMD_RST_EFU);
    ed_fifo_wr(&ack, 1); //use only without ed_wait_status 
    //ed_wait_status(SYSSTAT_FPGOK | SYSSTAT_CMDOK, 0);
    while (!str_cmp_len("EVERDRIVE", (u8 *) ADDR_GBA_ROM + 0xA0, 0));
    sys_reset();
}*/

u8 ed_sys_efu_unpack() {

    ed_cmd_tx(CMD_EFU_UNPACK);
    return ed_check_status();
}

u8 ed_sys_efu_install(u8 *path) {

    ed_cmd_tx(CMD_EFU_INSTALL);
    ed_tx_string(path);
    return ed_cmd_nresp(SUB_STATUS_UPD);
}

void ed_rtc_get(RtcTime *time) {

    ed_cmd_tx(CMD_RTC_GET);
    ed_fifo_rd(time, sizeof (RtcTime));
}

void ed_rtc_set(RtcTime *time) {

    ed_cmd_tx(CMD_RTC_SET);
    ed_fifo_wr(time, sizeof (RtcTime));
}

s32 ed_rtcc_curval() {

    u8 cmd = RTCC_GET_CURCAL;
    RtcTime time = {0};
    s32 resp;

    ed_cmd_tx(CMD_RTC_CAL);
    ed_fifo_wr(&time, sizeof (RtcTime));
    ed_fifo_wr(&cmd, 1);

    ed_fifo_rd(&resp, 4);

    return resp;
}

void ed_usb_wr(void *data, u16 len) {

    ed_cmd_tx(CMD_USB_WR);
    ed_fifo_wr(&len, 2);
    ed_fifo_wr(data, len);
}


//******************************************************************************

u8 ed_mcu_is_busy() {

    vu16 v1 = EDREG->SYSSTAT;
    vu16 v2 = EDREG->SYSSTAT;

    if ((v1 & SYSSTAT_BUSY) && v1 != v2) {
        return 1;
    } else {
        //skip if regs locked (v1 == v2)
        return 0;
    }
}

void ed_wait_mcu() {

    while (ed_mcu_is_busy());
}

void ed_run_dma() {

    u8 ack = 0;
    ed_fifo_wr(&ack, 1);
    ed_wait_mcu();
}

u8 ed_fifo_rd_skip(u16 len) {

    while (len--) {

        while ((EDREG->FIFOSTAT & FIFOSTAT_CPURXF));
        vu8 tmp = EDREG->FIFODATA;
        tmp++;
    }

    return 0;
}

u32 ed_min(u32 v1, u32 v2) {

    if (v1 < v2) {
        return v1;
    } else {
        return v2;
    }
}

void ed_rx_file_info(FileInfo *inf) {

    ed_fifo_rd(inf, 9);
    ed_rx_string(inf->file_name);
    inf->is_dir &= AT_DIR;
}

void ed_fifo_flush() {

    u32 tout = 2048;
    EDREG->FIFODATA = 0;
    EDREG->FIFODATA = 0;
    while ((EDREG->FIFOSTAT & FIFOSTAT_CPURXF) == 0 && tout--) {
        vu8 tmp = EDREG->FIFODATA;
        tmp++;
    }
}
//****************************************************************************** cmd var

void ed_cmd_tx(u8 cmd) {

    u8 buff[4];

    buff[0] = '+';
    buff[1] = '+' ^ 0xff;
    buff[2] = cmd;
    buff[3] = cmd ^ 0xff;

    ed_fifo_wr(buff, sizeof (buff));
}

void ed_scmd_tx(u8 cmd, u8 scmd) {
    ed_cmd_tx(cmd);
    ed_fifo_wr(&scmd, 1);
}

void ed_cmd_status(void *status) {

    ed_cmd_tx(CMD_STATUS);
    ed_fifo_rd(status, 4);
}
//****************************************************************************** disk io

u8 ed_fs_disk_init() {

    ed_cmd_tx(CMD_DISK_INIT);
    return ed_check_status();
}

u8 ed_fs_disk_goidle() {

    ed_cmd_tx(CMD_DISK_GOIDLE);
    return ed_check_status();
}

u8 ed_fs_file_open(u8 *path, u8 mode) {

    if (*path == 0)return ERR_NULL_PATH;
    ed_cmd_tx(CMD_F_FOPN);
    ed_fifo_wr(&mode, 1);
    ed_tx_string(path);
    return ed_check_status();
}

u8 ed_fs_file_close() {

    ed_cmd_tx(CMD_F_FCLOSE);
    return ed_check_status();
}

u8 ed_fs_file_read(void *dst, u32 len) {

    u8 ack = 0;
    u8 resp;

    if (len == 0) {
        return 0;
    }

    EpoXfer xfer = {
        .src_type = EPO_TYPE_FS, .src_addr = 0,
        .dst_type = EPO_TYPE_LINK_ACK, .dst_addr = 0,
        .len = len
    };

    ed_cmd_epo_xfer(&xfer);
    ed_fifo_wr(&ack, 1);

    resp = ed_fifo_rd_ack(dst, len);
    if (resp)return resp;

    return ed_check_status();
}

u8 ed_fs_file_read_fci(u32 addr, u32 len) {

    if (len == 0) {
        return 0;
    }

    EpoXfer xfer = {
        .src_type = EPO_TYPE_FS, .src_addr = 0,
        .dst_type = EPO_TYPE_FCI, .dst_addr = addr,
        .len = len
    };
    ed_cmd_epo_xfer(&xfer);
    ed_run_dma();

    return ed_check_status();

}

u8 ed_fs_file_write(void *src, u32 len) {

    u8 ack = 0;
    u8 resp;

    if (len == 0) {
        return 0;
    }

    EpoXfer xfer = {
        .src_type = EPO_TYPE_LINK_ACK, .src_addr = 0,
        .dst_type = EPO_TYPE_FS, .dst_addr = 0,
        .len = len
    };

    ed_cmd_epo_xfer(&xfer);
    ed_fifo_wr(&ack, 1);

    resp = ed_fifo_wr_ack(src, len);
    if (resp)return resp;

    return ed_check_status();
}

u8 ed_fs_file_write_fci(u32 addr, u32 len) {

    if (len == 0) {
        return 0;
    }

    EpoXfer xfer = {
        .src_type = EPO_TYPE_FCI, .src_addr = addr,
        .dst_type = EPO_TYPE_FS, .dst_addr = 0,
        .len = len
    };
    ed_cmd_epo_xfer(&xfer);
    ed_run_dma();

    return ed_check_status();
}

u8 ed_fs_file_copy(u8 *src, u8 *dst, u8 dst_mode) {

    ed_cmd_tx(CMD_F_FCP);
    ed_fifo_wr(&dst_mode, 1);
    ed_tx_string(src);
    ed_tx_string(dst);
    return ed_check_status();
}

u32 ed_fs_file_available() {

    u32 len[2];
    ed_cmd_tx(CMD_F_AVB);
    ed_fifo_rd(len, 8);

    return len[1];
}

u8 ed_fs_file_set_ptr(u32 addr) {

    ed_cmd_tx(CMD_F_FPTR);
    ed_fifo_wr(&addr, 4);
    return ed_check_status();
}

u8 ed_fs_file_info(u8 *path, FileInfo *inf) {

    u8 resp;

    ed_cmd_tx(CMD_F_FINFO);
    ed_tx_string(path);

    ed_fifo_rd(&resp, 1);
    if (resp)return resp;

    ed_rx_file_info(inf);

    return 0;
}

u8 ed_fs_file_del(u8 *path) {

    ed_cmd_tx(CMD_F_DEL);
    ed_tx_string(path);
    return ed_check_status();
}

u8 ed_fs_file_crc(u32 len, u32 *crc_base) {

    u8 resp;
    ed_cmd_tx(CMD_F_FCRC);
    ed_fifo_wr(&len, 4);
    ed_fifo_wr(crc_base, 4);

    ed_fifo_rd(&resp, 1);
    ed_fifo_rd(crc_base, 4);

    return resp;
}

u8 ed_fs_dir_make(u8 *path) {

    ed_cmd_tx(CMD_F_DIR_MK);
    ed_tx_string(path);
    return ed_check_status();
}

u8 ed_fs_dir_load(u8 *path, u8 dir_opt) {

    ed_cmd_tx(CMD_F_DIR_LD);
    ed_fifo_wr(&dir_opt, 1);
    ed_tx_string(path);

    return ed_check_status();
}

void ed_fs_dir_get_size(u16 *size) {

    ed_cmd_tx(CMD_F_DIR_SIZE);
    ed_fifo_rd(size, 2);
}

void ed_fs_dir_seek_idx(u16 *idx) {

    ed_cmd_tx(CMD_F_SEEK_IDX);
    ed_fifo_rd(idx, 2);
}

void ed_fs_dir_get_recs(u16 start_idx, u16 amount, u16 max_name_len) {

    ed_cmd_tx(CMD_F_DIR_GET);
    ed_fifo_wr(&start_idx, 2);
    ed_fifo_wr(&amount, 2);
    ed_fifo_wr(&max_name_len, 2);
}

u8 ed_fs_file_seek_pat(u8 *pat, u8 psize, u32 fsize, u32 *paddr) {

    u8 resp;
    ed_cmd_tx(CMD_F_SEEK_PAT);
    ed_fifo_wr(&fsize, 4);
    ed_fifo_wr(&psize, 1);
    ed_fifo_wr(pat, psize);

    ed_fifo_rd(&resp, 1);
    ed_fifo_rd(paddr, 4);
    return resp;
}

u8 ed_fs_file_test(u8 *path) {
    ed_cmd_tx(CMD_F_FTEST);
    ed_tx_string(path);
    return ed_check_status();
}

u8 ed_fs_dir_test(u8 *path) {
    ed_cmd_tx(CMD_F_DTEST);
    ed_tx_string(path);
    return ed_check_status();
}

u8 ed_fs_rx_next_rec(FileInfo *inf) {

    u8 resp;

    ed_fifo_rd(&resp, 1);
    if (resp)return resp;

    ed_rx_file_info(inf);

    return 0;
}
//****************************************************************************** memory io

void ed_fci_set(u8 val, u32 addr, u32 len) {

    ed_cmd_tx(CMD_FCI_SET);
    ed_fifo_wr(&addr, 4);
    ed_fifo_wr(&len, 4);
    ed_fifo_wr(&val, 1);
    ed_run_dma();
}

u8 ed_fci_test(u8 val, u32 addr, u32 len) {

    u8 resp;

    ed_cmd_tx(CMD_FCI_TST);
    ed_fifo_wr(&addr, 4);
    ed_fifo_wr(&len, 4);
    ed_fifo_wr(&val, 1);
    ed_run_dma();
    ed_fifo_rd(&resp, 1);

    return resp;
}

void ed_fci_rd(u32 addr, void *dst, u32 len) {

    u8 ack = 0;
    EpoType link_type = len > 1024 ? EPO_TYPE_LINK_ACK : EPO_TYPE_LINK;

    EpoXfer xfer = {
        .src_type = EPO_TYPE_FCI, .src_addr = addr,
        .dst_type = link_type, .dst_addr = 0,
        .len = len
    };

    ed_cmd_epo_xfer(&xfer);
    ed_fifo_wr(&ack, 1);

    if (link_type == EPO_TYPE_LINK_ACK) {
        ed_fifo_rd_ack(dst, len);
    } else {
        ed_fifo_rd(dst, len);
    }
}

void ed_fci_wr(u32 addr, void *src, u32 len) {

    u8 ack = 0;
    EpoType link_type = len > 1024 ? EPO_TYPE_LINK_ACK : EPO_TYPE_LINK;

    EpoXfer xfer = {
        .src_type = link_type, .src_addr = 0,
        .dst_type = EPO_TYPE_FCI, .dst_addr = addr,
        .len = len
    };

    ed_cmd_epo_xfer(&xfer);
    ed_fifo_wr(&ack, 1);

    if (link_type == EPO_TYPE_LINK_ACK) {
        ed_fifo_wr_ack(src, len);
    } else {
        ed_fifo_wr(src, len);
    }

    ed_wait_mcu();
}

void ed_fci_crc(u32 addr, u32 len, u32 *crc_base) {

    ed_cmd_tx(CMD_FCI_CRC);
    ed_fifo_wr(&addr, 4);
    ed_fifo_wr(&len, 4);
    ed_fifo_wr(crc_base, 4);
    ed_run_dma();

    ed_fifo_rd(crc_base, 4);
}

void ed_cmd_epo_xfer(EpoXfer *xfer) {

    ed_scmd_tx(CMD_EPO, EPO_SCMD_XFER);
    ed_fifo_wr(xfer, sizeof (EpoXfer));
}
//****************************************************************************** 

void ed_set_config(CartCfg *cfg) {

    u32 addr = ADDR_FCI_CFG + 256 - sizeof (CartCfg);

    if (cfg == 0) {
        ed_fci_set(0, addr, sizeof (CartCfg));
    } else {

        ed_fci_wr(addr, cfg, sizeof (CartCfg));
    }
}

void ed_set_cmode(u16 mode) {

    EDREG->CMODE = mode;
    cmode = mode;
}

u16 ed_bootmod_get() {

    return *(u16 *) ADDR_GBA_BMOD;
}

void ed_bootmod_set(u16 mode) {

    ed_fci_wr(ADDR_FCI_BMOD, &mode, 2);
}

void ed_wait_status(u8 status, u8 ack) {

    EDREG->SYSSTAT = status;

    ed_fifo_wr(&ack, 1);

    while (1) {

        for (int i = 0; i < 128; i++) {
            asm("nop");
        }

        EDREG->CMODE = CMOD_MENU;
        vu8 v1 = EDREG->SYSSTAT;
        vu8 v2 = EDREG->SYSSTAT;


        if ((v1 & 0xf0) != 0xA0)continue;
        if ((v2 & 0xf0) != 0xA0)continue;
        if ((v1 & SYSSTAT_STROBE) == (v2 & SYSSTAT_STROBE))continue;

        if ((v2 & status) == 0)break;
    }
}

u8 ed_boot_status() {
    return ed_cmd_nresp(SUB_STATUS_BOOT);
}

u8 ed_dev_rst_cfg() {
    ed_scmd_tx(CMD_DEV, DEV_SCMD_RST_CFG);
    ed_fifo_flush();
    return ed_check_status();
}

u8 ed_dev_stop() {
    ed_scmd_tx(CMD_DEV, DEV_SCMD_STOP);
    return ed_check_status();
}

u8 ed_dev_start() {

    u8 ack = 0;
    ed_scmd_tx(CMD_DEV, DEV_SCMD_START);
    ed_fifo_wr(&ack, 1);

    return ed_check_status();
}

u8 ed_dev_set_path(u8 *path, DevPathType path_type) {

    u8 type = path_type;

    ed_scmd_tx(CMD_DEV, DEV_SCMD_SET_PATH);
    ed_fifo_wr(&type, 1);
    ed_tx_string(path);

    return ed_check_status();
}

u8 ed_dev_set_cfg(DevCfg *cfg) {

    ed_scmd_tx(CMD_DEV, DEV_SCMD_SET_CFG);
    ed_fifo_wr(cfg, sizeof (DevCfg));
    return ed_check_status();
}

u8 ed_dev_pause() {

    ed_scmd_tx(CMD_DEV, DEV_SCMD_PAUSE);
    return ed_check_status();
}

u8 ed_dev_resume(u8 rst_gpak) {

    ed_scmd_tx(CMD_DEV, DEV_SCMD_RESUME);
    ed_fifo_wr(&rst_gpak, 1);
    return ed_check_status();
}

void ed_dev_han_stat(HanStatus *stat) {

    ed_scmd_tx(CMD_DEV, DEV_SCMD_HAN_STAT);
    ed_fifo_rd(stat, sizeof (HanStatus));
}

u8 ed_dev_han_cpmem() {

    ed_scmd_tx(CMD_DEV, DEV_SCMD_HAN_CPMEM);
    return ed_check_status();
}

void ed_dev_game_ctr() {
    ed_cmd_tx(CMD_GAME_CTR);
}

u8 ed_dev_rom_path(u8 *path, u8 path_type) {

    ed_cmd_tx(CMD_ROM_PATH);
    ed_fifo_wr(&path_type, 1); //0-rom
    ed_rx_string(path);
    return ed_check_status();
}