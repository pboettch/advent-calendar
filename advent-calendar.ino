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
  2,  // 5
  20,
  24,
  5,
  13,
  8,  // 10
  19,
  15,
  3,
  12,
  18,  // 15
  4,
  22,
  6,
  11,
  14,  // 20
  16,
  10,
  7,
  23,
  17,  // 25
  1,
  GRENIER,  // 27
};

#define ARRAY_SIZE(array) (sizeof(array)/sizeof(array[0]))
#define NUMPIXELS ARRAY_SIZE(leds)

#define E0_1 (1UL << 3)
#define E0_2 (1UL << 4)
#define E0_3 (1UL << 5)
#define E0_4 (1UL << 6)

#define E0_5 (1UL << 7)  // Stairs

#define E0_6 (1UL << 8)
#define E0_7 (1UL << 9)

#define E0_8 (1UL << 10)
#define E0_9 (1UL << 27)

#define E1_1 (1UL << 18)
#define E1_2 (1UL << 17)

#define E1_3 (1UL << 16)
#define E1_4 (1UL << 15)

#define E1_5 (1UL << 14)  // Stairs

#define E1_6 (1UL << 13)
#define E1_7 (1UL << 12)
#define E1_8 (1UL << 11)

#define E2_1 (1UL << 19)
#define E2_2 (1UL << 20)
#define E2_3 (1UL << 21)

#define E2_4 (1UL << 22)  // Stairs

#define E2_5 (1UL << 23)
#define E2_6 (1UL << 24)
#define E2_7 (1UL << 25)

static const struct {
  unsigned duration;
  unsigned long state;
} idle_sequences[] = {
  {60, 0},  
  {5, E0_5 | E1_5 | E2_4},
  {10, E0_5 | E1_5 | E2_4 | E2_5 },
  {2, E0_5 | E1_5 | E2_4 | E2_5 | E2_6},
  {10, E0_5 | E1_5 | E2_4 | E2_5 | E2_6 | E2_7},
  {10, E2_5 | E2_6 | E2_7},
  {1, E2_6 | E2_7},
  {30, E2_7},
};

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

    static unsigned long idle_state_timestamp = 0;
    static int idle_index = -1;

    if (!eeprom_read) {

      int val = EEPROM.read(0);
      if (val != 0xff) {
        door = val;
        led = door - 1;
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

      // Serial.print("led:");
      // Serial.print(led);
      // Serial.print(" door:");
      // Serial.print(door);
      // Serial.print(" pos:");
      // Serial.print(newPos);
      // Serial.print(" dir:");
      // Serial.println(new_direction);

     idle_index = -1;
    }
    
    if (idle_index == -1 || millis() > idle_state_timestamp) {
      idle_index++;
      if (idle_index >= ARRAY_SIZE(idle_sequences))
        idle_index = 0;
      idle_state_timestamp = millis() + idle_sequences[idle_index].duration * 1000;

      change = 1;
    }

    if (millis() > grenier_next_toggle) {
      grenier = !grenier;
      grenier_next_toggle = millis() + random(10, 30) * 1000;
      //Serial.print("toggle ");
      //Serial.print(grenier_next_toggle);
      //Serial.print(" millis ");
      //Serial.println(millis());

      change = 1;
    }

    if (change) {
      pixels.clear(); // Set all pixel colors to 'off'

      // GRB
      for (int i = 0; i < NUMPIXELS; i++) {
        unsigned long bit = (unsigned long) 1 << i;
        if (leds[i] == STREET)
          pixels.setPixelColor(i, pixels.Color(50 * street, 50 * street, 50 * street));
        else if (leds[i] == BASEMENT)
          pixels.setPixelColor(i, pixels.Color(50 * street, 50 * street, 50 * street));
        else if (leds[i] == GRENIER)
          pixels.setPixelColor(i, pixels.Color(50 * grenier, 50 * grenier, 0));
        else if (leds[i] == door) {
          // Serial.print("index:"); Serial.print(i); Serial.print(" "); Serial.println(door);
          pixels.setPixelColor(i, pixels.Color(200, 255, 0));
          EEPROM.update(0, door);
        } else if (idle_index >= 0 && idle_sequences[idle_index].state & bit) {
          pixels.setPixelColor(i, pixels.Color(200, 255, 0));
        }
      }

      pixels.show();   // Send the updated pixel colors to the hardware.
    }
#endif

    change = 0;
  }
}
