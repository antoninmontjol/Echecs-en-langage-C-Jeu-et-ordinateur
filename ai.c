#include "common.h"
#include "moves.h"
#include "utils.h"
#include "ai.h"
#include <time.h>

int evalBoard(Game *game) {
    int eval = 0;
    for (uint8_t x = 0 ; x < 8 ; x++) {
        for (uint8_t y = 0 ; y < 8 ; y++) {
            Piece *p = game->board[x][y];
            if (p != NULL)
                eval += getPieceValue(p) * (p->team * 2 - 1);
        }
    }

    return eval;
}

float centerPos(Piece *p) {
    return (-0.14 * fabsf(3.5 - p->pos.x) + 0.5) + (-0.14 * fabsf(3.5 - p->pos.y) + 0.5);
}

float cornerPos(Piece *p) {
    return 0.14 * fabsf(3.5 - p->pos.x) + 0.14 * fabsf(3.5 - p->pos.y);
}

float allyPos(Piece *p) {
    return fabsf((p->team * -7 + 7) - p->pos.y) / 7;
}

float ennemyPos(Piece *p) {
    return fabsf((p->team * 7) - p->pos.y) / 7;
}

float evalKnightPos(Piece *p) {
    return centerPos(p) * 0.8 + ennemyPos(p) * 0.2;
}

float evalRookPos(Piece *p) {
    return allyPos(p) * 0.4 + ennemyPos(p) * 0.5 + centerPos(p) * 0.1;
}

float evalKingPos(Piece *p) {
    return cornerPos(p) * 0.6 + allyPos(p) * 0.4;
}

float evalBishopPos(Piece *p) {
    return centerPos(p) * 0.4 + cornerPos(p) * 0.4 + allyPos(p) * 0.2;
}

float evalQueenPos(Piece *p) {
    return centerPos(p) * 0.6 + ennemyPos(p) * 0.4;
}

float evalPawnPos(Piece *p) {
    return centerPos(p) * 0.1 + ennemyPos(p) * 0.9;
}

float evalPos(Piece *p) {
    switch (p->type) {
        case KING:
            return evalKingPos(p);
            break;
        case QUEEN:
            return evalQueenPos(p);
            break;
        case ROOK:
            return evalRookPos(p);
            break;
        case KNIGHT:
            return evalKnightPos(p);
            break;
        case BISHOP:
            return evalBishopPos(p);
            break;
        case PAWN:
            return evalPawnPos(p);
            break;
        default:
            return 0.0;
    }
}

int minimax(Game *game, int depth, float alpha, float beta, Team minmax) {
    if (depth == 0)
        return evalBoard(game);
    
    // On harmonisera pour le minimiseur
    int max = NEGATIVE_INFINITY / 3;
    float maxPosGain = NEGATIVE_INFINITY;
    if (minmax == BLACK)
        max = POSITIVE_INFINITY / 3;
    int eval = evalBoard(game);
    if ((eval >= 9900) || (eval <= -9900))
        return eval;

    bool stopFlag = false;
    Move best;
    for (uint8_t x = 0 ; x < 8 ; x++) {
        for (uint8_t y = 0 ; y < 8 ; y++) {
            Piece *p = game->board[x][y];
            if (p != NULL) {
                if (p->team == minmax) {
                    for (uint8_t i = 0 ; i < p->moves.length ; i++) {
                        // Pour chaque coup possible
                        Move m = {p->pos, p->moves.moves[i], p->flags[i]};
                        float posBefore = evalPos(p);

                        Piece save[32];
                        for (uint8_t i = 0 ; i < 32 ; i++)
                            save[i] = *pieces[i];

                        Game new = *game;
                        move(&new, m.src, m.dst, m.flag);
                        update(&new);
 
                        // Aplication de l'aléatoire
                        srand(time(NULL));
                        float random = ((double) rand() / (double) RAND_MAX) * RANDOMNESS_WEIGHT;

                        float posGain = evalPos(p) - posBefore + random;
                        switch (new.gData.gameState) {
                            case 0:
                                eval = minimax(&new, depth-1, alpha, beta, !minmax);
                                break;
                            case 1:
                                eval = evalBoard(&new) * -0.2;
                                break;
                            case 2:
                                eval = NEGATIVE_INFINITY;
                                break;
                            case 3:
                                eval = POSITIVE_INFINITY;
                                break;
                        }
                        
                        if (((eval > max) || (eval == max && posGain > maxPosGain)) && minmax == WHITE) {
                            max = eval;
                            best = m;
                            maxPosGain = posGain;
                            
                            alpha = eval + maxPosGain / 5;
                            if (beta < alpha)
                                stopFlag = true;
                        }
                        stopFlag = false;

                        if (((eval < max) || (eval == max && posGain > maxPosGain)) && minmax == BLACK) {
                            max = eval;
                            best = m;
                            maxPosGain = posGain;
                            
                            beta = eval - maxPosGain / 5;
                            if (beta < alpha)
                                stopFlag = true;
                        }
                        stopFlag = false;

                        for (uint8_t i = 0 ; i < 32 ; i++)
                            *pieces[i] = save[i];
                        if (stopFlag == true)
                            goto end;
                    }
                }
            }
        }
    }

    end:
    if (depth == AI_DEPTH)
        bestMove = best;
    return max;
}