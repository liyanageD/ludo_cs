#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "type.h"

extern player_t red, green, yellow, blue;
extern player_t *playerPtr[];
extern piece_t *piecePtr[4][4];
extern short mstryCell;
extern short winners;
extern short roundNo;
short rollValue;
short nxtBlueIdx = -1;

// reset values of a piece
void resetp(piece_t *piece)
{
    piece->cap = 0;
    piece->dir = 0;
    piece->loc = -1;
    piece->moves = 2;
    piece->block = 1;
    piece->aprchCount = 0;
    piece->orgDir = 0;
    piece->affectedOn = 0;
    piece->effect = 0;
}

// generate random number
char rand_no(char range)
{
    return 1 + (char)(rand() % range);
}

// roll dice to choose first player
short rollf(player_t *player)
{
    short value;
    value = rand_no(6);
    printf("%s rolls %d\n", player->name, value);
    return value;
}

// choose first player
short fPlayer(player_t *playerPtr[])
{

    short round = 0;
    short maxValue_old;
    short values_old[4];

    while (1)
    {

        short values[4] = {0, 0, 0, 0};
        short maxValue = 0;
        short maxIndex = -1;
        short maxCount = 0;

        round++;

        for (int i = 0; i < 4; i++)
        {

            if (round != 1 && values_old[i] != maxValue_old)
            {
                continue;
            }

            values[i] = rollf(playerPtr[i]);

            if (values[i] > maxValue)
            {
                maxCount = 1;
                maxIndex = i;
                maxValue = values[i];
            }
            else if (values[i] == maxValue)
            {
                maxCount++;
            }
        }

        if (maxCount == 1)
        {
            printf("\n%s player has the highest roll and will begin the game.\n\n", playerPtr[maxIndex]->name);
            return maxIndex;
        }
        else
        {

            for (int i = 0; i < 4; i++)
            {
                values_old[i] = values[i];

                if (values[i] == maxValue)
                {
                    printf("%s player, ", playerPtr[i]->name);
                }
            }

            printf("\b\b have the same highest roll and will begin another round to select the first player.\n\n");

            maxValue_old = maxValue;
        }
    }
}

// roll dice
short roll(char playerIdx)
{
    player_t *player = playerPtr[playerIdx];
    short value;
    value = rand_no(6);
    printf("%s player rolled %d\n", player->name, value);
    return value;
}

// decide direction of a player by a coin toss
void toss(piece_t *x)
{
    short value = rand_no(2);

    if (value == 1)
    {
        x->dir = 1;
        x->orgDir = 1;
    }
    else
    {
        x->dir = -1;
        x->orgDir = -1;
    }
}

// *from base to starting point (blocked -> -1 | no pieces -> -2)
char base2x(char playerIdx)
{

    char pieceIdx = -1;
    char startIdx = 0;

    // set priority piece for blue player
    if (playerIdx == BLUE)
    {
        startIdx = nxtBlueIdx;
    }

    for (int i = 0; i < 4; i++)
    {
        char checkIdx = (startIdx + i) % 4;
        // looking for a piece in the base
        if (piecePtr[playerIdx][checkIdx]->loc == -1)
        {
            pieceIdx = checkIdx;
            break;
        }
    }

    // base is empty
    if (pieceIdx == -1)
    {
        return -2;
    }

    player_t *player = playerPtr[playerIdx];
    piece_t *piece = piecePtr[playerIdx][pieceIdx];

    short start = player->x;

    // avoid taking piece to x if it is blocked
    if (checkCell(playerIdx, start) == 3)
    {
        return -1;
    }

    // move piece from base to its starting point
    piece->loc = player->x;
    player->base--;

    // make a toss and set its direction
    toss(piece);

    printf("%s player moves piece %d to the starting point.\n", player->name, pieceIdx + 1);
    printf("%s player now has %d/4 on pieces on the board and %d/4 pieces on the base.\n",
           player->name, 4 - player->base - player->home, player->base);

    return pieceIdx;
};

// reurn a piece to its base
void back2bace(char playerIndex, char pieceIndex)
{

    player_t *player = playerPtr[playerIndex];
    piece_t *piece = piecePtr[playerIndex][pieceIndex];
    if (piece->loc >= 0 || piece->loc <= 51)
    {

        resetp(piece);
        player->base++;
    }
}

// returns the difference between destination and the current location (steps to move)
char getDiff(char playerIdx, char pieceIdx, short destination)
{
    player_t *player = playerPtr[playerIdx];
    piece_t *piece = piecePtr[playerIdx][pieceIdx];

    char diff;
    // clockwise
    if (piece->dir == 1 && piece->loc <= 51)
    {
        if (destination <= 51)
        {
            diff = destination - piece->loc;

            if (diff < 0)
            {
                diff += 52;
            }
        }
        else
        {
            diff = player->approach - piece->loc;

            diff += destination - player->homeStraight + 1;
        }
    }
    // counter-clockwise
    else if (piece->dir == -1 && piece->loc <= 51)
    {

        if (destination <= 51)
        {
            diff = piece->loc - destination;

            if (diff < 0)
            {
                diff += 52;
            }
        }
        else
        {
            diff = piece->loc - player->approach;
            if (diff < 0)
            {
                diff += 52;
            }

            diff += destination - player->homeStraight + 1;
        }
    }
    else if (piece->loc > 51)
    {
        diff = destination - piece->loc;
    }

    return diff;
}

// move piece to the destination. (not moved, returns 0)
char move(char player, char piece, char destination, char flag)
{

    player_t *p = playerPtr[player];      // p = pointer to player
    piece_t *x = piecePtr[player][piece]; // x = pointer to piece

    char diceValue = getDiff(player, piece, destination);

    // if piece in the base, exit.
    if (x->loc < 0)
    {
        return 0;
    }

    // exit if location and the destination is same
    if (x->loc == destination)
    {
        return 0;
    }

    // exit if the destination is not in the board
    if (destination < 0 || destination > p->homeStraight + 5)
    {
        return 0;
    }

    // clockwise movements, from standard
    if (x->dir == 1 && x->loc <= 51 && x->loc >= 0)
    {
        //  go to homestraight or home
        if (destination > 51)
        {

            // go to Home
            if (destination == p->homeStraight + 5)
            {
                p->home++;

                printf("%s moves piece %d from location %d to Home by %d units in clockwise direction.",
                       p->name, piece + 1, x->loc, diceValue);
            }

            // go to homestraight
            else
            {

                printf("%s moves piece %d from location %d to homestraight cell %d by %d units.", p->name, piece + 1,
                       x->loc, destination - p->homeStraight + 1, diceValue);
            }
        }
        // movements within standard path
        else
        {

            printf("%s moves piece %d from location %d to %d by %d units in clockwise direction.",
                   p->name, piece + 1, x->loc, destination, diceValue);

            /* // increase round by one when piece pass its 'x'
            if ((x->moves + diceValue) / 52 + 1 != x->round)
            {
                x->round++;
            } */
        }
    }
    // counterclockwise movements, from standard
    else if (x->dir == -1 && x->loc <= 51 && x->loc >= 0)
    {
        // go to Homestraight or Home
        if (destination >= 60)
        {

            // go from standard to Home
            if (destination == p->homeStraight + 5)
            {

                p->home++;
                printf("%s moves piece %d from location %d to Home by %d units in counter-clockwise direction..",
                       p->name, piece + 1, x->loc, diceValue);
            }

            // go from standard to homestraight
            else
            {

                printf("%s moves piece %d from location %d to homestraight cell %d by %d units in counter-clockwise direction.",
                       p->name, piece + 1, x->loc, destination - p->homeStraight + 1, diceValue);
            }
        }
        // movements within standard path
        else
        {

            /*  // increase round by one when piece pass its the cell before its approach
             if ((x->moves + 3 + diceValue) / 52 + 1 != x->round)
             {
                 x->round++;
             } */

            printf("%s moves piece %d from location %d to %d by %d units in counter-clockwise direction.", p->name, piece + 1,
                   x->loc, destination, diceValue);
        }
    }
    // movements within homestraight (direction doesn't matter)
    else if (x->loc >= 60)
    {

        short locOld = x->loc - p->homeStraight + 1;

        // move from homestraight
        if (destination <= p->homeStraight + 5)
        {

            // move to Home
            if (destination == p->homeStraight + 5)
            {
                p->home++;

                printf("%s moves piece %d from homestraight cell %d to Home by %d units.",
                       p->name, piece + 1, locOld, diceValue);
            }
            // move to homestraight
            else
            {

                printf("%s moves piece %d from homestraight cell %d to homestraight cell %d by %d units", p->name,
                       piece + 1, locOld, destination, diceValue);
            }
        }
        else
        // exceeding Home
        {

            printf("%s cannot moves piece %d from homestraight cell %d by %d units due to exceeding Home.", p->name,
                   piece + 1, x->loc - p->homeStraight + 1, diceValue);
        }
    }

    // set move count
    switch (x->dir)
    {
    // counter-clockwise moves
    case -1:
        x->moves -= diceValue;

        if (x->moves < 0)
        {
            x->moves += 52;
            x->aprchCount++;
        }

        break;
    // clockwise moves
    case 1:

        x->moves += diceValue;
        if (x->moves > 51)
        {
            x->aprchCount++;
            x->moves %= 52;
        }
    }

    // set destination
    x->loc = destination;

    if (!flag)
    {
        if (x->loc == mstryCell)
        {
            printf("\n************************************************%s player lands on a mystery cell**********************************************************************\n", p->name);
            printmap1();
            teleport(player, piece);
            printmap1();
        }
    }

    // set the place upon winners
    if (p->home == 4)
    {
        p->place = ++winners;
        printf("\n");
        switch (p->place)
        {
        case 1:
            printf("%s player wins!\n", p->name);
            break;
        case 2:
            printf("%s player gets 2nd place!\n", p->name);
            break;
        case 3:
            printf("%s player gets 3rd place!\n", p->name);
            break;
        case 4:
            printf("%s player gets 4th place!\n", p->name);
        }
        getchar();
    }

    // set rollValue by mystery effect
    if (x->effect == ENERGIZE)
    {
        rollValue *= 2;
    }
    else if (x->effect == SICK)
    {
        rollValue /= 2;
    }
    else if (x->effect == BRIEF)
    {
        rollValue = 0;
    }

    if (rollValue / x->block != diceValue && flag == 0)
    {
        // printf("%s does not have other pieces in the board to move instead of the blocked piece.\n", p->name);
        printf(" which is the cell before the block.\n");
    }
    else
    {
        printf("\n");
    }

    return 1;
}

