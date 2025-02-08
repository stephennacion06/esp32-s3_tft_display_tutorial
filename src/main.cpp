#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Define your chip-select and IRQ pins for the touchscreen


// Initialize touchscreen and TFT objects
XPT2046_Touchscreen touch(TOUCH_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

// Calibration constants (adjust these based on your measurements)
const int TS_MINX = 300;
const int TS_MAXX = 3800;
const int TS_MINY = 300;
const int TS_MAXY = 3800;

void setup() {
  Serial.begin(115200);

  // Initialize TFT display
  tft.init();
  tft.setRotation(1);  // Adjust based on your display orientation
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);

  // Display a welcome message
  tft.setCursor(50, 50);
  tft.println("Hello, ESP32-S3!");

  // Draw a button for touch testing
  tft.fillRoundRect(100, 200, 120, 50, 10, TFT_BLUE);
  tft.setCursor(120, 220);
  tft.setTextColor(TFT_WHITE);
  tft.println("Touch Me");

  // Initialize the touchscreen
  touch.begin();
}

void loop() {
  if (touch.touched()) {
    TS_Point p = touch.getPoint();  // Get raw touch data

    // If the touch is inverse, invert the mapping.
    // Here we invert both axes. Adjust if only one axis is reversed.
    int x = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
    int y = map(p.y, TS_MINY, TS_MAXY, tft.height(), 0);

    Serial.printf("Mapped touch at: X = %d, Y = %d\n", x, y);

    // Check if the touch is within the drawn button area
    if (x > 100 && x < 220 && y > 200 && y < 250) {
      tft.fillRoundRect(100, 200, 120, 50, 10, TFT_GREEN);
      tft.setCursor(120, 220);
      tft.setTextColor(TFT_BLACK);
      tft.println("Pressed!");
      delay(500);
      tft.fillRoundRect(100, 200, 120, 50, 10, TFT_BLUE);
      tft.setCursor(120, 220);
      tft.setTextColor(TFT_WHITE);
      tft.println("Touch Me");
    }
  }
}
