/**
 * @file globals.cpp
 * @brief Definisi (alokasi memori) semua variabel global.
 *
 * File ini adalah satu-satunya tempat variabel global benar-benar
 * dialokasikan. Semua modul lain hanya punya "extern" (lihat globals.h).
 */

#include "globals.h"

// =============================================
// WINDOW
// =============================================
const int WIDTH  = 800;
const int HEIGHT = 600;

// =============================================
// TIMER
// =============================================
float gameStartTime   = 0.0f;
float timeOffset      = 0.0f;
float currentGameTime = 0.0f;
bool  gameStarted     = false;
char  timeString[32]  = "Time: 0.00";

// =============================================
// GAME STATE
// =============================================
GameState currentGameState = TITLE_SCREEN;
bool rulesDisplayed = false;

// =============================================
// PLAYER
// Posisi awal player: tile (1,1) maze + 0.5 offset agar berada di tengah tile.
// =============================================
float playerX     = 1.5f;
float playerY     = 0.0f;
float playerZ     = 1.5f;
float playerAngle = 0.0f;
float playerSpeed   = 0.05f;
float rotationSpeed = 0.03f;
bool  gameCompleted = false;

char  congratsMessage[128] = "";
int   playerScore = 0;
char  scoreString[32] = "Kertas: 0";
char  finalscore[32]  = "";
char  finaltime[32]   = "";
char  livesString[32] = "Lives: 3";

// =============================================
// BOARD ANIMATION
// =============================================
float boardYOffset   = 0.0f;
float boardAnimSpeed = 1.0f;
float boardRotation  = 0.0f;

// =============================================
// PENALTIES / REWARDS
// =============================================
const float PENALTY_WRONG  = 5.0f;
const float PENALTY_SKIP   = 2.0f;
const float REWARD_CORRECT = 3.0f;

// =============================================
// MAZE
// Layout angka:
//   0 = jalan   1 = dinding   2 = goal   3 = board soal
// Maze berukuran 10x10; indeks maze[z][x] (row = z, col = x).
// =============================================
const int MAZE_WIDTH  = 10;
const int MAZE_HEIGHT = 10;

int maze[10][10] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 3, 0, 0, 1},
    {1, 0, 1, 1, 0, 1, 1, 1, 0, 1},
    {1, 0, 1, 0, 0, 0, 0, 1, 0, 1},
    {1, 0, 1, 0, 1, 1, 3, 1, 0, 1},
    {1, 0, 1, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 1, 0, 1, 1, 0, 1},
    {1, 0, 0, 0, 3, 0, 1, 0, 0, 1},
    {1, 3, 1, 1, 1, 0, 1, 0, 2, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

// mazeCopy: snapshot maze awal, dipakai saat restart agar board yang sudah
// dihapus selama gameplay bisa muncul kembali.
int mazeCopy[10][10];

// =============================================
// QUIZ
// =============================================
const int TOTAL_QUESTIONS = 20;

int  questionOrder[20];
bool questionUsed[20];
int  currentQuestionIndex = 0;
int  activeQuestion       = 0;

float quizStartTime = 0.0f;
float quizTimeLimit = 15.0f;
int   paperCellX    = -1;  // -1 berarti tidak ada board aktif
int   paperCellZ    = -1;

// =============================================
// LIFE SYSTEM
// =============================================
int playerLives     = 3;
const int MAX_LIVES = 3;

// =============================================
// VISUAL FEEDBACK
// =============================================
FeedbackType feedbackState    = FB_NONE;
float        feedbackTimer    = 0.0f;
const float  FEEDBACK_DURATION = 1.2f;

// =============================================
// QUIZ COOLDOWN
// =============================================
bool  quizJustExited    = false;
float quizCooldownTimer = 0.0f;
const float QUIZ_COOLDOWN = 2.0f;

// =============================================
// KEY DEBOUNCE
// =============================================
bool keyPressedLastFrame[GLFW_KEY_LAST] = {false};

// =============================================
// CAMERA MODE
// Default: first-person. Toggle dengan tombol V.
// =============================================
CameraMode cameraMode = FIRST_PERSON;
float tpDistance = 2.5f; // Jarak kamera ke belakang player (unit dunia)
float tpHeight   = 1.8f; // Ketinggian kamera di atas player