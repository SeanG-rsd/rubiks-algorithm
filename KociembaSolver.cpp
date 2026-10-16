
#include <algorithm>
#include <chrono>
#include <cstdint>
#include "KociembaSolver.h"

namespace
{
    const int MOVES = MoveGenerator::MOVES;

    // coordinate sizes
    const int TWISTS      = 2187;  // 3^7, the last corner's twist follows from the other seven
    const int FLIPS       = 2048;  // 2^11, the last edge's flip follows from the other eleven
    const int SLICES      = 495;   // 12 choose 4 sets of slots the middle layer edges can be in
    const int PERMS       = 40320; // 8! orders of the corners, or of the U and D layer edges
    const int SLICE_PERMS = 24;    // 4! orders of the middle layer edges

    // phase 1 never needs more than 12 moves and phase 2 never more than 18
    const int MAX_LENGTH = 30;

    // phase 1 solutions that need a longer phase 2 than this are skipped, it's faster to try more of them
    const int PHASE2_MAX = 12;

    const int PHASE1_MOVE_COUNT = 18;
    const int PHASE1_MOVES[PHASE1_MOVE_COUNT] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};

    const int PHASE2_MOVE_COUNT = 10;
    const int PHASE2_MOVES[PHASE2_MOVE_COUNT] = {0, 1, 2, 5, 8, 9, 10, 11, 14, 17}; // U U' U2 R2 F2 D D' D2 L2 B2

    // the U and D layer edge slots
    const int UD_SLOTS[8] = {0, 1, 2, 3, 8, 9, 10, 11};

    bool IsPhase2Move(int move)
    {
        int side = move / 3;
        return side == 0 || side == 3 || move % 3 == 2;
    }

    // the same side twice in a row, or opposite sides in both orders (U D is the same as D U)
    bool Redundant(int previous, int move)
    {
        int a = previous / 3;
        int b = move / 3;
        return a == b || (a % 3 == b % 3 && a > b);
    }

    // the cube unpacked to one int per piece, easier to work with than the bit fields
    struct Pieces
    {
        int edge[CubeState::EDGES];
        int flip[CubeState::EDGES];
        int corner[CubeState::CORNERS];
        int twist[CubeState::CORNERS];
    };

    Pieces Unpack(const CubeState& state)
    {
        Pieces p;

        for (int slot = 0; slot < CubeState::EDGES; slot++)
        {
            int field = static_cast<int>((state.GetEdgeState() >> (slot * 5)) & 0b11111);
            p.edge[slot] = field & 0b1111;
            p.flip[slot] = field >> 4;
        }

        for (int slot = 0; slot < CubeState::CORNERS; slot++)
        {
            int field = static_cast<int>((state.GetCornerState() >> (slot * 5)) & 0b11111);
            p.corner[slot] = field & 0b111;
            p.twist[slot] = field >> 3;
        }

        return p;
    }

    CubeState Pack(const Pieces& p)
    {
        std::uint64_t edgeState = 0;
        std::uint64_t cornerState = 0;

        for (int slot = 0; slot < CubeState::EDGES; slot++)
        {
            edgeState |= static_cast<std::uint64_t>((p.flip[slot] << 4) | p.edge[slot]) << (slot * 5);
        }

        for (int slot = 0; slot < CubeState::CORNERS; slot++)
        {
            cornerState |= static_cast<std::uint64_t>((p.twist[slot] << 3) | p.corner[slot]) << (slot * 5);
        }

        return CubeState(edgeState, cornerState);
    }

    int Choose(int n, int k)
    {
        if (k > n)
        {
            return 0;
        }

        int result = 1;
        for (int i = 0; i < k; i++)
        {
            result = result * (n - i) / (i + 1);
        }
        return result;
    }

    // where a permutation of 0..count-1 comes in sorted order, 0 for 0, 1, 2, ...
    int Rank(const int* values, int count)
    {
        int rank = 0;

        for (int i = 0; i < count; i++)
        {
            int smaller = 0;
            for (int j = i + 1; j < count; j++)
            {
                if (values[j] < values[i])
                {
                    smaller++;
                }
            }
            rank = rank * (count - i) + smaller;
        }

        return rank;
    }

    void Unrank(int rank, int* values, int count)
    {
        int smaller[CubeState::EDGES];
        for (int i = count - 1; i >= 0; i--)
        {
            smaller[i] = rank % (count - i);
            rank /= count - i;
        }

        int unused[CubeState::EDGES];
        for (int i = 0; i < count; i++)
        {
            unused[i] = i;
        }

        for (int i = 0; i < count; i++)
        {
            values[i] = unused[smaller[i]];
            for (int j = smaller[i]; j < count - 1 - i; j++)
            {
                unused[j] = unused[j + 1];
            }
        }
    }

    // ---- phase 1 coordinates ----

    int GetTwist(const Pieces& p)
    {
        int twist = 0;
        for (int slot = 0; slot < CubeState::CORNERS - 1; slot++)
        {
            twist = twist * 3 + p.twist[slot];
        }
        return twist;
    }

    void SetTwist(Pieces& p, int twist)
    {
        int total = 0;
        for (int slot = CubeState::CORNERS - 2; slot >= 0; slot--)
        {
            p.twist[slot] = twist % 3;
            total += p.twist[slot];
            twist /= 3;
        }
        p.twist[CubeState::CORNERS - 1] = (3 - total % 3) % 3;
    }

    int GetFlip(const Pieces& p)
    {
        int flip = 0;
        for (int slot = 0; slot < CubeState::EDGES - 1; slot++)
        {
            flip = flip * 2 + p.flip[slot];
        }
        return flip;
    }

    void SetFlip(Pieces& p, int flip)
    {
        int total = 0;
        for (int slot = CubeState::EDGES - 2; slot >= 0; slot--)
        {
            p.flip[slot] = flip % 2;
            total += p.flip[slot];
            flip /= 2;
        }
        p.flip[CubeState::EDGES - 1] = total % 2;
    }

    bool InSlice(int edge)
    {
        return edge >= 4 && edge <= 7; // FR, FL, BL, BR
    }

    // which 4 slots hold the middle layer edges, ignoring their order
    int GetSlice(const Pieces& p)
    {
        int slice = 0;
        int found = 0;
        for (int slot = 0; slot < CubeState::EDGES; slot++)
        {
            if (InSlice(p.edge[slot]))
            {
                found++;
                slice += Choose(slot, found);
            }
        }
        return slice;
    }

    void SetSlice(Pieces& p, int slice)
    {
        bool inSlice[CubeState::EDGES] = {};
        int left = 4;

        for (int slot = CubeState::EDGES - 1; slot >= 0 && left > 0; slot--)
        {
            if (Choose(slot, left) <= slice)
            {
                slice -= Choose(slot, left);
                inSlice[slot] = true;
                left--;
            }
        }

        int nextSlice = 4;
        int nextOther = 0;
        for (int slot = 0; slot < CubeState::EDGES; slot++)
        {
            if (inSlice[slot])
            {
                p.edge[slot] = nextSlice++;
            }
            else
            {
                p.edge[slot] = nextOther;
                nextOther = nextOther == 3 ? 8 : nextOther + 1;
            }
        }
    }

    // ---- phase 2 coordinates, only meaningful once phase 1 is done ----

    int GetCornerPerm(const Pieces& p)
    {
        return Rank(p.corner, CubeState::CORNERS);
    }

    void SetCornerPerm(Pieces& p, int perm)
    {
        Unrank(perm, p.corner, CubeState::CORNERS);
    }

    // order of the 8 U and D layer edges within the U and D layers
    int GetEdgePerm(const Pieces& p)
    {
        int values[8];
        for (int i = 0; i < 8; i++)
        {
            int edge = p.edge[UD_SLOTS[i]];
            values[i] = edge < 4 ? edge : edge - 4;
        }
        return Rank(values, 8);
    }

    void SetEdgePerm(Pieces& p, int perm)
    {
        int values[8];
        Unrank(perm, values, 8);
        for (int i = 0; i < 8; i++)
        {
            p.edge[UD_SLOTS[i]] = UD_SLOTS[values[i]];
        }
    }

    // order of the 4 middle layer edges within the middle layer
    int GetSlicePerm(const Pieces& p)
    {
        int values[4];
        for (int i = 0; i < 4; i++)
        {
            values[i] = p.edge[4 + i] - 4;
        }
        return Rank(values, 4);
    }

    void SetSlicePerm(Pieces& p, int perm)
    {
        int values[4];
        Unrank(perm, values, 4);
        for (int i = 0; i < 4; i++)
        {
            p.edge[4 + i] = values[i] + 4;
        }
    }

    // table[coordinate * MOVES + move] = the coordinate after the move
    template <typename Set, typename Get>
    void BuildMoveTable(const MoveGenerator& generator, std::uint16_t* table, int count, Set set, Get get, const int* moves, int moveCount)
    {
        for (int coordinate = 0; coordinate < count; coordinate++)
        {
            Pieces p = Unpack(CubeState());
            set(p, coordinate);
            CubeState state = Pack(p);

            for (int i = 0; i < moveCount; i++)
            {
                table[coordinate * MOVES + moves[i]] = static_cast<std::uint16_t>(get(Unpack(generator.MakeMove(state, moves[i]))));
            }
        }
    }

    // breadth first search out from the solved pair of coordinates:
    // prune[a * countB + b] = fewest moves that get both coordinates solved
    void BuildPruneTable(
        std::int8_t* prune,
        const std::uint16_t* moveA, int countA, int solvedA,
        const std::uint16_t* moveB, int countB, int solvedB,
        const int* moves, int moveCount
    ) {
        int size = countA * countB;
        std::fill(prune, prune + size, static_cast<std::int8_t>(-1));
        prune[solvedA * countB + solvedB] = 0;

        int added = 1;
        for (int depth = 0; added > 0; depth++)
        {
            added = 0;

            for (int index = 0; index < size; index++)
            {
                if (prune[index] != depth)
                {
                    continue;
                }

                int a = index / countB;
                int b = index % countB;

                for (int i = 0; i < moveCount; i++)
                {
                    int next = moveA[a * MOVES + moves[i]] * countB + moveB[b * MOVES + moves[i]];

                    if (prune[next] < 0)
                    {
                        prune[next] = static_cast<std::int8_t>(depth + 1);
                        added++;
                    }
                }
            }
        }
    }
}

