#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#define POSITIVE_INFINITY 9999
#define NEGATIVE_INFINITY -9999

#define COORDS_ARRAY_LENGTH 27

#define MIN_WINDOW_PADDING 0.1
#define WINDOW_MIN_SIZE_X 200
#define WINDOW_MIN_SIZE_Y 200
#define WINDOW_SIZE_X 600
#define WINDOW_SIZE_Y 600

#define TILE_CONTRAST 55

#define PAWN_VALUE 1
#define KNIGHT_VALUE 3
#define BISHOP_VALUE 3
#define ROOK_VALUE 5
#define QUEEN_VALUE 9
#define KING_VALUE POSITIVE_INFINITY

// Les modifications de la position de départ peuvent rendre le jeu instable.
#define STARTING_POSITION "rnbqkbnrpppppppp                                PPPPPPPPRNBQKBNR"
#define CHESS_THEME "classic"
#define START_POV WHITE

#define KNIGHT_OFFSETS {{2, 1}, {-2, 1}, {2, -1}, {-2, -1}, {1, 2}, {1, -2}, {-1, 2}, {-1, -2}}
#define BISHOP_OFFSETS {{1, 1}, {-1, 1}, {-1, -1}, {1, -1}}
#define ROOK_OFFSETS {{1, 0}, {-1, 0}, {0, 1}, {0, -1}}

#define TILES_COLORS {{240, 240, 240}, {35, 65, 117}}
#define TEXTURES_PATHS {"BP","BN","BB","BR","BQ","BK", "WP","WN","WB","WR","WQ","WK","END","INPUT"};
#define SOUNDS_PATHS {"dep.wav", "impact.wav"}
#define SOUNDS_LENGTH 2
#define TEXTURES_LENGTH 14

#define MOUSE_MOTION_TOLERANCE 5

#define AI_DEPTH 4
#define RANDOMNESS_WEIGHT 0.3

#endif