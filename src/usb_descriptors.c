#include "tusb.h"

#include "wup_protocol.h"

enum {
  ITF_NUM_VENDOR = 0,
  ITF_NUM_TOTAL
};

enum {
  STRID_LANGID = 0,
  STRID_MANUFACTURER,
  STRID_PRODUCT,
  STRID_SERIAL,
};

tusb_desc_device_t const desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = WUP_VID,
    .idProduct = WUP_PID,
    .bcdDevice = 0x0100,
    .iManufacturer = STRID_MANUFACTURER,
    .iProduct = STRID_PRODUCT,
    .iSerialNumber = STRID_SERIAL,
    .bNumConfigurations = 0x01,
};

uint8_t const* tud_descriptor_device_cb(void) {
  return (uint8_t const*)&desc_device;
}

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + 9 + 7 + 7)

uint8_t const desc_fs_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP,
                          250),

    9, TUSB_DESC_INTERFACE, ITF_NUM_VENDOR, 0, 2, TUSB_CLASS_VENDOR_SPECIFIC, 0x00, 0x00, 0,

    7, TUSB_DESC_ENDPOINT, WUP_EP_OUT, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(WUP_PACKET_SIZE),
    WUP_USB_POLL_INTERVAL_MS,

    7, TUSB_DESC_ENDPOINT, WUP_EP_IN, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(WUP_PACKET_SIZE),
    WUP_USB_POLL_INTERVAL_MS,
};

uint8_t const* tud_descriptor_configuration_cb(uint8_t index) {
  (void)index;
  return desc_fs_configuration;
}

static char const* const string_desc_arr[] = {
    (const char[]){0x09, 0x04},
    "Nintendo Co., Ltd.",
    "WUP-028",
    "00000001",
};

static uint16_t _desc_str[32];

uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
  (void)langid;

  uint8_t chr_count;
  if (index == STRID_LANGID) {
    _desc_str[1] = 0x0409;
    chr_count = 1;
  } else {
    if (index >= sizeof(string_desc_arr) / sizeof(string_desc_arr[0])) {
      return NULL;
    }

    const char* str = string_desc_arr[index];
    chr_count = (uint8_t)strlen(str);
    if (chr_count > 31) {
      chr_count = 31;
    }

    for (uint8_t i = 0; i < chr_count; i++) {
      _desc_str[1 + i] = str[i];
    }
  }

  _desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));
  return _desc_str;
}
