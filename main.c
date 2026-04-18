/******************************************************************************
 * File: main.c
 * Author: Vinh Nguyen
 * Date: April 03, 2026
 * 
 * * Description:
 * Main entry point and game loop state management for the interactive
 * Anteater Chess program. Orchestrates initialization, rendering, and 
 * the main execution loop for the game engine. Implements persistent I/O
 * game logging with asynchronous POSIX signal handling for safe termination,
 * and outputs terminal notifications for captures and special moves.
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "GameData.h"
#include "MoveValidation.h"
#include "ChessAI.h"

//=============================================================================

#define SZCODEVERSION "1.2.0"
#define MAX_INPUT_LENGTH 32

/* Global file pointer required for asynchronous access within the signal handler */
static FILE* g_pLogFile = NULL;

//=============================================================================

/*
 * POSIX Signal Handler for SIGINT (Ctrl+C).
 * Intercepts the terminal interrupt, flushes the final game state to the log, 
 * closes the file descriptor to prevent corruption, and exits the process safely.
 */
void HandleSigInt(int sig)
{
    printf("\nKeyboard interruption received. Exiting safely...\n");
    if (g_pLogFile) {
        fprintf(g_pLogFile, "{Game terminated via keyboard interruption (SIGINT)}\n");
        fclose(g_pLogFile);
    }
    exit(0);
}

/*
 * Static helper to handle file I/O for game logging and terminal UX events. 
 * Must be invoked PRIOR to ApplyMove() to accurately deduce capture states.
 */
static void LogMove(FILE* logFile, Board* pBoard, int turnCount, char color, int fRow, int fCol, int tRow, int tCol)
{
    Piece mover = pBoard->grid[fRow][fCol];
    Piece target = pBoard->grid[tRow][tCol];

    char fFileStr = (char)(fCol + 'A');
    int fRankStr = fRow + 1;
    char tFileStr = (char)(tCol + 'A');
    int tRankStr = tRow + 1;

    const char* colorStr = (color == 'w') ? "White" : "Black";
    const char* enemyStr = (color == 'w') ? "Black" : "White";
    char flags[64] = "";

    /* Evaluate state prior to ApplyMove mutation to deduce optional flags and print UX events */
    if (mover.type == 'K' && abs(tCol - fCol) == 2) {
        strcpy(flags, " (Castling)");
    }
    else if (mover.type == 'A' && target.type == 'P' && target.color != color) {
        strcpy(flags, " (Anteater Capture)");
        printf("\n>>> %s Anteater devoured enemy ants! <<<\n", colorStr);
    }
    else if (mover.type == 'P' && IsEnPassant(pBoard, fRow, fCol, tRow, tCol, color)) {
        strcpy(flags, " (En Passant)");
        printf("\n>>> %s Pawn captured via En Passant! <<<\n", colorStr);
    }
    else {
        /* Standard capture detection: target square contains an enemy piece */
        if (target.type != ' ' && target.color != color) {
            strcat(flags, " (Capture)");
            printf("\n>>> %s captured %s's %c! <<<\n", colorStr, enemyStr, target.type);
        }
        
        /* Promotion can occur simultaneously with a standard capture */
        if (mover.type == 'P' && ((color == 'w' && tRow == ROWS - 1) || (color == 'b' && tRow == 0))) {
            strcat(flags, " (Promotion)");
            printf("\n>>> %s Pawn was promoted to a Queen! <<<\n", colorStr);
        }
    }

    if (logFile) {
        fprintf(logFile, "%d. %s: %c%d -> %c%d%s\n", turnCount, colorStr, fFileStr, fRankStr, tFileStr, tRankStr, flags);
        /* Flush buffer immediately to prevent data loss on SIGINT or hard crash */
        fflush(logFile);
    }
}