struct KociembaSolver::Tables
{
    std::uint16_t twistMove[TWISTS * MOVES];
    std::uint16_t flipMove[FLIPS * MOVES];
    std::uint16_t sliceMove[SLICES * MOVES];
    std::uint16_t cornerPermMove[PERMS * MOVES];   // phase 2 moves only
    std::uint16_t edgePermMove[PERMS * MOVES];     // phase 2 moves only
    std::uint16_t slicePermMove[SLICE_PERMS * MOVES]; // phase 2 moves only

    std::int8_t twistSlicePrune[TWISTS * SLICES];
    std::int8_t flipSlicePrune[FLIPS * SLICES];
    std::int8_t twistFlipPrune[TWISTS * FLIPS];
    std::int8_t cornerSlicePrune[PERMS * SLICE_PERMS];
    std::int8_t edgeSlicePrune[PERMS * SLICE_PERMS];
};

struct KociembaSolver::Search
{
    int moves[MAX_LENGTH + 1];        // the moves being tried, phase 1 then phase 2
    CubeState states[MAX_LENGTH + 1]; // states[i] = the cube after the first i moves
    int computedDepth = 0;            // states[0..computedDepth] are up to date with moves

    int best[MAX_LENGTH + 1];
    int bestLength = MAX_LENGTH + 1;

