#ifndef __DISPLAY_H__
#define __DISPLAY_H__

int renderBoard(Game *game);
int highlightTiles(CoordsArray *tiles, uint8_t color[4]);
int updateDisplay(Game *game, Piece *selected, Coords *draggedPiecePos);
int renderPiece(Piece *selected, Coords *pos);
int renderTextureFullscreen(SDL_Texture *texture);

#endif