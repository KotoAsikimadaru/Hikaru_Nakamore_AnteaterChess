/******************************************************************************
 * File: GameData.c
 * Author: Vinh Nguyen
 * Date: April 03, 2026
 * 
 * * Description:
 * Implements the core board manipulation and rendering functions. 
 * Handles the memory initialization of the 8x10 board matrix to the 
 * Anteater Chess starting state and handles terminal-based ASCII 
 * grid output.
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "GameData.h"

//=============================================================================

void InitializeBoard(Board* pBoard)
{
    /* Initialize the board with no pieces w/ no color */
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            pBoard->grid[row][col].color = ' ';
            pBoard->grid[row][col].type = ' ';
        }
    }

    /* Set up pawns for both colors */
    for (int col = 0; col < COLS; col++) {
        pBoard->grid[1][col].color = 'w';
        pBoard->grid[1][col].type = 'P';

        pBoard->grid[6][col].color = 'b';
        pBoard->grid[6][col].type = 'P';
    }

    /* Set up higher level pieces for both colors */
    char pieceOrder[10] = {'R', 'N', 'B', 'A', 'Q', 'K', 'A', 'B', 'N', 'R'};

    for (int col = 0; col < COLS; col++) {
        pBoard->grid[0][col].color = 'w';
        pBoard->grid[0][col].type = pieceOrder[col];

        pBoard->grid[7][col].color = 'b';
        pBoard->grid[7][col].type = pieceOrder[col];
    }
}

//=============================================================================

void PrintBoard(Board* pBoard)
{
    printf("\n");
    for (int row = ROWS - 1; row >= 0; row--) {
        /* Print top border of the row */
        printf("  +----+----+----+----+----+----+----+----+----+----+\n");
        
        /* Print row number (1-indexed for display) */
        printf("%d |", row + 1);
        
        /* Print pieces */
        for (int col = 0; col < COLS; col++) {
            Piece p = pBoard->grid[row][col];
            if (p.type == ' ') {
                printf("    |");
            }
            else {
                printf(" %c%c |", p.color, p.type);
            }
        }
        printf("\n");
    }

    /* Print bottom border and column letters */
    printf("  +----+----+----+----+----+----+----+----+----+----+\n");
    printf("    A    B    C    D    E    F    G    H    I    J\n\n");
}