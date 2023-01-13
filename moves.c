#include "common.h"
#include "utils.h"
#include "display.h"
#include "moves.h"
#include "input.h"
#include "ai.h"

/* ----- ----- ----- ----- ----- GESTION DU ROI ----- ----- ----- ----- ----- */

// Permet d'obtenir tous les mouvements qui bloquent un échec au roi
CoordsArray getCheckRemovers(Game *game, Piece *attacker, Piece *king) {
	CoordsArray array;
	array.length = 0;
	short akx = attacker->pos.x - king->pos.x;
	short aky = attacker->pos.y - king->pos.y;

	for (uint8_t x = 0 ; x < 8 ; x++) {
		for (uint8_t y = 0 ; y < 8 ; y++) {
			Coords current = {x, y};

			short amx = attacker->pos.x - current.x;
			short amy = attacker->pos.y - current.y;

			// Calcul du déterminant
			float det = akx * amy - aky * amx;

			// Technique de calcul faite maison pour savoir si un point est entre deux autres.
			/*
			On note A et B les points qui font office de bornes, et C le point peut-être entre les deux.
			Si (Ax - Cx) * (Bx - Cx) est négatif, alors Cx est entre Ax et Bx.
			*/
			float calcX = (attacker->pos.x - current.x) * (king->pos.x - current.x);
			float calcY = (attacker->pos.y - current.y) * (king->pos.y - current.y);

			if (det == 0.0 && calcX <= 0 && calcY <= 0)
				push(&array, current);
		}
	}

	return array;
}

// Ne conserve que les mouvements autorisés dans une liste.
void restrictMoves(Game *game, CoordsArray *authorized, Piece *restricted) {
	for (uint8_t i = 0 ; i < restricted->moves.length ; i++) {
		Coords current = restricted->moves.moves[i];
		if (moveInList(authorized, current) == -1) {
			deleteMove(game, restricted, i);
			i--;
		}
	}
}

// Détecte si un roi est en échec.
bool inCheck(Game *game, Piece *king) {
    // On regarde si la case du roi est dangereuse.
    return game->bitboard[king->team][king->pos.x][king->pos.y];
}

// Restreint les mouvements des pièces clouées.
void applyPins(Game *game, Piece *king) {
    static Coords slidingOffsets[2][4] = {BISHOP_OFFSETS, ROOK_OFFSETS};
    Type seekedType = BISHOP;

    // On parcoure toutes les directions possibles.
    for (uint8_t i = 0 ; i < 8 ; i++) {
        Coords offsets = slidingOffsets[(int) floor(i / 4)][i%4];
        if (i > 3)
            seekedType = ROOK; // Passage aux mouvements de la tour.

        // On cherche deux pièces alignées.
        Piece *first = searchOffsets(game, king->pos, offsets, king->team, NO_TYPE);
        if (first != NULL) {
            Piece *second = searchOffsets(game, first->pos, offsets, !king->team, NO_TYPE);

            if (second != NULL) {
                if (second->type == seekedType || second->type == QUEEN) {
					CoordsArray authMoves = getCheckRemovers(game, second, king);
                    restrictMoves(game, &authMoves, first); // Application du clouage.
				}
            }
        }
    }
}

// Empêche le roi de se déplacer sur des cases dangereuses.
void limitKingMoves(Game *game, Piece *king) {
	for (uint8_t i = 0 ; i < king->moves.length ; i++) {

		Coords currentMove = king->moves.moves[i];
		if (game->bitboard[king->team][currentMove.x][currentMove.y] == true) {
			deleteMove(game, king, i);
			i--;
		}
	}
}

// Interdit les mouvements qui laissent le roi en échec.
void forceCheckEvasion(Game *game, Piece *king) {
	CoordsArray attackers = game->gData.kingAttackers[king->team];

	CoordsArray authMoves = getCheckRemovers(game, game->board[attackers.moves[0].x][attackers.moves[0].y], king);
	for (uint8_t x = 0 ; x < 8 ; x++) {
		for (uint8_t y = 0 ; y < 8 ; y++) {
			Piece *current = game->board[x][y];
			if (current != NULL) {
				if (current->team == king->team && current->type != KING) {
					if (attackers.length == 1)
						restrictMoves(game, &authMoves, current);
					else
						current->moves.length = 0;
				}
			}
		}
	}
}

// Détecte une fin de partie (pat, échec et mat).
int gameOver(Game *game) {
	/*
	0 : partie non terminée
	1 : pat
	2 : échec et mat du roi blanc
	3 : échec et mat du roi noir
	*/
	if (game->gData.repeatCount >= 6 || isLackingMaterial(game))
		return 1;
	
	if (game->gData.moves[WHITE] == 0 && game->turn == WHITE) {
		if (inCheck(game, whiteKing))
			return 2;
		return 1;
	}
	if (game->gData.moves[BLACK] == 0 && game->turn == BLACK) {
		if (inCheck(game, blackKing))
			return 3;
		return 1;
	}

	return 0;
}


