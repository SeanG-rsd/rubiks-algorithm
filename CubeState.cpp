
#include <cstring>
#include <stdexcept>
#include "CubeState.h"
#include "Maps.h"

// UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB

// 0 if the pieces are an even permutation of their slots, 1 if odd
static int PermutationParity(const int* pieces, int count)
{
    int parity = 0;

    for (int i = 0; i < count; i++)
    {
        for (int j = i + 1; j < count; j++)
        {
            if (pieces[i] > pieces[j])
            {
                parity ^= 1;
            }
        }
    }

    return parity;
}

CubeState::CubeState(const char* cubeState)
{
    if (cubeState == nullptr || strlen(cubeState) != 54)
    {
        throw std::invalid_argument("cube state must be 54 characters");
    }

    // the maps assume every center is its own face's color
    for (int center = 4; center < 54; center += 9)
    {
        if (cubeState[center] != Maps::index_to_color[center])
        {
            throw std::invalid_argument("centers must be in URFDLB order");
        }
    }

    std::uint64_t edgeState = 0;
    int edgePieces[EDGES];
    int flipTotal = 0;
    int seen = 0;

    for (int slot = 0; slot < EDGES; slot++)
    {
        char e1 = cubeState[Maps::edges[slot][0]];
        char e2 = cubeState[Maps::edges[slot][1]];

        int piece = -1;
        int flip = 0;

        for (int p = 0; p < EDGES && piece < 0; p++)
        {
            char check1 = Maps::index_to_color[Maps::edges[p][0]]; // the correct/final value of this position
            char check2 = Maps::index_to_color[Maps::edges[p][1]];

            if (e1 == check1 && e2 == check2)
            {
                piece = p;
                flip = 0;
            }
            else if (e1 == check2 && e2 == check1)
            {
                piece = p;
                flip = 1;
            }
        }

        if (piece < 0 || (seen & (1 << piece)))
        {
            throw std::invalid_argument("edges don't match a real cube");
        }

        seen |= 1 << piece;
        edgePieces[slot] = piece;
        flipTotal += flip;

        edgeState |= static_cast<std::uint64_t>((flip << 4) | piece) << (slot * 5);
    }

    std::uint64_t cornerState = 0;
    int cornerPieces[CORNERS];
    int twistTotal = 0;
    seen = 0;

    for (int slot = 0; slot < CORNERS; slot++)
    {
        const int* facelets = Maps::corners[slot];

        // find which facelet has the U/D color
        int twist = 0;
        while (twist < 3 && cubeState[facelets[twist]] != 'U' && cubeState[facelets[twist]] != 'D')
        {
            twist++;
        }

        int piece = -1;

        if (twist < 3)
        {
            // read the colors clockwise starting from the U/D one, so they line up with the piece's solved colors
            char c1 = cubeState[facelets[twist]];
            char c2 = cubeState[facelets[(twist + 1) % 3]];
            char c3 = cubeState[facelets[(twist + 2) % 3]];

            for (int p = 0; p < CORNERS && piece < 0; p++)
            {
                if (
                    c1 == Maps::index_to_color[Maps::corners[p][0]]
                    && c2 == Maps::index_to_color[Maps::corners[p][1]]
                    && c3 == Maps::index_to_color[Maps::corners[p][2]]
                ) {
                    piece = p;
                }
            }
        }

        if (piece < 0 || (seen & (1 << piece)))
        {
            throw std::invalid_argument("corners don't match a real cube");
        }

        seen |= 1 << piece;
        cornerPieces[slot] = piece;
        twistTotal += twist;

        cornerState |= static_cast<std::uint64_t>((twist << 3) | piece) << (slot * 5);
    }

    // states you can only get by taking the cube apart
    if (flipTotal % 2 != 0)
    {
        throw std::invalid_argument("an edge is flipped");
    }
    if (twistTotal % 3 != 0)
    {
        throw std::invalid_argument("a corner is twisted");
    }
    if (PermutationParity(edgePieces, EDGES) != PermutationParity(cornerPieces, CORNERS))
    {
        throw std::invalid_argument("two pieces are swapped");
    }

    this->EdgeState = edgeState;
    this->CornerState = cornerState;

    // edges

    // 11: R8-D6
    // 10: B8-D8
    // 09: L8-D4
    // 08: F8-D2
    // 07: R6-B4
    // 06: B6-L4
    // 05: L6-F4
    // 04: F6-R4
    // 03: U6-R2
    // 02: U2-B2
    // 01: U4-L2
    // 00: U8-F2

    // corners

    // 7: D9-R9-B7
    // 6: D7-L7-B9
    // 5: F7-L9-D1
    // 4: F9-R7-D3
    // 3: U3-B1-R3
    // 2: U1-L1-B3
    // 1: U7-F1-L3
    // 0: U9-F3-R1

    // UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB
}
