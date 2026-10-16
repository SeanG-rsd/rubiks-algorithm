#pragma once

 struct Maps
{
    // facelet index -> the color it has on a solved cube
    static constexpr char index_to_color[] = "UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB";

    // slots ordered by descending sticker dominance (U/D = 3, F/B = 2, L/R = 1)
    // an edge is flipped when its more dominant color isn't on the slot's first facelet
    static constexpr int edges[12][2] = {
        {7, 19},   // 00: U8-F2
        {3, 37},   // 01: U4-L2
        {1, 46},   // 02: U2-B2
        {5, 10},   // 03: U6-R2
        {23, 12},  // 04: F6-R4
        {21, 41},  // 05: F4-L6
        {50, 39},  // 06: B6-L4
        {48, 14},  // 07: B4-R6
        {28, 25},  // 08: D2-F8
        {30, 43},  // 09: D4-L8
        {34, 52},  // 10: D8-B8
        {32, 16}   // 11: D6-R8
    };

    // U/D facelet first, then clockwise around the corner (looking at it from outside)
    // a corner's twist is which of the three facelets holds its U/D color, so a clockwise
    // twist is always 1 no matter which slot the corner is in
    static constexpr int corners[8][3] = {
        {8, 9, 20},   // 0: U9-R1-F3
        {6, 18, 38},  // 1: U7-F1-L3
        {0, 36, 47},  // 2: U1-L1-B3
        {2, 45, 11},  // 3: U3-B1-R3
        {29, 26, 15}, // 4: D3-F9-R7
        {27, 44, 24}, // 5: D1-L9-F7
        {33, 53, 42}, // 6: D7-B9-L7
        {35, 17, 51}  // 7: D9-R9-B7
    };
};