/* ----- ----- ----- ----- ----- MOUVEMENTS PSEUDO-AUTORISES ----- ----- ----- ----- ----- */

void slidingMoves(Game* game, Piece* piece, Coords offsets[4]) {

	for (uint8_t offsetIdx = 0 ; offsetIdx < 4 ; offsetIdx++) {
		Coords cursor = piece->pos;
        cursor.x += offsets[offsetIdx].x;
        cursor.y += offsets[offsetIdx].y;

        while (!outOfBounds(&cursor)) {

            Piece *p = game->board[cursor.x][cursor.y];
            game->bitboard[!piece->team][cursor.x][cursor.y] = true;
			if (p != NULL) {
				if (p->team != piece->team) {
					registerMove(game, piece, cursor, NO_FLAG);
					
					if (p->type == KING) {
						cursor.x += offsets[offsetIdx].x;
						cursor.y += offsets[offsetIdx].y;
						if (!outOfBounds(&cursor))
							game->bitboard[!piece->team][cursor.x][cursor.y] = true;
					}
				}
				break;
			}
            else
                registerMove(game, piece, cursor, NO_FLAG);

			cursor.x += offsets[offsetIdx].x;
            cursor.y += offsets[offsetIdx].y;
        }
    }
}

void genKnightMoves(Game* game, Piece* knight) {
	static Coords offsets[8] = KNIGHT_OFFSETS;

	for (uint8_t i = 0 ; i < 8 ; i++) {
		Coords dst = knight->pos;
		dst.x += offsets[i].x;
		dst.y += offsets[i].y;

		if (!outOfBounds(&dst)) {
			Piece *p = game->board[dst.x][dst.y];
			// On ne va pas sur les cases alliées
			game->bitboard[!knight->team][dst.x][dst.y] = true;
			if (p != NULL) {
				if (p->team == knight->team)
					continue;
			}

			registerMove(game, knight, dst, NO_FLAG);
		}
	}
}

void genRookMoves(Game* game, Piece* rook) {
	slidingMoves(game, rook, (Coords[]) ROOK_OFFSETS);
}

void genBishopMoves(Game* game, Piece* bishop) {
	slidingMoves(game, bishop, (Coords[]) BISHOP_OFFSETS);
}

void genQueenMoves(Game* game, Piece* queen) {
	slidingMoves(game, queen, (Coords[]) ROOK_OFFSETS);
    slidingMoves(game, queen, (Coords[]) BISHOP_OFFSETS);
}

void genKingMoves(Game* game, Piece* king) {
	// Calcul des mouvements de base
	Piece *p;
	Coords dst;
	for (int8_t offsetX = -1 ; offsetX < 2 ; offsetX++) {
		for (int8_t offsetY = -1 ; offsetY < 2 ; offsetY++) {

			dst = coords(king->pos.x + offsetX, king->pos.y + offsetY);
			if (!outOfBounds(&dst)) {
				game->bitboard[!king->team][dst.x][dst.y] = true;
				p = game->board[dst.x][dst.y];
				if (p != NULL) {
					if (p->team == king->team)
						continue;
				}

				registerMove(game, king, dst, NO_FLAG);
			}
		}
	}

	// Roques
	if (king->mCount == 0 && king->pos.x == 4 && !inCheck(game, king)) {

		// On trouve la distance à la tour pour les deux roques.
		for (int8_t rookOffset = -4 ; rookOffset < 4 ; rookOffset += 7) {
			Piece *p = game->board[king->pos.x + rookOffset][king->pos.y];
			if (p != NULL) {
				if (p->type == ROOK && p->mCount == 0) {

					bool canCastle = true;
					for (int8_t c = 1 ; c < abs(rookOffset) ; c++) {
						Coords curLocation = coords(king->pos.x + c * (rookOffset / abs(rookOffset)), king->pos.y);
						if (game->board[curLocation.x][curLocation.y] != NULL || game->bitboard[king->team][curLocation.x][curLocation.y] == true)
							canCastle = false;
					}

					if (canCastle)
						registerMove(game, king, coords(king->pos.x + (rookOffset / abs(rookOffset) * 2), king->pos.y), CASTLE);
				}
			}
		}
	}
}

