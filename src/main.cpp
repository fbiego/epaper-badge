#include <Arduino.h>
#include <ChronosESP32.h>
#include "lvgl.h"
#include "ui/ui.h"
#include <Preferences.h>

#define ENABLE_GxEPD2_GFX 0
#include <GxEPD2_BW.h>

GxEPD2_BW<GxEPD2_213_B74, GxEPD2_213_B74::HEIGHT> display(GxEPD2_213_B74(/*CS=5*/ 14, /*DC=*/13, /*RST=*/10, /*BUSY=*/9)); // GDEM0213B74 122x250, SSD1680

#define GxEPD2_DISPLAY_CLASS GxEPD2_BW
#define GxEPD2_DRIVER_CLASS GxEPD2_213_GDEY0213B74 // GDEY0213B74 122x250, SSD1680, (FPC-A002 20.04.08)

#define SCR_WIDTH 256
#define SCR_HEIGHT 128
#define LVBUF ((SCR_WIDTH * SCR_HEIGHT / 8) + 8)

ChronosESP32 watch("Bagde");
Preferences prefs;

static lv_display_t *lvDisplay;
static uint8_t lvBuffer[LVBUF];
uint8_t *lvBuf;

bool qrData; // flags for data

void rotate90(const uint8_t *input, int width, int height)
{
  int newWidth = height;
  int newHeight = width;
  int inputBytesPerRow = width / 8;
  int outputBytesPerRow = newWidth / 8;

  for (int y = 0; y < height; ++y)
  {
    for (int x = 0; x < width; ++x)
    {
      // Calculate the input bit position
      int inputByteIndex = (y * inputBytesPerRow) + (x / 8);
      int inputBit = (input[inputByteIndex] >> (7 - (x % 8))) & 0x01;

      // Calculate the rotated position in the output
      int newX = y;
      int newY = width - x - 1;
      int outputByteIndex = (newY * outputBytesPerRow) + (newX / 8);
      int outputBitPos = 7 - (newX % 8);

      // Set the output bit
      if (inputBit)
      {
        lvBuffer[outputByteIndex] |= (1 << outputBitPos);
      }
      else
      {
        lvBuffer[outputByteIndex] &= ~(1 << outputBitPos);
      }
    }
  }
}

void my_disp_flush(lv_display_t *disp, const lv_area_t *area, unsigned char *data)
{
  int16_t width = area->x2 - area->x1 + 1;
  int16_t height = area->y2 - area->y1 + 1;

  rotate90((uint8_t *)data + 8, width, height);

  // Draw the rotated image
  display.drawImage(lvBuffer, area->y1, area->x1, height, width);

  lv_display_flush_ready(disp);
}

void configCallback(Config config, uint32_t a, uint32_t b)
{
  switch (config)
  {

  case CF_QR:
    // qr links
    if (a == 0){
      // individual qr links (b is the index)
      Serial.print("QR code: ");
      Serial.println(watch.getQrAt(b));
    }
    if (a == 1)
    {
      // end of qr links transmission
      Serial.print("QR Links received. Count: ");
      Serial.println(b);
      qrData = true;
    }
    break;
  }
}

static uint32_t my_tick(void)
{
  return millis();
}

void epd_setup()
{

  SPI.begin(12, -1, 11, 14);
  display.init(115200, true, 2, false);
  if (display.pages() > 1)
  {
    delay(100);
    Serial.print("pages = ");
    Serial.print(display.pages());
    Serial.print(" page height = ");
    Serial.println(display.pageHeight());
    delay(1000);
  }
  // display.clearScreen(); return;
  //  first update should be full refresh
  // display.setRotation(1);
  delay(1000);
}

void setup()
{

  Serial.begin(115200);

  // Initialization settings, executed once when the program starts
  pinMode(7, OUTPUT);    // Set pin 7 to output mode
  digitalWrite(7, HIGH); // Set pin 7 to high level to activate the screen power
  prefs.begin("my-app");

  epd_setup();

  lv_init();

  lv_tick_set_cb(my_tick);

  lvDisplay = lv_display_create(SCR_WIDTH, SCR_HEIGHT);
  lv_display_set_flush_cb(lvDisplay, my_disp_flush);
  lvBuf = (uint8_t *)malloc(LVBUF);
  lv_display_set_buffers(lvDisplay, lvBuf, NULL, LVBUF, LV_DISPLAY_RENDER_MODE_PARTIAL);

  // lv_obj_t *label1 = lv_label_create(lv_scr_act());
  // lv_obj_set_align(label1, LV_ALIGN_CENTER);
  // lv_obj_set_width(label1, SCR_WIDTH - 40);
  // lv_label_set_text(label1, "Hello from Elecrow ePaper, LVGL 9 here!");
  // lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  // lv_obj_set_style_text_font(label1, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);

  // lv_deinit();
  // digitalWrite(7, LOW); // Set pin 7 to low level to deactivate the screen power

  // esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR); // sleep for 30 minutes
  // esp_deep_sleep_start();

  ui_init();

  lv_label_set_text(ui_nameText, prefs.getString("name", "John Doe").c_str());
  lv_label_set_text(ui_titleText, prefs.getString("title", "Software Developer").c_str());
  lv_label_set_text(ui_emailText, prefs.getString("email", "john@doe.com").c_str());
  lv_barcode_update(ui_barCode, prefs.getString("link", "http://google.com").c_str());

  watch.setConfigurationCallback(configCallback);
  watch.begin();
  watch.set24Hour(true);
  watch.setBattery(85);

}

void loop()
{
  delay(100);
  lv_timer_handler(); // Update the UI-
  watch.loop();

  if (qrData){
    qrData = false;
    prefs.putString("name", watch.getQrAt(1).c_str()); // Wechat
    prefs.putString("title", watch.getQrAt(2).c_str()); //Facebook
    prefs.putString("email", watch.getQrAt(3).c_str()); //QQ
    prefs.putString("link", watch.getQrAt(4).c_str());  //Twitter

    ESP.restart();
  }
}