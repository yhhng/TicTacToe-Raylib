#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <float.h>

#define COLS 3 // Column
#define ROWS 3 // Row

// Naive Bayes
#define NUM_SQUARES 9
#define NUM_OUTCOMES 1
#define MAX_ROWS 958
#define MAX_LINE_LENGTH 50
double outcomecounts[2] = {0, 0}; // Store count of positives and negatives.
double possibilitytable[9][3][2]; //[x][y][z]. [x] Represent the squares. [y] represents 'x','o' or 'b'. [z] represents outcome "positive" or "negative" (0,1 respectively)
bool inNaiveBayes = false; 

// tic tac toe
bool restartFlag = false;
float loadingProgress = 0.0f;
float loadingBarWidth = 0.0f;
bool loadingComplete = false;
const int screenWidth = 600;
const int screenHeight = 600;
const Vector2 screenSize = {screenWidth, screenHeight};
bool winSoundflag = false;
bool loseSoundflag = false;
bool singleplayerFlag = false;

const int cellWidth = screenWidth / COLS;
const int cellHeight = screenHeight / ROWS;

const char *playerXWins = "PLAYER X WINS!";
const char *playerOWins = "PLAYER O WINS!";
const char *isaTie = "GAME TIE!";
const char *pressRToRestart = "Press 'R' to play again!";
const char *pressHToHome = "Press 'H' to go back main menu";
const char *playerXTurnPrompt = "PLAYER X TURN";
const char *playerOTurnPrompt = "PLAYER O TURN";

// Define name integer constants
typedef enum CellValue
{
    EMPTY,
    X,
    O
} CellValue;

// Create a structure to hold cell value
typedef struct Cell
{
    int i;
    int j;
    CellValue value;
} Cell;

Cell grid[COLS][ROWS];

int tilesOccupied;
//To hold gamestate value
typedef enum GameState
{
    PLAYERX,
    PLAYERO,
    END
} GameState;

GameState state;
CellValue winner;
Texture2D picture;

void CellDraw(Cell);
bool IndexIsValid(int, int);
bool CellMark(int, int, CellValue);
void GridInit(void);
void GameInit(void);
bool GameIsOver(CellValue *);
int UpdateDrawFrame(void);
bool IsMouseInsideButton(Rectangle buttonBounds);
void load(void);
int menu(void);
void restartProgram(void);

// Twoplayer
void PlayTicTacToe(void);

// AIPlayTicTacToe
void AIPlayTicTacToe(int searchdepth);
// computer
void computer(Cell grid[COLS][ROWS], int searchdepth);
// minimax
int minimax(Cell grid[COLS][ROWS], int searchdepth, int AIturn, int alpha, int beta);
// Naive Bayes
int dataextractor();
int learn(char trainx[MAX_ROWS][NUM_SQUARES], char *trainy[MAX_ROWS][NUM_OUTCOMES], double possibilitytable[9][3][2], double outcomecounts[2]);
int predictmove(double outcomecounts[2], double possibilitytable[9][3][2], Cell grid[COLS][ROWS]);
int predictdata(double outcomecounts[2], double possibilitytable[9][3][2], char *instance);
void crossValidate(char lines[MAX_ROWS][MAX_LINE_LENGTH], double possibilitytable[9][3][2], double outcomecounts[2]);
void nbPlayTicTacToe();

int main(void)
{
    do
    {
        menu();
    } while (restartFlag);
    
    return 0;
}

