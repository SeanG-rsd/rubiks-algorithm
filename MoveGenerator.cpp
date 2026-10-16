
#include <cstring>
#include <stdexcept>
#include <string>
#include "MoveGenerator.h"

namespace
{
    // a clockwise quarter turn of one side, looking straight at that side
    struct QuarterTurn
    {
        int edges[4];       // the edge in edges[k] moves to edges[k + 1], and the last one back to edges[0]
        int corners[4];     // same for corners
        int edgeFlip;       // F and B quarter turns flip the four edges they move
        int cornerTwist[4]; // twist added to the corner that lands in corners[k]
    };

    // slot numbers are from Maps.h, sides in URFDLB order
    const QuarterTurn QUARTER_TURNS[6] =
        {
            { {0, 1, 2, 3},   {0, 1, 2, 3}, 0, {0, 0, 0, 0} }, // U: UF > UL > UB > UR    UFR > UFL > UBL > UBR
            { {4, 3, 7, 11},  {4, 0, 3, 7}, 0, {1, 2, 1, 2} }, // R: FR > UR > BR > DR    DFR > UFR > UBR > DBR
            { {0, 4, 8, 5},   {0, 4, 5, 1}, 1, {1, 2, 1, 2} }, // F: UF > FR > DF > FL    UFR > DFR > DFL > UFL
            { {8, 11, 10, 9}, {4, 7, 6, 5}, 0, {0, 0, 0, 0} }, // D: DF > DR > DB > DL    DFR > DBR > DBL > DFL
            { {1, 5, 9, 6},   {1, 5, 6, 2}, 0, {1, 2, 1, 2} }, // L: UL > FL > DL > BL    UFL > DFL > DBL > UBL
            { {2, 6, 10, 7},  {3, 2, 6, 7}, 1, {2, 1, 2, 1} }, // B: UB > BL > DB > BR    UBR > UBL > DBL > DBR
        };

    const char SIDES[] = "URFDLB";

    const char* const MOVE_NAMES[MoveGenerator::MOVES] =
        {
            "U", "U'", "U2", "R", "R'", "R2", "F", "F'", "F2",
            "D", "D'", "D2", "L", "L'", "L2", "B", "B'", "B2"
        };
}

MoveGenerator::MoveGenerator()
{
    for (int side = 0; side < 6; side++)
    {
        const QuarterTurn& turn = QUARTER_TURNS[side];
        MoveTable quarter;

        for (int slot = 0; slot < CubeState::EDGES; slot++)
        {
            quarter.edgeFrom[slot] = slot;
            quarter.edgeFlip[slot] = 0;
        }
        for (int slot = 0; slot < CubeState::CORNERS; slot++)
        {
            quarter.cornerFrom[slot] = slot;
            quarter.cornerTwist[slot] = 0;
        }

        for (int k = 0; k < 4; k++)
        {
            int next = (k + 1) % 4;

            quarter.edgeFrom[turn.edges[next]] = turn.edges[k];
            quarter.edgeFlip[turn.edges[next]] = turn.edgeFlip;

            quarter.cornerFrom[turn.corners[next]] = turn.corners[k];
            quarter.cornerTwist[turn.corners[k]] = turn.cornerTwist[k];
        }

        MoveTable twice = Then(quarter, quarter);

        moves[side * 3 + 0] = quarter;
        moves[side * 3 + 1] = Then(twice, quarter); // three quarter turns = one inverse turn
        moves[side * 3 + 2] = twice;
    }
}

MoveGenerator::MoveTable MoveGenerator::Then(const MoveTable& first, const MoveTable& second)
{
    MoveTable result;

    for (int slot = 0; slot < CubeState::EDGES; slot++)
    {
        int from = second.edgeFrom[slot];

        result.edgeFrom[slot] = first.edgeFrom[from];
        result.edgeFlip[slot] = (first.edgeFlip[from] + second.edgeFlip[slot]) % 2;
    }

    for (int slot = 0; slot < CubeState::CORNERS; slot++)
    {
        int from = second.cornerFrom[slot];

        result.cornerFrom[slot] = first.cornerFrom[from];
        result.cornerTwist[slot] = (first.cornerTwist[from] + second.cornerTwist[slot]) % 3;
    }

    return result;
}

CubeState MoveGenerator::MakeMove(const CubeState& state, int move) const
{
    const MoveTable& table = moves[move];

    std::uint64_t edgeState = 0;

    for (int slot = 0; slot < CubeState::EDGES; slot++)
    {
        std::uint64_t field = (state.GetEdgeState() >> (table.edgeFrom[slot] * 5)) & 0b11111;

        field ^= static_cast<std::uint64_t>(table.edgeFlip[slot]) << 4;

        edgeState |= field << (slot * 5);
    }

    std::uint64_t cornerState = 0;

    for (int slot = 0; slot < CubeState::CORNERS; slot++)
    {
        std::uint64_t field = (state.GetCornerState() >> (table.cornerFrom[slot] * 5)) & 0b11111;

        std::uint64_t piece = field & 0b111;
        std::uint64_t twist = ((field >> 3) + static_cast<std::uint64_t>(table.cornerTwist[slot])) % 3;

        cornerState |= ((twist << 3) | piece) << (slot * 5);
    }

    return CubeState(edgeState, cornerState);
}

CubeState MoveGenerator::MakeMoves(const CubeState& state, const char* algorithm) const
{
    CubeState result = state;

    for (const char* c = algorithm; *c != '\0'; c++)
    {
        if (*c == ' ')
        {
            continue;
        }

        const char* side = strchr(SIDES, *c);

        if (side == nullptr)
        {
            throw std::invalid_argument(std::string("unknown move: ") + *c);
        }

        int move = static_cast<int>(side - SIDES) * 3;

        if (c[1] == '\'')
        {
            move += 1;
            c++;
        }
        else if (c[1] == '2')
        {
            move += 2;
            c++;
        }

        result = MakeMove(result, move);
    }

    return result;
}

const char* MoveGenerator::MoveName(int move)
{
    return MOVE_NAMES[move];
}
