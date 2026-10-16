#include <stdio.h>
#include <iostream>
#include "CubeState.h"
#include "MoveGenerator.h"

// UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB

int main()
{
    std::cout << "Hello World!\n";

    const char* initState = "UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB";

    CubeState state = CubeState(initState);

    std::cout << "Is Solved: " << state.IsSolved() << ".\n";

    MoveGenerator generator;

    state = generator.MakeMoves(state, "R U R' U' F2 D' L B2");
    std::cout << "After scramble, Is Solved: " << state.IsSolved() << ".\n";

    state = generator.MakeMoves(state, "B2 L' D F2 U R U' R'");
    std::cout << "After undoing it, Is Solved: " << state.IsSolved() << ".\n";
}