// move as a block. (not moved, returns 0)
char moveBlock(char playerIndex, char pieceIndex, short destination)
{

    player_t *player = playerPtr[playerIndex];
    piece_t *piece = piecePtr[playerIndex][pieceIndex];

    short blockLoc = piece->loc;

    short value = getDiff(playerIndex, pieceIndex, destination) * piece->block;

    destination = getDest(playerIndex, pieceIndex, value);

    // check otheer pieces in the block whether their destinations are different from current piece
    for (short i = 0; i < 4; i++)
    {
        piece_t *piece2 = piecePtr[playerIndex][i];

        if (piece2->loc != piece->loc || i == pieceIndex)
        {
            continue;
        }

        short destination2 = getDest(playerIndex, i, value);

        // if other piece is not going to home straight, set destination to its destination
        if (destination2 != destination && destination2 <= 51)
        {
            destination = destination2;
        }
    }

    // exit if piece gonna move to its location
    if (piece->loc == destination)
    {
        return 0;
    }

    // move the block
    for (char i = 0; i < 4; i++)
    {
        piece_t *piece2 = piecePtr[playerIndex][i];

        if (piece2->loc == blockLoc)
        {
            move(playerIndex, i, destination, 0);
            /* if (!move(playerIndex, i, destination))
            {
                return 0;
            } */
        }
    }
    return 1;
}

// move piece safely to the destination. (0-notMoved, 1-capture, 2-block, 3-moveAsBlock, 4-justMove)
char movesf(char playerIndex, char pieceIndex, short destination)
{
    player_t *player = playerPtr[playerIndex];
    piece_t *piece = piecePtr[playerIndex][pieceIndex];

    // short destination = getDest(playerIndex, pieceIndex, value);
    short cell;

    if (piece->loc == -1 || piece->loc == player->homeStraight + 5)
    {
        return 0;
    }

    if (checknCap(playerIndex, pieceIndex, destination))
    {
        return 1;
    }
    else if (block(playerIndex, pieceIndex, destination))
    {
        return 2;
    }
    else if (piece->block > 1 && moveBlock(playerIndex, pieceIndex, destination))
    {
        return 3;
    }
    else if (move(playerIndex, pieceIndex, destination, 0))
    {
        return 4;
    }
    else
    {
        return 0;
    }
}

// returns destination cell of a piece
short getDest(char player, char piece, char value)
{
    player_t *p = playerPtr[player];
    piece_t *x = piecePtr[player][piece];
    short destination;
    char homeStrCond = 0; // whether the piece is going to the homestraight

    // set value by mystery effect
    if (x->effect == ENERGIZE)
    {
        value *= 2;
    }
    else if (x->effect == SICK)
    {
        value /= 2;
    }
    else if (x->effect == BRIEF)
    {
        value = 0;
    }

    // set number of cells a block can move
    value = value / x->block;

    if (value == 0 || x->loc < 0 || x->loc == p->homeStraight + 5)
    {
        destination = x->loc;
        return destination;
    }

    // piece is on the standard path
    if (x->loc >= 0 && x->loc <= 51)
    {
        // go to Homestraight or Home
        switch (x->orgDir)
        {
            // original direction is clockwise
        case 1:
            switch (x->dir)
            {
                // current direction is clockwise
            case 1:
                if ((x->moves <= 51) && x->moves + value > 52 && x->cap > 1)
                {
                    destination = p->homeStraight + (x->moves + value - 52) - 1;
                    homeStrCond = 1;
                }

                break;
                // current direction is counter-clockwise
            case -1:
                if (x->moves > 0 && x->moves - value < 0 && x->cap > 1)
                {
                    destination = p->homeStraight + (value - x->moves) - 1;
                    homeStrCond = 1;
                }
                break;

            default:
                if (x->moves == 0 && x->cap > 1)
                {
                    destination = p->homeStraight + value - 1;
                    homeStrCond = 1;
                }
            }

            break;
            // original direction is counter-clockwise
        case -1:
            switch (x->dir)
            {

                // current direction is clockwise
            case 1:
                if ((x->moves <= 51) && x->moves + value > 52 && x->aprchCount > 1 && x->cap > 1)
                {
                    destination = p->homeStraight + (x->moves + value - 52) - 1;
                    homeStrCond = 1;
                }

                break;

                // current direction is counter-clockwise
            case -1:
                if (x->moves > 0 && x->moves - value < 0 && x->aprchCount > 1 && x->cap > 1)
                {
                    destination = p->homeStraight + (value - x->moves) - 1;
                    homeStrCond = 1;
                }
                break;

            default:
                if (x->moves == 0 && x->aprchCount > 1 && x->cap > 1)
                {
                    destination = p->homeStraight + value - 1;
                    homeStrCond = 1;
                }
            }
        }

        // movements within standard path
        if (!homeStrCond)
        {

            switch (x->dir)
            {
            // clockwise movements
            case 1:
                if (x->loc + value > 51)
                {
                    destination = x->loc + value - 52;
                }
                else
                {
                    destination = x->loc + value;
                }
                break;

            // counterclockwise movements
            case -1:
                if (x->loc - value < 0)
                {
                    destination = x->loc - value + 52;
                }
                else
                {
                    destination = x->loc - value;
                }
            }
        }
    }
    // movements within homestraight
    else if (x->loc + value <= p->homeStraight + 5)
    {

        destination = x->loc + value;
    }
    // exceeding Home
    else
    {
        destination = x->loc;
    }

    // if current piece is blocked set destination to adjacent cell
    if (destination <= 51)
    {
        short blockDest = isBlocked(player, piece, destination);

        if (blockDest != -1)
        {
            destination = blockDest;
        }
    }

    return destination;
}

// returns number of moves to reach home for a particular piece (without considering constraints)
short steps2home(char playerIndex, char pieceIndex)
{

    short steps;
    player_t *player = playerPtr[playerIndex];
    piece_t *piece = piecePtr[playerIndex][pieceIndex];

    // piece is in standard path
    if (piece->loc >= 0 && piece->loc <= 51)
    {

        switch (piece->dir)
        {
            // deirection of piece is clockwise
        case 1:
            // piece is on its approach
            if (piece->moves == 0)
            {
                steps = 6;
            }
            else
            {
                steps = 52 - piece->moves + 6;
            }

            break;
        // deirection of piece is counter-clockwise
        case -1:

            // piece is on its approach
            if (piece->moves == 0)
            {
                steps = 6;
            }
            else
            {
                steps = piece->moves + 6;
            }
        }
    }
    // piece is in homestraight
    else if (piece->loc >= 60)
    {
        steps = (player->homeStraight + 5) - piece->loc;
    }

    return steps;
}

// check whether a piece can capture an apponent piece or a block, returns index of the piece which can be captured, else -1.
short checkCap(char playerIndex, char pieceIndex, short destination)
{

    player_t *player = playerPtr[playerIndex];
    piece_t *piece = piecePtr[playerIndex][pieceIndex];

    // if destination is home, exit
    if (destination == player->homeStraight + 5 || piece->loc == player->homeStraight + 5)
    {
        return -1;
    }

    if (piece->loc != player->x && destination == piece->loc)
    {
        return -1;
    }

    // loop through apponent players
    for (char i = 1; i < 4; i++)
    {

        char apntPlayerIndex = (playerIndex + i) % 4;

        // loop through apponent pieces
        for (char j = 0; j < 4; j++)
        {
            piece_t *apntPiece = piecePtr[apntPlayerIndex][j];
            char apntBlock = apntPiece->block;

            // apponent piece can be captured
            if (apntPiece->loc == destination && (piece->block == apntPiece->block || apntPiece->block == 1))
            {

                return apntPlayerIndex * 4 + j;
            }
        }
    }

    // no apponent to be captured for the piece
    return -1;
}

// check whether a piece is blocked. if blocked, returns the adjacent location of the block. Else, returns -1.
short isBlocked(char playerIndex, char pieceIndex, short destination)
{

    player_t *player = playerPtr[playerIndex];
    piece_t *piece = piecePtr[playerIndex][pieceIndex];
    short checkUp2 = destination;

    // if piece is homestraight, it's obviously not blocked
    if (piece->loc > 51)
    {
        return -1;
    }

    // destination is homestraight
    if (destination >= 52)
    {

        checkUp2 = player->approach; // check upto approach cell
    }

    short i = piece->loc;

    switch (piece->dir)
    {
    // clockwise movements
    case 1:
        // loop through cells
        while (1)
        {
            i = i % 52;

            // loop through apponent players
            for (char j = 1; j < 4; j++)
            {

                char apntPlayerIndex = (playerIndex + j) % 4;

                // loop through apponent pieces
                for (char k = 0; k < 4; k++)
                {
                    piece_t *apntPiece = piecePtr[apntPlayerIndex][k];

                    // ignore if apponent piece can be captured
                    if (apntPiece->loc == i && i == destination && (apntPiece->block == piece->block || apntPiece->block == 1))
                    {

                        continue;
                    }

                    if (apntPiece->loc == i && apntPiece->block > 1)
                    {

                        return --i % 52;
                    }
                }
            }

            // base case
            if (i == checkUp2)
            {
                break;
            }

            i++;
        }
        break;

    // counter-clockwise movements
    case -1:
        // loop through cells

        while (1)
        {
            if (i < 0)
            {
                i = i + 52;
            }

            // loop through apponent players
            for (char j = 1; j < 4; j++)
            {

                char apntPlayerIndex = (playerIndex + j) % 4;

                // loop through apponent pieces
                for (char k = 0; k < 4; k++)
                {
                    piece_t *apntPiece = piecePtr[apntPlayerIndex][k];

                    if (apntPiece->loc == i && apntPiece->block > 1)
                    {
                        // ignore if apponent piece can be captured
                        if (i == destination && apntPiece->block == piece->block)
                        {

                            continue;
                        }

                        return ++i % 52;
                    }
                }
            }

            // base case
            if (i == checkUp2)
            {
                break;
            }
            i--;
        }
    }

    return -1;
}

// check whether a pice can make a block with a piece. if it can, returns 1.
char canBlock(char playerIndex, char pieceIndex, short destination)
{

    player_t *player = playerPtr[playerIndex];
    piece_t *piece = piecePtr[playerIndex][pieceIndex];

    // if destination is home, exit
    if (destination == player->homeStraight + 5 || piece->loc == player->homeStraight + 5)
    {
        return 0;
    }

    /* if (piece->loc != player->x && destination == piece->loc)
    {
        return 0;
    } */

    // loop through pieces
    for (short i = 0; i < 4; i++)
    {
        if (i == pieceIndex)
        {
            continue;
        }

        // same color piece is in the destination
        if (piecePtr[playerIndex][i]->loc == destination)
        {

            return 1;
        }
    }

    return 0;
}

