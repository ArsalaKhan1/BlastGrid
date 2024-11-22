#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define CELL_SIZE 28
#define MAX_ROWS 24
#define MAX_COLS 24

typedef struct 
{
    int rows;
    int cols;
} GameLevel;

typedef struct 
{
    int x;
    int y;
} Cell;

typedef struct 
{
    bool isBomb;
    bool isUncovered;
    bool isPinned;
    int adjacentBombs;
} GameCell;

GameCell grid[MAX_ROWS][MAX_COLS];

void MainScreen();
void ShowInstructions();
void InitializeGrid(GameLevel game_level);
void PlaceBombs(GameLevel game_level, int excludeX, int excludeY);
void CalculateAdjacentBombs(GameLevel game_level);
void UncoverCell(int x, int y, GameLevel game_level);
void DrawGameGrid(GameLevel game_level, Cell selectedCell);
bool CheckWin(GameLevel game_level);
void DiffuseRandomBomb(GameLevel game_level);

bool isDiffused = false; 
int TOTAL_BOMBS;

int main() 
{
    MainScreen();
    ShowInstructions();
    const int menuWidth = 400;
    const int menuHeight = 300;

    InitWindow(menuWidth, menuHeight, "Choose Level");
    int selectedLevel = 0; // 0 = not chosen, 1 = easy, 2 = medium, 3 = hard

    while (!WindowShouldClose() && selectedLevel == 0) 
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("Choose a Level", 100, 50, 30, DARKGRAY);
        DrawText("1: Easy (8x8, 10 bombs)", 50, 120, 20, GREEN);
        DrawText("2: Medium (16x16, 40 bombs)", 50, 160, 20, BLUE);
        DrawText("3: Hard (24x24, 99 bombs)", 50, 200, 20, RED);

        if (IsKeyPressed(KEY_ONE)) selectedLevel = 1;
        if (IsKeyPressed(KEY_TWO)) selectedLevel = 2;
        if (IsKeyPressed(KEY_THREE)) selectedLevel = 3;

        EndDrawing();
    }

    CloseWindow(); // Close menu window

    // Adjust game level settings based on selected level
    GameLevel game_level;
    if (selectedLevel == 1) 
    {
        game_level.rows = 8;
        game_level.cols = 8;
        TOTAL_BOMBS = 10;
    } 
    else if (selectedLevel == 2) 
    {
        game_level.rows = 16;
        game_level.cols = 16;
        TOTAL_BOMBS = 40;
    } 
    else if (selectedLevel == 3) 
    {
        game_level.rows = 24;
        game_level.cols = 24;
        TOTAL_BOMBS = 99;
    }
    
    int pins = TOTAL_BOMBS;

    const int screenWidth = game_level.cols * CELL_SIZE;
    const int screenHeight = (game_level.rows * CELL_SIZE + 50) < 300 ? 300 : (game_level.rows * CELL_SIZE + 50);

    InitWindow(screenWidth, screenHeight, "BlastGrid");
    srand(time(NULL));

    Cell selectedCell = {0, 0};

    InitializeGrid(game_level);

    bool gameEnded = false;
    bool gameWon = false;
    float startTime = GetTime();
    float elapsedTime = 0;
    bool isFirstMove = true;

    while (!WindowShouldClose()) 
    {
        if (!gameEnded) 
        {
            elapsedTime = GetTime() - startTime;

            if (IsKeyPressed(KEY_RIGHT)) 
                selectedCell.x = (selectedCell.x + 1) % game_level.cols;
            if (IsKeyPressed(KEY_LEFT)) 
                selectedCell.x = (selectedCell.x - 1 + game_level.cols) % game_level.cols;
            if (IsKeyPressed(KEY_DOWN)) 
                selectedCell.y = (selectedCell.y + 1) % game_level.rows;
            if (IsKeyPressed(KEY_UP)) 
                selectedCell.y = (selectedCell.y - 1 + game_level.rows) % game_level.rows;

            if (IsKeyPressed(KEY_P)) 
            {
                if (!grid[selectedCell.y][selectedCell.x].isUncovered) 
                {
                    grid[selectedCell.y][selectedCell.x].isPinned = !grid[selectedCell.y][selectedCell.x].isPinned;
                    pins -= grid[selectedCell.y][selectedCell.x].isPinned ? 1 : -1;
                }
            }

            if (IsKeyPressed(KEY_ENTER)) 
            {
                if (!grid[selectedCell.y][selectedCell.x].isPinned) 
                {
                    if (isFirstMove) 
                    {
                        PlaceBombs(game_level, selectedCell.x, selectedCell.y);
                        CalculateAdjacentBombs(game_level);
                        isFirstMove = false;
                    }

                    UncoverCell(selectedCell.x, selectedCell.y, game_level);

                    if (grid[selectedCell.y][selectedCell.x].isBomb) 
                    {
                        gameEnded = true;
                        gameWon = false;
                    } 
                    else if (CheckWin(game_level)) 
                    {
                        gameEnded = true;
                        gameWon = true;
                    }
                }
            }

            if (IsKeyPressed(KEY_D)) 
            {
                DiffuseRandomBomb(game_level);
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawGameGrid(game_level, selectedCell);

        char statsText[100];
        if (gameEnded) 
        {
            const char *endMessage = gameWon ? "You Won!" : "You Lose!";
            Color endColor = gameWon ? GREEN : RED;

            int messageWidth = MeasureText(endMessage, 40);
            int messageX = (screenWidth - messageWidth) / 2;
            int messageY = screenHeight / 2 - 20;

            DrawText(endMessage, messageX, messageY, 40, endColor);
        } 
        else 
        {
            snprintf(statsText, sizeof(statsText), "Time: %d sec | Pins: %d", (int)elapsedTime, pins);
            DrawText(statsText, 10, game_level.rows * CELL_SIZE + 10, 20, DARKGRAY);

            if (!isDiffused)
                DrawText("Press 'D' to diffuse a bomb!", 10, game_level.rows * CELL_SIZE + 30, 20, DARKGRAY);
            else
                DrawText("Bomb Diffused!", 10, game_level.rows * CELL_SIZE + 30, 20, GREEN);
        }

        EndDrawing();
    }

    CloseWindow();  
    return 0;
}

void MainScreen() 
{
    const int screenWidth = 600;
    const int screenHeight = 400;

    InitWindow(screenWidth, screenHeight, "Welcome");

    float StartTime = GetTime(); // Record the start time of the main screen

    while (!WindowShouldClose()) 
    {
        float elapsedTime = GetTime() - StartTime;

        if (elapsedTime > 3.0f) // Display the main screen for 3 seconds
        {
            CloseWindow(); // Close the screen
            return;
        }

        BeginDrawing();
        ClearBackground(YELLOW);

        // Game title
        DrawText("BLASTGRID", 120, 150, 60, BLACK);
        EndDrawing();
    }

    CloseWindow(); // Ensure window closes if exited early
}

void ShowInstructions()
{
    const int screenWidth = 550;
    const int screenHeight = 400;

    InitWindow(screenWidth, screenHeight, "Instructions");
    
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Game Instructions", 50, 20, 30, DARKGRAY);
        DrawText("- Use Arrow Keys to navigate the grid", 50, 80, 20, BLACK);
        DrawText("- Press 'P' to Pin/Unpin a cell", 50, 110, 20, BLACK);
        DrawText("- Press 'D' to Diffuse a random bomb (once)", 50, 140, 20, BLACK);
        DrawText("- Press 'Enter' to uncover a cell", 50, 170, 20, BLACK);
        DrawText("Goal: Uncover all non-bomb cells to win", 50, 220, 20, BLUE);
        DrawText("Avoid uncovering bombs!", 50, 250, 20, RED);

        DrawText("Press 'Space' to continue...", 50, 320, 20, DARKGRAY);

        if (IsKeyPressed(KEY_SPACE))
        {
            CloseWindow(); // Close instruction window
            return;        // Exit the instruction screen
        }

        EndDrawing();
    }
}

