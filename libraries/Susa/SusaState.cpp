#include <Constants.h>
#include <Susa.h>
#include "Arduino.h"


void PlayerTurnState::onEnter(Game* game) {
    if (FIRST_PLAYER) {
        game->turn++; // increment on each player turn if this player began
    }
    
    
    Serial.println("Player's turn " + String(game->turn));
    Serial.println("Score: " + String(game->updateScoreAndLights()));
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        Serial.print(game->positions[i].encode() + " | ");
    }
    Serial.println(';');
}

void PlayerTurnState::onTick(Game* game) {
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        if (!game->positions[i].locked) {// ignore locked pieces
            game->positions[i].read(); 
            game->positions[i].sample();

            if (game->positions[i].changed) {
                game->lastPlayed = i; // store last move so it can be reset if needed
                Serial.print("Player's move: ");
                Serial.println(game->positions[i].encode() + " - " + game->positions[i].raw());
                game->setState(OpponentTurnState::getInstance());
            }
        }
    }
}

void PlayerTurnState::onExit(Game* game) {
    game->updateScoreAndLights();
}

GameState& PlayerTurnState::getInstance() {
    static PlayerTurnState singleton;
    return singleton;
}



void OpponentTurnState::onEnter(Game* game) {
    if (!FIRST_PLAYER) {
        game->turn++; // if this player did not begin, increment on each opponent turn
    }

    Serial.println("Opponent's turn " + String(game->turn));
    Serial.println("Enter opponent move: ");
}

void OpponentTurnState::onTick(Game* game) {
    
    if (Serial.available() != 0) {
        // we've received the opponent's move
        String opponentMove = Serial.readString();


        // accept debug / override commands to correct any errors
        // view player score
        if (opponentMove.indexOf("score") >= 0) {
            // return player score
            Serial.println("Player score: " + String(game->score));
            // re-enter opponent state
            game->setState(OpponentTurnState::getInstance());
            return;
        }

        // reset previous move
        if (opponentMove.indexOf("retry") >= 0 || opponentMove.indexOf("reset") >= 0) {
            Serial.println("Resetting last move...");
            game->positions[game->lastPlayed].reset();
            game->turn -= 1;

            game->setState(PlayerTurnState::getInstance());
            return;
        }

        if (opponentMove.length() != 3) {
            Serial.println(opponentMove.length());
            Serial.println("Invalid move length - try again.");
            
            game->setState(OpponentTurnState::getInstance());
            return;
        } else {
            Serial.println("Opponent made move " + opponentMove);
            int position = keyToPosition(opponentMove[0]);
            Piece piece = keyToPiece(opponentMove[1]);

            Serial.println("Decoded " + String(position) + ", " + String(piece));            

            game->opponentBoard[position] = piece;

            game->setState(PlayerTurnState::getInstance());
        }       
    }
}

void OpponentTurnState::onExit(Game* game) {
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        Serial.print(String(game->opponentBoard[i]) + " | ");
    }
    Serial.println(";");
}

GameState& OpponentTurnState::getInstance() {
    static OpponentTurnState singleton;
    return singleton;
}