//  make a block, (0 - not possible to make a block)
char block(char playerIndex, char pieceIndex, short destination)
{

    player_t *player = playerPtr[playerIndex];
    piece_t *piece = piecePtr[playerIndex][pieceIndex];
    char count = 0;
    short s2hMax = 0; // maximum steps to home
    char maxIndex;    // index of the piece which has maximum steps to its home
    char check = 0;

    // if destination is home, exit
    if (destination == player->homeStraight + 5 || piece->loc == player->homeStraight + 5)
    {
        return 0;
    }

    // check whether a pice can make a block with a piece

    // loop through pieces
    for (short i = 0; i < 4; i++)
    {

        if (i == pieceIndex)
        {
            continue;
        }

        // same color piece is in the destination
        if (piecePtr[playerIndex][i]->loc == destination)
        {

            check = 1;
        }
    }

    // exit if cannot make a block
    if (!check)
    {
        return 0;
    }

    // moves the piece to the destination where is gonna be a block

    if (piece->block > 1)
    {
        moveBlock(playerIndex, pieceIndex, destination);
    }
    else
    {
        move(playerIndex, pieceIndex, destination, 0);
    }

    // loop through the pieces of the player
    for (short i = 0; i < 4; i++)
    {

        // determine pieces in the block
        if (piecePtr[playerIndex][i]->loc == destination)
        {
            char s2h = steps2home(playerIndex, i);

            // determine piece which has maximum steps to its home
            if (s2h > s2hMax)
            {
                s2hMax = s2h;
                maxIndex = i;
            }

            count++;
        }
    }

    // loop through the pieces of the player
    for (short i = 0; i < 4; i++)
    {

        // set direction of all player in the block and set size of block
        if (piecePtr[playerIndex][i]->loc == destination)
        {

            piecePtr[playerIndex][i]->block = count;
            piecePtr[playerIndex][i]->dir = piecePtr[playerIndex][maxIndex]->dir;
        }
    }
    return 1;
}

// player1's piece captures player2's piece
void capture(char player1Index, char piece1Index, char player2Index, char piece2Index)
{

    player_t *player1 = playerPtr[player1Index];
    piece_t *piece1 = piecePtr[player1Index][piece1Index];

    player_t *player2 = playerPtr[player2Index];
    piece_t *piece2 = piecePtr[player2Index][piece2Index];

    char playerLoc = piece1->loc;
    char apntLoc = piece2->loc;
    char apntBlock = piece2->block;

    if (piece1->block > 1)
    {
        char piece1Str[12];
        char piece2Str[12];
        char count1 = 0;
        char count2 = 0;

        // increment capture of all pieces in the block, loop through player1's pieces
        for (short i = 0; i < 4; i++)
        {
            piece_t *piece = piecePtr[player1Index][i];

            if (piece->loc == piece1->loc)
            {
                // put name of each pice to a string
                piece1Str[count1++] = 49 + i;
                piece1Str[count1++] = ',';
                piece1Str[count1++] = ' ';

                piece->cap += apntBlock / piece1->block;
            }
        }
        piece1Str[count1 - 2] = '\0';

        // return player2's block to its base,
        if (apntBlock > 1)
        {

            // loop through player2's pieces
            for (short i = 0; i < 4; i++)
            {
                piece_t *Piece = piecePtr[player2Index][i];

                if (Piece->loc == apntLoc)
                {
                    back2bace(player2Index, i);

                    // put name of each piece to a string
                    piece2Str[count2++] = 49 + i;
                    piece2Str[count2++] = ',';
                    piece2Str[count2++] = ' ';
                }
            }

            piece2Str[count2 - 2] = '\0';
        }
        else
        {
            // return player2's piece to its base
            back2bace(player2Index, piece2Index);

            // put name of the piece to a string
            piece2Str[0] = 49 + piece2Index;
            piece2Str[1] = '\0';
        }

        // moves player1's block to player2's piece's location
        // moveBlock(player1Index, piece1Index, apntLoc);

        // move the block
        for (char i = 0; i < 4; i++)
        {
            piece_t *piece2 = piecePtr[player1Index][i];

            if (piece2->loc == playerLoc)
            {
                move(player1Index, i, apntLoc, 0);
                /* if (!move(playerIndex, i, destination))
                {
                    return 0;
                } */
            }
        }

        printf("block of %s pieces %s lands on square %d, captures block of %s pieces %s, and returns it to the base.\n",
               player1->name, piece1Str, piece1->loc, player2->name, piece2Str);
    }

    else
    {

        // return player2's piece to its base
        back2bace(player2Index, piece2Index);

        // moves player1's piece to player2's piece's location
        move(player1Index, piece1Index, apntLoc, 0);

        // increment capture of player1's piece
        piece1->cap++;

        printf("%s piece %d lands on square %d, captures %s piece %d, and returns it to the base.\n",
               player1->name, piece1Index + 1, piece1->loc, player2->name, piece2Index + 1);
    }

    printf("%s player now has %d/4 pieces on the board and %d/4 pieces on the base.\n\n",
           player2->name, 4 - player2->base - player2->home, player2->base);
}

// if it's possible to capture, then capture it. (0 - cannot capture)
char checknCap(char playerIndex, char pieceIndex, short destination)
{
    player_t *player = playerPtr[playerIndex];
    piece_t *piece = piecePtr[playerIndex][pieceIndex];

    // if destination is home, exit
    if (destination == player->homeStraight + 5)
    {
        return 0;
    }

    char capIdx = checkCap(playerIndex, pieceIndex, destination);

    // not possible to capture
    if (capIdx == -1)
    {
        return 0;
    }

    capture(playerIndex, pieceIndex, capIdx / 4, capIdx % 4);
    return 1;
}

// sort an array
void sort(short *arr, short size)
{
    for (short i = 0; i < size - 1; i++)
    {
        short minIdx = i;
        for (short j = i + 1; j < size; j++)
        {
            if (arr[j] < arr[minIdx])
            {
                minIdx = j;
            }
        }
        // swap
        if (minIdx != i)
        {
            short temp = arr[i];
            arr[i] = arr[minIdx];
            arr[minIdx] = temp;
        }
    }
}

// sort a '[size][2]' array by first column
void sort2d(short (*arr)[2], short size)
{
    for (short i = 0; i < size - 1; i++)
    {
        short minIdx = i;
        for (short j = i + 1; j < size; j++)
        {
            if (arr[j][0] < arr[minIdx][0])
            {
                minIdx = j;
            }
        }
        // swap
        if (minIdx != i)
        {
            for (short j = 0; j < 2; j++)
            {
                short temp = arr[i][j];
                arr[i][j] = arr[minIdx][j];
                arr[minIdx][j] = temp;
            }
        }
    }
}

// check whether there is a piece or a block in the cell, returns if (no_piece -> 0, player's_piece -> 1, apponent_piece -> 2, apnt_block -> 3)
char checkCell(char playerIdx, short cell)
{

    // check for own piece in the cell
    for (short i = 0; i < 4; i++)
    {
        piece_t *piece = piecePtr[playerIdx][i];

        if (piece->loc == cell)
        {
            return 1;
        }
    }

    // check for apponent pieces in the cell
    for (short i = 1; i <= 3; i++)
    {
        char apntPlayerIdx = (playerIdx + i) % 4;

        for (short j = 0; j < 4; j++)
        {

            piece_t *apntPiece = piecePtr[apntPlayerIdx][j];

            if (apntPiece->loc == cell)
            {
                if (apntPiece->block > 1)
                {
                    return 3;
                }
                else
                {
                    return 2;
                }
            }
        }
    }

    // cell is free
    return 0;
}

