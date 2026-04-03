/******************************************************************************
 * File: main.c
 * Author: Vinh Nguyen
 * Date: April 03, 2026
 * 
 * * Description:
 * Main entry point and game loop state management for the interactive
 * Anteater Chess program. Orchestrates initialization, rendering, and 
 * the main execution loop for the game engine.
 *****************************************************************************/

#include "GameData.h"

//=============================================================================

int main()
{
    Board gameBoard;
    InitializeBoard(&gameBoard);
    PrintBoard(&gameBoard);

    return 0;
}