/*
  Example sketch with 3 analogue meters on the left (wider meters) and 3 buttons on the right.

  The analogue meter code is based on Bodmer's HX8357 example.
  In this version the meters are widened (using a scale factor of 1.3333)
  so that they almost fill the entire left column (≈320 pixels wide).

  The right column (320–480) displays 3 equally spaced buttons.
  Each button is clickable; every click for each channel cycles its text through:
    "V"  →  "A"  →  "R"
  (Initially, buttons show "Channel 1", "Channel 2", and "Channel 3".)

  This version uses a cooler color palette for the meters and buttons.
  The meter unit text now updates according to the channel's mode.

  Ensure your TFT_eSPI and XPT2046_Touchscreen libraries are configured correctly.
*/

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>           // Hardware-specific TFT library
#include <XPT2046_Touchscreen.h>  // Touchscreen library

// Define touch controller pins (adjust as needed)
#define TOUCH_CS 16
#define XPT2046_IRQ 7

// -------------------------
// Constants & Global Variables
// -------------------------

// Calibration constants – adjust these based on your touchscreen’s raw coordinate range.
#define TS_MINX 400
#define TS_MAXX 1200
#define TS_MINY 300
#define TS_MAXY 3600



// Use a scale factor to make the meter as wide as possible in a 320–pixel–wide column.
// With meterScale = 1.3333, the background width becomes 1.3333 * 239 ≈ 318 pixels.
const float meterScale = 1.3333f;
const int meterBgWidth  = static_cast<int>(meterScale * 239);  // ≈318 pixels wide
const int meterBgHeight = static_cast<int>(meterScale * 126);   // ≈168 pixels tall
const int NUM_METERS = 3;

// For this layout we assume a 480×320 display in landscape.
const int SCREEN_WIDTH  = 480;
const int SCREEN_HEIGHT = 320;

// The left column (0 to 320) will be used for the analogue meters.
// Since the original meter background is too tall (≈168 pixels) to stack three within 320 pixels,
// we scale the meter vertically to fit into equal slots.
const int meterSlotHeight = SCREEN_HEIGHT / NUM_METERS;  // ≈107 pixels per meter slot
const float vScale = (float)meterSlotHeight / (float)meterBgHeight;  // Vertical scale factor

// Global state arrays for each meter's needle
int old_analog[NUM_METERS];   // Last displayed value for each meter
float ltxArr[NUM_METERS];     // Saved x–factor for needle erasure per meter
uint16_t osxArr[NUM_METERS], osyArr[NUM_METERS]; // Saved needle end coordinates per meter

// Define columns for layout
const int leftColumnWidth = 320;                  // Meters occupy 0–320
const int rightColumnWidth = SCREEN_WIDTH - leftColumnWidth;  // Buttons occupy 320–480

// Instantiate TFT and touchscreen objects
TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen ts(TOUCH_CS, XPT2046_IRQ);

// For test purposes, a variable to drive sine–wave test data for the meters.
static int d = 0;

// -------------------------
// Button state variables
// -------------------------
// Each channel button initially shows its channel name. When clicked, it cycles through three modes.
// Default: –1 means show "%RH" in the meter and "Channel X" on the button.
// After clicks, the unit cycles: 0 → "V", 1 → "A", 2 → "R"
int channelMode[NUM_METERS] = { 0, 0, 0 };
const char* modeLabels[3] = {"V", "A", "R"};


// -------------------------
// Function Prototypes
// -------------------------
void analogMeter(int offsetY, int meterIndex);
void plotNeedle(int offsetY, int meterIndex, int value, byte ms_delay);
void drawButtons();
void checkButtons();

// -------------------------
// Setup
// -------------------------
void setup() {
  tft.init();
  tft.setRotation(1); // Landscape mode
  tft.fillScreen(TFT_BLACK);  // Fill entire screen with black
  Serial.begin(57600); // For debugging

  ts.begin();
  ts.setRotation(1);  // Set touch rotation if needed

  // Initialize each meter's state and draw its background in its vertical slot.
  for (int i = 0; i < NUM_METERS; i++) {
    old_analog[i] = -999;  // Force initial needle draw
    ltxArr[i] = 0;
    analogMeter(i * meterSlotHeight, i);
  }

  // Draw the buttons in the right column.
  drawButtons();
}

