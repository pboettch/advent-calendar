#include <Adafruit_NeoPixel.h>

#include <EEPROM.h>


// Which pin on the Arduino is connected to the NeoPixels?
#define PIN       12
#define NUMDOORS 24

#define STREET -1
#define BASEMENT -2
#define GRENIER -3

static const char leds[] = { // TODO + basement + roof + 2 street lights
  STREET,
  STREET,
  BASEMENT,
  21,
  9,
  2,
  20,
  24,
  5,
  13,
  8,
  19,
  15,
  3,
  12,
  18,
  4,
  22,
  6,
  11,
  14,
  16,
  10,
  7,
  23,
  17,
  1,
  GRENIER,
};

#define NUMPIXELS (sizeof(leds)/sizeof(leds[0]))

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#include <RotaryEncoder.h>

#define PIN_IN1 2
#define PIN_IN2 3

RotaryEncoder *encoder = nullptr;

void checkPosition()
{
  encoder->tick(); // just call tick() to check the state.
}

void setup()
{
  Serial.begin(9600);

  pinMode(PIN_IN1, INPUT_PULLUP);
  pinMode(PIN_IN2, INPUT_PULLUP);

  pinMode(12, OUTPUT);

  // use FOUR0 mode when PIN_IN1, PIN_IN2 signals are always LOW in latch position.
  encoder = new RotaryEncoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::TWO03);

  attachInterrupt(digitalPinToInterrupt(PIN_IN1), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_IN2), checkPosition, CHANGE);

  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
}


static enum {
  START = 0,
  NORMAL = 1,
} state = START;


void loop() {
  if (state == START) {
    pixels.clear(); // Set all pixel colors to 'off'

    for (int i = 0; i < NUMPIXELS; i++) {
      delay(250);
      pixels.setPixelColor(i, pixels.Color(50, 50, 0));
      pixels.show();   // Send the updated pixel colors to the hardware.
    }
    state = NORMAL;
    delay(1000);
  } else if (state == NORMAL) {

#if 10
    static int direction_change_count = 0;
    static int light = 0, val;

    static int pos;

    static int change = 1;
    static int street;
    static int grenier;
    static unsigned long grenier_next_toggle = 0;

    static int eeprom_read = 0;

    static int led = 0, door = 1;

    if (!eeprom_read) {

      int val = EEPROM.read(0);
      if (val != 0xff) {
        door = val;
      }

      eeprom_read = 1;
    }

    val = (val + analogRead(2)/10) / 2;
    if (val != light) {
      // Serial.print("light: ");
      // Serial.println(val);
      light = val;

      if (light > 25 && street == 0) {
        street = 1;
        change = 1;
        //digitalWrite(5, HIGH);
      } else if (light <= 25 && street == 1) {
        street = 0;
        change = 1;
        //digitalWrite(5, LOW);
      }
    }
#endif

#if 10
    encoder->tick(); // just call tick() to check the state.

    int newPos = encoder->getPosition();
    if (pos != newPos) {
      pos = newPos;

      int new_direction =  (int) encoder->getDirection();

      if (direction_change_count * new_direction < 0)
        direction_change_count = 0;
      direction_change_count += new_direction;

      if (abs(direction_change_count) >= 2) {
        led += direction_change_count < 0 ? -1 : 1;
        direction_change_count = 0;
      }

      while (led < 0)
        led += NUMDOORS;

      led %= NUMDOORS;

      door = led + 1;

      change = 1;

      Serial.print("led:");
      Serial.print(led);
      Serial.print(" door:");
      Serial.print(door);
      Serial.print(" pos:");
      Serial.print(newPos);
      Serial.print(" dir:");
      Serial.println(new_direction);
    }

    if (millis() > grenier_next_toggle) {
      grenier = !grenier;
      grenier_next_toggle = millis() + random(10, 30) * 1000;
      Serial.print("toggle ");
      Serial.print(grenier_next_toggle);
      Serial.print(" millis ");
      Serial.println(millis());

      change = 1;
    }


    if (change) {
      pixels.clear(); // Set all pixel colors to 'off'

      // GRB
      for (int i = 0; i < NUMPIXELS; i++) {
        if (leds[i] == STREET)
          pixels.setPixelColor(i, pixels.Color(50* street, 50* street, 50* street));
        else if (leds[i] == BASEMENT)
          pixels.setPixelColor(i, pixels.Color(50* street, 50* street, 50* street));
        else if (leds[i] == GRENIER)
          pixels.setPixelColor(i, pixels.Color(50* grenier, 50* grenier, 0));
        else if (leds[i] == door) {
          Serial.print("index:"); Serial.print(i); Serial.print(" "); Serial.println(door);
          pixels.setPixelColor(i, pixels.Color(200, 255, 0));
          EEPROM.update(0, door);
        }
      }

      pixels.show();   // Send the updated pixel colors to the hardware.
    }

#endif

    change = 0;
  }
}