    int maxLength;
    std::chrono::steady_clock::time_point deadline;
    long nodes = 0;

    Search(const CubeState& start, int length, double timeoutSeconds)
        : maxLength(length),
          deadline(std::chrono::steady_clock::now() + std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double>(timeoutSeconds)))
    {
        states[0] = start;
    }

    // out of time, and there's a solution to hand back
    bool OutOfTime() const
    {
        return bestLength <= MAX_LENGTH && std::chrono::steady_clock::now() > deadline;
    }
};

// a lower bound on the moves left in phase 1
int KociembaSolver::Phase1Distance(int twist, int flip, int slice) const
{
    const Tables& t = *tables;

    return std::max({
        t.twistSlicePrune[twist * SLICES + slice],
        t.flipSlicePrune[flip * SLICES + slice],
        t.twistFlipPrune[twist * FLIPS + flip]
    });
}

KociembaSolver::KociembaSolver() : tables(std::make_unique<Tables>())
{
    Tables& t = *tables;

    BuildMoveTable(generator, t.twistMove, TWISTS, SetTwist, GetTwist, PHASE1_MOVES, PHASE1_MOVE_COUNT);
    BuildMoveTable(generator, t.flipMove, FLIPS, SetFlip, GetFlip, PHASE1_MOVES, PHASE1_MOVE_COUNT);
    BuildMoveTable(generator, t.sliceMove, SLICES, SetSlice, GetSlice, PHASE1_MOVES, PHASE1_MOVE_COUNT);

    BuildMoveTable(generator, t.cornerPermMove, PERMS, SetCornerPerm, GetCornerPerm, PHASE2_MOVES, PHASE2_MOVE_COUNT);
    BuildMoveTable(generator, t.edgePermMove, PERMS, SetEdgePerm, GetEdgePerm, PHASE2_MOVES, PHASE2_MOVE_COUNT);
    BuildMoveTable(generator, t.slicePermMove, SLICE_PERMS, SetSlicePerm, GetSlicePerm, PHASE2_MOVES, PHASE2_MOVE_COUNT);

    int solvedSlice = GetSlice(Unpack(CubeState()));

    BuildPruneTable(t.twistSlicePrune, t.twistMove, TWISTS, 0, t.sliceMove, SLICES, solvedSlice, PHASE1_MOVES, PHASE1_MOVE_COUNT);
    BuildPruneTable(t.flipSlicePrune, t.flipMove, FLIPS, 0, t.sliceMove, SLICES, solvedSlice, PHASE1_MOVES, PHASE1_MOVE_COUNT);
    BuildPruneTable(t.twistFlipPrune, t.twistMove, TWISTS, 0, t.flipMove, FLIPS, 0, PHASE1_MOVES, PHASE1_MOVE_COUNT);

    BuildPruneTable(t.cornerSlicePrune, t.cornerPermMove, PERMS, 0, t.slicePermMove, SLICE_PERMS, 0, PHASE2_MOVES, PHASE2_MOVE_COUNT);
    BuildPruneTable(t.edgeSlicePrune, t.edgePermMove, PERMS, 0, t.slicePermMove, SLICE_PERMS, 0, PHASE2_MOVES, PHASE2_MOVE_COUNT);
}