// -------------------------
// Main Loop
// -------------------------
void loop() {
  // Update test values using sine waves with phase offsets.
  d += 4;
  if (d >= 360) d = 0;
  int meterVal[NUM_METERS];
  meterVal[0] = 50 + 50 * sin((d + 0) * 0.0174532925);
  meterVal[1] = 50 + 50 * sin((d + 120) * 0.0174532925);
  meterVal[2] = 50 + 50 * sin((d + 240) * 0.0174532925);

  // Update each meter's needle.
  for (int i = 0; i < NUM_METERS; i++) {
    int offsetY = i * meterSlotHeight;
    plotNeedle(offsetY, i, meterVal[i], 0);
  }

  // Check for touches in the button area.
  checkButtons();

  delay(35);
}

// -------------------------
// Draw an analogue meter background in the left column.
// offsetY: vertical offset for this meter's slot.
// meterIndex: index (0,1,2) to keep separate state.
// The drawing is scaled horizontally by meterScale and vertically by meterScale*vScale.
void analogMeter(int offsetY, int meterIndex) {
    int bgWidth = static_cast<int>(meterScale * 239);
    int bgHeight = static_cast<int>(meterScale * 126 * vScale);
    // Outer background: use a cool dark blue (NAVY)
    tft.fillRect(0, offsetY, bgWidth, bgHeight, TFT_NAVY);
    // Inner dial: use dark grey.
    tft.fillRect(5, offsetY + 3, static_cast<int>(meterScale * 230), static_cast<int>(meterScale * 119 * vScale), TFT_DARKGREY);
    tft.setTextColor(TFT_WHITE);

    // Draw ticks and labels. (Coordinates are scaled.)
    for (int i = -50; i < 51; i += 5) {
      int tl = 15; // Long tick length
      float sx = cos((i - 90) * 0.0174532925);
      float sy = sin((i - 90) * 0.0174532925);
      uint16_t x0 = static_cast<uint16_t>(sx * (meterScale * 100 + tl) + meterScale * 120);
      uint16_t y0 = static_cast<uint16_t>(sy * (meterScale * 100 + tl) * vScale + meterScale * 140 * vScale + offsetY);
      uint16_t x1 = static_cast<uint16_t>(sx * (meterScale * 100) + meterScale * 120);
      uint16_t y1 = static_cast<uint16_t>(sy * (meterScale * 100) * vScale + meterScale * 140 * vScale + offsetY);

      float sx2 = cos((i + 5 - 90) * 0.0174532925);
      float sy2 = sin((i + 5 - 90) * 0.0174532925);
      int x2 = static_cast<int>(sx2 * (meterScale * 100 + tl) + meterScale * 120);
      int y2 = static_cast<int>(sy2 * (meterScale * 100 + tl) * vScale + meterScale * 140 * vScale + offsetY);
      int x3 = static_cast<int>(sx2 * (meterScale * 100) + meterScale * 120);
      int y3 = static_cast<int>(sy2 * (meterScale * 100) * vScale + meterScale * 140 * vScale + offsetY);

      // Fill lower tick zone (0° to 25°) with a cool cyan.
      if (i >= 0 && i < 25) {
        tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_CYAN);
        tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_CYAN);
      }
      // Fill upper tick zone (25° to 50°) with a deep blue.
      if (i >= 25 && i < 50) {
        tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_BLUE);
        tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_BLUE);
      }

      if (i % 25 != 0) tl = 8;  // Shorter tick for non–label ticks

      // Recalculate tick coordinates.
      x0 = static_cast<uint16_t>(sx * (meterScale * 100 + tl) + meterScale * 120);
      y0 = static_cast<uint16_t>(sy * (meterScale * 100 + tl) * vScale + meterScale * 140 * vScale + offsetY);
      x1 = static_cast<uint16_t>(sx * (meterScale * 100) + meterScale * 120);
      y1 = static_cast<uint16_t>(sy * (meterScale * 100) * vScale + meterScale * 140 * vScale + offsetY);
      tft.drawLine(x0, y0, x1, y1, TFT_WHITE);

      // Draw labels at every 25° tick.
      if (i % 25 == 0) {
        x0 = static_cast<uint16_t>(sx * (meterScale * 100 + tl + 10) + meterScale * 120);
        y0 = static_cast<uint16_t>(sy * (meterScale * 100 + tl + 10) * vScale + meterScale * 140 * vScale + offsetY);
        switch (i / 25) {
          case -2: tft.drawCentreString("0", x0, y0 - 12, 2); break;
          case -1: tft.drawCentreString("25", x0, y0 - 9, 2); break;
          case 0:  tft.drawCentreString("50", x0, y0 - 7, 2); break;
          case 1:  tft.drawCentreString("75", x0, y0 - 9, 2); break;
          case 2:  tft.drawCentreString("100", x0, y0 - 12, 2); break;
        }
      }
      sx = cos((i + 5 - 90) * 0.0174532925);
      sy = sin((i + 5 - 90) * 0.0174532925);
      x0 = static_cast<uint16_t>(sx * (meterScale * 100) + meterScale * 120);
      y0 = static_cast<uint16_t>(sy * (meterScale * 100) * vScale + meterScale * 140 * vScale + offsetY);
      if (i < 50) tft.drawLine(x0, y0, x1, y1, TFT_WHITE);
    }
    // Draw unit labels using the current mode letter ("V", "A", or "R").
    tft.drawString(modeLabels[channelMode[meterIndex]], static_cast<int>(meterScale * (5 + 230 - 40)),
                   static_cast<int>(offsetY + meterScale * (119 - 20) * vScale), 2);
    tft.drawCentreString(modeLabels[channelMode[meterIndex]], static_cast<int>(meterScale * 120),
                         static_cast<int>(offsetY + meterScale * 70 * vScale), 4);

    tft.drawRect(5, offsetY + 3, static_cast<int>(meterScale * 230),
                 static_cast<int>(meterScale * 119 * vScale), TFT_WHITE);

    // Initially draw the needle at 0.
    plotNeedle(offsetY, meterIndex, 0, 0);
  }

