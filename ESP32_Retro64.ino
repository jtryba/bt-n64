#include <BleGamepad.h>

//uses 3.7v battery
//2.75v was measued at the voltage divider with full charge (4.2v) at battery
//out := map(2.75, 0, 3.3, 0, 4095) // result is 3413

//1.96 was measued at the voltage divider with 3.0v at battery
//out := map(1.96, 0, 3.3, 0, 4095)) // result is 2432

#define MAX_BATERY_VALUE 3413
#define MIN_BATERY_VALUE 2432

// pins
#define PIN_DEBUG 23
#define PIN_BAT 15
#define JOY_X         34
#define JOY_Y         35
#define BTN_A         13
#define BTN_B         12
#define BTN_C_UP      14
#define BTN_C_DOWN    27
#define BTN_C_LEFT    26
#define BTN_C_RIGHT   25
#define BTN_L         33
#define BTN_R         32
#define BTN_Z         4
#define PAD_UP        5
#define PAD_DOWN      18
#define PAD_LEFT      19
#define PAD_RIGHT     21
#define BTN_START     22
// end pins

#define BUTTON_COUNT  14

#define JOY_DEAD 7
#define JOY_RANGE 1150

int JOY_MIN = 700;  // will be calculated later in CalStick()
int JOY_MAX = 3100; // will be calculated later in CalStick()

const int btn[BUTTON_COUNT] = { BTN_A, BTN_B, BTN_Z, BTN_C_UP, BTN_C_DOWN, BTN_C_LEFT, BTN_C_RIGHT, BTN_L, BTN_R, PAD_UP, PAD_DOWN, PAD_LEFT, PAD_RIGHT, BTN_START };
const int joy_btn[BUTTON_COUNT] = { BUTTON_1, BUTTON_2, BUTTON_9, BUTTON_4, BUTTON_5, BUTTON_6, BUTTON_7, BUTTON_8, BUTTON_3, BUTTON_10, BUTTON_11, BUTTON_12, BUTTON_13, BUTTON_14 };

bool debug_enable = false;

// Joystick State
unsigned int cur_x = 0;
unsigned int cur_y = 0;
unsigned int raw_x = 0;
unsigned int raw_y = 0;

unsigned int lastButtonState = 0;

uint8_t lastBatteryLevel = 0;
uint8_t batteryLevel = 100;

BleGamepad bleGamepad("Retro64_BT", "jtryba", batteryLevel);

void setup()
{
  pinMode(PIN_DEBUG, INPUT_PULLUP);
  pinMode(JOY_X, INPUT);
  pinMode(JOY_Y, INPUT);
  for (int buttonIndex = 0; buttonIndex < BUTTON_COUNT; buttonIndex++)
  {
     pinMode(btn[buttonIndex], INPUT_PULLUP);
  }
  delay(10);
  
  if (debug_enable or digitalRead(PIN_DEBUG) == LOW)
  {
    debug_enable = true;
    Serial.begin(115200);
  }

  CalStick();
  if (debug_enable)
    Serial.println("Starting BLE");
    
  bleGamepad.begin();
}

void CalStick(void)
{
  if (debug_enable)
    Serial.println("Calibrating analog stick...");
  int t = 5;
  int x = 0;
  int y = 0;
  for (int i = 0; i < t; i++)
  {
    x += analogRead(JOY_X);
    y += analogRead(JOY_Y);
  }
  
  int center_x = x/t;
  int center_y = y/t;
  int center = (center_x+center_y)/2;
  
  JOY_MIN = center-JOY_RANGE;
  JOY_MAX = center+JOY_RANGE;

  if (debug_enable)
  {
    Serial.print("Calibration complete!");
    Serial.print("center_x: ");
    Serial.print(center_x);
    Serial.print("center_y: ");
    Serial.print(center_x);
    Serial.print("center_applied: ");
    Serial.print(center);
    Serial.print("joy_min: ");
    Serial.print(JOY_MIN);
    Serial.print("joy_max: ");
    Serial.println(JOY_MAX);
  }
}

signed int GetStick_x(void)
{    
  unsigned int l = analogRead(JOY_X);
  raw_x = l;
  if (l > JOY_MAX)
    l = JOY_MAX;
  if (l < JOY_MIN)
    l = JOY_MIN;
  signed int i = map(l, JOY_MIN, JOY_MAX, -127, 127);
  if (i < JOY_DEAD && i > -JOY_DEAD)
    return 0;
  return i;
}

signed int GetStick_y(void)
{
  unsigned int l = analogRead(JOY_Y);
  raw_y = l;
  if (l > JOY_MAX)
    l = JOY_MAX;
  if (l < JOY_MIN)
    l = JOY_MIN;
  signed int i = map(l, JOY_MIN, JOY_MAX, -127, 127);
  if (i < JOY_DEAD && i > -JOY_DEAD)
    return 0;
  return i;
}

void ReadBat(void)
{
  //int r1 = 100000;
  //int r2 = 680000;
  batteryLevel = map(analogRead(PIN_BAT), MIN_BATERY_VALUE, MAX_BATERY_VALUE, 0, 100);
  
  // required when powered via usb
  if (batteryLevel > 100)
    batteryLevel = 100;
}

void loop()
{
  if(bleGamepad.isConnected())
  {
  
    cur_x = GetStick_x();
    
    cur_y = GetStick_y();
    
    ReadBat();
  
    if (debug_enable)
    {
      char buf[20];
      memset(buf, 0, 20);
      sprintf(buf, "0x%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
        bitRead(lastButtonState, 0),
        bitRead(lastButtonState, 1),
        bitRead(lastButtonState, 2),
        bitRead(lastButtonState, 3),
        bitRead(lastButtonState, 4),
        bitRead(lastButtonState, 5),
        bitRead(lastButtonState, 6),
        bitRead(lastButtonState, 7),
        bitRead(lastButtonState, 8),
        bitRead(lastButtonState, 9),
        bitRead(lastButtonState, 10),
        bitRead(lastButtonState, 11),
        bitRead(lastButtonState, 12),
        bitRead(lastButtonState, 13)
      );
      Serial.print("Buttons:");
      Serial.print(buf);
      Serial.print(" X:");
      Serial.print(cur_x, DEC);
      Serial.print("(");
      Serial.print(raw_x, DEC);
      Serial.print(")");
      Serial.print(" Y:");
      Serial.print(cur_y, DEC);
      Serial.print("(");
      Serial.print(raw_y, DEC);
      Serial.print(")");
      Serial.print(" Battery:");
      Serial.println(batteryLevel, DEC);
    }

    if (batteryLevel != lastBatteryLevel)
    {
      lastBatteryLevel = batteryLevel;
      bleGamepad.setBatteryLevel(batteryLevel);
    }

    bleGamepad.setAxes(cur_x, cur_y);
    
    for (int buttonIndex = 0; buttonIndex < BUTTON_COUNT; buttonIndex++)
    {
      byte currentButtonState = !digitalRead(btn[buttonIndex]);
      if (currentButtonState != bitRead(lastButtonState, buttonIndex)) {
        bitWrite(lastButtonState, buttonIndex, currentButtonState);
        if (currentButtonState == 1)
          bleGamepad.press(joy_btn[buttonIndex]);
        else
          bleGamepad.release(joy_btn[buttonIndex]);
      }
    }
    delay(10);
  }
}
