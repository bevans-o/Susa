#ifndef Susa_h
#define Susa_h

#include <list>
#include <Constants.h>
#include <SusaState.h>
#include "Arduino.h"

class GameState;

enum Piece : int {
  NONE,
  DATE,
  LILY,
  POMEGRANATE,
  FOUNTAIN
};

Piece getType(int reading);
char pieceToKey(Piece piece);
char posToKey(int position);
Piece keyToPiece(char key);
int keyToPosition(char key);
int getX(int position);
int getY(int position);
std::list<int> getAdjacencies(int position);
std::list<int> getRowColumn(int position);
int scoresAgainst(Piece scorer, Piece other);


class Position {
    private:
        std::list<int> frame; // holds most recent readings
        void changeState();

    public:
        int index, x, y, pin, light, ticks;
        std::list<int> adjacencies; // the indices of adjacent positions
        std::list<int> rowColumn; // the indicies of positions in the same row or column, including this position
        Piece occupant; // the current, accepted occupant of the position
        Piece prevReading; // the occupant sampled in the previous frame
        bool changed, locked, shouldLock, isLit;
        Position();
        Position(int i, int pinNumber, int lightIndex, bool shouldLockPiece = false);
        void read();
        float sample();
        void reset();
        String toString();
        String encode();
        String raw();
};

class Game {
    public:
        int turn, score, lastPlayed;
        GameState* state;
        Position positions[WIDTH * HEIGHT];
        Piece opponentBoard[WIDTH * HEIGHT]; 

        Game();
        void init();
        void setState(GameState& newState);
        void tick();
        int updateScoreAndLights();
};


#endif