void InitializeGrid(GameLevel game_level) 
{
    for (int y = 0; y < game_level.rows; y++) 
    {
        for (int x = 0; x < game_level.cols; x++) 
        {
            grid[y][x] = (GameCell){.isBomb = false, .isUncovered = false, .isPinned = false, .adjacentBombs = 0};
        }
    }
}


void PlaceBombs(GameLevel game_level, int excludeX, int excludeY) 
{
    int bombsPlaced = 0;
    while (bombsPlaced < TOTAL_BOMBS) 
    {
        int x = rand() % game_level.cols;
        int y = rand() % game_level.rows;
        // Exclude the first cell
        if ((x != excludeX || y != excludeY) && !grid[y][x].isBomb) 
        {
            grid[y][x].isBomb = true;
            bombsPlaced++;
        }
    }
}

void CalculateAdjacentBombs(GameLevel game_level) 
{
    for (int y = 0; y < game_level.rows; y++) 
    {
        for (int x = 0; x < game_level.cols; x++) 
        {
            if (grid[y][x].isBomb) 
            {
                grid[y][x].adjacentBombs = -1; // Mark as a bomb
                continue;
            }

            int count = 0;
            for (int dy = -1; dy <= 1; dy++) 
            {
                for (int dx = -1; dx <= 1; dx++) 
                {
                    int nx = x + dx;
                    int ny = y + dy;

                    if (nx >= 0 && nx < game_level.cols && ny >= 0 && ny < game_level.rows && grid[ny][nx].isBomb) 
                    {
                        count++;
                    }
                }
            }
            grid[y][x].adjacentBombs = count;
        }
    }
}



