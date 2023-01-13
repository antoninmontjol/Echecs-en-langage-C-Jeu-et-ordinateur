#include "common.h"
#include "utils.h"
#include <ctype.h>

void clearBitboard(Game *game) {
	memset(game->bitboard, 0, sizeof(game->bitboard));
}

uint16_t getPieceValue(Piece* piece) {
	switch(piece->type) {
		case KING:
			return KING_VALUE;
		case QUEEN:
			return QUEEN_VALUE;
		case ROOK:
			return ROOK_VALUE;
		case BISHOP:
			return BISHOP_VALUE;
		case KNIGHT:
			return KNIGHT_VALUE;
		case PAWN:
			return PAWN_VALUE;
		default:
			return 0;
	}
}

uint8_t getPieceIndex(Piece *piece) {
	switch(piece->type) { 
		case KING:
			return 5;
		case QUEEN:
			return 4;
		case ROOK:
			return 3;
		case BISHOP:
			return 2;
		case KNIGHT:
			return 1;
		case PAWN:
			return 0;
		default:
			return 0;
	}
}

// Ajoute un mouvement à la liste des mouvements possibles d'une pièce
void push(CoordsArray *array, Coords el) {
	if (array->length < COORDS_ARRAY_LENGTH) {
		array->moves[array->length] = el;
		array->length++;
	}
}

// Supprime un mouvement d'une liste de mouvements possibles
void pop(CoordsArray *array, uint8_t index) {
	if (array->length > 0 && index < array->length) {
		Coords* src = &array->moves[index+1];
		Coords* dst = &array->moves[index];
		memcpy(dst, src, (array->length - index - 1) * sizeof(Coords));
		array->length--;
	}
}

// Détecte une sortie de plateau d'une position
uint8_t outOfBounds(Coords* c) {
	if (c->x > 7 || c->y > 7 || c->x < 0 || c->y < 0)
		return 1;
	return 0;
}

// Permet de libérer la mémoire utilisée par un plateau
void destroyBoard(Game* game) {
	for (uint8_t i = 0 ; i < 32 ; i++) {
		Piece *current = pieces[i];
		if (current != NULL) {
			game->board[current->pos.x][current->pos.y] = NULL;
			free(current);
			pieces[i] = NULL;
		}
	}
}

// Initialise une partie à partir d'une séquence de caractères
void initBoard(Game* game, const char* sequence) {
	int pieceIdx = 0;
	if (strlen(sequence) != 64) {
		return;
	}
	game->turn = WHITE;

	for (uint8_t x = 0 ; x < 8 ; x++) { // Pour chaque case du plateau
		for (uint8_t y = 0 ; y < 8 ; y++) {
			char chr = sequence[x+y*8]; // Caractère en cours
			if (chr != ' ') {
				enum Team t = WHITE;
				if (chr > 96) {
					t = BLACK; // Noir si lettre en minuscule
					chr -= 32; // Qu'on repasse en majuscule
				}

				game->board[x][y] = malloc(sizeof(Piece)); // Nouvelle pièce
				Piece *piece = game->board[x][y];
				piece->type = chr;
				piece->team = t;
				piece->pos = coords(x, y);
				piece->mCount = 0;
				piece->idx = pieceIdx;
				pieces[pieceIdx] = piece;
				pieceIdx++;
			}
			else
				game->board[x][y] = NULL;
		}
	}

	game->gData.moveCount = 0;
}

// Met à jour les données de la fenêtre
void updateWindowData(void) {
	// [!] : Peut être une condition pour vérifier qu'on peut soustraire sans aller dans le négatif...
	uint16_t smallerSide = resources.wData.sizeX;
	if (resources.wData.sizeY < resources.wData.sizeX)
		smallerSide = resources.wData.sizeY;
	resources.wData.bSize = smallerSide - 2 * (smallerSide * MIN_WINDOW_PADDING); // On dégage de l'espace sur les côtés si possible
	while (resources.wData.bSize % 8) // Alignement sur 8
		resources.wData.bSize -= 1;
	resources.wData.offsetX = (resources.wData.sizeX / 2) - (resources.wData.bSize / 2);
	resources.wData.offsetY = (resources.wData.sizeY / 2) - (resources.wData.bSize / 2);
}

// Charge les textures [!] : ajouter un retour pour la vérification
void loadTextures(void) {
	static const char* paths[TEXTURES_LENGTH] = TEXTURES_PATHS;

	for (uint8_t i = 0 ; i < TEXTURES_LENGTH ; i++) { // [i] : Changer aussi dans destroyTextures
		char string[30] = "../themes/";
		strcat(string, CHESS_THEME);
		strcat(string, "/");
		strcat(string, paths[i]);
		strcat(string, ".bmp");

		SDL_Surface* surface = SDL_LoadBMP(string);

		resources.textures[i] = SDL_CreateTextureFromSurface(resources.wData.renderer, surface);
		SDL_FreeSurface(surface);

		if (i >= 12) {
			SDL_SetTextureBlendMode(resources.textures[i], SDL_BLENDMODE_BLEND);
			SDL_SetTextureAlphaMod(resources.textures[i], 200);
		}
	}
}

// Charge les sons
void loadSounds(void) {
	static const char* paths[SOUNDS_LENGTH] = SOUNDS_PATHS;

	for (uint8_t i = 0 ; i < SOUNDS_LENGTH ; i++) {
		resources.sounds[i] = malloc(sizeof(Sound));
		SDL_LoadWAV(paths[i], &resources.sounds[i]->spec, &resources.sounds[i]->buffer, &resources.sounds[i]->length);
	}
}

