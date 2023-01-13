#ifndef __UTILS_H__
#define __UTILS_H__

uint16_t getPieceValue(Piece* piece);
uint8_t getPieceIndex(Piece *piece);
void push(CoordsArray *array, Coords el);
void pop(CoordsArray *array, uint8_t index);
uint8_t outOfBounds(Coords* c);
void destroyBoard(Game* game);
void initBoard(Game* game, const char* sequence);
void updateWindowData(void);
void clearBitboard(Game *game);

int8_t moveInList(CoordsArray *array, Coords m);
int playSound(Sound *sound);

Coords coordsWindowToBoard(Coords c);
Coords coords(int x, int y);
Piece *searchOffsets(Game *game, Coords start, Coords offset, Team team, Type type);
void pushWithFlag(Piece *piece, Coords move, Flag flag);
void deleteMove(Game *game, Piece *piece, uint8_t index);
void registerMove(Game *game, Piece *piece, Coords m, Flag flag);
bool moveCmp(Move m1, Move m2);
bool isLackingMaterial(Game *game);
void loadResources(void);
void freeResources(void);
Coords invertCoords(Coords c);
uint8_t invertCoord(uint8_t c);

#endif