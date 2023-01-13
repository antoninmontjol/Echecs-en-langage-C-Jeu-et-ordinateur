#include "common.h"
#include "moves.h"
#include "utils.h"
#include "input.h"
#include "display.h"

bool mouseDown(Game *game, Piece **selected, Coords click) {
    if (outOfBounds(&click))
        return false;
    Piece* clicked = game->board[click.x][click.y];

    if (*selected == NULL) {
        if (clicked != NULL && game->turn == clicked->team) {
            (*selected) = clicked;
        }
    }
    else {
        int8_t idx = moveInList(&(*selected)->moves, click);
        if (idx != -1) {
            move(game, (*selected)->pos, click, (*selected)->flags[idx]);
            update(game);
            *selected = NULL;
            return true;
        }
        else {
            if (clicked != NULL && game->turn == clicked->team) {
                (*selected) = clicked;
            }
            else
                *selected = NULL;
        }
    }
    return false;
}

bool mouseUp(Game *game, Piece **selected, Coords click) {
    if (outOfBounds(&click)) {
        *selected = NULL;
        return false;
    }

    if (*selected != NULL) {
        int8_t idx = moveInList(&(*selected)->moves, click);
        if (idx != -1) {
            move(game, (*selected)->pos, click, (*selected)->flags[idx]);
            update(game);
            *selected = NULL;
            return true;
        }
        else if (click.x != (*selected)->pos.x && click.y != (*selected)->pos.y) {
            *selected = NULL;
        }
    }
    return false;
}

Type promoteDialog(Game *game) {
    renderTextureFullscreen(resources.textures[13]);
    SDL_RenderPresent(resources.wData.renderer);
    while (true) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_KEYDOWN:;
                    SDL_Keycode keycode = event.key.keysym.sym;
                    switch (keycode) {
                        case SDLK_q:
                            return QUEEN;
                        case SDLK_n:
                            return KNIGHT;
                        case SDLK_r:
                            return ROOK;
                        case SDLK_b:
                            return BISHOP;
                    }
                    break;
                
                case SDL_WINDOWEVENT: // La fenêtre a changé de taille !
                    if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                        // Rafraîchir le rendu
                        resources.wData.sizeX = event.window.data1;
                        resources.wData.sizeY = event.window.data2;
                        updateWindowData();
                        renderBoard(game);
                        renderTextureFullscreen(resources.textures[13]);
                        SDL_RenderPresent(resources.wData.renderer);
                    }
                    break;
            }
        }
    }
}