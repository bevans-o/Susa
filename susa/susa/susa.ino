#include <Susa.h>
#include <Adafruit_NeoPixel.h>
#include <list>

// pin layout
const int sensorPins[] = {26, 25, 34, 39, 32, 14, 4, 36, 15, 33, 27, 12};  // Analog input pin that senses Vout
const int lightMap[] = {0, 1, 2, 3, 9, 8, 7, 6, 10, 11, 12, 13};           // the corresponding light index for each of the above pins

Adafruit_NeoPixel lights(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
Game game;
GameState* state;


void setup() {
  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN, LOW); // analog pin 12 is used for boot, so avoid powering the board until boot complete
  delay(2000);
  digitalWrite(POWER_PIN, HIGH);

  Serial.begin(9600);

  // initialise position objects
  for (int i = 0; i < WIDTH * HEIGHT; i++) {
    Position pos (i, sensorPins[i], lightMap[i], true);
    game.positions[i] = pos;
    Serial.println(pos.toString());    
  }

  // initialise lights
  lights.begin();
  lights.setBrightness(255);
  lights.show();

  // initialise game
  game.init();
}

void loop() {
  game.tick();
  
  // keep lighting external to state (for now) to avoid importing extra libraries into the susa library
  for (int i = 0; i < WIDTH * HEIGHT; i++) {
    int colour[] = { 0, 0, 0 };
    if (game.positions[i].isLit) {
      colour[0] = 255;
      colour[1] = 245; 
      colour[2] = 225; // lit tile colour
    } else {
      if (game.positions[i].occupant != Piece::NONE) { // if not lit, add gentle light based on piece type as confirmation
        colour[0] = 10;
        colour[1] = 5;
        colour[2] = 10;
      }
    }

    lights.setPixelColor(game.positions[i].light, colour[0], colour[1], colour[2]);
  }

  lights.show();

  delay(100);
}
