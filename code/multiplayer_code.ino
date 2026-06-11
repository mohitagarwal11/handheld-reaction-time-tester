// RefleX Duel — 2-Player Competitive Reaction Time Tester

#include <U8g2lib.h>
#include <Wire.h>

const int RED_LED_PIN = 23;
const int BUTTON_LEFT_PIN = 18;
const int BUTTON_RIGHT_PIN = 19;

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R1, U8X8_PIN_NONE);

int scoreLeft = 0;
int scoreRight = 0;

// Config for clockwise rotation and reset game
const unsigned long LONG_PRESS_MS = 2000;
const int DISP_W = 64;   // logical display width  (R1 rotated)
const int DISP_H = 128;  // logical display height (R1 rotated)


// input helpers

bool isLeftPressed() {
  return digitalRead(BUTTON_LEFT_PIN) == LOW;
}
bool isRightPressed() {
  return digitalRead(BUTTON_RIGHT_PIN) == LOW;
}
bool eitherPressed() {
  return isLeftPressed() || isRightPressed();
}
bool bothPressed() {
  return isLeftPressed() && isRightPressed();
}

//Blocks until neither button is held down debouncer
void waitForBothRelease() {
  while (isLeftPressed() || isRightPressed()) delay(10);
}

/**
 * Draws a string horizontally centered at the given y baseline.
 * Uses getStrWidth() so it works for any font set before calling.
 */
void drawCentered(const char* str, int y) {
  int x = (DISP_W - (int)u8g2.getStrWidth(str)) / 2;
  if (x < 0) x = 0;
  u8g2.drawStr(x, y, str);
}

// for showing 1 or 2 or 3 line texts
void showText(const char* line1, const char* line2 = nullptr, const char* line3 = nullptr) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  if (line3) {
    drawCentered(line1, 45);
    drawCentered(line2, 65);
    drawCentered(line3, 85);
  } else if (line2) {
    drawCentered(line1, 55);
    drawCentered(line2, 78);
  } else {
    drawCentered(line1, 68);
  }
  u8g2.sendBuffer();
}

// for the large countdown text
void showCountdown(int n) {
  char buf[4];
  snprintf(buf, sizeof(buf), "%d", n);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso32_tf);
  drawCentered(buf, 80);
  u8g2.sendBuffer();
}

// shows the score after each round
void showScores(unsigned long ms) {
  char lBuf[12], rBuf[12], tBuf[16];
  snprintf(lBuf, sizeof(lBuf), "L: %d", scoreLeft);
  snprintf(rBuf, sizeof(rBuf), "R: %d", scoreRight);
  snprintf(tBuf, sizeof(tBuf), "%lu ms", ms);

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  drawCentered(tBuf, 25);
  drawCentered("Score:", 65);
  drawCentered(lBuf, 90);
  drawCentered(rBuf, 115);
  u8g2.sendBuffer();
}

// score manager
void awardPoint(int player) {
  if (player == 0) scoreLeft++;
  else scoreRight++;
}

// for when we reset the game
void resetScores() {
  scoreLeft = 0;
  scoreRight = 0;
}

// winner decider
int waitForFirstPress() {
  while (true) {
    bool left = isLeftPressed();
    bool right = isRightPressed();

    if (left && !right) return 0;
    if (right && !left) return 1;
    // and if same then we do a random
    if (left && right) return (int)random(0, 2);
  }
}

// game reset manager
bool checkResetHold() {
  if (!isLeftPressed()) return false;
  unsigned long t = millis();
  while (isLeftPressed()) {
    if (millis() - t >= LONG_PRESS_MS) {
      resetScores();
      showText("Scores", "Reset!");
      delay(1500);
      waitForBothRelease();
      return true;
    }
    delay(10);
  }
  return false;  // released too early
}


void setup() {
  ledcAttach(RED_LED_PIN, 5000, 8);
  ledcWrite(RED_LED_PIN, 0);

  pinMode(BUTTON_LEFT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT_PIN, INPUT_PULLUP);

  Wire.begin(4, 5);
  u8g2.begin();
  // Serial.begin(115200);

  showText("Press", "any btn", "to start");
}


void loop() {
  // idle if none pressed
  if (!eitherPressed()) return;

  waitForBothRelease();
  // for debounce
  delay(50);

  // countdown as usual
  showCountdown(3);
  delay(1000);
  showCountdown(2);
  delay(1000);
  showCountdown(1);
  delay(1000);

  showText("Get", "ready...");

  //RANDOM WAIT again
  unsigned long waitTime = (unsigned long)random(1000, 5001);
  unsigned long waitStart = millis();
  bool falseStart = false;
  int falsePlayer = -1;

  // to early check
  while (millis() - waitStart < waitTime) {
    if (isLeftPressed()) {
      falseStart = true;
      falsePlayer = 0;
      break;
    }
    if (isRightPressed()) {
      falseStart = true;
      falsePlayer = 1;
      break;
    }
    delay(5);
  }

  // to early manager
  if (falseStart) {
    showText("Too", "early!", (falsePlayer == 0) ? "Left!" : "Right!");
    waitForBothRelease();
    delay(2000);
    showText("Press", "any btn", "to start");
    return;
  }

  //start
  ledcWrite(RED_LED_PIN, 50);
  showText("GO!");
  unsigned long reactionStart = millis();

  int winner = waitForFirstPress();
  unsigned long reactionTime = millis() - reactionStart;

  ledcWrite(RED_LED_PIN, 0);

  awardPoint(winner);

  const char* winnerLabel = (winner == 0) ? "Left" : "Right";
  showText("Winner:", winnerLabel);

  // Serial.printf("Winner: %s | Reaction: %lu ms | Score L:%d R:%d\n", winnerLabel, reactionTime, scoreLeft, scoreRight);

  delay(1500);
  showScores(reactionTime);

  // if both buttons pressed then check for long press reset before starting a round
  waitForBothRelease();
  while (true) {
    if (checkResetHold()) {
      showText("Press", "any btn", "to start");
      return;
    }
    if (eitherPressed()) break;
    delay(10);
  }
  waitForBothRelease();
  delay(50);
}