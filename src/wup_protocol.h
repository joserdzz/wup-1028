#ifndef WUP_PROTOCOL_H
#define WUP_PROTOCOL_H

#include <stdbool.h>
#include <stdint.h>

#define WUP_VID 0x057e
#define WUP_PID 0x0337

#define WUP_EP_OUT 0x02
#define WUP_EP_IN 0x81
#define WUP_PACKET_SIZE 37
#define WUP_USB_POLL_INTERVAL_MS 1

#define WUP_CMD_START 0x13
#define WUP_CMD_RUMBLE 0x11
#define WUP_REPORT_TAG 0x21

typedef void (*wup_host_report_cb_t)(const uint8_t* data, uint16_t len);
typedef void (*wup_host_mount_cb_t)(bool mounted);

void wup_host_set_callbacks(wup_host_report_cb_t report_cb, wup_host_mount_cb_t mount_cb);
bool wup_host_send(const uint8_t* data, uint16_t len);
bool wup_host_mounted(void);

#endif