// -------------------------
// Update and draw the needle for a given meter.
// offsetY: vertical offset for this meter's slot.
// meterIndex: index (0,1,2) for separate state.
// value: new value to display (typically 0 to 100).
// ms_delay: delay between increments (0 for instant movement).
void plotNeedle(int offsetY, int meterIndex, int value, byte ms_delay) {
    // Draw the numeric value using white text on a dark blue background.
    tft.setTextColor(TFT_WHITE, TFT_NAVY);
    char buf[8];
    dtostrf(value, 4, 0, buf);
    tft.drawRightString(buf, static_cast<int>(meterScale * 40),
                        static_cast<int>(offsetY + meterScale * (119 - 20) * vScale), 2);

    if (value < -10) value = -10;
    if (value > 110) value = 110;

    while (old_analog[meterIndex] != value) {
      if (old_analog[meterIndex] < value) old_analog[meterIndex]++;
      else old_analog[meterIndex]--;
      if (ms_delay == 0) old_analog[meterIndex] = value;

      float sdeg = map(old_analog[meterIndex], -10, 110, -150, -30);
      float sx = cos(sdeg * 0.0174532925);
      float sy = sin(sdeg * 0.0174532925);
      float tx = tan((sdeg + 90) * 0.0174532925);

      // Erase old needle (draw over with dial background, using TFT_DARKGREY)
      tft.drawLine(static_cast<int>(meterScale * (120 + 20 * ltxArr[meterIndex] - 1)),
                   static_cast<int>(offsetY + meterScale * (140 - 20) * vScale),
                   osxArr[meterIndex] - 1, osyArr[meterIndex], TFT_DARKGREY);
      tft.drawLine(static_cast<int>(meterScale * (120 + 20 * ltxArr[meterIndex])),
                   static_cast<int>(offsetY + meterScale * (140 - 20) * vScale),
                   osxArr[meterIndex], osyArr[meterIndex], TFT_DARKGREY);
      tft.drawLine(static_cast<int>(meterScale * (120 + 20 * ltxArr[meterIndex] + 1)),
                   static_cast<int>(offsetY + meterScale * (140 - 20) * vScale),
                   osxArr[meterIndex] + 1, osyArr[meterIndex], TFT_DARKGREY);

      // --- Erase the previous unit text ---
      int unitWidth  = 80;
      int unitHeight = 30;
      int unitX = static_cast<int>(meterScale * 120) - unitWidth / 2;
      int unitY = static_cast<int>(offsetY + meterScale * 70 * vScale) - unitHeight / 2;
      tft.fillRect(unitX, unitY, unitWidth, unitHeight, TFT_DARKGREY);

      // Redraw the unit text using the current mode letter.
      tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
      tft.drawCentreString(modeLabels[channelMode[meterIndex]], static_cast<int>(meterScale * 120),
                             static_cast<int>(offsetY + meterScale * 70 * vScale), 4);

      // Save the current needle's coordinates for erasure next time.
      ltxArr[meterIndex] = tx;
      osxArr[meterIndex] = static_cast<uint16_t>(meterScale * (sx * 98 + 120));
      osyArr[meterIndex] = static_cast<uint16_t>(offsetY + meterScale * (sy * 98 + 140) * vScale);

      // Draw the new needle with cooler colors: core in TFT_CYAN and outline in TFT_MAGENTA.
      tft.drawLine(static_cast<int>(meterScale * (120 + 20 * ltxArr[meterIndex] - 1)),
                   static_cast<int>(offsetY + meterScale * (140 - 20) * vScale),
                   osxArr[meterIndex] - 1, osyArr[meterIndex], TFT_CYAN);
      tft.drawLine(static_cast<int>(meterScale * (120 + 20 * ltxArr[meterIndex])),
                   static_cast<int>(offsetY + meterScale * (140 - 20) * vScale),
                   osxArr[meterIndex], osyArr[meterIndex], TFT_MAGENTA);
      tft.drawLine(static_cast<int>(meterScale * (120 + 20 * ltxArr[meterIndex] + 1)),
                   static_cast<int>(offsetY + meterScale * (140 - 20) * vScale),
                   osxArr[meterIndex] + 1, osyArr[meterIndex], TFT_CYAN);

      if (abs(old_analog[meterIndex] - value) < 10) ms_delay += ms_delay / 5;
      delay(ms_delay);
    }
}

