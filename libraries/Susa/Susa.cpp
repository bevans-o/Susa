#include <list>
#include "Arduino.h"
#include <Constants.h>
#include <SusaState.h>



Piece getType(int reading) {
  if (reading < Rref * 2) {
      if (reading > 10000) {      // 4k resistors -> ~6000-9000 :::: PURPLE
        return Piece::FOUNTAIN;
      }
      else if (reading > 5000) { // 3k resistors -> ~5000-6000 :::: GREEN
        return Piece::POMEGRANATE;
      }
      else if (reading > 1500) { // 2k resistors -> ~3500-4500 :::: YELLOW
        return Piece::LILY;
      }
      else {                  // 1k resistors -> ~<1000  :::: BLUE
        return Piece::DATE;
      }
  } else {
    return Piece::NONE;     
  }
}

char pieceToKey(Piece piece) {
  return pieceMap[piece];
}

char posToKey(int position) {
  return posMap[position];
}

Piece keyToPiece(char key) {
  for (int i = 0; i < 5; i++) {
    if (pieceMap[i] == key) {
      return static_cast<Piece>(i);
    }
  }
}

int keyToPosition(char key) {
  for (int i = 0; i < WIDTH * HEIGHT; i++) {
    if (posMap[i] == key) {
      return i;
    }
  }
}

int getX(int position) {
  return position % WIDTH;
}

int getY(int position) {
  return position / WIDTH;
}


// get all adjacencies to this index (including diagonal) but not this index
std::list<int> getAdjacencies(int position) {
  std::list<int> adjacencies; 

  int x = getX(position);
  int y = getY(position);

  if (y != 0) {
    adjacencies.push_back(position - WIDTH);
    if (x != WIDTH - 1) { adjacencies.push_back(position - WIDTH + 1); } // top left
    if (x != 0) { adjacencies.push_back(position - WIDTH - 1); }
  }

  if (x != WIDTH - 1) { adjacencies.push_back(position + 1); }
  if (x != 0) { adjacencies.push_back(position - 1); }

  if (y != HEIGHT - 1) {
    adjacencies.push_back(position + WIDTH);
    if (x != WIDTH - 1) { adjacencies.push_back(position + WIDTH + 1); }
    if (x != 0) { adjacencies.push_back(position + WIDTH - 1); }
  }

  return adjacencies; 
}


// get all indices in the same row or column (including this one)
std::list<int> getRowColumn(int position) {
  std::list<int> rowColumn;

  int x = getX(position);
  int y = getY(position);

  for (int i = 0; i < WIDTH * HEIGHT; i++) {
    if (getX(i) == x || getY(i) == y) {
      rowColumn.push_back(i);
    }
  }

  return rowColumn;
}

int scoresAgainst(Piece scorer, Piece other) {
  if (scorer > other && other != Piece::NONE) { 
    return 1;
  } else {
    return 0;
  }
}



/*
* Represents a position on the board. Tracks and filters readings from a pin.
*/

Position::Position() {
  index = 0;
  x = 0;
  y = 0;
  pin = 0;
  light = 0;
  occupant = Piece::NONE;
  prevReading = Piece::NONE;
  ticks = 0;
  changed = false;
  shouldLock = false;
  locked = false;
}

Position::Position(int i, int pinNumber, int lightIndex, bool shouldLockPiece) {
  index = i;
  x = getX(i);
  y = getY(i);
  pin = pinNumber;
  light = lightIndex;
  occupant = Piece::NONE;
  prevReading = Piece::NONE;
  adjacencies = getAdjacencies(i);
  rowColumn = getRowColumn(i);
  ticks = 0;
  changed = false;
  isLit = false;
  shouldLock = shouldLockPiece;
  locked = false;
}

void Position::read() {

  if (locked) {
    return;
  }

  float reading = analogRead(pin);
  float Vout = Vin * reading / 4095; // Convert Vout to volts
  int resistance = Rref * (1 / ((Vin / Vout) - 1)); // Formula to calculate tested resistor's value

  if (frame.size() == FRAME_SAMPLE_LENGTH) {
    frame.pop_front();
  }

  if (resistance > Rref * 2) { 
    frame.push_back(NO_CONNECTION); // standardise infinite readings so we can remove them from the filter frame
  } else {
    frame.push_back(resistance);        
  }
}