// behaviour of red player
char redPlayer()
{

    char value = 0;
    short count6 = 0;
    char bonus = 0;
    char isMovedAny = 0;
    // char piece;

    do
    {

        // pass the roll if wins
        if (red.place != 0)
        {
            return isMovedAny;
        }

        bonus = 0;
        char isMoved = 0;

        value = roll(RED);
        rollValue = value;
        // char pass;

        // increment count3 when 'three' is rolled
        if (value == 3)
        {
            red.count3++;

            // if any piece in Kotuwa, returns it to base
            
            for (short i = 0; i < 4; i++)
            {
                piece_t *piece = piecePtr[RED][i];

                
                if (piece->effect == BRIEF)
                {
                    printf("%s piece %d is movement-restricted and has rolled three consecutively. Teleporting piece %d to base.\n",
                    "Red", i + 1, i + 1);
                    back2bace(RED, i);
                }
            }
        }
        else
        {
            red.count3 = 0;
        }

        // increment count when six is rolled
        if (value == 6)
        {
            count6++;
        }
        else
        {
            // pass to the next player if there is not any player in the board
            if (red.base == (4 - red.home))
            {
                break;
            }

            count6 = 0;
        }

        // check whether player has rolled 'six' 3 consecutive times
        if (count6 == 3)

        {

            // pass to the next player
            break;
        }

        // take piece to the board if there is no piece in the board
        if (red.base == (4 - red.home) && value == 6)
        {
            short capIndex;
            char pieceIdx;

            pieceIdx = base2x(RED);

            // check whether starting cell is not blocked
            if (pieceIdx != -1)
            {

                if ((capIndex = checkCap(RED, pieceIdx, red.x)) != -1)
                {

                    capture(RED, pieceIdx, capIndex / 4, capIndex % 4);
                    bonus = 1;
                }
                else if (canBlock(RED, pieceIdx, red.x))
                {
                    block(RED, pieceIdx, red.x);
                }

                continue;
            }
        }

        // capture by breaking the block
        /* {
            // loop through red pieces
            for (short i = 0; i < 4; i++)
            {

                piece_t *piece = piecePtr[RED][i];

                // looking for a block
                if (piece->block > 1)
                {

                    // break the block by moving 1 pice or more pices
                    // for (char blockSize = 1; blockSize < 3; blockSize++)
                    //{

                    char blockTemp = piece->block; // block size of the piece before break the block
                    char dirTemp = piece->dir;     // direction of the block
                    short locTemp = piece->loc;    // location of the block
                    char didCap = 0;               // whether a pice or block captured a piece or a block
                    char restValue;                // after breaking the block by moving a pice by partial value, value remaing for rest of block pieces to move

                    piece->dir = piece->orgDir;
                    piece->block = 1; // blockSize;

                    short destination = getDest(RED, i, value);
                    short checkUp2 = destination; // check cell up to this cell whether there is a pice or a block that can be captured

                    // if destination of the piece or  the block is homestraight, check up to approach cell
                    if (destination > 51)
                    {
                        checkUp2 = red.approach;
                    }

                    char moveCount = 1;
                    short j;
                    switch (piece->dir)
                    {

                    case 1:
                        j = piece->loc;

                        while (1)
                        {
                            j = j % 52;

                            if (isBlocked(RED, i, j) == -1)
                            {
                                break;
                            }

                            short capIndex;

                            if (capIndex = checkCap(RED, i, j) != -1)
                            {
                                capture(RED, i, capIndex / 4, capIndex % 4);
                                didCap = 1;
                                bonus = 1;
                                restValue = value - moveCount;
                                break;
                            }

                            moveCount++;

                            // base case
                            if (j == checkUp2)
                            {
                                break;
                            }

                            j++;
                        }
                        break;

                    case -1:

                        j = piece->loc;

                        while (1)
                        {

                            if (j < 0)
                            {
                                j = j + 52;
                            }

                            if (isBlocked(RED, i, j) == -1)
                            {
                                break;
                            }

                            short capIndex;

                            if (capIndex = (RED, i, j) != -1)
                            {
                                capture(RED, i, capIndex / 4, capIndex % 4);
                                didCap = 1;
                                bonus = 1;
                                restValue = value - moveCount;
                                break;
                            }

                            moveCount++;

                            // base case
                            if (j == checkUp2)
                            {
                                break;
                            }
                            j--;
                        }
                    }

                    if (didCap == 0)
                    {
                        piece->dir = dirTemp;
                        piece->block = blockTemp;
                    }
                    else
                    {

                        break;
                    }
                }
                else
                {
                    continue;
                }
            }
        } */

        // capture

        char capCount = 0;  // number of pieces that can capture a piece
        short stepMin = 60; // minimum  number of moves a piece(which gonna be captured) has to go to the home
        char minIndex;      // index of the piece which can capture the pice closest to its home
        char minCapIndex;   // index of the piece which is gonna be captured and closest to its home

        // check whether there is a posibility to capture a piece
        for (int i = 0; i < 4; i++)
        {

            piece_t *piece = piecePtr[RED][i];

            // if piece is not in the standard, check next piece
            if (piece->loc > 51 || piece->loc < 0)
            {
                continue;
            }

            short destination = getDest(RED, i, value);
            short blockDest = isBlocked(RED, i, destination);

            // if current piece is blocked set destination to adjacent cell
            if (blockDest != -1)
            {
                destination = blockDest;
            }

            // if its not gonna move, check next piece.
            if (destination == piece->loc)
            {
                continue;
            }

            // skip checking curernt piece unless destination is the standard path
            if (destination > 51)
            {
                continue;
            }

            short capIndex; // Index of the pice which is gonna be captured

            // skip checking curernt piece if it cannot capture a piece
            if ((capIndex = checkCap(RED, i, destination)) == -1)
            {

                continue;
            }
            else
            // piece can capture a pieece
            {

                // looks for the piece which is colsest to its home
                if (steps2home(capIndex / 4, capIndex % 4) < stepMin)
                {
                    stepMin = steps2home(capIndex / 4, capIndex % 4);
                    minIndex = i;
                    minCapIndex = capIndex;
                }

                capCount++;
            }
        }

        // if there is a piece which can be captured, capture it
        if (capCount > 0)
        {

            // capture the piece

            capture(RED, minIndex, minCapIndex / 4, minCapIndex % 4);
            bonus = 1;
            isMovedAny = 1;
            continue;
        }

        // try to capture (with breaking a block)
        capCount = 0; // number of pieces that can capture a piece
        stepMin = 60; // minimum  number of moves a piece(which gonna be captured) has to go to the home

        for (int i = 0; i < 4; i++)
        {
            piece_t *piece = piecePtr[RED][i];

            // if piece is not in the standard, check next piece
            if (piece->loc > 51 || piece->loc < 0)
            {
                continue;
            }

            if (piece->block > 1)
            {
                // dtore previos values
                char dirOld = piece->dir;
                char blockOld = piece->block;

                piece->block = 1;           // change block to '1' for the current piece.
                piece->dir = piece->orgDir; // change direction to original direction

                short destination = getDest(RED, i, value);
                short blockDest = isBlocked(RED, i, destination);

                // if current piece is blocked set destination to adjacent cell
                if (blockDest != -1)
                {
                    destination = blockDest;
                }

                // if its not gonna move, check next piece.
                if (destination == piece->loc)
                {
                    piece->block = blockOld; // change block to previous.
                    piece->dir = dirOld;     // change direction to previous.
                    continue;
                }

                // if it is gonna capture someone, capture it.
                short capIdx, s2h;
                if ((capIdx = checkCap(RED, i, destination)) != -1)
                {
                    // looks for the piece which is colsest to its home
                    if ((s2h = steps2home(capIdx / 4, capIdx % 4)) < stepMin)
                    {
                        stepMin = s2h;
                        minIndex = i;
                        minCapIndex = capIdx;
                    }

                    capCount++;
                }

                piece->block = blockOld; // change block to previous.
                piece->dir = dirOld;     // change direction to previous.
            }
        }

        // if there is a piece which can be captured, capture it
        if (capCount > 0)
        {
            piecePtr[RED][minIndex]->block = 1;                             // change block to '1' for the current piece.
            piecePtr[RED][minIndex]->dir = piecePtr[RED][minIndex]->orgDir; // change direction to original direction

            // update block value of rest of the members
            for (short j = 0; j < 4; j++)
            {

                if (j == minIndex)
                {
                    continue;
                }

                piece_t *piece2 = piecePtr[RED][j];

                if (piecePtr[RED][minIndex]->loc == piece2->loc)
                {
                    piece2->block--;
                }
            }

            // capture the piece

            capture(RED, minIndex, minCapIndex / 4, minCapIndex % 4);
            bonus = 1;
            isMovedAny = 1;
            continue;
        }

        // take a piece from base to starting point(without blocking)
        if (value == 6 && red.base > 0)
        {

            short capIndex;
            char pieceIdx;

            // check whether there is no red piece in starting cell
            if (checkCell(RED, red.x) != 1)
            {
                pieceIdx = base2x(RED);

                // check whether starting cell is not blocked
                if (pieceIdx != -1)
                {

                    if ((capIndex = checkCap(RED, pieceIdx, red.x)) != -1)
                    {

                        capture(RED, pieceIdx, capIndex / 4, capIndex % 4);
                        bonus = 1;
                    }
                    continue;
                }
            }
        }

        //        *** there is no piece which can be captured  ***
        // make a random move

        // try to break a block then move
        for (int i = 0; i < 4; i++)
        {

            piece_t *piece = piecePtr[RED][i];

            // skip checking curernt piece if it is in home or base
            if (piece->loc == -1 || piece->loc == red.homeStraight + 5)
            {
                continue;
            }

            if (piece->block > 1)
            {
                // store previos values
                char dirOld = piece->dir;
                char blockOld = piece->block;

                piece->block = 1;           // change block to '1' for the current piece.
                piece->dir = piece->orgDir; // change direction to original direction

                short destination = getDest(RED, i, value);
                short blockDest = isBlocked(RED, i, destination);

                // if current piece is blocked set destination to adjacent cell
                if (blockDest != -1)
                {
                    destination = blockDest;
                }

                // if its not gonna move, check next piece.
                if (destination == piece->loc)
                {
                    piece->block = blockOld; // change block to previous.
                    piece->dir = dirOld;     // change direction to previous.
                    continue;
                }

                // if it is gonna make another block, check next
                if (canBlock(RED, i, destination))
                {
                    piece->block = blockOld; // change block to previous.
                    piece->dir = dirOld;     // change direction to previous.

                    continue;
                }
                else
                {

                    // update block value of rest of the members
                    for (short j = 0; j < 4; j++)
                    {

                        if (j == i)
                        {
                            continue;
                        }

                        piece_t *piece2 = piecePtr[RED][j];

                        if (piece->loc == piece2->loc)
                        {
                            piece2->block--;
                        }
                    }

                    // move the piece
                    movesf(RED, i, destination);
                    isMoved = 1;
                    break;
                }
            }
        }

        // if moved, go to next iteration
        if (isMoved)
        {
            isMovedAny = 1;
            continue;
        }

        // try to move (single/block) (without making a block)
        for (short i = 0; i < 4; i++)
        {
            piece_t *piece = piecePtr[RED][i];

            // if piece is not in board, check next
            if (piece->loc < 0 || piece->loc == red.homeStraight + 5)
            {
                continue;
            }

            short destination = getDest(RED, i, value);
            short blockDest = isBlocked(RED, i, destination);

            // if blocked, set destination to adjacent cell
            if (blockDest != -1)
            {
                destination = blockDest;
            }

            // if piece is not going to move, check next.
            if (piece->loc == destination)
            {
                continue;
            }

            // if it is gonna make a block, check next.
            if (canBlock(RED, i, destination))
            {
                continue;
            }

            if (piece->block == 1)
            {
                movesf(RED, i, destination);
                isMoved = 1;
                break;
            }
            else
            {
                moveBlock(RED, i, destination);
                isMoved = 1;
                break;
            }
        }

        // if moved, go to next iteration
        if (isMoved)
        {
            isMovedAny = 1;
            continue;
        }

        // take a piece from base to starting point(with blocking)

        if (value == 6 && red.base > 0)
        {

            short capIndex;
            char pieceIdx;

            pieceIdx = base2x(RED);

            // check whether starting cell is not blocked
            if (pieceIdx != -1)
            {

                if ((capIndex = checkCap(RED, pieceIdx, red.x)) != -1)
                {

                    capture(RED, pieceIdx, capIndex / 4, capIndex % 4);
                    bonus = 1;
                }
                else if (canBlock(RED, pieceIdx, red.x))
                {
                    block(RED, pieceIdx, red.x);
                }

                continue;
            }
        }

        // try to move a (single/block) then, (make a block)
        for (short i = 0; i < 4; i++)
        {
            piece_t *piece = piecePtr[RED][i];

            // if piece is not in board, check next
            if (piece->loc < 0 || piece->loc == red.homeStraight + 5)
            {
                continue;
            }

            short destination = getDest(RED, i, value);
            short blockDest = isBlocked(RED, i, destination);

            // if blocked, set destination to adjacent cell
            if (blockDest != -1)
            {
                destination = blockDest;
            }

            // if piece is not going to move, check next.
            if (piece->loc == destination)
            {
                continue;
            }

            // if it is gonna make a block, block.
            if (canBlock(RED, i, destination))
            {
                block(RED, i, destination);
                isMoved = 1;
                isMovedAny = 1;
                break;
            }
        }

    } while (value == 6 || bonus);

    return isMovedAny;
}

