#ifndef __AI_H__
#define __AI_H__

int evalBoard(Game *game);
int minimax(Game *game, int depth, float alpha, float beta, Team minmax);
float evalPos(Piece *p);

#endif