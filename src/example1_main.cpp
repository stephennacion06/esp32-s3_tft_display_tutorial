/*
  Example sketch for TFT_eSPI library with XPT2046 Touch
  Draws a 3D cube on the TFT screen that you can drag by touching.

  Original cube code from:
  http://forum.freetronics.com/viewtopic.php?f=37&t=5495
*/

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>         // Hardware-specific TFT library
#include <XPT2046_Touchscreen.h>  // Touchscreen library

// Optionally define colors if not already defined by TFT_eSPI
#ifndef TFT_BLACK
  #define TFT_BLACK 0x0000
#endif
#ifndef TFT_WHITE
  #define TFT_WHITE 0xFFFF
#endif

// Define touch controller pins (adjust as needed)
#define TOUCH_CS 16
#define XPT2046_IRQ 7

// Instantiate display and touch objects
TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen ts(TOUCH_CS, XPT2046_IRQ);

// Global variables for cube and touch handling
int16_t h, w;
int inc = -2;

float xx, xy, xz;
float yx, yy, yz;
float zx, zy, zz;
float fact;

int Xan = 0, Yan = 0;  // Rotation angles
int Xoff, Yoff, Zoff;  // Projection offsets

// Variables to track touch dragging
bool touchActive = false;
int lastTouchX = 0;
int lastTouchY = 0;

// 3D and 2D point and line structures
struct Point3d {
  int x;
  int y;
  int z;
};

struct Point2d {
  int x;
  int y;
};

struct Line3d {
  Point3d p0;
  Point3d p1;
};

struct Line2d {
  Point2d p0;
  Point2d p1;
};

int LinestoRender;     // Number of lines to render
int OldLinestoRender;  // Previously rendered line count

Line3d Lines[20];
Line2d Render[20];
Line2d ORender[20];

// Function prototypes
void cube();
void SetVars();
void ProcessLine(Line2d *ret, Line3d vec);
void RenderImage();

void setup() {
  // Initialize display
  tft.init();
  h = tft.height();
  w = tft.width();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // Initialize touchscreen
  ts.begin();
  // Optionally, adjust TS calibration here if needed.

  cube();  // Build the cube geometry

  fact = 180.0 / 3.14159259;  // Degrees to radians conversion factor

  // Center the 3D space in the TFT screen and set initial Z offset.
  Xoff = 240;
  Yoff = 160;
  Zoff = 550;
}

void loop() {
  // If the screen is touched, adjust rotation angles based on drag
  if (ts.touched()) {
    // Get touch coordinates. (Depending on the library, you may need to call getPoint())
    TS_Point p = ts.getPoint();
    // The raw touch coordinates might need mapping; adjust factor if necessary.
    int touchX = p.x;
    int touchY = p.y;

    if (!touchActive) {
      // First touch: store the initial touch position
      lastTouchX = touchX;
      lastTouchY = touchY;
      touchActive = true;
    } else {
      // Compute difference from last touch point and update angles
      int dx = touchX - lastTouchX;
      int dy = touchY - lastTouchY;

      // Adjust sensitivity as needed (here 0.5 is an arbitrary factor)
      Xan += dx * 0.5;
      Yan += dy * 0.5;

      lastTouchX = touchX;
      lastTouchY = touchY;
    }
  } else {
    // No touch detected: reset touch flag and auto-rotate the cube
    touchActive = false;
    Xan = (Xan + 1) % 360;
    Yan = (Yan + 1) % 360;
  }

  SetVars();  // Update transformation parameters

  // Zoom in and out on the Z axis within limits.
  Zoff += inc;
  if (Zoff > 500) {
    inc = -1; // Switch to zoom in
  } else if (Zoff < 160) {
    inc = 1;  // Switch to zoom out
  }

  // Process each line: store the old projection and compute the new projection.
  for (int i = 0; i < LinestoRender; i++) {
    ORender[i] = Render[i];
    ProcessLine(&Render[i], Lines[i]);
  }

  RenderImage();  // Draw the cube

  delay(14);  // Delay to reduce flicker
}

