#ifndef TYPE_H

#define TYPE_H

enum players
{
    RED,
    GREEN,
    YELLOW,
    BLUE
};



enum mystery
{
    BHAWANA = 1,
    KOTUWA,
    PITA_KOTUWA,
    BASE,
    X,
    APPROACH
};

enum effects
{
    ENERGIZE = 1,
    SICK,
    BRIEF
};

// structure for pieces
typedef struct
{
    short loc; //absolute location of the piece (-1 -> Base, {65, 75, 85, 95} -> Home, {0 to 51} -> standard path)
    short cap; //number of times a piece has captured apponent piece
    short dir;    // current direction of piece
    short orgDir; // original direction of piece
    short moves; // relative location corresponding to player's approach cell (for yellow this is absolute)
    short aprchCount; // number of times a pice passes its approach
    char block; // size of block (number of members in the block)
    char effect; // effect after reach mystery cell (0 -> no effect)
    short affectedOn; //round number that the effect happened

} piece_t;

// structure for players
typedef struct playerStr
{

    char name[7]; // color of the player
    piece_t p1;//piece 1
    piece_t p2;//piece 2
    piece_t p3;//piece 3
    piece_t p4;//piece 4
    char base;            // number of pices in the base
    char home;            // number of pices in the home
    const short x;        // ID of starting point of a piece
    const short approach; // ID of approach cell
    short homeStraight;   // ID of starting cell in the homestraight
    char place; // winning place of the player
    char count3; //number of times a player rolled 3 consecutively

} player_t;

// function prototype

// reset values of a piece
void resetp(piece_t *piece);

// generate random number
short rand_no(short range);

// roll dice to choose first player
short rollf(player_t *player);

// choose first player
short fPlayer(player_t *playerPtr[]);

// roll dice
short roll(char playerIdx);

// decide direction of a player by a coin toss
void toss(piece_t *x);

// *from base to starting point
char base2x(char playerIdx);

// reurn a piece to its base
void back2bace(char playerIndex, char pieceIndex);

// return a piece or a block back to its home and return the size of block
char back2baceBlock(char playerIndex, char pieceIndex);

// returns the difference between destination and the current location (steps to move)
char getDiff(char playerIdx, char pieceIdx, short destination);

// move piece to the destination. (not moved, returns 0)
char move(char player, char piece, char destination, char flag);

// move as a block. (not moved, returns 0)
char moveBlock(char playerIndex, char pieceIndex, short value);

// returns destination cell of a piece
short getDest(char player, char piece, char value);

// returns number of moves to reach home for a particular piece (without considering constraints)
short steps2home(char playerIndex, char pieceIndex);

// check whether a piece can capture an apponent piece or a block, returns index of the piece which can be captured, else -1.
short checkCap(char playerIndex, char pieceIndex, short destination);

// check whether a piece is blocked. if blocked, returns the adjacent location of the block. Else, returns -1.
short isBlocked(char playerIndex, char pieceIndex, short destination);

// check whether a pice can make a block with a piece. if it can, returns 1.
char canBlock(char playerIndex, char pieceIndex, short destination);

// *********************************** make a block *****************************
char block(char playerIndex, char pieceIndex, short destination);

// player1's piece captures player2's piece
void capture(char player1Index, char piece1Index, char player2Index, char piece2Index);

// if it's possible to capture, then capture it. (0 - cannot capture)
char checknCap(char playerIndex, char pieceIndex, short destination);

// sort an array
void sort(short *arr, short size);

// sort a '[size][2]' array by first column
void sort2d(short (*arr)[2], short size);

// check whether there is a piece or a block in the cell, returns if (no_piece -> 0, player's_piece -> 1, apponent_piece -> 2, apnt_block -> 3)
char checkCell(char playerIdx, short cell);

// behaviour of red player
char redPlayer(void);

// behaviour of Green player
char greenPlayer(void);

// behaviour of yellow player
char yellowPlayer(void);

// behaviour of blue player
char bluePlayer(void);

void play(void);

void printmap1();

// generate mistery cell  on new position
void genMstryCell(void);

// print positions of each piece at the end of a round
void printmap(void);

// teleport to random locations (1/6), when a piece reach a mystery cell
void teleport(char playerIdx, char pieceIdx);

// reset mystery effects of each pieces
void rstEffect();

#endif