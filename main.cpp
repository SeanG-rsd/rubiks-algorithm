#include <stdio.h>
#include <iostream>
#include <string>
#include "CubeState.h"
#include "MoveGenerator.h"
#include "KociembaSolver.h"

// UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB

int main()
{
    std::cout << "Hello World!\n";

    const char* initState = "UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB";

    CubeState state = CubeState(initState);

    std::cout << "Is Solved: " << state.IsSolved() << ".\n";

    MoveGenerator generator;

    state = generator.MakeMoves(state, "R U R' U' F2 D' L B2 U2 R' F D2 L' B U' R2 F' D B2 L U");
    std::cout << "After scramble, Is Solved: " << state.IsSolved() << ".\n";

    std::cout << "Building solver tables...\n";
    KociembaSolver solver;

    std::string solution = solver.Solve(state);
    std::cout << "Solution: " << solution << "\n";

    state = generator.MakeMoves(state, solution.c_str());
    std::cout << "After solution, Is Solved: " << state.IsSolved() << ".\n";
}
