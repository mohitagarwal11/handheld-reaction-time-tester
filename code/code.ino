// for the i2c display that uses SH1106 drivers
#include <U8g2lib.h>
#include <Wire.h>

// the GPIO pin numbers
const int GREEN_LED_PIN = 21;
const int YELLOW_LED_PIN = 22;
const int RED_LED_PIN = 23;
const int BUTTON_PIN = 18;

// display init
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// to turn off all leds
void allOff() {
  ledcWrite(GREEN_LED_PIN, 0);
  ledcWrite(YELLOW_LED_PIN, 0);
  ledcWrite(RED_LED_PIN, 0);
}

// to turn on all leds
void allOn() {
  ledcWrite(GREEN_LED_PIN, 60);
  ledcWrite(YELLOW_LED_PIN, 60);
  ledcWrite(RED_LED_PIN, 60);
}

// boolean to check if button is pressed or not
bool buttonPressed() {
  return digitalRead(BUTTON_PIN) == LOW;
}

// a debouncer so button spamming and long preses are not registered
void waitForButtonRelease() {
  while (buttonPressed()) {
    delay(10);
  }
}

// this shows the text on the screen in 2 line format can be modified for more lines or other modification like font, size, etc...
void showText(const char* line1, const char* line2 = nullptr) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(0, 20, line1);
  if (line2) {
    u8g2.drawStr(0, 45, line2);
  }
  u8g2.sendBuffer();
}

// this prints the reaction time of the user on the screen
void showReactionTime(unsigned long ms) {
  char buf[20];
  snprintf(buf, sizeof(buf), "%lu ms", ms);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(0, 20, "Time:");
  u8g2.drawStr(0, 45, buf);
  u8g2.sendBuffer();
}

// this is run once at startup as init of the whole thing
void setup() {
  ledcAttach(GREEN_LED_PIN, 5000, 8);
  ledcAttach(YELLOW_LED_PIN, 5000, 8);
  ledcAttach(RED_LED_PIN, 5000, 8);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Wire.begin(4, 5);
  u8g2.begin();

  Serial.begin(115200);

  allOn();
  showText("Press btn", "to start");
}

// this runs on infinite loop until we unplug
void loop() {
  if (!buttonPressed()) return;

  waitForButtonRelease();

  // this is the countdown part
  showText("3");
  allOn();
  delay(1000);

  showText("2");
  ledcWrite(RED_LED_PIN, 0);
  delay(1000);

  showText("1");
  ledcWrite(YELLOW_LED_PIN, 0);
  delay(1000);

  ledcWrite(GREEN_LED_PIN, 0);
  showText("Press when", "all light up!");

  // Random wait
  unsigned long waitTime = random(1000, 5000);
  unsigned long startWait = millis();

  // this is if the user presses it before all the leds light up
  while (millis() - startWait < waitTime) {
    if (buttonPressed()) {
      showText("Too early!", "Try again");
      allOff();
      waitForButtonRelease();
      delay(2000);
      allOn();
      showText("Press btn", "to start");
      return;
    }
  }

  // timer begin
  allOn();
  showText("GO!");
  unsigned long reactionStart = millis();

  // infinite wait till the user reacts
  while (!buttonPressed()) {}

  unsigned long reactionTime = millis() - reactionStart;

  allOff();
  showReactionTime(reactionTime);
  Serial.print("Reaction Time: ");
  Serial.print(reactionTime);
  Serial.println(" ms");

  //infinite wait till user wants to wait in the results screen
  waitForButtonRelease();
  while (!buttonPressed()) {}
  waitForButtonRelease();

  // reset to start
  allOn();
  showText("Press btn", "to start");
}