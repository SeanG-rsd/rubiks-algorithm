#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdint>

class CubeState
{
protected:
    std::uint64_t EdgeState;
    std::uint64_t CornerState;

public:
    const std::uint64_t EdgesSolved = 0b01011'01010'01001'01000'00111'00110'00101'00100'00011'00010'00001'00000;
    const std::uint64_t CornersSolved = 0b00111'00110'00101'00100'00011'00010'00001'00000;

    CubeState(long edgeState, long cornerState)
    {
        EdgeState = edgeState;
        CornerState = cornerState;
    }

    CubeState(const char* cubeState);

    bool IsSolved() { return EdgesSolved == EdgeState && CornersSolved == CornerState; }
};