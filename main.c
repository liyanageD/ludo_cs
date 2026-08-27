#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "type.h"
#include "logic.c"

// declare players and initialize values
player_t red = {"red", -1, 0, 0, 0, 2, 0, 1, 0, 0, -1, 0, 0, 0, 2, 0, 1, 0, 0, -1, 0, 0, 0, 2, 0, 1, 0, 0, -1, 0, 0, 0, 2, 0, 1, 0, 0, 4, 0, 28, 26, 80, 0};
player_t green = {"green", -1, 0, 0, 0, 2, 0, 1, 0, 0, -1, 0, 0, 0, 2, 0, 1, 0, 0, -1, 0, 0, 0, 2, 0, 1, 0, 0, -1, 0, 0, 0, 2, 0, 1, 0, 0, 4, 0, 41, 39, 90, 0};
player_t yellow = {"yellow", -1, 0, 0, 0, 2, 0, 1, 0, 0, -1, 0, 0, 0, 2, 0, 1, 0, 0, -1, 0, 0, 0, 2, 0, 1, 0, 0, -1, 0, 0, 0, 2, 0, 1, 0, 0, 4, 0, 2, 0, 60, 0};
player_t blue = {"blue", -1, 0, 0, 0, 2, 0, 1, 0, 0, -1, 0, 0, 0, 2, 0, 1, 0, 0, -1, 0, 0, 0, 2, 0, 1, 0, 0, -1, 0, 0, 0, 2, 0, 1, 0, 0, 4, 0, 15, 13, 70, 0};

player_t *playerPtr[] = {&red, &green, &yellow, &blue};

piece_t *piecePtr[4][4] = {{&red.p1, &red.p2, &red.p3, &red.p4}, {&green.p1, &green.p2, &green.p3, &green.p4}, {&yellow.p1, &yellow.p2, &yellow.p3, &yellow.p4}, {&blue.p1, &blue.p2, &blue.p3, &blue.p4}};

short mstryCell = -2;
short winners = 0; // number players who completed
short roundNo = 0;

int main()
{
    system("cls");
    srand(time(0));
    // srand(10);

    // print players and pieces they have
    printf("The red player has four (04) pieces named R1, R2, R3, and R4.\n");
    printf("The green player has four (04) pieces named G1, G2, G3, and G4.\n");
    printf("The yellow player has four (04) pieces named Y1, Y2, Y3, and Y4.\n");
    printf("The blue player has four (04) pieces named B1, B2, B3, and B4.\n");

    // choose first player
    short fIndex = fPlayer(playerPtr);

    printf("The order of a single round is %s, %s, %s, and %s.\n\n", playerPtr[fIndex]->name,
           playerPtr[(fIndex + 1) % 4]->name, playerPtr[(fIndex + 2) % 4]->name, playerPtr[(fIndex + 3) % 4]->name);

    char (*funcPtrs[4])() = {redPlayer, greenPlayer, yellowPlayer, bluePlayer}; // array of pointer to player's function

    // loop for round

    while (1)
    {
        char isMoved;

        // single round
        for (int i = 0; i < 4; i++)
        {
            // call the next player
            isMoved = funcPtrs[(fIndex + i) % 4]();

            // if anyone takes its piece to board, round count begins
            if (roundNo == 0 && isMoved == 1)
            {
                roundNo = 1;
            }

            if (winners == 3)
            {
                break;
            }
        }

        // game is over
        if (winners == 3)
        {
            printmap();
            break;
        }

        // reset mystery effects on each piece, if effect passed 4 rounds
        rstEffect();

        // generate mystery cell for every 4 rounds.(after 2nd round)
        if (roundNo >= 2 && ((roundNo - 2) % 4 == 0))
        {
            genMstryCell();
        }

        // print positions of each piece
        printmap();

        // getchar();

        // increment round, if any player has done a move
        if (roundNo)
        {

            roundNo++;
        }
    }

    return 0;
}