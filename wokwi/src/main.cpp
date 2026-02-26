#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>

// Initialize TFT
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

// --- CONFIGURATION ---
#define SCREEN_W 240
#define SCREEN_H 240
#define BGCOLOR TFT_BLACK
#define EYECOLOR TFT_WHITE
#define BG_DARK_BLUE 0x0005
#define V_DARK 0x0110
#define TFT_YELLOW 0xFFE0

// Eye Parameters
int eyeW = 56;
int eyeH_Max = 56;
int borderRadius = 16;
int spaceBetween = 24;
int blinkThickness = 6;
int blinkWidth = 64;
float blinkThreshold = 12.0;

// Animation Variables
float curLx, curLy, curLh;
float targetLx, targetLy, targetLh;
unsigned long blinkTimer = 0;
unsigned long idleTimer = 0;
unsigned long moodTimer = 0;

enum Mood { NORMAL, TIRED, ANGRY, HAPPY };
Mood currentMood = NORMAL;

// Dashboard Variables
float realRPM = 0, realSPD = 0, realTMP = 0;

// Helper Functions
void printCentered(String s, int y, int size, uint16_t color) {
  spr.setTextSize(size);
  spr.setTextColor(color);
  int x = 120 - (s.length() * 6 * size / 2);
  spr.setCursor(x, y);
  spr.print(s);
}

// Eye Drawing Logic
void drawEyesToSprite() {
  spr.fillSprite(BGCOLOR);
  int drawW, drawH, drawRadius, finalY;
  int xL, xR;

  if (curLh < blinkThreshold) {
    drawW = blinkWidth; drawH = blinkThickness; drawRadius = 3;
    finalY = (int)curLy + (eyeH_Max / 2) - (blinkThickness / 2);
    xL = (int)curLx - (blinkWidth - eyeW) / 2;
    xR = xL + blinkWidth + (spaceBetween - (blinkWidth - eyeW));
  } else {
    drawW = eyeW; drawH = (int)curLh; drawRadius = borderRadius;
    finalY = (int)curLy + (eyeH_Max - drawH) / 2;
    xL = (int)curLx; xR = xL + eyeW + spaceBetween;
  }

  spr.fillRoundRect(xL, finalY, drawW, drawH, drawRadius, EYECOLOR);
  spr.fillRoundRect(xR, finalY, drawW, drawH, drawRadius, EYECOLOR);

  if (curLh >= blinkThreshold) {
    int mH = drawH / 2;
    if (currentMood == ANGRY) {
      spr.fillTriangle(xL, finalY, xL + eyeW, finalY, xL + eyeW, finalY + mH, BGCOLOR);
      spr.fillTriangle(xR, finalY, xR + eyeW, finalY, xR, finalY + mH, BGCOLOR);
    } else if (currentMood == TIRED) {
      spr.fillTriangle(xL, finalY, xL + eyeW, finalY, xL, finalY + mH, BGCOLOR);
      spr.fillTriangle(xR, finalY, xR + eyeW, finalY, xR + eyeW, finalY + mH, BGCOLOR);
    } else if (currentMood == HAPPY) {
      spr.fillRoundRect(xL - 2, finalY + mH, eyeW + 4, eyeH_Max, borderRadius, BGCOLOR);
      spr.fillRoundRect(xR - 2, finalY + mH, eyeW + 4, eyeH_Max, borderRadius, BGCOLOR);
    }
  }
}

void updateEyesLogic() {
  unsigned long now = millis();
  if (now > moodTimer) {
    if (currentMood == NORMAL) currentMood = TIRED;
    else if (currentMood == TIRED) currentMood = ANGRY;
    else if (currentMood == ANGRY) currentMood = HAPPY;
    else currentMood = NORMAL;
    moodTimer = now + 4000;
  }
  if (now > blinkTimer) { targetLh = 0; blinkTimer = now + random(2000, 5000); }
  if (now > idleTimer) { targetLx = random(40, 90); targetLy = random(70, 105); idleTimer = now + random(2000, 6000); }
  float speed = (targetLh == 0 || curLh < 10) ? 0.40 : 0.20;
  curLx += (targetLx - curLx) * 0.20; curLy += (targetLy - curLy) * 0.20; curLh += (targetLh - curLh) * speed;
  if (curLh < 1.0) targetLh = eyeH_Max;
}

// Dashboard Logic
uint16_t getSpeedColor(float spd) {
  if (spd < 40) return TFT_BLUE;
  if (spd < 80) return TFT_YELLOW;
  return TFT_GREEN;
}

uint16_t getRPMColor(float rpm) {
  if (rpm < 2000) return TFT_YELLOW;
  if (rpm < 4000) return 0xFA20;
  return TFT_RED;
}

void drawDashboard(float rpm, float spd, float tmp) {
  spr.fillSprite(BG_DARK_BLUE);
  spr.drawArc(120, 120, 115, 100, 150, 390, V_DARK, BG_DARK_BLUE);
  spr.drawArc(120, 120, 95, 85, 150, 390, V_DARK, BG_DARK_BLUE);

  int endAngleSpd = 150 + (int)((spd / 120.0) * 240.0);
  int endAngleRpm = 150 + (int)((rpm / 5000.0) * 240.0);

  if(spd > 0.1) spr.drawArc(120, 120, 115, 100, 150, endAngleSpd, getSpeedColor(spd), BG_DARK_BLUE);
  if(rpm > 0.1) spr.drawArc(120, 120, 95, 85, 150, endAngleRpm, getRPMColor(rpm), BG_DARK_BLUE);

  spr.setTextColor(TFT_CYAN); spr.setTextSize(3);
  spr.setCursor(85, 50); spr.print((int)tmp); spr.setTextSize(1); spr.print(" C");

  spr.setTextColor(TFT_WHITE); spr.setTextSize(5);
  int xRpm = (rpm < 1000) ? 95 : 75;
  spr.setCursor(xRpm, 100); spr.print((int)rpm);

  spr.setTextColor(TFT_YELLOW); spr.setTextSize(4);
  int xSpd = (spd < 100) ? 95 : 75;
  spr.setCursor(xSpd, 160); spr.print((int)spd);
  spr.setTextSize(1); spr.print(" km/h");

  spr.pushSprite(0, 0);
}

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(0);
  spr.createSprite(SCREEN_W, SCREEN_H);
  spr.setSwapBytes(true);

  // Initialize Eye Targets
  targetLh = eyeH_Max;
  targetLx = (SCREEN_W - (eyeW * 2 + spaceBetween)) / 2;
  targetLy = (SCREEN_H - eyeH_Max) / 2;
  curLx = targetLx; curLy = targetLy; curLh = eyeH_Max;

  blinkTimer = millis() + 2000;
  idleTimer = millis() + 4000;
  moodTimer = millis() + 4000;
}

void loop() {
  static unsigned long switchModeTimer = 0;
  static bool showDashboard = false;

  // Toggle mode every 10 seconds for demo
  if (millis() > switchModeTimer) {
    showDashboard = !showDashboard;
    switchModeTimer = millis() + 10000;
  }

  if (showDashboard) {
    // Simulate OBD Data
    realRPM = random(800, 4800);
    realSPD = map(realRPM, 800, 4800, 0, 120);
    realTMP = 92;
    drawDashboard(realRPM, realSPD, realTMP);
    delay(50);
  } else {
    // Animate Eyes
    updateEyesLogic();
    drawEyesToSprite();
    spr.pushSprite(0, 0);
    delay(16);
  }
}
