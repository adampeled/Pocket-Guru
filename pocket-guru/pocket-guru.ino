// Helpful article: https://www.makerguides.com/character-i2c-lcd-arduino-tutorial/

// Include the libraries:
// LiquidCrystal_I2C.h: https://github.com/johnrickman/LiquidCrystal_I2C
#include <Wire.h> // Library for I2C communication
#include <LiquidCrystal_I2C.h> // Library for LCD

const int WIDTH = 16;

// Wiring: SDA pin is connected to A4 and SCL pin to A5.
// Connect to LCD via I2C, default address 0x27 (A0-A2 not jumpered)
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, WIDTH, 2); // Change to (0x27,20,4) for 20x4 LCD.

const int buttonPin = 2;
int buttonState = 0;
int lastButtonState = 0;

const int NORMAL_REFRESH_SEC = 10;
const int BLAST_REFRESH_SEC = 1;

int secondsUntilRefresh = NORMAL_REFRESH_SEC;
int currentMessageIndex = 0;
unsigned long lastRefreshMillis = 0;
unsigned long blastStartMillis = 0;

const char *messages[] = {
  "Smile, breathe,", "and go slowly.",
  "Joy favors", "the kind.",
  "Whenever you go,", "there you are.",
  "All is well.", "",
  "You are not", "your thoughts.",
  "You can totally", "do this.",
  "Stay hungry", "Stay foolish",
  "Reach for", "the stars.",
  "This too", "shall pass.",
  "No feeling", "is final.",
  "Dance lightly", "with life.",
  "Creativity is", "like electricity",
  "Wisdom begins", "in wonder.",
  "You are", "your choices."
};

int totalMessages = 0;

// Custom characters
uint8_t heart[8] = {0x0,0xa,0x1f,0x1f,0xe,0x4,0x0};

const int SWEEP_CHAR = 0;
const int SWEEP_LENGTH = 8;

const int BLAST_DURATION_SEC = 8;

const int SCROLLING = 0;
const int SWEEPING = 1;
const int BLASTING = 2;

int state = SCROLLING;

void setup() {
  // Initialize the LCD:
  lcd.init();
  lcd.backlight();

  lcd.createChar(0, heart);

  // Initialize the button as an input:
  pinMode(buttonPin, INPUT_PULLUP);

  // The number of elements in the array is the first division.
  // Since each message is split into two elements (first line and second line),
  // dividing by two gives us the total number of messages.
  totalMessages = (sizeof(messages) / sizeof(messages[0])) / 2;

  lastRefreshMillis = millis();

  Serial.begin(9600);
}

void loop() {
  if(state == SCROLLING) scroll();
  else if(state == SWEEPING) sweep();
  else if(state == BLASTING) blast();

  // Read button input:
  buttonState = digitalRead(buttonPin);
  // Button was just pressed
  if (lastButtonState == HIGH && buttonState == LOW) {
    if(state != SWEEPING && state != BLASTING) {
      state = SWEEPING;
    }
  }

  lastButtonState = buttonState;
}

void scroll() {
  // If enough time has passed, show the next message
  unsigned long currentMillis = millis();
  if (currentMillis - lastRefreshMillis >= secondsUntilRefresh * 1000) {
    currentMessageIndex = (currentMessageIndex + 1) % totalMessages;
    lcd.clear();
    lastRefreshMillis = currentMillis;
  }
  
  printMessage(currentMessageIndex);
}

void sweep() {
  lcd.clear();

  drawClearChars(SWEEP_LENGTH);

  for(int i = 0; i < SWEEP_LENGTH - 2; i++) {
    lcd.scrollDisplayLeft();
  }

  for(int i = 0; i < WIDTH + SWEEP_LENGTH; i++) {
    delay(100);
    lcd.scrollDisplayRight();
  }

  lcd.clear();

  blastStartMillis = millis();
  state = BLASTING;
}

void drawClearChars(int count) {
  for(int i = 0; i < count; i++) {
    lcd.setCursor(i, 0);
    lcd.write(SWEEP_CHAR);
    lcd.setCursor(i, 1);
    lcd.write(SWEEP_CHAR);
  }
}

void blast() {
  secondsUntilRefresh = BLAST_REFRESH_SEC;

  scroll();
  
  if (millis() - blastStartMillis >= BLAST_DURATION_SEC * 1000) {
    secondsUntilRefresh = NORMAL_REFRESH_SEC;
    state = SCROLLING;
  }
}

void printMessage(int index) {
  char *line1 = messages[index * 2];
  char *line2 = messages[(index * 2) + 1];

  printLine(line1, 0);
  printLine(line2, 1);
}

void printLine(char *message, int line) {
  int length = strlen(message);
  // No need to do anything if the message is blank, so return
  if (length <= 0) return;

  // This calculation centers the message on the screen
  int cursorStart = ceil((WIDTH - length) / 2.0);

  lcd.setCursor(cursorStart, line);
  lcd.print(message);
}
