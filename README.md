# LUDO-CS — A LUDO-like Simulation in C

**Course:** SCS 1301 — Data Structures and Program Design using C
**Institution:** University of Colombo School of Computing (UCSC)
**Author:** Damitha Liyanage 

A command-line simulation of *LUDO-CS*, an extended variant of the classic board game LUDO, implemented in C. The simulation runs with no user interaction — once compiled and executed, it plays out the game automatically according to fixed player behaviours and prints a full log of game events to the console.

## Overview

LUDO-CS extends traditional LUDO with additional mechanics such as:

- Bidirectional movement (clockwise/counterclockwise, decided by a coin toss)
- Blocks and blockades, including block movement and breaking rules
- Bonus rolls on captures
- A recurring **mystery cell** that teleports pieces to special locations (Bhawana, Kotuwa, Pita-Kotuwa, Base, X, or Approach), each with its own effect (speed boost, sickness, briefing delay, direction reversal, etc.)
- Four players with distinct, hard-coded strategies: an aggressive Red, a blocking-focused Green, a win-focused Yellow, and a mystery-cell-seeking random Blue

Full game rules and player behaviour specifications are defined in the assignment brief (`Assignment_1.pdf`).

## Files

| File | Description |
|---|---|
| `type.h` | Type/struct definitions and function prototypes used across the program (`player_t`, `piece_t`, etc.) |
| `logic.c` | Core game logic — dice rolls, piece movement, capturing, blocking, mystery cell effects, and player behaviours |
| `main.c` | Entry point; drives the game loop using the functions defined in `logic.c` |

## Data Structures

- **`player_t`** — represents a player (colour, dice roll state, and its 4 pieces stored as `piece_t` members). Four global `player_t` variables (one per colour) are used, avoiding the overhead of passing large structs as function arguments. An array of pointers to `player_t` enables cyclic, colour-agnostic access to players, and an array of function pointers (`char (*funcPtrs[4])()`) allows each player's turn function (`redPlayer()`, `greenPlayer()`, `yellowPlayer()`, `bluePlayer()`) to be invoked in rotation starting from any index.
- **`piece_t`** — a nested struct (child of `player_t`) representing an individual piece's state: position, direction, capture count, active effects, etc. A 2D array of pointers to `piece_t` allows O(1) access to any piece by `[player index][piece index]`.
- **Board** — not represented as a dedicated array. Each piece stores its own location, so the board state is derived from piece positions rather than duplicated in a separate 52-cell structure, keeping space complexity down.

`char` is used for most fields (instead of `int`/`short`) since values rarely exceed `CHAR_MAX`, reducing memory footprint; `short` is used only where required (e.g. round counters).

## Building

```bash
gcc -o ludo main.c logic.c
```

## Running

```bash
./ludo
```

The program requires no input — it simulates the entire game automatically and prints all status and event messages (dice rolls, moves, captures, blocks, mystery cell teleports, round summaries, and the final winner) to standard output.

## Report

See `Ludo-CS-Report.pdf` (compiled from the accompanying LaTeX source) for details on:

1. The structures used to represent the board and pieces
2. Justification for the structures used
3. Discussion of the program's time/space efficiency

## Notes

- This was my first large-scale C assignment — feedback and suggestions are welcome via issues/PRs.
