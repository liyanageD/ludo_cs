#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct piece
{
    short loc;
    short cap;
    short dir;
    short moves;
    short round;
};
typedef struct playerStr
{
    char name[7];
    struct piece p1;
    struct piece p2;
    struct piece p3;
    struct piece p4;
    char base;
    char home;
    const char x;
    const char approach;
    short homeStraight;

} playert;

// declare players and initialize values
playert red = {"red", -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 4, 0, 28, 26, 80};
playert green = {"green", -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 4, 0, 41, 39, 90};
playert yellow = {"yellow", -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 4, 0, 2, 0, 60};
playert blue = {"blue", -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 4, 0, 15, 13, 70};

playert *playerPtr[] = {&red, &green, &yellow, &blue};

struct piece *piecePtr[4][4] = {{&red.p1, &red.p2, &red.p3, &red.p4}, {&green.p1, &green.p2, &green.p3, &green.p4}, {&yellow.p1, &yellow.p2, &yellow.p3, &yellow.p4}, {&blue.p1, &blue.p2, &blue.p3, &blue.p4}};

void resetp(struct piece *x)
{
    x->cap = 0;
    x->dir = 0;
    x->loc = -1;
    x->moves = 0;
}

int rand_no(int range)
{
    return 1 + rand() % range;
}

short rollf(playert *player)
{
    short value;
    value = rand_no(6);
    printf("%s rolls %d\n", player->name, value);
    return value;
}

short fPlayer(playert *playerPtr[])
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

short roll(playert *player)
{
    short value;
    value = rand_no(6);
    printf("%s player rolled %d\n", player->name, value);
    return value;
}

void toss(struct piece *x)
{
    if (rand_no(2) == 1)
    {
        x->dir = 1;
    }
    else
    {
        x->dir = -1;
    }
}

// *from base to starting poin
void base2x(char p)
{

    for (int i = 0; i < 4; i++)
    {
        if (piecePtr[p][i]->loc == -1)
        {
            piecePtr[p][i]->loc = playerPtr[p]->x;
            playerPtr[p]->base--;
            printf("%s player moves piece X%d to the starting point.\n", playerPtr[p]->name, i + 1);
            printf("%s player now has %d/4 on pieces on the board and %d/4 pieces on the base.\n\n",
                   playerPtr[p]->name, 4 - playerPtr[p]->base - playerPtr[p]->home, playerPtr[p]->base);

            piecePtr[p][i]->round++;

            toss(piecePtr[p][i]);

            break;
        }
    }
};

void move(char player, char piece, char destination, char diceValue)
{
    playert *p = playerPtr[player];
    struct piece *x = piecePtr[player][piece];

    // clockwise movements
    if (x->dir == 1)
    {

        if (destination > 51)
        {
            // go from standard to Homestraight or Home
            if (x->loc <= 51 && x->loc >= 0)
            {

                // go from standard to Home
                if (destination == p->homeStraight + 5)
                {
                    p->home++;
                    printf("%s moves piece %d from location %d to Home by %d units in clockwise direction.",
                           p->name, piece + 1, x->loc, diceValue);

                    // go from standard to homestraight
                }
                else
                {
                    printf("%s moves piece %d from location %d to homestraight cell %d by %d units.", p->name, piece + 1,
                           x->loc, destination - p->homeStraight + 1, diceValue);
                }
            }
            // movements within homestraight
            else
            {
                short locOld = x->loc - p->homeStraight + 1;

                // move from homestraight to Home or homestraight
                if (destination <= p->homeStraight + 5)
                {

                    // move from homestraight to Home
                    if (destination == p->homeStraight + 5)
                    {
                        printf("%s moves piece %d from homestraight cell %d to Home by %d units.", p->name, piece + 1, locOld, diceValue);
                        p->home++;
                    }
                    // move from homestraight to homestraight
                    else
                    {
                        printf("%s moves piece %d from homestraight cell %d to homestraight cell %d by %d units.", p->name,
                               piece + 1, locOld, destination - p->homeStraight + 1, diceValue);
                    }
                }
                // exceeding Home
                else
                {
                    printf("%s cannot moves piece %d from homestraight cell %d by %d units due to exceeding Home.", p->name,
                           piece + 1, locOld + 1, diceValue);

                    destination = x->loc;
                    diceValue = 0;
                }
            }
        }
        // movements within standard path
        else
        {

            if (destination > 51)
            {
                x->round++;
            }

            printf("%s moves piece %d from location %d to %d by %d units in clockwise direction.",
                   p->name, piece + 1, x->loc, destination, diceValue);
        }
    }
    // counterclockwise movements
    else if (x->dir == -1)
    {

        if (destination > 51)
        {
            // go from standard to Homestraight or Home
            if (x->loc <= 51 && x->loc >= 0)
            {

                // go from standard to Home
                if (destination == p->homeStraight + 5)
                {
                    p->home++;
                    printf("%s moves piece %d from location %d to Home by %d units in counter-clockwise direction..",
                           p->name, piece + 1, x->loc, diceValue);

                    // go from standard to homestraight
                }
                else
                {
                    printf("%s moves piece %d from location %d to homestraight cell %d by %d units in counter-clockwise direction.",
                           p->name, piece + 1, x->loc, destination - p->homeStraight + 1, diceValue);
                }
                
            }
            
        }
        // movements within standard path
        else
        {
            short locOld = x->loc;
            if (x->loc - value < 0)
            {
                x->loc -= value + 52;
                x->round++;
            }
            else
            {
                x->loc -= value;
            }

            x->moves += value;
            printf("%s moves piece %d from location %d to %d by %d units in counter-clockwise direction.", p->name, piece + 1,
                   locOld, x->loc, value);
        }
    }


    // movements within homestraight
            if (x->loc > 51)
            {
                short locOld = x->loc - p->homeStraight + 1;

                // move from homestraight to Home or homestraight
                if (destination <= p->homeStraight + 5)
                {

                    // move from homestraight to Home
                    if (destination == p->homeStraight + 5)
                    {
                        printf("%s moves piece %d from homestraight cell %d to Home by %d units.", 
                        p->name, piece + 1, locOld, diceValue);
                    }
                    // move from homestraight to homestraight
                    else
                    {
                        printf("%s moves piece %d from homestraight cell %d to homestraight cell %d by %d units", p->name,
                               piece + 1, locOld, destination, diceValue);
                    }
                    
                }
                else
                // exceeding Home
                {
                    printf("%s cannot moves piece %d from homestraight cell %d due to exceeding Home.", p->name,
                           piece + 1, x->loc - p->homeStraight + 1);
                }
                
            }
}