KociembaSolver::~KociembaSolver() = default;

std::string KociembaSolver::Solve(const CubeState& state, int maxLength, double timeoutSeconds) const
{
    Search search(state, maxLength, timeoutSeconds);

    Pieces p = Unpack(state);
    int twist = GetTwist(p);
    int flip = GetFlip(p);
    int slice = GetSlice(p);

    int distance = Phase1Distance(twist, flip, slice);

    // once phase 1 alone is as long as the best solution, nothing shorter is left to find.
    // each phase 1 length is searched to the end before stopping, since a later phase 1 of the
    // same length can give a much shorter total (for an R scramble, phase 1 "R" is tried before "R'")
    for (int depth1 = distance; depth1 < search.bestLength; depth1++)
    {
        if (Phase1(search, twist, flip, slice, 0, depth1) || search.bestLength <= search.maxLength)
        {
            break;
        }
    }

    std::string solution;

    for (int i = 0; i < search.bestLength; i++)
    {
        if (i > 0)
        {
            solution += ' ';
        }
        solution += MoveGenerator::MoveName(search.best[i]);
    }

    return solution;
}

// tries every phase 1 of exactly `togo` more moves, returns true if time ran out
bool KociembaSolver::Phase1(Search& search, int twist, int flip, int slice, int depth, int togo) const
{
    if (togo == 0)
    {
        // ending on a phase 2 move means the same position was already tried one move earlier
        if (depth > 0 && IsPhase2Move(search.moves[depth - 1]))
        {
            return false;
        }

        return StartPhase2(search, depth);
    }

    if ((++search.nodes & 0xFFFF) == 0 && search.OutOfTime())
    {
        return true;
    }

    const Tables& t = *tables;

    for (int move = 0; move < MOVES; move++)
    {
        if (depth > 0 && Redundant(search.moves[depth - 1], move))
        {
            continue;
        }

        int newTwist = t.twistMove[twist * MOVES + move];
        int newFlip = t.flipMove[flip * MOVES + move];
        int newSlice = t.sliceMove[slice * MOVES + move];

        int distance = Phase1Distance(newTwist, newFlip, newSlice);

        // can't finish in time, or finished early: nothing under 5 moves leaves phase 2's group and comes back
        // ending on a non phase 2 move, so that branch can't produce a new phase 1 solution
        if (distance > togo - 1 || (distance == 0 && togo - 1 > 0 && togo - 1 < 5))
        {
            continue;
        }

        search.moves[depth] = move;
        search.computedDepth = std::min(search.computedDepth, depth);

        if (Phase1(search, newTwist, newFlip, newSlice, depth + 1, togo - 1))
        {
            return true;
        }
    }

    return false;
}