float Position::sample() {

  if (locked) { // make sure locked pieces do not change even if sampled
    changed = false;
    return -1;
  }

  int sum = 0;
  int count = 0;

  std::list<int>::iterator i;
  for (i = frame.begin(); i != frame.end(); i++) {
    if (*i != NO_CONNECTION) { // ignore infinite readings
      sum += *i; 
      count++; 
    }  
  }

  if (frame.size() == 0 || count == 0 || sum == 0) {
    return NO_CONNECTION;
  }

  int sample = sum / count;

  changed = false; // assume nothing has changed until proven otherwise

  if (getType(sample) != occupant) {
    if (getType(sample) == prevReading) {
      ticks++;

      if (ticks > OCCUPANT_DELAY) {
        // then change state
        changed = true;
        occupant = getType(sample);

        if (shouldLock && getType(sample) != Piece::NONE) {
          Serial.println("A piece has been placed, locking! " + String(occupant));
          locked = true;
        };
      }
    }


  } else {
    ticks = 0; // if we have the expected reading, reset
  }

  prevReading = getType(sample);

  return sum / count;
}

void Position::reset() {
  // reset position as if there was no tile on it
  locked = false;
  occupant = Piece::NONE;
  prevReading = Piece::NONE;
  changed = false;
  frame.clear();
  isLit = false;
}

String Position::toString() {
  return "Position " + String(index) + " at " + String(x) + ", " + String(y) + " with pin:light " + String(pin) + ":" + String(light);
}

String Position::encode() {
  return String(posToKey(index)) + String(pieceToKey(occupant));
}

String Position::raw() {
  return "(Position " + String(index) + ", " + String(occupant) + ")";
}



Game::Game() {}

void Game::init() {
  if (FIRST_PLAYER) {
    state = &PlayerTurnState::getInstance();
    Serial.println("Player Turn");
  } else {
    state = &OpponentTurnState::getInstance();
    Serial.println("Opponent Turn");
  }

  state->onEnter(this);
}

void Game::setState(GameState& newState) {
  state->onExit(this);
  state = &newState;
  state->onEnter(this);
}


void Game::tick() {
  state->onTick(this);
}


// basically all the actual game logic is in this method??
// 1. pieces score 1 point for each adjacent piece of a lower value on your board
// 2. if a piece is in the same position as a lower-valued opponent piece, it scores the value of that piece
// 3. pieces light up if there is an enemy piece of the same value in the same row or column
// 4. if a piece of value 4 (a fountain) is fully surrounded by the opponent, it scores nothing
int Game::updateScoreAndLights() {
  int sum = 0;
  for (int i = 0; i < WIDTH * HEIGHT; i++) {
    // skip empty tiles
    if (positions[i].occupant == Piece::NONE) {
      continue;
    }

    // SCORE TILE -> break these out to Position methods? (and pass opponentBoard)

    std::list<int>::iterator j;
    if (positions[i].occupant == Piece::FOUNTAIN) {
      // deal with fountain cancellation - fountain scores no points when surrounded
      bool surrounded = true;
      for (j = positions[i].adjacencies.begin(); j != positions[i].adjacencies.end(); j++) {
        if (opponentBoard[*j] == Piece::NONE) {
          // fountain is not surrounded, it can score
          surrounded = false;
        }
      }

      if (surrounded) {
        Serial.println("Player fountain surrounded.");
        continue;
      }
    }
    
    for (j = positions[i].adjacencies.begin(); j != positions[i].adjacencies.end(); j++) {
      sum += scoresAgainst(positions[i].occupant, positions[*j].occupant);
    }

    sum += opponentBoard[i] * scoresAgainst(positions[i].occupant, opponentBoard[i]);


    // LIGHT TILE

    for (j = positions[i].rowColumn.begin(); j != positions[i].rowColumn.end(); j++) {
      if (opponentBoard[*j] == positions[i].occupant) {
        // light tile if there is a tile of the same type in the opposing rowcolumn
        positions[i].isLit = true;
      }
    }
  }

  score = sum;
  return score;
}