void RenderImage() {
  // Erase old lines by redrawing them in black.
  for (int i = 0; i < OldLinestoRender; i++) {
    tft.drawLine(ORender[i].p0.x, ORender[i].p0.y,
                 ORender[i].p1.x, ORender[i].p1.y, TFT_BLACK);
  }

  // Draw new lines in color.
  for (int i = 0; i < LinestoRender; i++) {
    uint16_t color = TFT_BLUE;
    if (i < 4)
      color = TFT_RED;
    else if (i > 7)
      color = TFT_GREEN;
    tft.drawLine(Render[i].p0.x, Render[i].p0.y,
                 Render[i].p1.x, Render[i].p1.y, color);
  }

  OldLinestoRender = LinestoRender;
}

void SetVars() {
  // Convert rotation angles to radians.
  float Xan2 = Xan / fact;
  float Yan2 = Yan / fact;

  // Compute sine and cosine for the current angles.
  float s1 = sin(Yan2);
  float s2 = sin(Xan2);
  float c1 = cos(Yan2);
  float c2 = cos(Xan2);

  // Set up the transformation matrix components.
  xx = c1;
  xy = 0;
  xz = -s1;

  yx = s1 * s2;
  yy = c2;
  yz = c1 * s2;

  zx = s1 * c2;
  zy = -s2;
  zz = c1 * c2;
}

void ProcessLine(Line2d *ret, Line3d vec) {
  // Process a 3D line and compute its 2D projection.
  int x1 = vec.p0.x, y1 = vec.p0.y, z1 = vec.p0.z;
  int x2 = vec.p1.x, y2 = vec.p1.y, z2 = vec.p1.z;

  int xv1 = (x1 * xx) + (y1 * xy) + (z1 * xz);
  int yv1 = (x1 * yx) + (y1 * yy) + (z1 * yz);
  int zv1 = (x1 * zx) + (y1 * zy) + (z1 * zz);

  float zvt1 = zv1 - Zoff;
  int rx1 = 0, ry1 = 0;
  int Ok = 0;
  if (zvt1 < -5) {
    rx1 = 256 * (xv1 / zvt1) + Xoff;
    ry1 = 256 * (yv1 / zvt1) + Yoff;
    Ok = 1;
  }

  int xv2 = (x2 * xx) + (y2 * xy) + (z2 * xz);
  int yv2 = (x2 * yx) + (y2 * yy) + (z2 * yz);
  int zv2 = (x2 * zx) + (y2 * zy) + (z2 * zz);

  float zvt2 = zv2 - Zoff;
  int rx2 = 0, ry2 = 0;
  if (zvt2 < -5) {
    rx2 = 256 * (xv2 / zvt2) + Xoff;
    ry2 = 256 * (yv2 / zvt2) + Yoff;
  } else {
    Ok = 0;
  }

  if (Ok == 1) {
    ret->p0.x = rx1;
    ret->p0.y = ry1;
    ret->p1.x = rx2;
    ret->p1.y = ry2;
  }
  // Additional out-of-bound checks can be added as needed.
}

void cube() {
  // Define the 12 line segments that form a cube.

  // Front Face
  Lines[0].p0 = { -50, -50,  50 };
  Lines[0].p1 = {  50, -50,  50 };

  Lines[1].p0 = {  50, -50,  50 };
  Lines[1].p1 = {  50,  50,  50 };

  Lines[2].p0 = {  50,  50,  50 };
  Lines[2].p1 = { -50,  50,  50 };

  Lines[3].p0 = { -50,  50,  50 };
  Lines[3].p1 = { -50, -50,  50 };

  // Back Face
  Lines[4].p0 = { -50, -50, -50 };
  Lines[4].p1 = {  50, -50, -50 };

  Lines[5].p0 = {  50, -50, -50 };
  Lines[5].p1 = {  50,  50, -50 };

  Lines[6].p0 = {  50,  50, -50 };
  Lines[6].p1 = { -50,  50, -50 };

  Lines[7].p0 = { -50,  50, -50 };
  Lines[7].p1 = { -50, -50, -50 };

  // Connecting edges between front and back faces
  Lines[8].p0  = { -50, -50,  50 };
  Lines[8].p1  = { -50, -50, -50 };

  Lines[9].p0  = {  50, -50,  50 };
  Lines[9].p1  = {  50, -50, -50 };

  Lines[10].p0 = { -50,  50,  50 };
  Lines[10].p1 = { -50,  50, -50 };

  Lines[11].p0 = {  50,  50,  50 };
  Lines[11].p1 = {  50,  50, -50 };

  LinestoRender = 12;
  OldLinestoRender = LinestoRender;
}
