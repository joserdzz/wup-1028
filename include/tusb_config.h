#ifndef WUP_1028_TUSB_CONFIG_H
#define WUP_1028_TUSB_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#define CFG_TUSB_OS OPT_OS_PICO

#define CFG_TUD_ENABLED 1
#define CFG_TUH_ENABLED 1
#define CFG_TUH_RPI_PIO_USB 1

#ifndef CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_SECTION
#endif

#ifndef CFG_TUSB_MEM_ALIGN
#define CFG_TUSB_MEM_ALIGN __attribute__((aligned(4)))
#endif

#define CFG_TUD_ENDPOINT0_SIZE 64
#define CFG_TUD_VENDOR 1
#define CFG_TUD_VENDOR_RX_BUFSIZE 64
#define CFG_TUD_VENDOR_TX_BUFSIZE 64
#define CFG_TUD_VENDOR_EPSIZE 64

#define CFG_TUH_ENUMERATION_BUFSIZE 256
#define CFG_TUH_HUB 0
#define CFG_TUH_DEVICE_MAX 1
#define CFG_TUH_ENDPOINT_MAX 4
#define CFG_TUH_API_EDPT_XFER 1

#ifdef __cplusplus
}
#endif

#endif
