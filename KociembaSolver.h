#pragma once

#include <memory>
#include <string>
#include "CubeState.h"
#include "MoveGenerator.h"

/*

Kociemba's two-phase algorithm

phase 1: get the cube into the group <U, D, R2, L2, F2, B2>. that means every corner twist and
         edge flip is 0, and the four middle layer edges (FR, FL, BL, BR) are in the middle layer
phase 2: solve the rest using only those moves, so phase 1's work is never undone

both phases are IDA* searches. each phase boils the cube down to a few small numbers (coordinates),
move tables say what every move does to each coordinate, and pruning tables give the fewest moves
needed to finish the phase, so branches that can't finish in time get cut early.

after the first solution it keeps looking for shorter ones (a longer phase 1 can allow a much
shorter phase 2)

*/

class KociembaSolver
{

private:
    struct Tables;
    struct Search;

    MoveGenerator generator;
    std::unique_ptr<Tables> tables;

    int Phase1Distance(int twist, int flip, int slice) const;
    bool Phase1(Search& search, int twist, int flip, int slice, int depth, int togo) const;
    bool StartPhase2(Search& search, int depth) const;
    bool Phase2(Search& search, int cornerPerm, int edgePerm, int slicePerm, int depth, int togo) const;

public:

    // builds the move and pruning tables (about 12 MB), takes a moment
    KociembaSolver();
    ~KociembaSolver();

    // returns the moves separated by spaces, e.g. "R U2 F' D", or "" if the cube is already solved.
    // returns the shortest solution found, stopping once it has one of maxLength moves or fewer
    // or once timeoutSeconds runs out (it always keeps going until it has at least one solution)
    std::string Solve(const CubeState& state, int maxLength = 20, double timeoutSeconds = 3.0) const;

};
