#pragma once

#include <cstdint>

class CubeState
{
protected:
    std::uint64_t EdgeState;
    std::uint64_t CornerState;

public:
    static constexpr int EDGES   = 12;
    static constexpr int CORNERS =  8;

    // each slot is a 5-bit field starting at bit (slot * 5)
    // edges:   bits 0-3 = piece, bit 4 = flip
    // corners: bits 0-2 = piece, bits 3-4 = twist
    static constexpr std::uint64_t EdgesSolved = 0b01011'01010'01001'01000'00111'00110'00101'00100'00011'00010'00001'00000;
    static constexpr std::uint64_t CornersSolved = 0b00111'00110'00101'00100'00011'00010'00001'00000;

    CubeState(std::uint64_t edgeState, std::uint64_t cornerState)
    {
        EdgeState = edgeState;
        CornerState = cornerState;
    }

    // throws std::invalid_argument if the string isn't a solvable cube
    CubeState(const char* cubeState);

    std::uint64_t GetEdgeState() const { return EdgeState; }
    std::uint64_t GetCornerState() const { return CornerState; }

    bool IsSolved() const { return EdgesSolved == EdgeState && CornersSolved == CornerState; }
};
