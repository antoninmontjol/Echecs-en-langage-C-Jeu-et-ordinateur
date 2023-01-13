#include "chess.h"

Piece *pieces[32] = {NULL};
Piece *whiteKing = NULL;
Piece *blackKing = NULL;
Resources resources;
Move bestMove;
Team POV = START_POV;
bool onEstDansLeTurfu;

int main(int argc, char** argv) {
    Game game;
    game.gData.repeatCount = 0;
    Piece *selected = NULL;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_InitSubSystem(SDL_INIT_AUDIO);
    
    game.gData.kingAttackers[0].length = 0;
    game.gData.kingAttackers[1].length = 0;
    initBoard(&game, STARTING_POSITION);
    loadResources();

    renderBoard(&game);

    whiteKing = game.board[4][7];
    blackKing = game.board[4][0];

    // Permet de limiter les rafraîchissements de l'écran en ignorant des mouvements de souris minimes.
    int mmCount = 0;
    update(&game);
    game.turn = WHITE;
    
    while (true) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_QUIT:
                    goto leave;

                case SDL_WINDOWEVENT: // La fenêtre a changé de taille !
                    if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                        // Rafraîchir le rendu
                        resources.wData.sizeX = event.window.data1;
                        resources.wData.sizeY = event.window.data2;
                        updateWindowData();
                        updateDisplay(&game, selected, NULL);
                    }
                    break;

                case SDL_MOUSEMOTION:
                    mmCount++;
                    if (mmCount >= MOUSE_MOTION_TOLERANCE) {
                        if (selected != NULL) {
                            if (SDL_GetMouseState(NULL, NULL) == 1) {
                                int x = event.button.x;
                                int y = event.button.y;
                                Coords mouse = coords(x, y);

                                updateDisplay(&game, selected, &mouse);
                            }
                        }
                        mmCount = 0;
                    }

                    break;

                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) { // [!] : Déléguer à d'autres fonctions
                        Coords pos = coordsWindowToBoard(coords(event.button.x, event.button.y));

                        if (mouseDown(&game, &selected, pos))
                            playSound(resources.sounds[0]);

                        updateDisplay(&game, selected, NULL);
                    }
                    break;

                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT) { // [!] : Déléguer à d'autres fonctions
                        Coords pos = coordsWindowToBoard(coords(event.button.x, event.button.y));

                        if (mouseUp(&game, &selected, pos))
                            playSound(resources.sounds[0]);

                        updateDisplay(&game, selected, NULL);
                    }
                    break;
                
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_SPACE:
                            onEstDansLeTurfu = true;
                            minimax(&game, AI_DEPTH, NEGATIVE_INFINITY, POSITIVE_INFINITY, game.turn);
                            onEstDansLeTurfu = false;
                            move(&game, bestMove.src, bestMove.dst, bestMove.flag);
                            playSound(resources.sounds[0]);
                            update(&game);
                            updateDisplay(&game, NULL, NULL);
                            break;
                        case SDLK_TAB:
                            POV = !POV;
                            updateDisplay(&game, selected, NULL);
                            break;
                    }
                    break;

                default:
                    break;
            }

            if (game.gData.gameState != 0) {
                playSound(resources.sounds[1]);
                renderBoard(&game);
                SDL_RenderPresent(resources.wData.renderer);
                FILE *f = fopen("result.txt", "w");
                if (f != NULL) {
                    fprintf(f, "Légende des codes de fin de partie :\n1 : égalité\n2 : victoire des noirs\n3 : victoire des blancs\n");
                    fprintf(f, "Code de fin de partie : %d.\n", game.gData.gameState);
                    fclose(f);
                }
            }
        }
    }

    leave:
    destroyBoard(&game);

    freeResources();

    SDL_Quit();

    return 0;
}