// behaviour of Green player
char greenPlayer()
{

    char value;          // the value get by rolling the dice
    short count6 = 0;    // number of times the dice is rolled '6'
    char isMovedAny = 0; // whether the player have done at least one move
    char bonus;

    do
    {

        // pass the roll if wins
        if (green.place != 0)
        {
            return isMovedAny;
        }

        char isMoved = 0;

        bonus = 0;
        value = roll(GREEN);

        rollValue = value;




        // increment count3 when 'three' is rolled
        if (value == 3)
        {
            green.count3++;

            // if any piece in Kotuwa, returns it to base
            
            for (short i = 0; i < 4; i++)
            {
                piece_t *piece = piecePtr[GREEN][i];

                
                if (piece->effect == BRIEF)
                {
                     printf("%s piece %d is movement-restricted and has rolled three consecutively. Teleporting piece %d to base.\n",
                    "Green", i + 1, i + 1);
                    back2bace(GREEN, i);
                }
            }
        }
        else
        {
            green.count3 = 0;
        }

        // increment count when "6" is rolled.
        if (value == 6)
        {
            count6++;
        }
        else
        {
            // pass to the next player if there is no piece in the board
            if (green.base == (4 - green.home))
            {
                break;
            }
            count6 = 0;
        }

        // if '6' is rolled 3times consecutively, break blocks if player has. if doesnt, ignore the roll and pass to the next player
        if (count6 == 3)
        {
            break;
        }

        // take a piece to the board if there is no pieces in the board
        if (value == 6 && green.base == 4 - green.home)
        {
            short capIdx;
            char pieceIdx = base2x(GREEN);

            if (pieceIdx != -1)
            {
                if ((capIdx = checkCap(GREEN, pieceIdx, green.x)) != -1)
                {
                    capture(GREEN, pieceIdx, capIdx / 4, capIdx % 4);
                    bonus = 1;
                }
                else if (canBlock(GREEN, pieceIdx, green.x))
                {

                    block(GREEN, pieceIdx, green.x);
                }

                continue;
            }
        }

        // check whether it's possible to make a block
        for (int i = 0; i < 4; i++)
        {
            piece_t *piece = piecePtr[GREEN][i];
            short destination = getDest(GREEN, i, value);
            short blockDest = isBlocked(GREEN, i, destination);

            // if piece is not in the board, check next piece
            if (piece->loc == green.homeStraight + 5 || piece->loc < 0)
            {
                continue;
            }

            // if current piece is blocked set destination to adjacent cell
            if (blockDest != -1)
            {
                destination = blockDest;
            }

            // if its not gonna move, check next piece.
            if (destination == piece->loc)
            {
                continue;
            }

            // make a block, if it is possible.
            if (canBlock(GREEN, i, destination))
            {
                block(GREEN, i, destination);
                isMoved = 1;
                break;
            }
        }

        // if moved, go to next iteration
        if (isMoved)
        {
            isMovedAny = 1;
            continue;
        }

        // check whether it is spossible to get a pice to the board from the base
        if (value == 6 && green.base != 0)
        {
            short capIdx;
            char pieceIdx = base2x(GREEN);

            if (pieceIdx != -1)
            {
                if ((capIdx = checkCap(GREEN, pieceIdx, green.x)) != -1)
                {
                    capture(GREEN, pieceIdx, capIdx / 4, capIdx % 4);
                    bonus = 1;
                }
                else if (canBlock(GREEN, pieceIdx, green.x))
                {

                    block(GREEN, pieceIdx, green.x);
                }
                continue;
            }
        }

        // try to move a block
        for (int i = 0; i < 4; i++)
        {
            piece_t *piece = piecePtr[GREEN][i];
            short destination = getDest(GREEN, i, value);
            short blockDest = isBlocked(GREEN, i, destination);

            // if piece is not in the board, check next piece
            if (piece->loc == green.homeStraight + 5 || piece->loc < 0)
            {
                continue;
            }

            // if current piece is blocked set destination to adjacent cell
            if (blockDest != -1)
            {
                destination = blockDest;
            }

            // if its not gonna move, check next piece.
            if (destination == piece->loc)
            {
                continue;
            }

            // if it is gonna capture someone, check next piece
            if (checkCap(GREEN, i, destination) != -1)
            {
                continue;
            }

            // move the block
            if (piece->block > 1)
            {
                moveBlock(GREEN, i, destination);
                isMoved = 1;
                break;
            }
        }

        // if moved, go to next iteration
        if (isMoved)
        {
            isMovedAny = 1;
            continue;
        }

        // try to move single piece
        for (int i = 0; i < 4; i++)
        {
            piece_t *piece = piecePtr[GREEN][i];
            short destination = getDest(GREEN, i, value);
            short blockDest = isBlocked(GREEN, i, destination);

            // if piece is not in the board, check next piece
            if (piece->loc == green.homeStraight + 5 || piece->loc < 0)
            {
                continue;
            }

            // if current piece is blocked set destination to adjacent cell
            if (blockDest != -1)
            {
                destination = blockDest;
            }

            // if its not gonna move, check next piece.
            if (destination == piece->loc)
            {
                continue;
            }

            // if it is gonna capture someone, check next piece
            if (checkCap(GREEN, i, destination) != -1)
            {
                continue;
            }

            // move the piece/block
            if (piece->block == 1)
            {

                movesf(GREEN, i, destination);
            }
            else
            {

                moveBlock(GREEN, i, destination);
            }

            isMoved = 1;
            break;
        }

        // if moved, go to next iteration
        if (isMoved)
        {
            isMovedAny = 1;
            continue;
        }

        // try capture (block/single)
        for (int i = 0; i < 4; i++)
        {
            piece_t *piece = piecePtr[GREEN][i];
            short destination = getDest(GREEN, i, value);
            short blockDest = isBlocked(GREEN, i, destination);

            // if piece is not in the standard, check next piece
            if (piece->loc > 51 || piece->loc < 0)
            {
                continue;
            }

            // if current piece is blocked set destination to adjacent cell
            if (blockDest != -1)
            {
                destination = blockDest;
            }

            // if its not gonna move, check next piece.
            if (destination == piece->loc)
            {
                continue;
            }

            // capture someone
            short capIdx;
            if ((capIdx = checkCap(GREEN, i, destination)) != -1)
            {
                capture(GREEN, i, capIdx / 4, capIdx % 4);
                bonus = 1;
                isMoved = 1;
                break;
            }
        }

        // if moved, go to next iteration
        if (isMoved)
        {
            isMovedAny = 1;
            continue;
        }

        // try to break a block(with blocking)
        for (int i = 0; i < 4; i++)
        {
            piece_t *piece = piecePtr[GREEN][i];

            // if piece is not in the board, check next piece
            if (piece->loc == green.homeStraight + 5 || piece->loc < 0)
            {
                continue;
            }

            if (piece->block > 1)
            {
                // dtore previos values
                char dirOld = piece->dir;
                char blockOld = piece->block;

                piece->block = 1;           // change block to '1' for the current piece.
                piece->dir = piece->orgDir; // change direction to original direction

                short destination = getDest(GREEN, i, value);
                short blockDest = isBlocked(GREEN, i, destination);

                // if current piece is blocked set destination to adjacent cell
                if (blockDest != -1)
                {
                    destination = blockDest;
                }

                // if its not gonna move, check next piece.
                if (destination == piece->loc)
                {
                    piece->block = blockOld; // change block to previous.
                    piece->dir = dirOld;     // change direction to previous.
                    continue;
                }

                // if it is gonna capture someone, check next piece
                if (checkCap(GREEN, i, destination) != -1)
                {
                    piece->block = blockOld; // change block to previous.
                    piece->dir = dirOld;     // change direction to previous.
                    continue;
                }

                if (canBlock(GREEN, i, destination))
                {
                    // update block value of rest of the members
                    for (short j = 0; j < 4; j++)
                    {

                        if (j == i)
                        {
                            continue;
                        }

                        piece_t *piece2 = piecePtr[GREEN][j];

                        if (piece->loc == piece2->loc)
                        {
                            piece2->block--;
                        }
                    }

                    // make a block
                    block(GREEN, i, destination);

                    isMoved = 1;
                    break;
                }
                else
                {

                    piece->block = blockOld; // change block to previous.
                    piece->dir = dirOld;     // change direction to previous.
                }
            }
        }

        // if moved, go to next iteration
        if (isMoved)
        {
            isMovedAny = 1;
            continue;
        }

        // try to break a block(without captureing)
        for (int i = 0; i < 4; i++)
        {
            piece_t *piece = piecePtr[GREEN][i];

            // if piece is not in the board, check next piece
            if (piece->loc == green.homeStraight + 5 || piece->loc < 0)
            {
                continue;
            }

            if (piece->block > 1)
            {
                // dtore previos values
                char dirOld = piece->dir;
                char blockOld = piece->block;

                piece->block = 1;           // change block to '1' for the current piece.
                piece->dir = piece->orgDir; // change direction to original direction

                short destination = getDest(GREEN, i, value);
                short blockDest = isBlocked(GREEN, i, destination);

                // if current piece is blocked set destination to adjacent cell
                if (blockDest != -1)
                {
                    destination = blockDest;
                }

                // if its not gonna move, check next piece.
                if (destination == piece->loc)
                {
                    piece->block = blockOld; // change block to previous.
                    piece->dir = dirOld;     // change direction to previous.
                    continue;
                }

                // if it is gonna capture someone, check next piece
                if (checkCap(GREEN, i, destination) != -1)
                {
                    piece->block = blockOld; // change block to previous.
                    piece->dir = dirOld;     // change direction to previous.
                    continue;
                }

                // update block value of rest of the members
                for (short j = 0; j < 4; j++)
                {

                    if (j == i)
                    {
                        continue;
                    }

                    piece_t *piece2 = piecePtr[GREEN][j];

                    if (piece->loc == piece2->loc)
                    {
                        piece2->block--;
                    }
                }

                // move the piece

                if (movesf(GREEN, i, destination))
                {
                    isMoved = 1;
                    break;
                }
                else
                {
                    piece->block = blockOld; // change block to previous.
                    piece->dir = dirOld;     // change direction to previous.
                    continue;
                }
            }
        }

        // if moved, go to next iteration
        if (isMoved)
        {
            isMovedAny = 1;
            continue;
        }

        // try to break a block(with captureing)
        for (int i = 0; i < 4; i++)
        {
            piece_t *piece = piecePtr[GREEN][i];

            // if piece is not in the standard, check next piece
            if (piece->loc > 51 || piece->loc < 0)
            {
                continue;
            }

            if (piece->block > 1)
            {
                // dtore previos values
                char dirOld = piece->dir;
                char blockOld = piece->block;

                piece->block = 1;           // change block to '1' for the current piece.
                piece->dir = piece->orgDir; // change direction to original direction

                short destination = getDest(GREEN, i, value);
                short blockDest = isBlocked(GREEN, i, destination);

                // if current piece is blocked set destination to adjacent cell
                if (blockDest != -1)
                {
                    destination = blockDest;
                }

                // if it's not gonna move, check next piece.
                if (destination == piece->loc || destination > 51)
                {
                    piece->block = blockOld; // change block to previous.
                    piece->dir = dirOld;     // change direction to previous.
                    continue;
                }

                // if it is gonna capture someone, capture it.
                short capIdx;
                if ((capIdx = checkCap(GREEN, i, destination)) != -1)
                {
                    // update block value of rest of the members
                    for (short j = 0; j < 4; j++)
                    {

                        if (j == i)
                        {
                            continue;
                        }

                        piece_t *piece2 = piecePtr[GREEN][j];

                        if (piece->loc == piece2->loc)
                        {
                            piece2->block--;
                        }
                    }

                    // capture the piece
                    capture(GREEN, i, capIdx / 4, capIdx % 4);
                    bonus = 1;
                    isMoved = 1;
                    isMovedAny = 1;
                    break;
                }
                else
                // no possible captures for the current pice
                {

                    piece->block = blockOld; // change block to previous.
                    piece->dir = dirOld;     // change direction to previous.
                }
            }
        }

    } while (value == 6 || bonus == 1);

    return isMovedAny;
}

