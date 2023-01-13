#ifndef __UI_H__
#define __UI_H__

bool mouseDown(Game *game, Piece **selected, Coords click);
bool mouseUp(Game *game, Piece **selected, Coords click);
Type promoteDialog(Game *game);

#endif