#ifndef SusaState_h
#define SusaState_h

#include <Constants.h>
#include <Susa.h>
#include "Arduino.h"

class Game;

class GameState {
    public:
        virtual void onEnter(Game* game) = 0;
        virtual void onTick(Game* game) = 0;
        virtual void onExit(Game* game) = 0;
        virtual ~GameState() {} // deconstructor
};

class PlayerTurnState : public GameState {
    public:
        void onEnter(Game* game);
        void onTick(Game* game);
        void onExit(Game* game);
        static GameState& getInstance();
    private:
        PlayerTurnState() {};
};

class OpponentTurnState : public GameState {
    public:
        void onEnter(Game* game);
        void onTick(Game* game);
        void onExit(Game* game);
        static GameState& getInstance();
    private:
        OpponentTurnState() {};
};

#endif