int menu(void)
{
    dataextractor(); // Train data 
    SetTraceLogLevel(LOG_ERROR); // Set higher log level for raylib
    //  Initialization of the game window
    InitWindow(600, 600, "Tic Tac Toe Game");
    // Initialize audio device
    InitAudioDevice();

start:
    Music music = LoadMusicStream("resources/music.mp3");
    picture = LoadTexture("resources/tictactoe.png");

    PlayMusicStream(music);
    // Button positions and sizes
    Rectangle singleButtonBounds = {150, 250, 300, 50};
    Rectangle multiplayerButtonBounds = {150, 350, 300, 50};
    Rectangle quitButtonBounds = {150, 450, 300, 50};

    // for Easy AI level
    Rectangle easyButtonBounds = {150, 250, 300, 50};
    // for Impossible AI level
    Rectangle impossibleButtonBounds = {150, 320, 300, 50};
    // for Naive Bayes AI level
    Rectangle naivebayesButtonBounds = {150, 390, 300, 50};
    // Back to home page
    Rectangle homeButtonBounds = {150, 460, 300, 50};

    Image logoImage = LoadImage("resources/logo.png");
    if (logoImage.data == NULL)
    {
        // Handle the error, e.g., display an error message.
        printf("Error: Failed to load logo image.\n");
    }

    SetTargetFPS(60);

    // Main game loop
    while (!WindowShouldClose())
    {
        UpdateMusicStream(music);
        // Check if the mouse is inside the "Single Player" button
        bool singleButtonClicked = IsMouseInsideButton(singleButtonBounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
        // Check if the mouse is inside the "Multiplayer" Button
        bool multiplayerButtonClicked = IsMouseInsideButton(multiplayerButtonBounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
        // Check if the mouse is inside the "QUIT GAME" button
        bool quitButtonClicked = IsMouseInsideButton(quitButtonBounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

        BeginDrawing();

        ClearBackground(WHITE);

        // Draw the logo at the top center
        DrawTexture(LoadTextureFromImage(logoImage), (GetScreenWidth() - logoImage.width) / 2, 50, WHITE);

        // Draw "PLAYER VS AI" button
        DrawRectangleRec(singleButtonBounds, (singleButtonClicked) ? SKYBLUE : LIGHTGRAY);
        DrawRectangleLinesEx(singleButtonBounds, 2, GRAY);
        int textWidth = MeasureText("PLAYER VS AI", 20);
        int textHeight = 20;
        int textP = singleButtonBounds.x + (singleButtonBounds.width - textWidth) / 2;
        int textQ = singleButtonBounds.y + (singleButtonBounds.height - textHeight) / 2;
        DrawText("PLAYER VS AI", textP, textQ, 20, DARKGRAY);

        // Draw "TWO PLAYERS" button
        DrawRectangleRec(multiplayerButtonBounds, (multiplayerButtonClicked) ? SKYBLUE : LIGHTGRAY);
        DrawRectangleLinesEx(multiplayerButtonBounds, 2, GRAY);
        int text2Width = MeasureText("2 PLAYERS", 20);
        int text2Height = 20;
        int textA = multiplayerButtonBounds.x + (multiplayerButtonBounds.width - text2Width) / 2;
        int textB = multiplayerButtonBounds.y + (multiplayerButtonBounds.height - text2Height) / 2;
        DrawText("2 PLAYERS", textA, textB, 20, DARKGRAY);

        // Draw "QUIT GAME" button
        DrawRectangleRec(quitButtonBounds, (quitButtonClicked) ? SKYBLUE : LIGHTGRAY);
        DrawRectangleLinesEx(quitButtonBounds, 2, GRAY);
        int text3Width = MeasureText("QUIT GAME", 20);
        int text3Height = 20;
        int textX = quitButtonBounds.x + (quitButtonBounds.width - text3Width) / 2;
        int textY = quitButtonBounds.y + (quitButtonBounds.height - text3Height) / 2;
        DrawText("QUIT GAME", textX, textY, 20, DARKGRAY);

        EndDrawing();

        if (singleButtonClicked)
        {
            while (!WindowShouldClose())
            {
                UpdateMusicStream(music);
                // Drawing the UI Layouts
                bool easyButtonClicked = IsMouseInsideButton(easyButtonBounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
                bool impossibleButtonClicked = IsMouseInsideButton(impossibleButtonBounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
                bool naivebayesClicked = IsMouseInsideButton(naivebayesButtonBounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
                bool homeButtonClicked = IsMouseInsideButton(homeButtonBounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

                BeginDrawing();
                ClearBackground(WHITE);

                const char *difftext = "Difficulty Select";
                int fontSize = 40;
                int textWidth = MeasureText(difftext, fontSize);
                int textK = (screenWidth - textWidth) / 2;
                int textL = 120;
                DrawText(difftext, textK, textL, fontSize, DARKGRAY);

                DrawRectangleRec(easyButtonBounds, (easyButtonClicked) ? SKYBLUE : LIGHTGRAY);
                DrawRectangleLinesEx(easyButtonBounds, 2, GRAY);
                int text4Width = MeasureText("EASY", 20);
                int text4Height = 20;
                int textC = easyButtonBounds.x + (easyButtonBounds.width - text4Width) / 2;
                int textD = easyButtonBounds.y + (easyButtonBounds.height - text4Height) / 2;
                DrawText("EASY", textC, textD, 20, DARKGRAY);

                DrawRectangleRec(impossibleButtonBounds, (impossibleButtonClicked) ? SKYBLUE : LIGHTGRAY);
                DrawRectangleLinesEx(impossibleButtonBounds, 2, GRAY);
                int text5Width = MeasureText("IMPOSSIBLE", 20);
                int text5Height = 20;
                int textE = impossibleButtonBounds.x + (impossibleButtonBounds.width - text5Width) / 2;
                int textF = impossibleButtonBounds.y + (impossibleButtonBounds.height - text5Height) / 2;
                DrawText("IMPOSSIBLE", textE, textF, 20, DARKGRAY);

                DrawRectangleRec(naivebayesButtonBounds, (naivebayesClicked) ? SKYBLUE : LIGHTGRAY);
                DrawRectangleLinesEx(naivebayesButtonBounds, 2, GRAY);
                int text6Width = MeasureText("NAIVE BAYES", 20);
                int text6Height = 20;
                int textM = naivebayesButtonBounds.x + (naivebayesButtonBounds.width - text6Width) / 2;
                int textN = naivebayesButtonBounds.y + (naivebayesButtonBounds.height - text6Height) / 2;
                DrawText("NAIVE BAYES", textM, textN, 20, DARKGRAY);

                DrawRectangleRec(homeButtonBounds, (homeButtonClicked) ? SKYBLUE : LIGHTGRAY);
                DrawRectangleLinesEx(homeButtonBounds, 2, GRAY);
                int text7Width = MeasureText("BACK TO HOME", 20);
                int text7Height = 20;
                int textG = homeButtonBounds.x + (homeButtonBounds.width - text7Width) / 2;
                int textH = homeButtonBounds.y + (homeButtonBounds.height - text7Height) / 2;
                DrawText("BACK TO HOME", textG, textH, 20, DARKGRAY);

                EndDrawing();

                // Easy Button and Impossible Button//
                if (easyButtonClicked)
                {
                    singleplayerFlag = true;
                    load();
                    while (!loadingComplete)
                    {
                        load();
                    }
                    AIPlayTicTacToe(0); // 0 is the searchdepth limit for easy mode
                }
                else if (impossibleButtonClicked)
                {
                    singleplayerFlag = true;
                    load();
                    while (!loadingComplete)
                    {
                        load();
                    }
                    AIPlayTicTacToe(8); // 8 is the searchdepth limit for impossible mode
                }
                else if (naivebayesClicked)
                {
                    singleplayerFlag = true;
                    inNaiveBayes = true;
                    load();
                    while (!loadingComplete)
                    {
                        load();
                    }
                    nbPlayTicTacToe();
                }
                else if (homeButtonClicked)
                {
                    goto start;
                }
            }
            break;

        }

        if (multiplayerButtonClicked)
        {
            UpdateMusicStream(music);
            load();
            while (!loadingComplete)
            {
                load();
            }
            PlayTicTacToe(); // Loading is complete; proceed to the next part of your program
        }

        if (quitButtonClicked)
        {
            exit(0);
        }
    }

    // De-Initialization
    UnloadTexture(picture);
    UnloadImage(logoImage);
    UnloadMusicStream(music);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}

void load(void)
{
    if (loadingProgress < 1.0f)
    {
        loadingProgress += 0.1f; // Simulate loading progress (adjust this value)

        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Draw loading bar background
        DrawRectangle(100, screenHeight / 2 - 10, screenWidth - 200, 20, GRAY);

        // Draw loading bar
        DrawRectangle(100, screenHeight / 2 - 10, (int)((screenWidth - 200) * loadingProgress), 20, SKYBLUE);

        // Draw loading text
        DrawText(TextFormat("Loading... %.0f%%", loadingProgress * 100), 400, screenHeight / 2 - 30, 20, DARKGRAY);

        EndDrawing();
    }
    else
    {
        // Set the loading completion flag
        loadingComplete = true;
    }
}
// Function to check if the mouse is within a button's bounds
bool IsMouseInsideButton(Rectangle buttonBounds)
{
    Vector2 mouse = GetMousePosition();
    return CheckCollisionPointRec(mouse, buttonBounds);
}

// Tic Tac Toe game (player vs player) function
void PlayTicTacToe(void)
{
    // Initialize Tic Tac Toe game variables
    GameInit();
    while (!WindowShouldClose())
    {
        UpdateDrawFrame();
        if (WindowShouldClose())
        {
            exit(0);
        }
    }
}
//For button H to return home
void restartProgram(void)
{
    const char *programName = "./main"; 

    execlp(programName, programName, NULL);

    // If execlp fails then exit program
    perror("exec");
    exit(EXIT_FAILURE);
}

int UpdateDrawFrame(void)
{
    Rectangle restartButtonBounds = (Rectangle){screenWidth / 2 - MeasureText(pressRToRestart, 20) / 2 - 10, screenHeight * 0.75f - 20, MeasureText(pressRToRestart, 20) + 20, 40};
    Rectangle menuButtonBounds = (Rectangle){screenWidth / 2 - MeasureText(pressHToHome, 20) / 2 - 10, screenHeight * 0.75f - 45, MeasureText(pressHToHome, 20) + 20, 40};
    Sound pop = LoadSound("resources/pop.mp3");
    Sound win = LoadSound("resources/win.mp3");
    Sound lose = LoadSound("resources/lose.mp3");
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    {
        Vector2 mPos = GetMousePosition();
        int indexI = mPos.x / cellWidth;
        int indexJ = mPos.y / cellHeight;

        if (state != END && IndexIsValid(indexI, indexJ))
        {
            if (CellMark(indexI, indexJ, state == PLAYERX ? X : O))
            {
                if (tilesOccupied < 5 || !GameIsOver(&winner))
                {
                    PlaySound(pop);
                    state = (state == PLAYERX) ? PLAYERO : PLAYERX;
                }
                else
                {
                    state = END;
                }
            }
        }
    }
    // To check if game has ended and if key R is pressed and is in Naive Bayes game mode
    if (state == END && IsKeyPressed(KEY_R) && inNaiveBayes == true)
    {
        dataextractor(); // Retrain data for new game
        GameInit();
    }
    // To check if game has ended and if key R is pressed
    else if (state == END && IsKeyPressed(KEY_R))
    {
        winSoundflag = false;
        loseSoundflag = false;
        GameInit();
    }
    // To check if game has ended and if key H is pressed
    if (state == END && IsKeyPressed(KEY_H))
    {
        inNaiveBayes = false; // exit from naive bayes
        restartFlag = true;
        winSoundflag = false;
        loseSoundflag = false;
        singleplayerFlag = false;
        restartProgram();
    }

    BeginDrawing();

    ClearBackground(WHITE);

    for (int i = 0; i < COLS; i++)
    {
        for (int j = 0; j < ROWS; j++)
        {
            CellDraw(grid[i][j]);
        }
    }

    if (state == END)
    {
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(WHITE, 0.8f));
        if (winner == X)
        {
            if(!winSoundflag)
            {
                winSoundflag = true;
                PlaySound(win);
            }
            DrawText(playerXWins, screenWidth / 2 - MeasureText(playerXWins, 40) / 2, screenHeight / 2 - 10, 40, DARKGRAY);
        }

        if (winner == O)
        {
            if(!winSoundflag && !singleplayerFlag)
            {
                winSoundflag = true;
                PlaySound(win);
            }
            else if(!loseSoundflag && singleplayerFlag)
            {
                loseSoundflag = true;
                PlaySound(lose);
            }
            DrawText(playerOWins, screenWidth / 2 - MeasureText(playerOWins, 40) / 2, screenHeight / 2 - 10, 40, DARKGRAY);
        }

        if (winner == EMPTY)
        {
            if(!loseSoundflag)
            {
                loseSoundflag = true;
                PlaySound(lose);
            }
            DrawText(isaTie, screenWidth / 2 - MeasureText(isaTie, 40) / 2, screenHeight / 2 - 10, 40, DARKGRAY);
        }

        DrawText(pressRToRestart, restartButtonBounds.x + 10, restartButtonBounds.y + 10, 20, DARKGRAY);
        DrawText(pressHToHome, menuButtonBounds.x + 10, menuButtonBounds.y + 10, 20, DARKGRAY);
    }
    else
    {
        const char *text = (state == PLAYERX) ? playerXTurnPrompt : playerOTurnPrompt;

        Vector2 messagePosition = (Vector2){screenWidth / 2, 40};

        int measure = MeasureText(text, 40);
        Rectangle textRect = (Rectangle){messagePosition.x - measure / 2 - 40, messagePosition.y - 40, measure + 80, 80};

        DrawRectangleRec(textRect, Fade(WHITE, 0.8f));
        DrawText(text, messagePosition.x - measure / 2, messagePosition.y - 20, 40, DARKGRAY);
    }
    EndDrawing();
    return 0;
}

void CellDraw(Cell cell)
{
    Rectangle dest = {cell.i * cellWidth, cell.j * cellHeight, cellWidth, cellHeight};

    Rectangle source;
    switch (cell.value)
    {
    case X:
        source = (Rectangle){0, 0, 100, 100};
        DrawTexturePro(picture, source, dest, (Vector2){0, 0}, 0, WHITE);
        break;
    case O:
        source = (Rectangle){100, 0, 100, 100};
        DrawTexturePro(picture, source, dest, (Vector2){0, 0}, 0, WHITE);
        break;
    default:
        break;
    }

    DrawRectangleLines(cell.i * cellWidth, cell.j * cellHeight, cellWidth, cellHeight, BLACK);
}

bool IndexIsValid(int i, int j)
{
    return i >= 0 && i < COLS && j >= 0 && j < ROWS;
}

bool CellMark(int i, int j, CellValue value)
{
    if (grid[i][j].value != EMPTY || value == EMPTY)
    {
        return false;
    }

    grid[i][j].value = value;
    tilesOccupied++;

    return true;
}

void GridInit(void)
{
    for (int i = 0; i < COLS; i++)
    {
        for (int j = 0; j < ROWS; j++) // Col 1 Draw 3 Rows... Continue loop col 2 Draw 3 Rows....
        {
            grid[i][j] = (Cell){
                .i = i,
                .j = j,
                .value = EMPTY};
        }
    }
}

void GameInit(void)
{
    restartFlag = false;
    winSoundflag = false;
    loseSoundflag = false;
    GridInit();
    state = PLAYERX;
    tilesOccupied = 0;
}

bool GameIsOver(CellValue *winner)
{
    // DIAGONALLY
    // TOP LEFT TO BOTTOM RIGHT
    if (grid[0][0].value != EMPTY && grid[0][0].value == grid[1][1].value && grid[1][1].value == grid[2][2].value)
    {
        *winner = grid[0][0].value;
        return true;
    }

    // BOTTOM LEFT TO TOP RIGHT
    if (grid[0][2].value != EMPTY && grid[0][2].value == grid[1][1].value && grid[1][1].value == grid[2][0].value)
    {
        *winner = grid[0][2].value;
        return true;
    }

    // HORIZONTALLY
    // TOP LEFT TO TOP RIGHT
    if (grid[0][0].value != EMPTY && grid[0][0].value == grid[1][0].value && grid[1][0].value == grid[2][0].value)
    {
        *winner = grid[0][0].value;
        return true;
    }

    // MIDDLE LEFT TO MIDDLE RIGHT
    if (grid[0][1].value != EMPTY && grid[0][1].value == grid[1][1].value && grid[1][1].value == grid[2][1].value)
    {
        *winner = grid[0][1].value;
        return true;
    }

    // BOTTOM LEFT TO BOTTOM RIGHT
    if (grid[0][2].value != EMPTY && grid[0][2].value == grid[1][2].value && grid[1][2].value == grid[2][2].value)
    {
        *winner = grid[0][2].value;
        return true;
    }

    // VERTICALLY
    // TOP LEFT TO BOTTOM LEFT
    if (grid[0][0].value != EMPTY && grid[0][0].value == grid[0][1].value && grid[0][1].value == grid[0][2].value)
    {
        *winner = grid[0][0].value;
        return true;
    }

    // MIDDLE TOP TO MIDDLE BOTTOM
    if (grid[1][0].value != EMPTY && grid[1][0].value == grid[1][1].value && grid[1][1].value == grid[1][2].value)
    {
        *winner = grid[1][0].value;
        return true;
    }

    // TOP RIGHT TO BOTTOM RIGHT
    if (grid[2][0].value != EMPTY && grid[2][0].value == grid[2][1].value && grid[2][1].value == grid[2][2].value)
    {
        *winner = grid[2][0].value;
        return true;
    }
    // TO CHECK FOR TIE
    if (grid[0][0].value != EMPTY && grid[1][0].value != EMPTY && grid[2][0].value != EMPTY && grid[0][1].value != EMPTY && grid[1][1].value != EMPTY && grid[2][1].value != EMPTY && grid[0][2].value != EMPTY && grid[1][2].value != EMPTY && grid[2][2].value != EMPTY)
    {
        *winner = EMPTY;
        return true;
    }

    return false;
}
/*************************************************/
/*Tic Tac Toe game (single player vs AI) function*/
/*************************************************/
void AIPlayTicTacToe(int searchdepth)
{
    // Initialize Tic Tac Toe game variables
    GameInit();

    while (!WindowShouldClose())
    {
        UpdateDrawFrame(); //update the game state after human player makes a move
        if (GameIsOver(&winner)) //check if condition is True when the game is over
        {
            state = END; //switch state to END
            UpdateDrawFrame(); //update the game state
        }
        if (state == PLAYERO) //check if it is the AI's turn
        {
            computer(grid, searchdepth); //AI makes a move
            UpdateDrawFrame(); //update the game state
            state = PLAYERX; //switch back to human player X
        }
        if (WindowShouldClose()) //check if the window is closed
        {
            exit(0); //exit the program
        }
    }
}

/**********************************************/
/*Function to predict the next move for the AI*/
/**********************************************/
void computer(Cell grid[COLS][ROWS], int searchdepth)
{
    int i, j, index_x, index_y; //index_x and index_y are the coordinates of the AI's chosen move
    int currentpt; //score of current move
    int bestpt = INT_MIN; //INT_MIN this is to ensure the initial best score bestpt will be lower than any possible score
    int alpha = INT_MIN; //-INFINITY, initial value of α = - ∞ worst conditions for a maximizer
    int beta = INT_MAX; //INFINITY, initial value of β = + ∞ worst conditions for a minimizer

    for (i = 0; i < COLS; i++) //loop through all the COLS cells in the grid
    {
        for (j = 0; j < ROWS; j++) //loop through all the ROWS cells in the grid
        {
            if (grid[i][j].value == EMPTY) //check if the cell is empty
            {
                grid[i][j].value = O; //simulate a move by the AI               
                currentpt = minimax(grid,searchdepth,0, alpha, beta); //0 is used to indicate that it is the human player's turn
                alpha = fmax(alpha, currentpt); //For a child minimizer node, α is the best score the parent maximizer node was guaranteed to get is α = max (α, best score)
                grid[i][j].value = EMPTY; //reset the grid to its original state after simulating a move
                if (currentpt > bestpt) //check if the current move is better than the previous best move
                {
                    bestpt = currentpt; //update the best score
                    index_x = i; //update the coordinates of the best move
                    index_y = j; //update the coordinates of the best move
                }
            }
        }
    }
    grid[index_x][index_y].value = O; //AI's chosen move
}

/********************************************************************************************************/
/*Minimax function determines the best move by evaluating all possible moves and outcomes of the game up*/
/*to the specified searchdepth limit.                                                                   */
/********************************************************************************************************/
int minimax(Cell grid[COLS][ROWS], int searchdepth, int AIturn, int alpha, int beta)
{
    CellValue winner; //winner is the outcome of the game

    if (GameIsOver(&winner)) //check if condition is True when the game is over
    {
        if (winner == O) //check if AI wins
        {
            return 1; //return 1 if AI wins
        }
        else if (winner == X) //check if human player wins
        {
            return -1; //return -1 if human player wins
        }
        else if (winner == EMPTY) //check if it is a tie
        {
            return 0; //return 0 if it is a tie
        }
    }

    if (searchdepth == 0) //check if the searchdepth limit has been reached
    {
        return 0; //return 0 if the searchdepth limit has been reached
    }

    //INT_MIN is used to ensure the initial best score bestScore will be lower than any possible score, INT_MAX is used to ensure the initial best score bestScore will be higher than any possible score
    int bestScore = (AIturn) ? INT_MIN : INT_MAX; 

    for (int i = 0; i < COLS; i++) //loop through all the COLS cells in the grid
    {
        for (int j = 0; j < ROWS; j++) //loop through all the ROWS cells in the grid
        {
            if (grid[i][j].value == EMPTY) //check if the cell is empty
            {
                grid[i][j].value = (AIturn) ? O : X; //AIturn is true when the AI is making a move, false when the player is making a move

                int currentScore = minimax(grid, searchdepth - 1, !AIturn, alpha, beta); //!AIturn is used to switch between AI and human player

                grid[i][j].value = EMPTY; //reset the grid to its original state after simulating a move

                if (AIturn) //check if it is the AI's turn
                {
                    bestScore = fmax(bestScore, currentScore); //maximizing AI's score
                    alpha = fmax(alpha, bestScore); //For a child minimizer node, α is the best score the parent maximizer node was guaranteed to get is α = max (α, best score)
                
                }
                else //check if it is the human player's turn
                {
                    bestScore = fmin(bestScore, currentScore); //minimizing human player's score
                    beta = fmin(beta, bestScore); //For a child maximizer node, β is the best score the parent minimizer node was guaranteed to get is β = min (β, best score) 
                }
            }           

            if (beta <= alpha) //if β ≤ α, we can stop evaluating the rest of the children since the parent minimizer node will never choose this node
            {
                break;
            }
        }
    }

    return bestScore; //return the best score
}
/*************************************************/
/*               NAIVE BAYES AREA                */
/*************************************************/
void nbPlayTicTacToe()
{
    // Initialize Tic Tac Toe game variables
    GameInit();

    while (!WindowShouldClose())
    {
        UpdateDrawFrame();
        if (GameIsOver(&winner))
        {
            state = END;
            UpdateDrawFrame();
        }
        if (state == PLAYERO)
        {
            predictmove(outcomecounts, possibilitytable, grid);
            UpdateDrawFrame();
            state = PLAYERX;
        }
        if (WindowShouldClose())
        {
            exit(0);
        }
    }
}

/********************************************************************************************************/
/*Extracts data from txt file, shuffles and puts them into data structure specified in project document */
/*creates training and testing set in specified ratio.                                                  */
/********************************************************************************************************/
int dataextractor()
{
    char lines[MAX_ROWS][MAX_LINE_LENGTH]; // To store each line in dataset
    char linesTemp[MAX_ROWS][MAX_LINE_LENGTH]; // To use in strtok
    char x[MAX_ROWS][NUM_SQUARES]; // 958 rows, 9 squares
    char y[MAX_ROWS][NUM_OUTCOMES][MAX_LINE_LENGTH]; // 958 rows, 2 outcomes

    // Open the file in read mode
    FILE *fptr;
    fptr = fopen("resources/tic-tac-toe.data", "r");
    if (fptr == NULL)
    {
        printf("Error opening the file.\n");
        return 1; // Exit the program with an error code
    }

    //Extract each line out of text file
    int lineCount = 0;
    while (fgets(lines[lineCount], MAX_LINE_LENGTH, fptr) != NULL)
    {
        lineCount++;
    }

    // Replace '\n' with '\0'
    for (int i = 0; i < MAX_ROWS; i++)
    {
        lines[i][strcspn(lines[i], "\n")] = '\0'; 
    }

    // Initialize random seed for rand()
    srand(time(0));
    // Shuffle the lines using Fisher-Yates algorithm
    for (int i = MAX_ROWS - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        if (i != j)
        {
            char temp[MAX_LINE_LENGTH];
            strcpy(temp, lines[i]);
            strcpy(lines[i], lines[j]);
            strcpy(lines[j], temp);
        }
    }

    //Copy onto temp array for manipulation
    for (int i = 0; i < MAX_ROWS; i++)
    {
        strcpy(linesTemp[i], lines[i]); 
    }

    // Put data into x and y arrays according to project specifications specified
    for (int row = 0; row < MAX_ROWS; row++)
    {
        char *elements[10];
        char *token = strtok(linesTemp[row], ",");
        int i = 0;

        // Seperates the data using "," and puts each of it into an element array
        while (token != NULL)
        {
            elements[i] = token;
            token = strtok(NULL, ",");
            i++;
        }

        // Puts the element into X 
        for (int square = 0; square < NUM_SQUARES; square++)
        {
            x[row][square] = elements[square][0];
        }

        // Puts the element into Y 
        for (int outcomes = 0; outcomes < NUM_OUTCOMES; outcomes++)
        {
            strcpy(y[row][outcomes], elements[9]);
        }
    }
    // Close the file
    fclose(fptr);

    // Calculate the split point for training
    int split = (int)(0.8 * MAX_ROWS);
    int square, outcomes;

    // Create training sets
    char trainingSetX[MAX_ROWS][NUM_SQUARES];
    char *trainingSetY[MAX_ROWS][NUM_OUTCOMES];

    for (int row = 0; row < split; row++)
    {
        for (square = 0; square < NUM_SQUARES; square++)
        {
            trainingSetX[row][square] = x[row][square];
        }
    }
    for (int row = 0; row < split; row++)
    {
        for (outcomes = 0; outcomes < NUM_OUTCOMES; outcomes++)
        {
            trainingSetY[row][outcomes] = y[row][outcomes];
        }
    }

    // Initialise possibilitytable to all zeros
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            for (int k = 0; k < 2; k++)
            {
                possibilitytable[i][j][k] = 0;
            }
        }
    }
    learn(trainingSetX, trainingSetY, possibilitytable, outcomecounts);
    crossValidate(lines, possibilitytable, outcomecounts);

    return 0; // Return 0 to indicate successful execution
}

int learn(char trainx[MAX_ROWS][NUM_SQUARES], char *trainy[MAX_ROWS][NUM_OUTCOMES], double possibilitytable[9][3][2], double outcomecounts[2])
{
    int rows = 0.8 * 958; // Account for rows in testing set
    const char str[] = "positive";

    // Count no. of positive and negatives
    for (int i = 0; i < rows; i++)
    {
        if (strcmp(trainy[i][0], str) == 0)
        {
            outcomecounts[0]++; // outcomecounts[0] is positive
        }
        else
        {
            outcomecounts[1]++; // outcomecounts[1] is negative
        }
    }

    // Count occurrances of each 'x,o,b' positive/negative for each square
    for (int r = 0; r < rows; r++)
    {
        for (int i = 0; i < 9; i++)
        {
            if (trainx[r][i] == 'x')
            {
                if (strcmp(trainy[r][0], str) == 0)
                {
                    possibilitytable[i][0][0]++; // Positive outcome x
                }
                else
                {
                    possibilitytable[i][0][1]++; // Negative outcome x
                }
            }
            if (trainx[r][i] == 'o')
            {
                if (strcmp(trainy[r][0], str) == 0)
                {
                    possibilitytable[i][1][0]++; // Positive outcome o
                }
                else
                {
                    possibilitytable[i][1][1]++; // Negative outcome o
                }
            }
            if (trainx[r][i] == 'b')
            {
                if (strcmp(trainy[r][0], str) == 0)
                {
                    possibilitytable[i][2][0]++; // Positive outcome b
                }
                else
                {
                    possibilitytable[i][2][1]++; // Negative outcome b
                }
            }
        }
    }

    // Compute the possibility of each feature in each square given positive or negative outcome
    for (int s = 0; s < 9; s++) // iterate through the squares
    {
        for (int f = 0; f < 3; f++) // iterate through the featues: x,o,b
        {
            for (int o = 0; o < 2; o++) // iterate through the outcomes
            {
                possibilitytable[s][f][o] /= outcomecounts[o];
            }
        }
    }

    // Normalize label probabilities
    for (int o = 0; o < 2; o++)
    {
        outcomecounts[o] /= rows; 
    }
    return 0;
}

int predictmove(double outcomecounts[2], double possibilitytable[9][3][2], Cell grid[COLS][ROWS])
{
    double result[NUM_SQUARES]; // To store win probability of each square
    int index = 0; // To store grid features
    double maxResult = -DBL_MIN;
    int bestsquare;
    char instance[NUM_SQUARES];

    // Extract data from grid and into array
    for (int col = 0; col < 3; col++)
    {
        for (int row = 0; row < 3; row++)
        {
            if (grid[col][row].value == X)
            {
                index = row * 3 + col;
                instance[index] = 'x';
            }
            else if (grid[col][row].value == O)
            {
                index = row * 3 + col;
                instance[index] = 'o';
            }
            else
            {
                index = row * 3 + col;
                instance[index] = 'b';
            }
        }
    }

    // For each square blank square, calculate the probability of win for "O"
    for (int s = 0; s < 9; s++)
    {
        if (instance[s] == 'b') // If current square is blank
        {
            instance[s] = 'o'; // Let current square be 'o'
            result[s] = outcomecounts[1]; // outcomecounts[1] is negative for 'X', positive for 'O'
            for (int s2 = 0; s2 < 9; s2++) // Iterate through current gameboard
            {   
                int feature;
                if (instance[s2] == 'x')
                {
                    feature = 0;
                }
                else if (instance[s2] == 'o')
                {
                    feature = 1;
                }
                else
                {
                    feature = 2;
                }
                result[s] *= possibilitytable[s2][feature][1]; // Calculate probability
            }
            instance[s] = 'b';
        }
        if (result[s] > maxResult && instance[s] == 'b') // Compare win probability of square as it iterates
        {
            maxResult = result[s];
            bestsquare = s;
        }
    }
    int col = bestsquare % 3; // Convert to column index
    int row = bestsquare / 3; // Convert to row index
    grid[col][row].value = O; // AI's chosen move

    return 0;
}

int predictdata(double outcomecounts[2], double possibilitytable[9][3][2], char *instance)
{
    double result[2] = {0, 0};
    double maxResult = -DBL_MIN; // Initialised as lowest possible value
    int maxIndex;
    int feature = 0;

    // Seperates the data using "," and puts each of it into an element array.
    char *elements[10];
    char *token = strtok(instance, ",");
    int i = 0;
    while (token != NULL)
    {
        elements[i] = token;
        token = strtok(NULL, ",");
        i++;
    }

    // Predict positive or negative for passed instance
    for (int o = 0; o < 2; o++) // Calculate for both outcomes, negative and positive
    {
        result[o] = outcomecounts[o];
        for (int s = 0; s < 9; s++) // Calculate for all squares
        {
            if (strcmp(elements[s], "x") == 0)
            {
                feature = 0;
            }
            else if (strcmp(elements[s], "o") == 0)
            {
                feature = 1;
            }
            else
            {
                feature = 2;
            }
            result[o] *= possibilitytable[s][feature][o]; 
        }
        if (result[o] > maxResult) // Compare and save highest result
        {
            maxResult = result[o];
            maxIndex = o;
        }
    }

    if (maxIndex == 0)
    {
        return 0; // Prediction is positive
    }
    else
        return 1; // Prediction is negative
}

void crossValidate(char lines[MAX_ROWS][MAX_LINE_LENGTH], double possibilitytable[9][3][2], double outcomecounts[2])
{
    int split = (int)(0.8 * MAX_ROWS);
    int truepositive = 0, truenegative = 0, falsepositive = 0, falsenegative = 0, errorcount = 0, count = 0;
    double validcount = 0.0;

    // Test Split
    for (int i = split; i < 958; i++) // Test split is row 766 to 958. Data is already shuffled from previously.
    {
        int predictedoutcome;
        char instance[MAX_LINE_LENGTH];

        // Copy current line into instance for prediction 
        strcpy(instance, lines[i]);
        predictedoutcome = predictdata(outcomecounts, possibilitytable, instance);

        // This is to retrieve "positive" or "negative" from the instance.
        char *elements2[10];
        char *token = strtok(lines[i], ",");
        int i = 0;

        while (token != NULL)
        {
            elements2[i] = token;
            token = strtok(NULL, ",");
            i++;
        }

        // Count, compare & catagorise predicted results
        if (predictedoutcome == 0 && strcmp(elements2[9], "positive") == 0)
        {
            truepositive++;
            validcount++;
        }
        else if (predictedoutcome == 0 && strcmp(elements2[9], "negative") == 0)
        {
            falsepositive++;
            errorcount++;
        }
        if (predictedoutcome == 1 && strcmp(elements2[9], "negative") == 0)
        {
            truenegative++;
            validcount++;
        }
        else if (predictedoutcome == 1 && strcmp(elements2[9], "positive") == 0)
        {
            falsenegative++;
            errorcount++;
        }
        count++;
    }
    printf("True Positive: %d \nFalse Positive: %d \nTrue Negative: %d \nFalse Negative: %d \n", truepositive, falsepositive, truenegative, falsenegative);
    validcount /= count; // Calculate accuracy
    printf("Accuracy: %f\n", validcount);
    fflush(stdout);
}