// Libère les sons
void destroySounds(void) {
	for (uint8_t i = 0 ; i < SOUNDS_LENGTH ; i++) {
		SDL_FreeWAV(resources.sounds[i]->buffer);
		free(resources.sounds[i]);
	}
}

// Joue un son
int playSound(Sound *sound) {
	SDL_PauseAudioDevice(resources.deviceId, 0);
	int success = SDL_QueueAudio(resources.deviceId, sound->buffer, sound->length);
	return success;
}

// Libère les textures de la mémoire
void destroyTextures(void) {
	for (uint8_t i = 0 ; i < TEXTURES_LENGTH ; i++) {
		SDL_DestroyTexture(resources.textures[i]);
	}
}

Coords coordsWindowToBoard(Coords c) {
	uint8_t tSize = resources.wData.bSize / 8;
	c.x -= resources.wData.offsetX;
	c.y -= resources.wData.offsetY;

	c = coords(abs((uint16_t) floor(c.x / tSize) - 7 * (!POV)), abs((uint16_t) floor(c.y / tSize) - 7 * (!POV)));
	return c;
}

Coords coords(int x, int y) {
	Coords c = {x, y};
	return c;
}

int8_t moveInList(CoordsArray *array, Coords m) {
	int8_t idx = -1;
	for (uint8_t i = 0 ; i < array->length ; i++) {
		if (array->moves[i].x == m.x && array->moves[i].y == m.y)
			idx = i;
	}
	return idx;
}

// Permet de localiser la première pièce rencontrée dans une certaine direction. On peut rechercher des caractéristiques précises.
Piece* searchOffsets(Game *game, Coords start, Coords offset, Team team, Type type) {
	start.x += offset.x;
	start.y += offset.y;

	while (!outOfBounds(&start)) {
		Piece* p = game->board[start.x][start.y];
		if (p != NULL) {
				if ((team != NO_TEAM && p->team != team) || (type != NO_TYPE && p->type != type))
					return NULL;
				return p;
		}

		start.x += offset.x;
		start.y += offset.y;
	}
	return NULL;
}

void pushWithFlag(Piece *piece, Coords move, Flag flag) {
	if (piece->moves.length < COORDS_ARRAY_LENGTH) {
		piece->moves.moves[piece->moves.length] = move;
		piece->flags[piece->moves.length] = flag;
		piece->moves.length++;
	}
}

// Supprime un mouvement d'une liste de mouvements possibles
void deleteMove(Game *game, Piece *piece, uint8_t index) {
	if (piece->moves.length > 0 && index < piece->moves.length) {
		memcpy(&piece->moves.moves[index], &piece->moves.moves[index+1], (piece->moves.length - index - 1) * sizeof(Coords));
		memcpy(&piece->flags[index], &piece->flags[index+1], (piece->moves.length - index - 1) * sizeof(Flag));
		piece->moves.length--;
		game->gData.moves[piece->team]--;
	}
}

// Enregistre un coup dans la liste de coups possibles d'une pièce, actualise le danger des cases et détecte les échecs au roi.
void registerMove(Game *game, Piece *piece, Coords m, Flag flag) {
	pushWithFlag(piece, m, flag);
	game->bitboard[!piece->team][m.x][m.y] = true;
	game->gData.moves[piece->team]++;

	Piece *p = game->board[m.x][m.y];
	if (p != NULL) {
		if (p->type == KING && p->team != piece->team)
			push(&game->gData.kingAttackers[p->team], piece->pos);
	}
}

bool moveCmp(Move m1, Move m2) {
	if (m1.src.x != m2.src.x || m1.src.y != m2.src.y || m1.dst.x != m2.dst.x || m1.dst.y != m2.dst.y || m1.flag != m2.flag)
		return false;
	return true;
}

bool isLackingMaterial(Game *game) {
	uint8_t minorCount[2] = {0};
	for (uint8_t x = 0 ; x < 8 ; x++) {
		for (uint8_t y = 0 ; y < 8 ; y++) {
			Piece *p = game->board[x][y];
			if (p != NULL) {
				if (p->type == PAWN || p->type == ROOK || p->type == QUEEN)
					return false;
				else if (p->type == BISHOP || p->type == KNIGHT)
					minorCount[p->team]++;
			}
		}
	}

	if (minorCount[0] <= 1 && minorCount[1] <= 1)
		return true;
	return false;
}

void loadResources(void) {
	resources.wData.sizeX = WINDOW_SIZE_X;
	resources.wData.sizeY = WINDOW_SIZE_Y;
	resources.wData.window = SDL_CreateWindow("Le jeu d'échecs", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_SIZE_X, WINDOW_SIZE_Y, SDL_WINDOW_RESIZABLE);
	resources.wData.renderer = SDL_CreateRenderer(resources.wData.window, -1, SDL_RENDERER_SOFTWARE);
	SDL_SetWindowMinimumSize(resources.wData.window, WINDOW_MIN_SIZE_X, WINDOW_MIN_SIZE_Y);

    loadTextures();

    loadSounds();

    resources.deviceId = SDL_OpenAudioDevice(NULL, 0, &resources.sounds[0]->spec, NULL, 0);

    updateWindowData();
}

void freeResources(void) {
	SDL_CloseAudioDevice(resources.deviceId);

    destroyTextures();
    destroySounds();
    SDL_DestroyRenderer(resources.wData.renderer);
    SDL_DestroyWindow(resources.wData.window);
}

Coords invertCoords(Coords c) {
	return coords(abs(c.x - 7 * (!POV)), abs(c.y - 7 * (!POV)));
}

uint8_t invertCoord(uint8_t c) {
	return abs(c - 7 * (!POV));
}