// the first `depth` moves finish phase 1, look for a phase 2 that beats the best solution so far
bool KociembaSolver::StartPhase2(Search& search, int depth) const
{
    if (search.OutOfTime())
    {
        return true;
    }

    int limit = std::min(search.bestLength - 1 - depth, PHASE2_MAX);

    if (limit < 0)
    {
        return false;
    }

    // phase 2's coordinates aren't tracked during phase 1, so play the moves out on the cube
    for (int i = search.computedDepth; i < depth; i++)
    {
        search.states[i + 1] = generator.MakeMove(search.states[i], search.moves[i]);
    }
    search.computedDepth = depth;

    const Tables& t = *tables;

    Pieces p = Unpack(search.states[depth]);
    int cornerPerm = GetCornerPerm(p);
    int edgePerm = GetEdgePerm(p);
    int slicePerm = GetSlicePerm(p);

    int distance = std::max(t.cornerSlicePrune[cornerPerm * SLICE_PERMS + slicePerm], t.edgeSlicePrune[edgePerm * SLICE_PERMS + slicePerm]);

    for (int depth2 = distance; depth2 <= limit; depth2++)
    {
        if (Phase2(search, cornerPerm, edgePerm, slicePerm, depth, depth2))
        {
            search.bestLength = depth + depth2;
            std::copy(search.moves, search.moves + search.bestLength, search.best);

            return false;
        }
    }

    return false;
}

bool KociembaSolver::Phase2(Search& search, int cornerPerm, int edgePerm, int slicePerm, int depth, int togo) const
{
    // the pruning tables are only 0 when everything is solved
    if (togo == 0)
    {
        return true;
    }

    const Tables& t = *tables;

    for (int move : PHASE2_MOVES)
    {
        if (depth > 0 && Redundant(search.moves[depth - 1], move))
        {
            continue;
        }

        int newCorner = t.cornerPermMove[cornerPerm * MOVES + move];
        int newEdge = t.edgePermMove[edgePerm * MOVES + move];
        int newSlice = t.slicePermMove[slicePerm * MOVES + move];

        int distance = std::max(t.cornerSlicePrune[newCorner * SLICE_PERMS + newSlice], t.edgeSlicePrune[newEdge * SLICE_PERMS + newSlice]);

        if (distance > togo - 1)
        {
            continue;
        }

        search.moves[depth] = move;

        if (Phase2(search, newCorner, newEdge, newSlice, depth + 1, togo - 1))
        {
            return true;
        }
    }

    return false;
}