// returns destination cell of a piece
short getDest(char player, char piece, char value)
{
    playert *p = playerPtr[player];
    struct piece *x = piecePtr[player][piece];
    short destination;

    // clockwise movements
    if (x->dir == 1)
    {

        if (x->moves + value > 50 + (x->round - 1) * 52 && x->cap >= 1)
        {
            // go from standard to Homestraight or Home
            if (x->loc <= 51 && x->loc >= 0)
            {

                short locOld = x->loc;
                destination = x->loc + value - (p->approach + 1) + p->homeStraight;

                // movements within homestraight
            }
            else if (x->loc + value <= p->homeStraight + 5)
            {

                destination = x->loc + value;

                // exceeding Home
            }
            else
            {
                destination = NULL;
            }
            // movements within standard path
        }
        else
        {

            if (x->loc + value > 51)
            {
                destination = x->loc + value - 52;
            }
            else
            {
                destination = x->loc + value;
            }
        }
    }
    // counterclockwise movements
    else if (x->dir == -1)
    {

        if (x->moves + value > (x->round) * 54 && x->cap >= 1)
        {
            // go from standard to Homestraight or Home
            if (x->loc <= 51 && x->loc >= 0)
            {

                destination = x->loc + value - (p->approach + 1) + p->homeStraight;

                // movements within homestraight
            }
            else if (x->loc + value <= p->homeStraight + 5)
            {

                destination = x->loc + value;

                // move from homestraight to Home

                // exceeding Home
            }
            else
            {
                destination = NULL;
            }

            // movements within standard path
        }
        else
        {
            short locOld = x->loc;
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

    return destination;
}

// behaviour of red player

void redPlayer()
{

    char value = 0;
    short count = 1;

    do
    {
        value = roll(&red);

        if (value == 6 && count == 3)
        {
            break;
        }

        if (red.base == 4 && value == 6)
        {
            base2x(0);
        }

        count++;

    } while (value == 6 && count <= 3);
}

void play()
{
    // set seed
    srand(time(0));

    // print players and pieces they have
    printf("The red player has four (04) pieces named R1, R2, R3, and R4.\n");
    printf("The green player has four (04) pieces named G1, G2, G3, and G4.\n");
    printf("The yellow player has four (04) pieces named Y1, Y2, Y3, and Y4.\n");
    printf("The blue player has four (04) pieces named B1, B2, B3, and B4.\n");

    // choose first player
    short fIndex = fPlayer(playerPtr);

    printf("The order of a single round is %s, %s, %s, and %s.\n\n", playerPtr[fIndex]->name,
           playerPtr[(fIndex + 1) % 4]->name, playerPtr[(fIndex + 2) % 4]->name, playerPtr[(fIndex + 3) % 4]->name);

    // loop for round
    // while(1){

    //}
}

int main()
{
    srand(time(0));
    system("cls");
    // play();
    redPlayer();
    // printf("%d ",green.p2.dir);
    return 0;
}