void genPawnMoves(Game *game, Piece *pawn) {
	int8_t inc = (pawn->team * -2) + 1; // Direction de mouvement y du pion
	uint8_t startingPos = pawn->team * 5 + 1; // Rangée de départ

	// Droit devant
	Coords dst = {pawn->pos.x, pawn->pos.y + inc};
	if (!outOfBounds(&dst)) {
		if (game->board[dst.x][dst.y] == NULL) {
			bool state = game->bitboard[!pawn->team][dst.x][dst.y];
			registerMove(game, pawn, dst, NO_FLAG);
			game->bitboard[!pawn->team][dst.x][dst.y] = state;

			dst.y += inc; // Plus loin
			if (pawn->pos.y == startingPos && game->board[dst.x][dst.y] == NULL) {
				bool state = game->bitboard[!pawn->team][dst.x][dst.y];
				registerMove(game, pawn, dst, NO_FLAG);
				game->bitboard[!pawn->team][dst.x][dst.y] = state;
			}
		}
	}

	// Mouvements diagonaux
	for (int8_t i = -1 ; i < 2 ; i += 2) {
		dst = coords(pawn->pos.x + i, pawn->pos.y + inc);
		if (outOfBounds(&dst))
			continue;
		game->bitboard[!pawn->team][dst.x][dst.y] = true;

		// Diagonales classiques
		Piece* p = game->board[dst.x][dst.y];
		if (p != NULL) {
			if (p->team != pawn->team)
				registerMove(game, pawn, dst, NO_FLAG);
		}

		// En passant
		p = game->board[dst.x][dst.y-inc];
		if (p != NULL) {
			if (p->team != pawn->team && p->mCount == (unsigned) 1 && p->pos.y == (signed) p->team + 3 && p->type == PAWN) {
				if (game->gData.lastMoves[0].dst.x == dst.x && game->gData.lastMoves[0].dst.y == dst.y-inc) // Si le mouvement vient d'être joué
					registerMove(game, pawn, dst, EN_PASSANT);
			}
		}
	}
}

/* ----- ----- ----- ----- ----- UTILITAIRES ----- ----- ----- ----- ----- */

bool move(Game *game, Coords src, Coords dst, Flag flag) {
	game->board[dst.x][dst.y] = game->board[src.x][src.y];
	game->board[src.x][src.y] = NULL;

	Piece *p = game->board[dst.x][dst.y];
	p->pos = dst;
	p->mCount++;
	p->moves.length = 0;
	game->gData.moveCount++;

	if (flag == CASTLE) {
		if (dst.x > src.x) { // Petit roque
			game->board[5][dst.y] = game->board[7][dst.y];
			game->board[7][dst.y] = NULL;
			game->board[5][dst.y]->pos = coords(5, dst.y);
		}
		else { // Grand roque
			game->board[3][dst.y] = game->board[0][dst.y];
			game->board[0][dst.y] = NULL;
			game->board[3][dst.y]->pos = coords(3, dst.y);
		}
	}
	else if (flag == EN_PASSANT) {
		game->board[dst.x][dst.y - ((p->team * -2) + 1)] = NULL;
	}

	// Promotion
	if (p->type == PAWN && p->pos.y == (signed) (7 - p->team * 7)) {
		if (!onEstDansLeTurfu)
			p->type = promoteDialog(game);
		else
			p->type = QUEEN;
	}

	// [?] : Créer une fonction séparée pour l'historique des coups ?
	// On actualise l'historique des coups
	Move m = {src, dst, flag};

	if (moveCmp(m, game->gData.lastMoves[3]))
		game->gData.repeatCount++;
	else
		game->gData.repeatCount = 0;

	game->gData.lastMoves[3] = game->gData.lastMoves[2];
	game->gData.lastMoves[2] = game->gData.lastMoves[1];
	game->gData.lastMoves[1] = game->gData.lastMoves[0];
	game->gData.lastMoves[0] = m;

	return 0;
}

void genMoves(Game *game, Piece *piece) {
	piece->moves.length = 0;
	switch (piece->type) {
		case PAWN:
			genPawnMoves(game, piece);
			break;
		case KNIGHT:
			genKnightMoves(game, piece);
			break;
		case BISHOP:
			genBishopMoves(game, piece);
			break;
		case ROOK:
			genRookMoves(game, piece);
			break;
		case QUEEN:
			genQueenMoves(game, piece);
			break;
		case KING:
			genKingMoves(game, piece);
			break;
		default:
			break;
	}
}

void genAllMoves(Game *game) {
	for (uint8_t x = 0 ; x < 8 ; x++) {
		for (uint8_t y = 0 ; y < 8 ; y++) {
			if (game->board[x][y] != NULL) {
				if (game->board[x][y]->type != KING)
					genMoves(game, game->board[x][y]);
			}
		}
	}

	whiteKing->moves.length = 0;
	blackKing->moves.length = 0;
	genKingMoves(game, whiteKing);
	genKingMoves(game, blackKing);
}

void update(Game *game) {
	game->gData.kingAttackers[0].length = 0;
	game->gData.kingAttackers[1].length = 0;
	game->gData.moves[0] = 0;
	game->gData.moves[1] = 0;
	clearBitboard(game);
	genAllMoves(game);
	limitKingMoves(game, whiteKing);
	limitKingMoves(game, blackKing);

	applyPins(game, whiteKing);
	applyPins(game, blackKing);

	if (inCheck(game, whiteKing) == true)
		forceCheckEvasion(game, whiteKing);
	if (inCheck(game, blackKing) == true)
		forceCheckEvasion(game, blackKing);

	game->turn = !game->turn;

	game->gData.gameState = gameOver(game);
}