#include <Adafruit_NeoPixel.h>


// Which pin on the Arduino is connected to the NeoPixels?
#define PIN       6

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 5 // Popular NeoPixel ring size

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#include <RotaryEncoder.h>

#define PIN_IN1 2
#define PIN_IN2 3

RotaryEncoder *encoder = nullptr;

static int pos = 0, new_pos = 0;

void checkPosition()
{
#if 0
  if (digitalRead(PIN_IN2) == 1)
    new_pos++;
  else
    new_pos--;
#endif 
  
  encoder->tick(); // just call tick() to check the state.
}

#define DELAYVAL 500 // Time (in milliseconds) to pause between pixels

void setup() {
  Serial.begin(9600);
  
  pinMode(PIN_IN1, INPUT_PULLUP);
  pinMode(PIN_IN2, INPUT_PULLUP);

  pinMode(5, OUTPUT);

  // use FOUR0 mode when PIN_IN1, PIN_IN2 signals are always LOW in latch position.
  encoder = new RotaryEncoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::TWO03);

  attachInterrupt(digitalPinToInterrupt(PIN_IN1), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_IN2), checkPosition, CHANGE);

  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
}

int led = 0;
int change = 1;
int street = 0;

void loop() {
#if 0
  if (pos != new_pos) {
    Serial.print("new_pos:");
    Serial.println(new_pos);
    pos = new_pos;
  }
#endif

  static int direction_change_count = 0;
  static int light = 0, val;

  val = (val + analogRead(2)/10) / 2;
  if (val != light) {
    Serial.print("light: ");
    Serial.println(val);
    light = val;

    if (light > 35) {
      street = 1;
      change = 1;
      digitalWrite(5, HIGH);
    } else {
      street = 0;
      change = 1;
      digitalWrite(5, LOW);
    }
  }
  
#if 01
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
      led += NUMPIXELS;
       
    led %= NUMPIXELS;
    
    change = 1;

    Serial.print("led:");
    Serial.print(led);
    Serial.print(" pos:");
    Serial.print(newPos);
    Serial.print(" dir:");
    Serial.println(new_direction);
  }

  if (change) {
    pixels.clear(); // Set all pixel colors to 'off'
    pixels.setPixelColor(led, pixels.Color(0, 50, 0));
    pixels.setPixelColor(4, pixels.Color(50 * street, 50 * street, 0 * street));
    pixels.show();   // Send the updated pixel colors to the hardware.
  }
  change = 0;
  
#endif
}
