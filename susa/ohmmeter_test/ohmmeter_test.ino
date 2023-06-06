#include <Susa.h>
#include <Adafruit_NeoPixel.h>
#include <list>

// pin layout
const int sensorPins[] = {26, 25, 34, 39, 32, 14, 4, 36, 15, 33, 27, 12};  // Analog input pin that senses Vout
const int lightMap[] = {0, 1, 2, 3, 9, 8, 7, 6, 10, 11, 12, 13};           // the corresponding light index for each of the above pins

Adafruit_NeoPixel lights(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
Position positions[WIDTH * HEIGHT];
Game game;
GameState* state;


void setup() { 
  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN, LOW); // analog pin 12 is used for boot, so avoid powering the board until boot complete
  delay(2000);
  digitalWrite(POWER_PIN, HIGH);

  Serial.begin(9600);

  for (int i = 0; i < 20; i++) {
    Serial.println("lalalala");    
  }

  // initialise position objects
  for (int i = 0; i < WIDTH * HEIGHT; i++) {
    Position pos (i, sensorPins[i], lightMap[i]);
    positions[i] = pos;
    Serial.println(pos.toString());    
  }

  lights.begin();
  lights.setBrightness(255);
  lights.show();

  // initialise game
  game.init();
}

void loop() {
  Serial.print("min:0,max:10000,");

  //game.tick();
  
  for (int i = 0; i < WIDTH * HEIGHT; i++) {
    positions[i].read(); 
    int resistance = positions[i].sample();
    
    logReading(i, resistance);

    if (resistance < 25000) {
      int brightness = 255;

      if (resistance > 10000) {      // 4k resistors -> ~6000-9000 :::: PURPLE
        lights.setPixelColor(positions[i].light, brightness / 2, 0, brightness);
      }
      else if (resistance > 5000) { // 3k resistors -> ~5000-6000 :::: GREEN
        lights.setPixelColor(positions[i].light, 0, brightness, 0);
      }
      else if (resistance > 1500) { // 2k resistors -> ~3500-4500 :::: YELLOW
        lights.setPixelColor(positions[i].light, brightness, brightness, 0);
      }
      else {                  // 1k resistors -> ~<1000  :::: BLUE
        lights.setPixelColor(positions[i].light, 0, 0, brightness);
      }
    }
    else {
      lights.setPixelColor(positions[i].light, 200, 0, 0);      
    }
        
  }

  Serial.println(";");
  lights.show();

  delay(100);
}



void logReading(int i, int reading) {
  Serial.print(String("R" + String(i) + ":"));
  if (reading == NO_CONNECTION) {
    Serial.print("max");
  } else {
    Serial.print(reading);
  }
  Serial.print(",");
}
