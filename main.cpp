#include <stdio.h>
#include <iostream>
#include "CubeState.h"

// UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB

int main()
{
    std::cout << "Hello World!\n";

    const char* initState = "UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB";

    CubeState state = CubeState(initState);

    std::cout << "Is Solved: " << state.IsSolved() << ".\n";
}