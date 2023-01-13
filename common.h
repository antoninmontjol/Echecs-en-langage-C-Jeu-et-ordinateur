#ifndef __COMMON_H__
#define __COMMON_H__

#include "include/SDL.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "settings.h"

typedef uint8_t bool;
#define true 1
#define false 0

typedef struct WindowData {
	SDL_Window *window;
	SDL_Renderer *renderer;
	uint16_t sizeX;
	uint16_t sizeY;
	uint16_t offsetX;
	uint16_t offsetY;
	uint16_t bSize;
} WindowData;

typedef enum Type {
	PAWN = 'P',
	BISHOP = 'B',
	KNIGHT = 'N',
	ROOK = 'R',
	QUEEN = 'Q',
	KING = 'K',
	NO_TYPE
} Type;

typedef enum pType {
	HUMAN,
	AI
} pType;

typedef enum Team {
	BLACK = 0,
	WHITE = 1,
	NO_TEAM
} Team;

typedef struct Coords {
	int x;
	int y;
} Coords;

typedef enum Flag {
	NO_FLAG = 0,
	EN_PASSANT = 1,
	CASTLE = 2
} Flag;

typedef struct CoordsArray {
	Coords moves[COORDS_ARRAY_LENGTH]; // Mouvements possibles au max avec une pièce : 27
	uint8_t length;
} CoordsArray;

typedef struct Piece {
	enum Type type;
	enum Team team;
	CoordsArray moves;
	Flag flags[COORDS_ARRAY_LENGTH];
	unsigned int mCount;
	Coords pos;
	uint8_t idx;
} Piece;

typedef struct pHandler { // Si deux structures différentes pour l'IA et l'humain, on peut créer une union
	enum pType type;
	enum Team color;
} pHandler;

typedef struct Move {
	Coords src;
	Coords dst;
	Flag flag;
} Move;

typedef struct GameData {
	CoordsArray kingAttackers[2];
	Move lastMoves[4];
	uint8_t repeatCount;
	uint16_t moves[2];
	int gameState;
	uint16_t moveCount;
} GameData;

typedef struct Game {
	Team turn;
	struct Piece* board[8][8];
	bool bitboard[2][8][8];
	struct pHandler handlers[2];
	GameData gData;
} Game;

typedef struct Sound {
	SDL_AudioSpec spec;
	uint32_t length;
	uint8_t *buffer;
} Sound;

typedef struct Resources {
	WindowData wData;
	SDL_Texture *textures[TEXTURES_LENGTH];
	Sound *sounds[SOUNDS_LENGTH];
	SDL_AudioDeviceID deviceId;
} Resources;

/* ----- ----- CHANTIER ----- ----- */

typedef struct MinimaxThing {
	Move m;
	unsigned int eval;
} MinimaxThing;

/* ----- ---- VOILA ----- ----- */

extern Piece *pieces[32];
extern Piece* whiteKing;
extern Piece* blackKing;
extern Resources resources;
extern Move bestMove;
extern bool onEstDansLeTurfu;
extern Team POV;

#endif