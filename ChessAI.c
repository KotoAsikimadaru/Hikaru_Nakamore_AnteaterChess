/******************************************************************************
 * File: ChessAI.c
 * Author: Jonghyun Choi, Vinh Nguyen
 * Date: April 14, 2026
 *
 * * Description:
 * Implements the core strategy logic for the computer player module.
 * Handles the evaluation of board positions, material advantage calculations,
 * and the execution of the Minimax algorithm to generate competitive moves 
 * for the PvE game modes.
 *****************************************************************************/

#include <stdlib.h>
#include <time.h>
#include "ChessAI.h"
#include "MoveValidation.h"

//=============================================================================

/* * Centipawn piece values for static evaluation.
 * Anteater (400) is valued slightly higher than minor pieces due to its 
 * multi-capture threat potential, but less than a Rook.
 */
static const int VAL_PAWN     = 100;
static const int VAL_KNIGHT   = 300;
static const int VAL_BISHOP   = 300;
static const int VAL_ANTEATER = 400;
static const int VAL_ROOK     = 500;
static const int VAL_QUEEN    = 900;
static const int VAL_KING     = 10000;

//=============================================================================

/*
 * Calculates the material advantage for the AI color.
 * Positive score means AI is winning; negative means human is winning.
 */
int EvaluateBoard(Board* pBoard, char aiColor)
{
    int score = 0;
    char humanColor = (aiColor == 'w') ? 'b' : 'w';

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            Piece p = pBoard->grid[r][c];
            if (p.type == ' ') continue;

            int pieceValue = 0;
            switch (p.type) {
                case 'P': pieceValue = VAL_PAWN;     break;
                case 'N': pieceValue = VAL_KNIGHT;   break;
                case 'B': pieceValue = VAL_BISHOP;   break;
                case 'A': pieceValue = VAL_ANTEATER; break;
                case 'R': pieceValue = VAL_ROOK;     break;
                case 'Q': pieceValue = VAL_QUEEN;    break;
                case 'K': pieceValue = VAL_KING;     break;
            }

            if (p.color == aiColor) {
                score += pieceValue;
            } else if (p.color == humanColor) {
                score -= pieceValue;
            }
        }
    }

    return score;
}

//=============================================================================

Move DetermineAIMove(Board* pBoard, char aiColor, char difficulty)
{
    MoveList legalMoves;
    GenerateLegalMoves(pBoard, aiColor, &legalMoves);

    /* Fallback failsafe: If no moves exist, return a zeroed move (Checkmate handled in main) */
    if (legalMoves.count == 0) {
        Move nullMove = {0, 0, 0, 0};
        return nullMove;
    }

    /* Difficulty 1: Easy (Random Move) */
    if (difficulty == '1') {
        srand(time(NULL));
        int randomIndex = rand() % legalMoves.count;
        return legalMoves.moves[randomIndex];
    }

    /* Difficulty 2 & 3 Placeholder: Will implement Minimax here */
    Move fallbackMove = legalMoves.moves[0];
    return fallbackMove;
}