// behaviour of yellow player
char yellowPlayer()
{

    char value;       // the value get by rolling the dice
    short count6 = 0; // number of times the dice is rolled
    char bonus = 0;
    char isMovedAny = 0; // whether the player have done at least one move

    do
    {
        // pass the roll if wins
        if (yellow.place != 0)
        {
            return isMovedAny;
        }

        // printmap();

        bonus = 0;
        char isMoved = 0;
        value = roll(YELLOW);

        rollValue = value;


        // increment count3 when 'three' is rolled
        if (value == 3)
        {
            yellow.count3++;

            // if any piece in Kotuwa, returns it to base
            
            for (short i = 0; i < 4; i++)
            {
                piece_t *piece = piecePtr[YELLOW][i];

                
                if (piece->effect == BRIEF)
                {
                     printf("%s piece %d is movement-restricted and has rolled three consecutively. Teleporting piece %d to base.\n",
                    "Yellow", i + 1, i + 1);
                    back2bace(YELLOW, i);
                }
            }
        }
        else
        {
            yellow.count3 = 0;
        }



        // increment count only if player rolls '6'
        if (value == 6)
        {
            count6++;
        }
        else
        {
            // pass to the next player if there is no piece in the board
            if (yellow.base == (4 - yellow.home))
            {
                break;
            }
            count6 = 0;
        }

        // if '6' is rolled 3times consecutively, break blocks if player has. if doesnt, ignore the roll and pass to the next player
        if (count6 == 3)
        {
            break;
        }

        // get a piece to the board if base is not empty
        if (yellow.base > 0 && value == 6)
        {
            short capIdx;
            char pieceIdx = base2x(YELLOW);

            if (pieceIdx != -1)
            {
                if ((capIdx = checkCap(YELLOW, pieceIdx, yellow.x)) != -1)
                {
                    capture(YELLOW, pieceIdx, capIdx / 4, capIdx % 4);
                    bonus = 1;
                }
                else if (canBlock(YELLOW, pieceIdx, yellow.x))
                {

                    block(YELLOW, pieceIdx, yellow.x);
                }
                continue;
            }
        }

        // try capture (block/single)
        for (int i = 0; i < 4; i++)
        {
            piece_t *piece = piecePtr[YELLOW][i];

            // if piece is not in the standard, check next piece
            if (piece->loc > 51 || piece->loc < 0)
            {
                continue;
            }

            short destination = getDest(YELLOW, i, value);
            short blockDest = isBlocked(YELLOW, i, destination);

            // if current piece is blocked set destination to adjacent cell
            if (blockDest != -1)
            {
                destination = blockDest;
            }

            // if its not gonna move, check next piece.
            if (destination == piece->loc || destination > 51)
            {
                continue;
            }

            // capture someone
            short capIdx;
            if ((capIdx = checkCap(YELLOW, i, destination)) != -1)
            {
                capture(YELLOW, i, capIdx / 4, capIdx % 4);
                bonus = 1;
                isMoved = 1;
                break;
            }
        }

        // if moved, go to next iteration
        if (isMoved)
        {
            isMovedAny = 1;
            continue;
        }

        // try to capture (with breaking a block)
        for (int i = 0; i < 4; i++)
        {
            piece_t *piece = piecePtr[YELLOW][i];

            // if piece is not in the standard, check next piece
            if (piece->loc > 51 || piece->loc < 0)
            {
                continue;
            }

            if (piece->block > 1)
            {
                // store previos values
                char dirOld = piece->dir;
                char blockOld = piece->block;

                piece->block = 1;           // change block to '1' for the current piece.
                piece->dir = piece->orgDir; // change direction to original direction

                short destination = getDest(YELLOW, i, value);
                short blockDest = isBlocked(YELLOW, i, destination);

                // if current piece is blocked set destination to adjacent cell
                if (blockDest != -1)
                {
                    destination = blockDest;
                }

                // if its not gonna move, check next piece.
                if (destination == piece->loc || destination > 51)
                {
                    piece->block = blockOld; // change block to previous.
                    piece->dir = dirOld;     // change direction to previous.
                    continue;
                }

                // if it is gonna capture someone, capture it.
                short capIdx;
                if ((capIdx = checkCap(YELLOW, i, destination)) != -1)
                {
                    // update block value of rest of the members
                    for (short j = 0; j < 4; j++)
                    {

                        if (j == i)
                        {
                            continue;
                        }

                        piece_t *piece2 = piecePtr[YELLOW][j];

                        if (piece->loc == piece2->loc)
                        {
                            piece2->block--;
                        }
                    }

                    // capture the piece
                    capture(YELLOW, i, capIdx / 4, capIdx % 4);
                    bonus = 1;
                    isMoved = 1;
                    isMovedAny = 1;
                    break;
                }
                else
                // no possible captures for the current pice
                {

                    piece->block = blockOld; // change block to previous.
                    piece->dir = dirOld;     // change direction to previous.
                }
            }
        }

        // if moved, go to next iteration
        if (isMoved)
        {
            isMovedAny = 1;
            continue;
        }

        // move closest piece to home

        short step2h[4][2] = {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}}; // [stepsToHome][pieceIndex]

        // add steps to home of each piece to the array
        for (short i = 0; i < 4; i++)
        {
            piece_t *piece = piecePtr[YELLOW][i];

            // if piece is in home or base, check next piece
            if (piece->loc == yellow.homeStraight + 5 || piece->loc < 0)
            {
                continue;
            }

            step2h[i][0] = steps2home(YELLOW, i);
            step2h[i][1] = i;
        }

        // sort the array by step count
        sort2d(step2h, 4);

        // try moving starting from the piece closest to its home
        for (short i = 0; i < 4; i++)
        {
            if (step2h[i][0] == -1)
            {
                continue;
            }

            char pieceIdx = (short)step2h[i][1];
            piece_t *piece = piecePtr[YELLOW][pieceIdx];

            short destination = getDest(YELLOW, pieceIdx, value);
            short blockDest = isBlocked(YELLOW, pieceIdx, destination);

            // if current piece is blocked set destination to adjacent cell
            if (blockDest != -1)
            {
                destination = blockDest;
            }

            // move only if destination is not equal to current location
            if (destination != piece->loc)
            {

                // try to move the piece
                if (canBlock(YELLOW, pieceIdx, destination))
                {
                    block(YELLOW, pieceIdx, destination);
                    isMoved = 1;
                    break;
                }
                else
                {
                    if (piece->block == 1)
                    {
                        movesf(YELLOW, pieceIdx, destination);
                        isMoved = 1;
                        break;
                    }
                    else
                    {
                        moveBlock(YELLOW, pieceIdx, destination);
                        isMoved = 1;
                        break;
                    }
                }
            }

            // try to break a block then move
            if (piece->block > 1)
            {
                // dtore previos values
                char dirOld = piece->dir;
                char blockOld = piece->block;

                piece->block = 1;           // change block to '1' for the current piece.
                piece->dir = piece->orgDir; // change direction to original direction

                short destination = getDest(YELLOW, pieceIdx, value);
                short blockDest = isBlocked(YELLOW, pieceIdx, destination);

                // if current piece is blocked set destination to adjacent cell
                if (blockDest != -1)
                {
                    destination = blockDest;
                }

                // if its not gonna move, check next piece.
                if (destination == piece->loc)
                {
                    piece->block = blockOld; // change block to previous.
                    piece->dir = dirOld;     // change direction to previous.
                    continue;
                }

                // if it is gonna make another block, make a block
                if (canBlock(YELLOW, pieceIdx, destination))
                {
                    // update block value of rest of the members
                    for (short j = 0; j < 4; j++)
                    {

                        if (j == pieceIdx)
                        {
                            continue;
                        }

                        piece_t *piece2 = piecePtr[YELLOW][j];

                        if (piece->loc == piece2->loc)
                        {
                            piece2->block--;
                        }
                    }

                    // make a block
                    block(YELLOW, pieceIdx, destination);

                    isMoved = 1;
                    break;
                }
                else
                {
                    // update block value of rest of the members
                    for (short j = 0; j < 4; j++)
                    {

                        if (j == pieceIdx)
                        {
                            continue;
                        }

                        piece_t *piece2 = piecePtr[YELLOW][j];

                        if (piece->loc == piece2->loc)
                        {
                            piece2->block--;
                        }
                    }

                    // move the piece
                    movesf(YELLOW, pieceIdx, destination);
                    isMoved = 1;
                    break;
                }
            }
        }

        // if moved, go to next iteration
        if (isMoved)
        {
            isMovedAny = 1;
            continue;
        }

    } while (value == 6 || bonus == 1);

    return isMovedAny;
}

