#include <string.h>

#include "hardware/clocks.h"
#include "pico/bootrom.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pico/sync.h"
#include "pio_usb.h"
#include "tusb.h"

#include "wup_protocol.h"

#define HOST_DP_PIN 2
#define LED_PIN 25

static critical_section_t g_report_lock;
static uint8_t g_latest_report[64];
static uint16_t g_latest_report_len;
static volatile bool g_report_pending;
static volatile bool g_adapter_mounted;

static void host_report_cb(const uint8_t* data, uint16_t len) {
  if (len > sizeof(g_latest_report)) {
    len = sizeof(g_latest_report);
  }

  critical_section_enter_blocking(&g_report_lock);
  memcpy(g_latest_report, data, len);
  g_latest_report_len = len;
  g_report_pending = true;
  critical_section_exit(&g_report_lock);
}

static void host_mount_cb(bool mounted) {
  g_adapter_mounted = mounted;
  gpio_put(LED_PIN, mounted);
}

static void core1_main(void) {
  sleep_ms(10);

  pio_usb_configuration_t pio_cfg = PIO_USB_DEFAULT_CONFIG;
  pio_cfg.pin_dp = HOST_DP_PIN;
  pio_cfg.pinout = PIO_USB_PINOUT_DPDM;
  tuh_configure(1, TUH_CFGID_RPI_PIO_USB_CONFIGURATION, &pio_cfg);
  tuh_init(1);

  while (true) {
    tuh_task();
  }
}

static void forward_pc_out_to_adapter(void) {
  uint8_t out[64];
  while (tud_vendor_available()) {
    uint32_t count = tud_vendor_read(out, sizeof(out));
    if (count > 0 && g_adapter_mounted) {
      (void)wup_host_send(out, (uint16_t)count);
    }
  }
}

static void forward_adapter_in_to_pc(void) {
  uint8_t report[64];
  uint16_t report_len = 0;

  critical_section_enter_blocking(&g_report_lock);
  if (g_report_pending) {
    report_len = g_latest_report_len;
    memcpy(report, g_latest_report, report_len);
    g_report_pending = false;
  }
  critical_section_exit(&g_report_lock);

  if (report_len > 0 && tud_vendor_mounted() && tud_vendor_write_available() >= report_len) {
    tud_vendor_write(report, report_len);
    tud_vendor_write_flush();
  }
}

int main(void) {
  set_sys_clock_khz(120000, true);
  stdio_init_all();

  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 0);

  critical_section_init(&g_report_lock);
  wup_host_set_callbacks(host_report_cb, host_mount_cb);

  multicore_reset_core1();
  multicore_launch_core1(core1_main);

  tud_init(0);

  while (true) {
    tud_task();
    forward_pc_out_to_adapter();
    forward_adapter_in_to_pc();

    if (tud_vendor_mounted() && !g_adapter_mounted) {
      uint8_t idle_report[WUP_PACKET_SIZE] = {0};
      idle_report[0] = WUP_REPORT_TAG;
      tud_vendor_write(idle_report, sizeof(idle_report));
      tud_vendor_write_flush();
      sleep_ms(1);
    }
  }
}

void tud_vendor_rx_cb(uint8_t itf, uint8_t const* buffer, uint16_t bufsize) {
  (void)itf;
  if (bufsize > 0 && g_adapter_mounted) {
    (void)wup_host_send(buffer, bufsize);
  }
}

void tud_mount_cb(void) {
  uint8_t start = WUP_CMD_START;
  (void)wup_host_send(&start, 1);
}

void tud_umount_cb(void) {
}
