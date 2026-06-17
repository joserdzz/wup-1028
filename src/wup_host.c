#include <string.h>

#include "tusb.h"
#include "host/usbh_pvt.h"
#include "wup_protocol.h"

typedef struct {
  uint8_t daddr;
  uint8_t itf_num;
  uint8_t ep_in;
  uint8_t ep_out;
  bool mounted;
  bool in_busy;
  bool out_busy;
  CFG_TUSB_MEM_ALIGN uint8_t in_buf[64];
  CFG_TUSB_MEM_ALIGN uint8_t out_buf[64];
} wup_host_t;

static wup_host_t g_wup;
static wup_host_report_cb_t g_report_cb;
static wup_host_mount_cb_t g_mount_cb;

static bool wup_submit_in(void);

void wup_host_set_callbacks(wup_host_report_cb_t report_cb, wup_host_mount_cb_t mount_cb) {
  g_report_cb = report_cb;
  g_mount_cb = mount_cb;
}

bool wup_host_mounted(void) {
  return g_wup.mounted;
}

bool wup_host_send(const uint8_t* data, uint16_t len) {
  if (!g_wup.mounted || g_wup.out_busy || len == 0 || len > sizeof(g_wup.out_buf)) {
    return false;
  }

  memcpy(g_wup.out_buf, data, len);
  g_wup.out_busy = true;
  if (!usbh_edpt_xfer(g_wup.daddr, g_wup.ep_out, g_wup.out_buf, len)) {
    g_wup.out_busy = false;
    return false;
  }

  return true;
}

static bool wup_host_init(void) {
  memset(&g_wup, 0, sizeof(g_wup));
  return true;
}

static bool wup_host_deinit(void) {
  memset(&g_wup, 0, sizeof(g_wup));
  return true;
}

static bool wup_host_open(uint8_t rhport, uint8_t dev_addr,
                          tusb_desc_interface_t const* itf_desc, uint16_t max_len) {
  (void)rhport;

  if (itf_desc->bInterfaceClass != TUSB_CLASS_VENDOR_SPECIFIC || itf_desc->bNumEndpoints < 2) {
    return false;
  }

  uint16_t vid = 0;
  uint16_t pid = 0;
  if (!tuh_vid_pid_get(dev_addr, &vid, &pid) || vid != WUP_VID || pid != WUP_PID) {
    return false;
  }

  uint8_t ep_in = 0;
  uint8_t ep_out = 0;
  uint8_t found = 0;
  uint8_t const* p_desc = tu_desc_next(itf_desc);
  uint8_t const* desc_end = ((uint8_t const*)itf_desc) + max_len;

  while (p_desc < desc_end && found < itf_desc->bNumEndpoints) {
    if (tu_desc_type(p_desc) == TUSB_DESC_ENDPOINT) {
      tusb_desc_endpoint_t const* ep = (tusb_desc_endpoint_t const*)p_desc;
      if ((ep->bmAttributes.xfer == TUSB_XFER_INTERRUPT || ep->bmAttributes.xfer == TUSB_XFER_BULK) &&
          tu_edpt_packet_size(ep) <= 64 && tuh_edpt_open(dev_addr, ep)) {
        if (tu_edpt_dir(ep->bEndpointAddress) == TUSB_DIR_IN) {
          ep_in = ep->bEndpointAddress;
        } else {
          ep_out = ep->bEndpointAddress;
        }
        found++;
      }
    }
    p_desc = tu_desc_next(p_desc);
  }

  if (!ep_in || !ep_out) {
    return false;
  }

  memset(&g_wup, 0, sizeof(g_wup));
  g_wup.daddr = dev_addr;
  g_wup.itf_num = itf_desc->bInterfaceNumber;
  g_wup.ep_in = ep_in;
  g_wup.ep_out = ep_out;
  return true;
}

static bool wup_host_set_config(uint8_t dev_addr, uint8_t itf_num) {
  if (dev_addr != g_wup.daddr || itf_num != g_wup.itf_num) {
    return false;
  }

  g_wup.mounted = true;
  if (g_mount_cb) {
    g_mount_cb(true);
  }

  uint8_t start = WUP_CMD_START;
  (void)wup_host_send(&start, 1);
  (void)wup_submit_in();

  usbh_driver_set_config_complete(dev_addr, itf_num);
  return true;
}

static bool wup_submit_in(void) {
  if (!g_wup.mounted || g_wup.in_busy) {
    return false;
  }

  g_wup.in_busy = true;
  if (!usbh_edpt_xfer(g_wup.daddr, g_wup.ep_in, g_wup.in_buf, sizeof(g_wup.in_buf))) {
    g_wup.in_busy = false;
    return false;
  }

  return true;
}

static bool wup_host_xfer_cb(uint8_t dev_addr, uint8_t ep_addr, xfer_result_t result,
                             uint32_t xferred_bytes) {
  if (dev_addr != g_wup.daddr) {
    return false;
  }

  if (ep_addr == g_wup.ep_out) {
    g_wup.out_busy = false;
    return true;
  }

  if (ep_addr == g_wup.ep_in) {
    g_wup.in_busy = false;
    if (result == XFER_RESULT_SUCCESS && xferred_bytes > 0 && g_report_cb) {
      g_report_cb(g_wup.in_buf, (uint16_t)xferred_bytes);
    }
    (void)wup_submit_in();
    return true;
  }

  return false;
}

static void wup_host_close(uint8_t dev_addr) {
  if (dev_addr == g_wup.daddr) {
    if (g_mount_cb && g_wup.mounted) {
      g_mount_cb(false);
    }
    memset(&g_wup, 0, sizeof(g_wup));
  }
}

static usbh_class_driver_t const g_wup_driver[] = {{
    .name = "WUP028",
    .init = wup_host_init,
    .deinit = wup_host_deinit,
    .open = wup_host_open,
    .set_config = wup_host_set_config,
    .xfer_cb = wup_host_xfer_cb,
    .close = wup_host_close,
}};

usbh_class_driver_t const* usbh_app_driver_get_cb(uint8_t* driver_count) {
  *driver_count = 1;
  return g_wup_driver;
}