// behaviour of blue player
char bluePlayer()
{

    char value;       // the value get by rolling the dice
    short count6 = 0; // number of times the dice is rolled
    char bonus = 0;
    char isMovedAny = 0; // whether the player have done at least one move

    do
    {
        // pass the roll if wins
        if (blue.place != 0)
        {
            return isMovedAny;
        }

        // printmap();

        bonus = 0;
        char isMoved = 0;
        value = roll(BLUE);

        rollValue = value;


        // increment count3 when 'three' is rolled
        if (value == 3)
        {
            blue.count3++;

            // if any piece in Kotuwa, returns it to base
            
            for (short i = 0; i < 4; i++)
            {
                piece_t *piece = piecePtr[BLUE][i];

                
                if (piece->effect == BRIEF)
                {
                     printf("%s piece %d is movement-restricted and has rolled three consecutively. Teleporting piece %d to base.\n",
                    "Blue", i + 1, i + 1);
                    back2bace(BLUE, i);
                }
            }
        }
        else
        {
            blue.count3 = 0;
        }

        // increment count only if player rolls '6'
        if (value == 6)
        {
            count6++;
        }
        else
        {
            // pass to the next player if there is not piece in the board
            if (blue.base == (4 - blue.home))
            {
                break;
            }
            count6 = 0;
        }

        // if '6' is rolled 3times consecutively, break blocks if player has. if doesnt, ignore the roll and pass to the next player
        if (count6 == 3)
        {
            break;
        }

        // try possible moves according to the priority

        for (int p = 0; p < 4; p++)
        {
            nxtBlueIdx = (nxtBlueIdx + 1) % 4;

            // set current piece by priority
            piece_t *piece = piecePtr[BLUE][nxtBlueIdx];

            // get priority  piece to the board if it's not in base
            if (blue.base > 0 && value == 6 && piece->loc == -1)
            {
                short capIdx;
                char pieceIdx = base2x(BLUE);

                if (pieceIdx != -1)
                {
                    if ((capIdx = checkCap(BLUE, pieceIdx, yellow.x)) != -1)
                    {
                        capture(BLUE, pieceIdx, capIdx / 4, capIdx % 4);
                        bonus = 1;
                    }
                    else if (canBlock(BLUE, pieceIdx, blue.x))
                    {

                        block(BLUE, pieceIdx, blue.x);
                    }
                    isMoved = 1;
                    break;
                }
            }

            // if piece is not in the board, try next piece
            if (piece->loc == -1 || piece->loc == blue.homeStraight + 5)
            {
                continue;
            }

            // get destination of current piece
            short destination = getDest(BLUE, nxtBlueIdx, value);

            // if piece is not gonna move, consider next piece
            if (destination == piece->loc && piece->block == 1 || destination > blue.homeStraight + 5)
            {
                continue;
            }

            // prioritize landing or not landing on mystery cell  on direction

            // clockwise moves, avoid mystery cells
            if (piece->dir == 1)
            {

                // try move to a cell which is not mystery.
                if (destination != mstryCell && destination != piece->loc)
                {
                    // move/block/capture the piece/block
                    char movedState = movesf(BLUE, nxtBlueIdx, destination);

                    if (movedState != 0)
                    {

                        // check it captured any piece
                        if (movedState == 1)
                        {
                            bonus = 1;
                        }

                        isMoved = 1;
                        break;
                    }
                }

                // try to break a block and move to a cell which is not mystery.
                if (piece->block > 1)
                {

                    // store previos values
                    char dirOld = piece->dir;
                    char blockOld = piece->block;

                    piece->block = 1;           // change block to '1' for the current piece.
                    piece->dir = piece->orgDir; // change direction to original direction

                    short destination = getDest(BLUE, nxtBlueIdx, value);

                    // if its not gonna move, check next piece.
                    if (destination == piece->loc || destination > blue.homeStraight + 5)
                    {
                        piece->block = blockOld; // change block to previous.
                        piece->dir = dirOld;     // change direction to previous.
                        continue;
                    }

                    // if it is gonna move avoiding mystery cell, move it.

                    if (destination != mstryCell)
                    {
                        // update block value of rest of the members
                        for (short j = 0; j < 4; j++)
                        {

                            if (j == nxtBlueIdx)
                            {
                                continue;
                            }

                            piece_t *piece2 = piecePtr[BLUE][j];

                            if (piece->loc == piece2->loc)
                            {
                                piece2->block--;
                            }
                        }

                        // move/block/capture the piece/block
                        char movedState = movesf(BLUE, nxtBlueIdx, destination);

                        // check it captured any piece
                        if (movedState == 1)
                        {
                            bonus = 1;
                        }

                        isMoved = 1;
                        break;
                    }
                    // destination is mystery
                    else
                    {

                        piece->block = blockOld; // change block to previous.
                        piece->dir = dirOld;     // change direction to previous.
                    }
                }

                // try move to mystery
                if (destination == mstryCell && destination != piece->loc)
                {
                    // move/block/capture the piece/block
                    char movedState = movesf(BLUE, nxtBlueIdx, destination);

                    if (movedState != 0)
                    {

                        // check it captured any piece
                        if (movedState == 1)
                        {
                            bonus = 1;
                        }

                        isMoved = 1;
                        break;
                    }
                }

                // try to move to mystery by breaking a block
                if (piece->block > 1)
                {

                    // store previos values
                    char dirOld = piece->dir;
                    char blockOld = piece->block;

                    piece->block = 1;           // change block to '1' for the current piece.
                    piece->dir = piece->orgDir; // change direction to original direction

                    short destination = getDest(BLUE, nxtBlueIdx, value);

                    // if its not gonna move, check next piece.
                    if (destination == piece->loc || destination > blue.homeStraight + 5)
                    {
                        piece->block = blockOld; // change block to previous.
                        piece->dir = dirOld;     // change direction to previous.
                        continue;
                    }

                    // if it is gonna move avoiding mystery cell, move it.

                    if (destination == mstryCell)
                    {
                        // update block value of rest of the members
                        for (short j = 0; j < 4; j++)
                        {

                            if (j == nxtBlueIdx)
                            {
                                continue;
                            }

                            piece_t *piece2 = piecePtr[BLUE][j];

                            if (piece->loc == piece2->loc)
                            {
                                piece2->block--;
                            }
                        }

                        // move/block/capture the piece/block
                        char movedState = movesf(BLUE, nxtBlueIdx, destination);

                        // check it captured any piece
                        if (movedState == 1)
                        {
                            bonus = 1;
                        }

                        isMoved = 1;
                        break;
                    }

                    // destination is not mystery
                    else
                    {

                        piece->block = blockOld; // change block to previous.
                        piece->dir = dirOld;     // change direction to previous.
                    }
                }
            }
            // counter-clockwise moves
            else
            {

                // try move to mystery
                if (destination == mstryCell && destination != piece->loc)
                {
                    // move/block/capture the piece/block
                    char movedState = movesf(BLUE, nxtBlueIdx, destination);

                    if (movedState != 0)
                    {

                        // check it captured any piece
                        if (movedState == 1)
                        {
                            bonus = 1;
                        }

                        isMoved = 1;
                        break;
                    }
                }

                // try to move to mystery by breaking a block
                if (piece->block > 1)
                {

                    // store previos values
                    char dirOld = piece->dir;
                    char blockOld = piece->block;

                    piece->block = 1;           // change block to '1' for the current piece.
                    piece->dir = piece->orgDir; // change direction to original direction

                    short destination = getDest(BLUE, nxtBlueIdx, value);

                    // if its not gonna move, check next piece.
                    if (destination == piece->loc || destination > blue.homeStraight + 5)
                    {
                        piece->block = blockOld; // change block to previous.
                        piece->dir = dirOld;     // change direction to previous.
                        continue;
                    }

                    // if it is gonna move avoiding mystery cell, move it.

                    if (destination == mstryCell)
                    {
                        // update block value of rest of the members
                        for (short j = 0; j < 4; j++)
                        {

                            if (j == nxtBlueIdx)
                            {
                                continue;
                            }

                            piece_t *piece2 = piecePtr[BLUE][j];

                            if (piece->loc == piece2->loc)
                            {
                                piece2->block--;
                            }
                        }

                        // move/block/capture the piece/block
                        char movedState = movesf(BLUE, nxtBlueIdx, destination);

                        // check it captured any piece
                        if (movedState == 1)
                        {
                            bonus = 1;
                        }

                        isMoved = 1;
                        break;
                    }

                    // destination is not mystery
                    else
                    {

                        piece->block = blockOld; // change block to previous.
                        piece->dir = dirOld;     // change direction to previous.
                    }
                }

                // try move to a cell which is not mystery.
                if (destination != mstryCell && destination != piece->loc)
                {
                    // move/block/capture the piece/block
                    char movedState = movesf(BLUE, nxtBlueIdx, destination);

                    if (movedState != 0)
                    {

                        // check it captured any piece
                        if (movedState == 1)
                        {
                            bonus = 1;
                        }

                        isMoved = 1;
                        break;
                    }
                }

                // try to break a block and move to a cell which is not mystery.
                if (piece->block > 1)
                {

                    // store previos values
                    char dirOld = piece->dir;
                    char blockOld = piece->block;

                    piece->block = 1;           // change block to '1' for the current piece.
                    piece->dir = piece->orgDir; // change direction to original direction

                    short destination = getDest(BLUE, nxtBlueIdx, value);

                    // if its not gonna move, check next piece.
                    if (destination == piece->loc || destination > blue.homeStraight + 5)
                    {
                        piece->block = blockOld; // change block to previous.
                        piece->dir = dirOld;     // change direction to previous.
                        continue;
                    }

                    // if it is gonna move avoiding mystery cell, move it.

                    if (destination != mstryCell)
                    {
                        // update block value of rest of the members
                        for (short j = 0; j < 4; j++)
                        {

                            if (j == nxtBlueIdx)
                            {
                                continue;
                            }

                            piece_t *piece2 = piecePtr[BLUE][j];

                            if (piece->loc == piece2->loc)
                            {
                                piece2->block--;
                            }
                        }

                        // move/block/capture the piece/block
                        char movedState = movesf(BLUE, nxtBlueIdx, destination);

                        // check it captured any piece
                        if (movedState == 1)
                        {
                            bonus = 1;
                        }

                        isMoved = 1;
                        break;
                    }
                    // destination is mystery
                    else
                    {

                        piece->block = blockOld; // change block to previous.
                        piece->dir = dirOld;     // change direction to previous.
                    }
                }
            }

            if (isMoved == 1)
            {
                isMovedAny = 1;
                break;
            }
        }

    } while (value == 6 || bonus == 1);

    return isMovedAny;
}

