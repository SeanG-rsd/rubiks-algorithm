
#include <iostream>
#include <bitset>
#include "CubeState.h"
#include "Maps.h"

// UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB

CubeState::CubeState(const char* cubeState)
{
    Maps maps;

    const int EDGES   = 12;
    const int CORNERS =  8;

    std::uint64_t edgeState = 0;

    for (int i = EDGES - 1; i >= 0; i--)
    {
        char e1 = cubeState[maps.edges[i][0]];
        char e2 = cubeState[maps.edges[i][1]];

        for (std::pair e : maps.edges)
        {
            char check1 = maps.index_to_color[e.second[0]];
            char check2 = maps.index_to_color[e.second[1]];

            if (
                (e1 == check1 && e2 == check2)
                || (e1 == check2 && e2 == check1)
            ) {
                // check orientation here
                int dom1 = maps.color_to_dominant_value[e1];
                int dom2 = maps.color_to_dominant_value[e2];

                if (dom1 > dom2)
                {
                    
                } else {

                }


                int orient = 0;

                std::uint64_t fullInfo = (orient << 5) | (e.first);
                
                edgeState |= fullInfo << (i * 5);
            }
        }
    } 

    std::uint64_t cornerState = 0;

    for (int i = CORNERS - 1; i >= 0; i--)
    {
        char c1 = cubeState[maps.corners[i][0]];
        char c2 = cubeState[maps.corners[i][1]];
        char c3 = cubeState[maps.corners[i][2]];

        for (std::pair e : maps.corners)
        {
            char check1 = maps.index_to_color[e.second[0]];
            char check2 = maps.index_to_color[e.second[1]];
            char check3 = maps.index_to_color[e.second[2]];

            if (
                (c1 == check1 && c2 == check2 && c3 == check3)
                || (c1 == check1 && c2 == check3 && c3 == check2)
                || (c1 == check2 && c2 == check1 && c3 == check3)
                || (c1 == check2 && c2 == check3 && c3 == check1)
                || (c1 == check3 && c2 == check1 && c3 == check2)
                || (c1 == check3 && c2 == check2 && c3 == check1)
            ) {
                // check orientation here
                int orient = 0;

                std::uint64_t fullInfo = (orient << 5) | (e.first);
                
                cornerState |= fullInfo << (i * 5);
            }
        }
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