void UncoverCell(int x, int y, GameLevel game_level) 
{
    // Bounds check
    if (x < 0 || y < 0 || x >= game_level.cols || y >= game_level.rows) 
        return;

    // Check if already uncovered
    if (grid[y][x].isUncovered) 
        return;

    // Skip uncovering if the cell is pinned, but don't block recursion
    if (grid[y][x].isPinned) 
        return;

    // Uncover the cell
    grid[y][x].isUncovered = true;

    // Stop if this cell is a bomb
    if (grid[y][x].isBomb) 
        return;

    // If no adjacent bombs, recursively uncover neighboring cells
    if (grid[y][x].adjacentBombs == 0) 
    {
        for (int dy = -1; dy <= 1; dy++) 
        {
            for (int dx = -1; dx <= 1; dx++) 
            {
                if (dx != 0 || dy != 0) // Skip the center cell
                { 
                    UncoverCell(x + dx, y + dy, game_level);
                }
            }
        }
    }
}



bool CheckWin(GameLevel game_level) 
{
  
    for (int y = 0; y < game_level.rows; y++) 
    {
        for (int x = 0; x < game_level.cols; x++)
            {
            if (!grid[y][x].isBomb && !grid[y][x].isUncovered) 
            {
                return false;
            }
        }
    }
    return true;
}

void DrawGameGrid(GameLevel game_level, Cell selectedCell) 
{
    for (int y = 0; y < game_level.rows; y++) 
    {
        for (int x = 0; x < game_level.cols; x++) 
        {
            Rectangle cellRect = {x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE};

            // Determine cell color based on state
            Color cellColor = DARKGRAY;
            if (grid[y][x].isUncovered) 
            {
                cellColor = grid[y][x].isBomb ? RED : LIGHTGRAY;
            }
            else if (grid[y][x].isPinned) 
            {
                cellColor = BLUE;
            }

            // Highlight the selected cell
            if (x == selectedCell.x && y == selectedCell.y) 
            {
                DrawRectangleRec(cellRect, YELLOW);
            } 
            else 
            {
                DrawRectangleRec(cellRect, cellColor);
            }

            // Draw the number of adjacent bombs if uncovered and not a bomb
            if (grid[y][x].isUncovered && grid[y][x].adjacentBombs > 0 && !grid[y][x].isBomb) 
            {
                char adjBombsText[2];
                snprintf(adjBombsText, 2, "%d", grid[y][x].adjacentBombs);
                DrawText(adjBombsText, x * CELL_SIZE + CELL_SIZE / 4, y * CELL_SIZE + CELL_SIZE / 4, 20, BLACK);
            }

            // Draw cell borders
            DrawRectangleLines(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, BLACK);
        }
    }
}

void DiffuseRandomBomb(GameLevel game_level) 
{
    if (isDiffused) // Only allow one use
        return; 

    // Systematically search for the first bomb to diffuse
    for (int y = 0; y < game_level.rows; y++) 
    {
        for (int x = 0; x < game_level.cols; x++) 
        {
            if (grid[y][x].isBomb && !grid[y][x].isUncovered) 
            {
                // Found a bomb to diffuse
                grid[y][x].isBomb = false;  // Remove the bomb
                grid[y][x].isPinned = false; // Remove the pin if present
                grid[y][x].isUncovered = true; // Uncover the cell
                isDiffused = true; // Mark the power-up as used

                // Recalculate adjacent bombs around the diffused cell
                CalculateAdjacentBombs(game_level); 
                return;
            }
        }
    }
}