// -------------------------
// Draw 3 equally spaced buttons in the right column.
// -------------------------
void drawButtons() {
    int buttonAreaX = leftColumnWidth; // Buttons start at x = 320
    int buttonAreaWidth = rightColumnWidth; // 160 pixels wide
    int slotHeight = SCREEN_HEIGHT / 3;  // ≈107 pixels per slot
    int buttonHeight = 80;
    int verticalMargin = (slotHeight - buttonHeight) / 2;

    for (int i = 0; i < 3; i++) {
      int btnX = buttonAreaX + 10;
      int btnY = i * slotHeight + verticalMargin;
      int btnWidth = buttonAreaWidth - 20;
      int btnHeight = buttonHeight;
      // Use TFT_NAVY as the default button background for high contrast.
      tft.fillRect(btnX, btnY, btnWidth, btnHeight, TFT_NAVY);
      tft.drawRect(btnX, btnY, btnWidth, btnHeight, TFT_WHITE);

      char label[20];
      sprintf(label, "%s", modeLabels[channelMode[i]]);
      tft.drawCentreString(label, btnX + btnWidth / 2, btnY + btnHeight / 2 - 8, 2);
    }
  }

// Calibration constants – adjust these based on your touchscreen’s raw coordinate range.
#define TS_MINX 400
#define TS_MAXX 1200
#define TS_MINY 300
#define TS_MAXY 3600

// -------------------------
// Check for touches in the right column and update button states.
// -------------------------
void checkButtons() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();

    // Print raw touch coordinates.
    Serial.print("Raw Touch coordinates: x = ");
    Serial.print(p.x);
    Serial.print(", y = ");
    Serial.println(p.y);

    // Map raw touch coordinates to display coordinates.
    int mappedX = map(p.x, TS_MINX, TS_MAXX, 0, SCREEN_WIDTH);
    // Invert Y axis mapping so that higher raw values map to lower Y coordinates.
    int mappedY = map(p.y, TS_MINY, TS_MAXY, SCREEN_HEIGHT, 0);

    Serial.print("Mapped Touch coordinates: x = ");
    Serial.print(mappedX);
    Serial.print(", y = ");
    Serial.println(mappedY);

    // Only process touches in the right column (where the buttons are drawn).
    if (mappedX >= leftColumnWidth) {
      int slotHeight = SCREEN_HEIGHT / 3;
      int verticalMargin = (slotHeight - 80) / 2;
      for (int i = 0; i < 3; i++) {
        int btnX = leftColumnWidth + 10;
        int btnY = i * slotHeight + verticalMargin;
        int btnWidth = rightColumnWidth - 20;
        int btnHeight = 80;

        // Check if the mapped touch coordinate falls inside this button's area.
        if (mappedX >= btnX && mappedX <= (btnX + btnWidth) &&
            mappedY >= btnY && mappedY <= (btnY + btnHeight)) {
          Serial.print("Button ");
          Serial.print(i + 1);
          Serial.println(" pressed");

          // Cycle through the modes: V -> A -> R -> V ...
          channelMode[i] = (channelMode[i] + 1) % 3;

          // Highlight the pressed button using TFT_PURPLE.
          tft.fillRect(btnX, btnY, btnWidth, btnHeight, TFT_PURPLE);
          tft.drawRect(btnX, btnY, btnWidth, btnHeight, TFT_WHITE);
          char label[20];
          sprintf(label, "%s", modeLabels[channelMode[i]]);
          tft.drawCentreString(label, btnX + btnWidth / 2, btnY + btnHeight / 2 - 8, 2);

          drawButtons();  // Redraw buttons in their normal state.
          break; // Exit after processing the first matching button.
        }
      }
    }
    delay(100);
  }
}
