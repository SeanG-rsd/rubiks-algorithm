#pragma once

#include "CubeState.h"

/*

18 possible moves
6 sides * 3 scenarios [rotate, inverse rotate, double rotate]

move = side * 3 + scenario, sides in URFDLB order (same as the facelet string)
0 = U, 1 = U', 2 = U2, 3 = R, 4 = R', 5 = R2, ... 17 = B2

*/

class MoveGenerator
{

public:
    static constexpr int MOVES = 18;

private:
    // after the move, slot j holds the piece that was in slot ...From[j],
    // with ...Flip[j] / ...Twist[j] added to that piece's orientation
    struct MoveTable
    {
        int edgeFrom[CubeState::EDGES];
        int edgeFlip[CubeState::EDGES];
        int cornerFrom[CubeState::CORNERS];
        int cornerTwist[CubeState::CORNERS];
    };

    MoveTable moves[MOVES];

    // the table for doing `first` and then `second`
    static MoveTable Then(const MoveTable& first, const MoveTable& second);

public:

    MoveGenerator();

    CubeState MakeMove(const CubeState& state, int move) const;

    // applies a space separated sequence like "R U R' U' F2"
    CubeState MakeMoves(const CubeState& state, const char* algorithm) const;

    // "U", "U'", "U2", "R", ...
    static const char* MoveName(int move);

};
