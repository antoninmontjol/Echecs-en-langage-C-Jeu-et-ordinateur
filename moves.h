#ifndef __MOVES_H__
#define __MOVES_H__

bool inCheck(Game *game, Piece *king);
void applyPins(Game *game, Piece *king);
void limitKingMoves(Game *game, Piece *king);
void forceCheckEvasion(Game *game, Piece *king);
int gameOver(Game *game);
void genMoves(Game *game, Piece *piece);
bool move(Game *game, Coords src, Coords dst, Flag flag);
void update(Game *game);

#endif