int main()
{
    /* Register the signal handler for terminal interruptions */
    signal(SIGINT, HandleSigInt);

    /* Initializing the game board */
    Board gameBoard;
    InitializeBoard(&gameBoard);

    /* Prompts user for mode selection */
    char playerColor = ' ', gameMode = ' ', aiDifficulty = ' ';
    printf("Welcome to Anteater Chess by Team Hikaru Naka-more!\n");

    while (gameMode != '1' && gameMode != '2') {
        printf("\nPlease Select Mode\n1. Play a Friend\n2. Play Bots\n");

        /* Handle invalid input and clear stdin buffer */
        if (scanf(" %c", &gameMode) != 1) {
            while (getchar() != '\n');
        }
    }

    /* Skip side selection if PvP */
    if (gameMode == '1') {
        playerColor = 'w';
    }
    /* Prompts user for ai difficulty */
    else if (gameMode == '2') {
        while (aiDifficulty != '1' && aiDifficulty != '2' && aiDifficulty != '3') {
            printf("\nPlease Select Difficulty\n1. Easy\n2. Medium\n3. Hard\n");

            if (scanf(" %c", &aiDifficulty) != 1) {
                while (getchar() != '\n');
            }
        }
    }

    /* Prompt the user for side selection */
    while (playerColor != 'w' && playerColor != 'b') {    
        printf("\nPlease choose your side ('w' for white / 'b' for black): ");
        
        if (scanf(" %c", &playerColor) != 1) {
            while (getchar() != '\n');
        }
    }

    /* Clean up dangling \n after successful scanf */
    while (getchar() != '\n');

    /* Initialize file I/O for game logging */
    g_pLogFile = fopen("game_log.txt", "w");
    if (!g_pLogFile) {
        printf("CRITICAL WARNING: Unable to open game_log.txt for writing.\n");
    }

    /* Setup game variables */
    char moveInput[MAX_INPUT_LENGTH];
    char currentTurn = 'w';
    char fromCol, toCol;
    int fromRow, toRow, gameOver = 0;
    int fullTurnCount = 1;

    /* Begin game */
    while (!gameOver) {
        PrintBoard(&gameBoard);

        /* Pre-turn State Evaluation */
        if (IsInCheck(&gameBoard, currentTurn)) {
            if (IsCheckmate(&gameBoard, currentTurn)) {
                printf("\nCHECKMATE! %s wins the game.\n", (currentTurn == 'w') ? "Black" : "White");
                
                if (g_pLogFile) {
                    fprintf(g_pLogFile, "{Game Over: %s wins by Checkmate}\n", (currentTurn == 'w') ? "Black" : "White");
                    fflush(g_pLogFile);
                }
                
                gameOver = 1;
                break;
            }
            printf("\nWARNING: %s is in CHECK!\n", (currentTurn == 'w') ? "White" : "Black");
        }

        int humanTurn = (gameMode == '1' || currentTurn == playerColor);
        if (humanTurn) {
            printf("\n%s's turn. Enter your move (e.g., 'E2 E4'): ",  (currentTurn == 'w') ? "White" : "Black");
            
            if (fgets(moveInput, sizeof(moveInput), stdin) != NULL) {
                if (sscanf(moveInput, " %c%d %c%d", &fromCol, &fromRow, &toCol, &toRow) == 4) {
                    /* ASCII conversion */
                    int fCol = fromCol - (fromCol >= 'a' ? 'a' : 'A');
                    int fRow = fromRow - 1;
                    int tCol = toCol - (toCol >= 'a' ? 'a' : 'A');
                    int tRow = toRow - 1;

                    /* Validate bounds before accessing board array */
                    if ((fRow >= 0 && fRow < ROWS) && (tRow >= 0 && tRow < ROWS) && 
                        (fCol >= 0 && fCol < COLS) && (tCol >= 0 && tCol < COLS)) {
                            
                        if (IsValidMove(&gameBoard, fRow, fCol, tRow, tCol, currentTurn)) {
                            
                            /* Call LogMove before applying state mutations to deduce capture data */
                            LogMove(g_pLogFile, &gameBoard, fullTurnCount, currentTurn, fRow, fCol, tRow, tCol);
                            
                            ApplyMove(&gameBoard, fRow, fCol, tRow, tCol);
                            
                            /* Standard chess turns increment after Black completes their move */
                            if (currentTurn == 'b') {
                                fullTurnCount++;
                            }
                            currentTurn = (currentTurn == 'w') ? 'b' : 'w';
                        }
                        else {
                            printf("Illegal move. Try again.\n");
                        }
                    }
                    else {
                        printf("Error: Coordinates out of bounds.\n");
                    }
                }
                else {
                    printf("Invalid input. Please try again.\n");
                }
            }
            else {
                /* Synchronous EOF capture (e.g., Ctrl+D) */
                printf("\nInput stream closed. Exiting game.\n");
                if (g_pLogFile) {
                    fprintf(g_pLogFile, "{Game terminated via keyboard interruption (EOF)}\n");
                    fflush(g_pLogFile);
                }
                gameOver = 1;
            }
        }
        else {
            Move aiMove = DetermineAIMove(&gameBoard, currentTurn, aiDifficulty);
            
            /* Safety check to ensure the bot generated a coordinate */
            if (aiMove.fRow != 0 || aiMove.fCol != 0
                || aiMove.tRow != 0 || aiMove.tCol != 0
                || gameBoard.grid[0][0].type != ' ') {
                
                /* UX Enhancement: Announce the bot's generated move to the terminal */
                printf("\nBot played: %c%d -> %c%d\n", 
                       (char)(aiMove.fCol + 'A'), aiMove.fRow + 1, 
                       (char)(aiMove.tCol + 'A'), aiMove.tRow + 1);

                LogMove(g_pLogFile, &gameBoard, fullTurnCount, currentTurn, aiMove.fRow, aiMove.fCol, aiMove.tRow, aiMove.tCol);
                ApplyMove(&gameBoard, aiMove.fRow, aiMove.fCol, aiMove.tRow, aiMove.tCol);
                
                if (currentTurn == 'b') {
                    fullTurnCount++;
                }
                currentTurn = (currentTurn == 'w') ? 'b' : 'w';
            }
            /* Should only trigger if checkmate logic failed to catch endgame state */
            else {
                gameOver = 1;
            }
        }
    }

    if (g_pLogFile) {
        fclose(g_pLogFile);
    }

    return 0;
}