// generate mistery cell  on new position
void genMstryCell()
{

    char randNo;

    // generate random numbers until get unique one
    while (1)
    {
        randNo = rand_no(52) - 1; // generate random number (0 - 51)

        int i;
        for (i = 0; i < 16; i++)
        {

            piece_t *piece = piecePtr[i / 4][i % 4];

            // if its on a cell which has a pice or same as previous mystery cell, try different random number
            if (piece->loc == randNo || randNo == mstryCell)
            {
                break;
            }
        }

        // random number can be used as mystery cell
        if (i == 16)
        {

            mstryCell = randNo;
            return;
        }
    }
}

// print location of each pieces at end of each round
void printmap()
{

    printf("Round %d\n", roundNo);

    for (int i = 0; i < 4; i++)
    {
        player_t *player = playerPtr[i];

        printf("%s player now has %d/4 on pieces on the board and %d/4 pieces on the base.\n",
               player->name, 4 - player->base - player->home, player->base);
        printf("============================================================================\n");
        printf("Location of pieces %s\n", player->name);
        printf("============================================================================\n");

        for (int j = 0; j < 4; j++)
        {
            piece_t *piece = piecePtr[i][j];

            char location[6];

            if (piece->loc == -1)
            {

                sprintf(location, "%s", "Base");
            }
            else if (piece->loc == player->homeStraight + 5)
            {

                sprintf(location, "%s", "Home");
            }
            else
            {

                sprintf(location, "%d", piece->loc);
            }

            printf("Piece %d - > %s.\n", j + 1, location);
        }
        printf("\n");
    }
    if (roundNo >= 2)
    {
        printf("The mystery cell is at %d and will be at that location for the next %d values.\n", mstryCell, 4 - (roundNo - 2) % 4);
    }
    else
    {
        printf("\n");
    }
}

//testing purpose
void printmap1()
{
    for (int i = 0; i < 4; i++)
    {
        player_t *player = playerPtr[i];
        printf("%s\t", player->name);

        for (int j = 0; j < 4; j++)
        {
            piece_t *piece = piecePtr[i][j];
            printf("p%d (%02d).%d (%02d/%02d)(%02d)\t\t", j + 1, piece->loc, piece->block, piece->dir, piece->orgDir, piece->cap);
        }
        printf("\n");
    }
}

// teleport to random locations (1/6), when a piece reach a mystery cell
void teleport(char playerIdx, char pieceIdx)
{

    player_t *player = playerPtr[playerIdx];
    piece_t *piece = piecePtr[playerIdx][pieceIdx];

    if (piece->loc != mstryCell)
    {
        return;
    }

    while (1)
    {
        char destination;
        char isTeleported = 0;
        char randNo = rand_no(6);

        switch (randNo)
        {
        case BHAWANA:

            destination = 9;

            // check whether destination is blockeed or gonna make block
            if (checkCell(playerIdx, destination) == 3 || canBlock(playerIdx, pieceIdx, destination))
            {
                break;
            }

            char randNo2 = rand_no(2);

            for (char i = 0; i < 4; i++)
            {
                piece_t *piece = piecePtr[playerIdx][i];

                if (piece->loc == mstryCell)
                {

                    printf("%s piece %d teleported to Bhawana.\n", player->name, pieceIdx + 1);

                    switch (randNo2)
                    {

                    case ENERGIZE:

                        printf("%s piece %d feels energized, and movement speed doubles.\n", player->name, pieceIdx + 1);

                        piece->effect = ENERGIZE;
                        piece->affectedOn = roundNo;

                        break;

                    case SICK:

                        printf("%s piece %d feels sick, and movement speed halves.\n", player->name, pieceIdx + 1);

                        piece->effect = SICK;
                        piece->affectedOn = roundNo;
                    }
                }
            }

            char capIdx = checkCap(playerIdx, pieceIdx, destination);

            // not possible to capture
            if (capIdx != -1)
            {

                capture(playerIdx, pieceIdx, capIdx / 4, capIdx % 4);
                isTeleported = 2;
            }
            else
            {

                for (char i = 0; i < 4; i++)
                {
                    piece_t *piece = piecePtr[playerIdx][i];

                    if (piece->loc == mstryCell)
                    {
                        move(playerIdx, i, destination, 1);
                    }
                }
                isTeleported = 2;
            }

            break;

        case KOTUWA:

            destination = 27;

            // check whether destination is blockeed or gonna make block
            if (checkCell(playerIdx, destination) == 3 || canBlock(playerIdx, pieceIdx, destination))
            {
                break;
            }

            for (char i = 0; i < 4; i++)
            {
                piece_t *piece = piecePtr[playerIdx][i];

                if (piece->loc == mstryCell)
                {
                    printf("%s piece %d teleported to Kotuwa.\n", player->name, i + 1);
                    printf("%s piece %d attends briefing and cannot move for four rounds\n", player->name, i + 1);
                    piece->effect = BRIEF;
                    piece->affectedOn = roundNo;
                }
            }

            capIdx = checkCap(playerIdx, pieceIdx, destination);

            // not possible to capture
            if (capIdx != -1)
            {

                capture(playerIdx, pieceIdx, capIdx / 4, capIdx % 4);
                isTeleported = 2;
            }
            else
            {

                for (char i = 0; i < 4; i++)
                {
                    piece_t *piece = piecePtr[playerIdx][i];

                    if (piece->loc == mstryCell)
                    {
                        move(playerIdx, i, destination, 1);
                    }
                }
                isTeleported = 1;
            }

            break;
        case PITA_KOTUWA:

            // clockwise move
            if (piece->dir == 1)
            {

                destination = 46;

                // check whether destination is blockeed
                if (checkCell(playerIdx, destination) == 3 || canBlock(playerIdx, pieceIdx, destination))
                {
                    break;
                }

                for (char i = 0; i < 4; i++)
                {
                    piece_t *piece = piecePtr[playerIdx][i];

                    if (piece->loc == mstryCell)
                    {
                        printf("%s piece %d teleported to Pita-Kotuwa.\n", player->name, i + 1);
                        printf("%s piece %d which was moving clockwise, has changed to moving counterclockwise.\n", player->name, i + 1);
                        piece->dir = -1;
                        piece->orgDir = -1;
                    }
                }

                capIdx = checkCap(playerIdx, pieceIdx, destination);

                // not possible to capture
                if (capIdx != -1)
                {

                    capture(playerIdx, pieceIdx, capIdx / 4, capIdx % 4);
                    isTeleported = 2;
                }
                else
                {

                    for (char i = 0; i < 4; i++)
                    {
                        piece_t *piece = piecePtr[playerIdx][i];

                        if (piece->loc == mstryCell)
                        {
                            move(playerIdx, i, destination, 1);
                        }
                    }
                    isTeleported = 1;
                }
            }
            // counter-clockwise move
            else
            {
                destination = 27;

                // check whether destination is blockeed or gonna make block
                if (checkCell(playerIdx, destination) == 3 || canBlock(playerIdx, pieceIdx, destination))
                {
                    break;
                }

                for (char i = 0; i < 4; i++)
                {
                    piece_t *piece = piecePtr[playerIdx][i];

                    if (piece->loc == mstryCell)
                    {
                        printf("%s piece %d teleported to Pita-Kotuwa.\n", player->name, i + 1);
                        printf("%s piece %d is moving in a counterclockwise direction. Teleporting to Kotuwa from Pita-Kotuwa.\n", player->name, i + 1);
                        piece->effect = BRIEF;
                        piece->affectedOn = roundNo;
                    }
                }

                capIdx = checkCap(playerIdx, pieceIdx, destination);

                // not possible to capture
                if (capIdx != -1)
                {

                    capture(playerIdx, pieceIdx, capIdx / 4, capIdx % 4);
                    isTeleported = 2;
                }
                else
                {

                    for (char i = 0; i < 4; i++)
                    {
                        piece_t *piece = piecePtr[playerIdx][i];

                        if (piece->loc == mstryCell)
                        {
                            move(playerIdx, i, destination, 1);
                        }
                    }
                    isTeleported = 1;
                }
            }

            break;

        case BASE:
            destination = -1;

            for (char i = 0; i < 4; i++)
            {
                piece_t *piece = piecePtr[playerIdx][i];

                if (piece->loc == mstryCell)
                {
                    printf("%s piece %d teleported to Base.\n", player->name, i + 1);
                    back2bace(playerIdx, i);
                    isTeleported = 1;
                }
            }

            break;
        case X:
            destination = player->x;

            // check whether destination is blockeed or gonna make block
            if (checkCell(playerIdx, destination) == 3 || canBlock(playerIdx, pieceIdx, destination))
            {
                break;
            }

            for (char i = 0; i < 4; i++)
            {
                piece_t *piece = piecePtr[playerIdx][i];

                if (piece->loc == mstryCell)
                {
                    printf("%s piece %d teleported to X.\n", player->name, i + 1);
                }
            }

            capIdx = checkCap(playerIdx, pieceIdx, destination);

            // not possible to capture
            if (capIdx != -1)
            {

                capture(playerIdx, pieceIdx, capIdx / 4, capIdx % 4);
                isTeleported = 2;
            }
            else
            {

                for (char i = 0; i < 4; i++)
                {
                    piece_t *piece = piecePtr[playerIdx][i];

                    if (piece->loc == mstryCell)
                    {
                        move(playerIdx, i, destination, 1);
                    }
                }
                isTeleported = 1;
            }

            break;

        case APPROACH:

            destination = player->approach;

            // check whether destination is blockeed or gonna make block
            if (checkCell(playerIdx, destination) == 3 || canBlock(playerIdx, pieceIdx, destination))
            {
                break;
            }

            for (char i = 0; i < 4; i++)
            {
                piece_t *piece = piecePtr[playerIdx][i];

                if (piece->loc == mstryCell)
                {
                    printf("%s piece %d teleported to Approach.\n", player->name, i + 1);
                }
            }

            capIdx = checkCap(playerIdx, pieceIdx, destination);

            // not possible to capture
            if (capIdx != -1)
            {

                capture(playerIdx, pieceIdx, capIdx / 4, capIdx % 4);
                isTeleported = 2;
            }
            else
            {

                for (char i = 0; i < 4; i++)
                {
                    piece_t *piece = piecePtr[playerIdx][i];

                    if (piece->loc == mstryCell)
                    {
                        move(playerIdx, i, destination, 1);
                    }
                }
                isTeleported = 1;
            }
        }

        if (isTeleported)
        {
            break;
        }
    }
}

// reset mystery effects of each pieces
void rstEffect()
{

    for (int i = 0; i < 16; i++)
    {

        piece_t *piece = piecePtr[i / 4][i % 4];

        // reset effects
        if (piece->affectedOn != 0 || piece->affectedOn + 4 == roundNo)
        {
            piece->affectedOn = 0;
            piece->effect = 0;
        }
    }
}
