#include "common.h"
#include "utils.h"
#include "display.h"

int renderBoard(Game *game) {
	if (SDL_RenderClear(resources.wData.renderer))
	 	return 1;
	
	// On dessine le fond
	SDL_Rect background = {0, 0, resources.wData.sizeX, resources.wData.sizeY};
	if (SDL_SetRenderDrawColor(resources.wData.renderer, 30, 30, 30, 255))
		return 1;
	if (SDL_RenderFillRect(resources.wData.renderer, &background))
		return 1;

	uint8_t tileSize = resources.wData.bSize / 8;
	uint8_t color;
	uint8_t colors[2][3] = TILES_COLORS;

	for (uint8_t x = 0 ; x < 8 ; x++) {
		for (uint8_t y = 0 ; y < 8 ; y++) {
			uint8_t displayX = invertCoord(x);
			uint8_t displayY = invertCoord(y);
			color = ((displayX+displayY) % 2);
			if (SDL_SetRenderDrawColor(resources.wData.renderer, colors[color][0], colors[color][1], colors[color][2], 255))
				return 1;

			SDL_Rect rect = {resources.wData.offsetX + (displayX * tileSize), resources.wData.offsetY + (displayY * tileSize), tileSize, tileSize};
			if (SDL_RenderFillRect(resources.wData.renderer, &rect))
				return 1;

			if (game->board[x][y] != NULL) {
				// Dessiner la pièce
				SDL_Texture *texture = resources.textures[getPieceIndex(game->board[x][y]) + (game->board[x][y]->team * 6)];
				if (SDL_RenderCopy(resources.wData.renderer, texture, NULL, &rect))
					return 1;
			}
		}
	}

	if (game->gData.gameState != 0) {
		if (renderTextureFullscreen(resources.textures[12]))
			return 1;
	}

	return 0;
}

int highlightTiles(CoordsArray *tiles, uint8_t color[4]) {
	if (SDL_SetRenderDrawBlendMode(resources.wData.renderer, SDL_BLENDMODE_BLEND) || SDL_SetRenderDrawColor(resources.wData.renderer, color[0], color[1], color[2], color[3]))
		return 1;

	const uint16_t tileSize = resources.wData.bSize / 8;
	for (uint8_t i = 0 ; i < tiles->length ; i++) {
		SDL_Rect rect = {resources.wData.offsetX + invertCoord(tiles->moves[i].x) * tileSize, resources.wData.offsetY + invertCoord(tiles->moves[i].y) * tileSize, tileSize, tileSize};

		if (SDL_RenderFillRect(resources.wData.renderer, &rect))
			return 1;
	}

	return 0;
}

int updateDisplay(Game *game, Piece *selected, Coords *draggedPiecePos) {
	if (renderBoard(game))
		return 1;

	if (game->gData.moveCount > 0) {
		// Dernier coup joué
		CoordsArray array;
		array.length = 2;
		array.moves[0] = game->gData.lastMoves[0].src;
		array.moves[1] = game->gData.lastMoves[0].dst;
		if (highlightTiles(&array, (uint8_t[]) {255, 255, 0, 75}))
			return 1;
	}

	// Mouvements proposés
	if (selected != NULL) {
		if (highlightTiles(&selected->moves, (uint8_t[]) {20, 200, 200, 150}))
			return 1;
	}

	// Fantôme de la pièce déplacée
	if (draggedPiecePos != NULL) {
		if (renderPiece(selected, draggedPiecePos))
			return 1;
	}

	SDL_RenderPresent(resources.wData.renderer);
	return 0;
}

int renderPiece(Piece *selected, Coords *pos) {
	// On admet que selected != NULL

	static SDL_Texture *pText;
	static uint8_t textIdx = -1;

	uint8_t idx = getPieceIndex(selected) + (selected->team * 6);
	if (idx != textIdx) {
		textIdx = idx;
		pText = resources.textures[idx];
	}

	uint8_t tSize = resources.wData.bSize / 8;
	SDL_Rect rect = {pos->x - (tSize / 2), pos->y - (tSize / 2), tSize, tSize};

	// [?] : Essayer de créer une autre textures sur laquelle on n'aurait pas besoin d'appliquer l'alpha plusieurs fois
	if (SDL_SetTextureAlphaMod(pText, 100))
		return 1;
	if (SDL_RenderCopy(resources.wData.renderer, pText, NULL, &rect) == -1)
		return 1;
	if (SDL_SetTextureAlphaMod(pText, 255))
		return 1;

	return 0;
}

int renderTextureFullscreen(SDL_Texture *texture) {
	SDL_Rect rect = {resources.wData.offsetX, resources.wData.offsetY, resources.wData.bSize, resources.wData.bSize};

	if (SDL_RenderCopy(resources.wData.renderer, texture, NULL, &rect) == -1)
		return 1;
	
	return 0;
}