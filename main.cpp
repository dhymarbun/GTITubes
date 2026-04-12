#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>

// Window dimensions
const int WIDTH = 800;
const int HEIGHT = 600;
float gameStartTime = 0.0f;
float currentGameTime = 0.0f;
bool gameStarted = false;
char timeString[32];

// =============================================
// QUIZ SYSTEM - 10 Questions (Easy/Medium/Hard)
// =============================================
struct Question {
    const char* question;
    const char* options[4];
    int correct; // 0=A, 1=B, 2=C, 3=D
    int difficulty; // 0=easy, 1=medium, 2=hard
};

Question questions[] = {
    // EASY (0-2)
    {"[MATH] Berapa hasil dari 5 + 3 x 2?",
     {"A. 16", "B. 11", "C. 13", "D. 10"}, 1, 0},
    {"[CS] Big-O dari pencarian linear array N?",
     {"A. O(log N)", "B. O(N^2)", "C. O(N)", "D. O(1)"}, 2, 0},
    {"[MATH] Berapa nilai dari 2^8?",
     {"A. 128", "B. 512", "C. 64", "D. 256"}, 3, 0},

    // MEDIUM (3-6)
    {"[MATH] Limit x->0 dari (sin x)/x = ?",
     {"A. 0", "B. Tak terdefinisi", "C. 1", "D. Infinity"}, 2, 1},
    {"[CS] Struktur data LIFO adalah?",
     {"A. Queue", "B. Stack", "C. Tree", "D. Graph"}, 1, 1},
    {"[MATH] Turunan dari f(x) = x^3 + 2x adalah?",
     {"A. 3x + 2", "B. x^2 + 2", "C. 3x^2 + 2", "D. 3x^2"}, 2, 1},
    {"[CS] Binary dari desimal 42 adalah?",
     {"A. 101010", "B. 100110", "C. 110100", "D. 101100"}, 0, 1},

    // HARD (7-9)
    {"[MATH] Integral dari 1/x dx adalah?",
     {"A. x + C", "B. ln|x| + C", "C. 1/x^2 + C", "D. e^x + C"}, 1, 2},
    {"[CS] Kompleksitas waktu worst-case QuickSort?",
     {"A. O(N log N)", "B. O(log N)", "C. O(N^2)", "D. O(N)"}, 2, 2},
    {"[MATH] Jika f(x)=e^(2x), maka f''(x)=?",
     {"A. e^(2x)", "B. 2e^(2x)", "C. 4e^(2x)", "D. e^(4x)"}, 2, 2},
};
const int TOTAL_QUESTIONS = 10;

// Shuffled question order (easy->medium->hard maintained)
int questionOrder[TOTAL_QUESTIONS];
bool questionUsed[TOTAL_QUESTIONS];
int currentQuestionIndex = 0; // index into questionOrder[]
int activeQuestion = 0;       // actual question index shown

float quizStartTime  = 0.0f;
float quizTimeLimit  = 15.0f;
int   paperCellX     = -1;
int   paperCellZ     = -1;

// =============================================
// LIFE SYSTEM
// =============================================
int playerLives = 3;
const int MAX_LIVES = 3;

// =============================================
// VISUAL FEEDBACK
// =============================================
enum FeedbackType { FB_NONE, FB_CORRECT, FB_WRONG };
FeedbackType feedbackState  = FB_NONE;
float        feedbackTimer  = 0.0f;
const float  FEEDBACK_DURATION = 1.2f; // seconds

// =============================================
// QUIZ RE-TRIGGER PREVENTION
// =============================================
bool quizJustExited     = false;
float quizCooldownTimer = 0.0f;
const float QUIZ_COOLDOWN = 2.0f; // seconds after exit before can retrigger

// =============================================
// GAME STATES
// =============================================
enum GameState {
    TITLE_SCREEN,
    PLAYING,
    COMPLETED,
    RULES_SCREEN,
    QUIZ_SCREEN,
    GAME_OVER      // NEW
};
enum GameState currentGameState = TITLE_SCREEN;
bool rulesDisplayed = false;

// =============================================
// PLAYER
// =============================================
float playerX = 1.5f;
float playerY = 0.0f;
float playerZ = 1.5f;
float playerAngle = 0.0f;
float playerSpeed = 0.05f;
float rotationSpeed = 0.03f;
bool  gameCompleted = false;
char  congratsMessage[128];
int   playerScore = 0;
char  scoreString[32];
char  finalscore[32];
char  finaltime[32];
char  livesString[32];

// Board animation (replaces gold bar)
float boardYOffset   = 0.0f;
float boardAnimSpeed = 1.0f;
float boardRotation  = 0.0f;

// Penalties / rewards
const float PENALTY_WRONG  = 5.0f;
const float PENALTY_SKIP   = 2.0f;
const float REWARD_CORRECT = 3.0f;

// =============================================
// MAZE
// =============================================
const int MAZE_WIDTH  = 10;
const int MAZE_HEIGHT = 10;
int maze[MAZE_WIDTH][MAZE_HEIGHT] = {
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
int mazeCopy[MAZE_WIDTH][MAZE_HEIGHT];

void copyMazeArray(int dest[MAZE_WIDTH][MAZE_HEIGHT], int src[MAZE_WIDTH][MAZE_HEIGHT]) {
    for (int z = 0; z < MAZE_HEIGHT; z++)
        for (int x = 0; x < MAZE_WIDTH; x++)
            dest[z][x] = src[z][x];
}

// =============================================
// SHUFFLE QUESTION ORDER (easy->medium->hard)
// =============================================
void shuffleQuestions() {
    // Separate by difficulty, shuffle within each tier, then concat
    int easy[3]   = {0,1,2};
    int medium[4] = {3,4,5,6};
    int hard[3]   = {7,8,9};

    // Fisher-Yates shuffle each group
    for (int i=2; i>0; i--) { int j=rand()%(i+1); int t=easy[i]; easy[i]=easy[j]; easy[j]=t; }
    for (int i=3; i>0; i--) { int j=rand()%(i+1); int t=medium[i]; medium[i]=medium[j]; medium[j]=t; }
    for (int i=2; i>0; i--) { int j=rand()%(i+1); int t=hard[i]; hard[i]=hard[j]; hard[j]=t; }

    for (int i=0;i<3;i++) questionOrder[i]   = easy[i];
    for (int i=0;i<4;i++) questionOrder[3+i] = medium[i];
    for (int i=0;i<3;i++) questionOrder[7+i] = hard[i];

    for (int i=0;i<TOTAL_QUESTIONS;i++) questionUsed[i]=false;
    currentQuestionIndex = 0;
}

// Get next question index (sequential in shuffled order, wraps)
int getNextQuestion() {
    // find next unused in order
    for (int i = currentQuestionIndex; i < TOTAL_QUESTIONS; i++) {
        if (!questionUsed[questionOrder[i]]) {
            currentQuestionIndex = i;
            return questionOrder[i];
        }
    }
    // all used => reset and restart shuffle
    shuffleQuestions();
    return questionOrder[0];
}

// =============================================
// SETUP CAMERA
// =============================================
void setupCamera() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    float lookX = sin(playerAngle);
    float lookZ = -cos(playerAngle);
    gluLookAt(playerX, playerY+0.5f, playerZ,
              playerX+lookX, playerY+0.5f, playerZ+lookZ,
              0.0f, 1.0f, 0.0f);
}

// =============================================
// DRAW CUBE
// =============================================
void drawCube(float size) {
    float h = size / 2.0f;
    GLfloat normals[][3]  = {
        {0,0,1},{0,0,-1},{1,0,0},{-1,0,0},{0,1,0},{0,-1,0}
    };
    GLfloat verts[][3] = {
        {-h,-h, h},{ h,-h, h},{ h, h, h},{-h, h, h},
        {-h,-h,-h},{-h, h,-h},{ h, h,-h},{ h,-h,-h},
        { h,-h, h},{ h,-h,-h},{ h, h,-h},{ h, h, h},
        {-h,-h, h},{-h, h, h},{-h, h,-h},{-h,-h,-h},
        {-h, h, h},{ h, h, h},{ h, h,-h},{-h, h,-h},
        {-h,-h, h},{-h,-h,-h},{ h,-h,-h},{ h,-h, h}
    };
    glBegin(GL_QUADS);
    for (int f=0;f<6;f++) { glNormal3fv(normals[f]); for(int v=0;v<4;v++) glVertex3fv(verts[f*4+v]); }
    glEnd();
}

// =============================================
// COLLISION
// =============================================
bool checkCollision(float newX, float newZ) {
    float buf = 0.15f;
    int cx=(int)newX, cz=(int)newZ;
    if(cx<0||cx>=MAZE_WIDTH||cz<0||cz>=MAZE_HEIGHT) return true;
    if(maze[cz][cx]==1) return true;
    float fx=newX-cx, fz=newZ-cz;
    if(fx>(1-buf)&&cx+1<MAZE_WIDTH  &&maze[cz][cx+1]==1) return true;
    if(fx<buf    &&cx-1>=0          &&maze[cz][cx-1]==1) return true;
    if(fz>(1-buf)&&cz+1<MAZE_HEIGHT &&maze[cz+1][cx]==1) return true;
    if(fz<buf    &&cz-1>=0          &&maze[cz-1][cx]==1) return true;
    if(fx>(1-buf)&&fz>(1-buf)&&cx+1<MAZE_WIDTH &&cz+1<MAZE_HEIGHT&&maze[cz+1][cx+1]==1) return true;
    if(fx<buf    &&fz<buf    &&cx-1>=0          &&cz-1>=0         &&maze[cz-1][cx-1]==1) return true;
    if(fx>(1-buf)&&fz<buf    &&cx+1<MAZE_WIDTH  &&cz-1>=0         &&maze[cz-1][cx+1]==1) return true;
    if(fx<buf    &&fz>(1-buf)&&cx-1>=0          &&cz+1<MAZE_HEIGHT&&maze[cz+1][cx-1]==1) return true;
    return false;
}

bool isNearBoard(float px, float pz, int x, int z) {
    float dx=px-(x+0.5f), dz=pz-(z+0.5f);
    return sqrtf(dx*dx+dz*dz) < 0.9f;
}

// =============================================
// RENDER MAZE - QUIZ BOARD (new object design)
// =============================================
void renderMaze() {
    boardYOffset  = sinf((float)glfwGetTime() * boardAnimSpeed) * 0.06f;
    boardRotation += 0.4f;
    if (boardRotation > 360.0f) boardRotation -= 360.0f;

    for (int x=0; x<MAZE_WIDTH; x++) {
        for (int z=0; z<MAZE_HEIGHT; z++) {
            if (maze[z][x] == 1) {
                GLfloat grey[]={0.5f,0.5f,0.5f,1.0f};
                glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,grey);
                glPushMatrix();
                glTranslatef(x+0.5f,0.5f,z+0.5f);
                drawCube(1.0f);
                glPopMatrix();

            } else if (maze[z][x] == 2) {
                GLfloat red[]={1.0f,0.0f,0.0f,1.0f};
                glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,red);
                glPushMatrix();
                glTranslatef(x+0.5f,0.5f,z+0.5f);
                drawCube(1.0f);
                glPopMatrix();

            } else if (maze[z][x] == 3) {
                // === QUIZ CHECKPOINT BOARD ===
                glPushMatrix();
                glTranslatef(x+0.5f, boardYOffset, z+0.5f);
                glRotatef(boardRotation, 0.0f, 1.0f, 0.0f);

                GLfloat no_spec[]={0,0,0,1};
                GLfloat no_shin[]={0};

                // -- Tiang vertikal (coklat kayu) --
                GLfloat wood[]={0.55f,0.27f,0.07f,1.0f};
                glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,wood);
                glMaterialfv(GL_FRONT,GL_SPECULAR,no_spec);
                glMaterialfv(GL_FRONT,GL_SHININESS,no_shin);
                glPushMatrix();
                glTranslatef(0.0f, 0.1f, 0.0f);
                glScalef(0.08f, 1.0f, 0.08f);
                drawCube(1.0f);
                glPopMatrix();

                // -- Board kayu (papan) --
                GLfloat darkwood[]={0.4f,0.2f,0.05f,1.0f};
                glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,darkwood);
                glPushMatrix();
                glTranslatef(0.0f, 0.72f, 0.04f);
                glScalef(0.7f, 0.45f, 0.06f);
                drawCube(1.0f);
                glPopMatrix();

                // -- Kertas putih di depan board --
                GLfloat white[]={1.0f,1.0f,1.0f,1.0f};
                glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,white);
                glPushMatrix();
                glTranslatef(0.0f, 0.72f, 0.075f);
                glScalef(0.58f, 0.35f, 0.01f);
                drawCube(1.0f);
                glPopMatrix();

                // -- Garis biru simulasi teks di kertas --
                GLfloat lineblue[]={0.1f,0.2f,0.8f,1.0f};
                glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,lineblue);
                for (int ln=0; ln<4; ln++) {
                    glPushMatrix();
                    glTranslatef(0.0f, 0.82f - ln*0.07f, 0.082f);
                    glScalef(0.4f, 0.012f, 0.005f);
                    drawCube(1.0f);
                    glPopMatrix();
                }

                // -- Tanda tanya kuning di tengah board --
                GLfloat yellow[]={1.0f,0.9f,0.0f,1.0f};
                glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,yellow);
                glPushMatrix();
                glTranslatef(0.18f, 0.72f, 0.083f);
                glScalef(0.06f, 0.18f, 0.005f);
                drawCube(1.0f);
                glPopMatrix();

                glPopMatrix(); // end board rotate+translate
            }
        }
    }
}

// =============================================
// KEY STATE
// =============================================
bool keyPressedLastFrame[GLFW_KEY_LAST] = {false};

// =============================================
// HELPER: apply answer result
// =============================================
void applyCorrectAnswer() {
    maze[paperCellZ][paperCellX] = 0;
    playerScore++;
    sprintf(scoreString, "Kertas: %d", playerScore);
    gameStartTime += REWARD_CORRECT; // reduce elapsed time = bonus
    feedbackState = FB_CORRECT;
    feedbackTimer = (float)glfwGetTime();
    questionUsed[activeQuestion] = true;
    currentQuestionIndex++;
    printf("[QUIZ] Benar! +1 skor, bonus -%0.0f detik\n", REWARD_CORRECT);
}

void applyWrongAnswer() {
    playerLives--;
    sprintf(livesString, "Lives: %d", playerLives);
    gameStartTime -= PENALTY_WRONG; // increase elapsed time = penalty
    feedbackState = FB_WRONG;
    feedbackTimer = (float)glfwGetTime();
    printf("[QUIZ] Salah! -%d nyawa, penalti +%0.0f detik\n", 1, PENALTY_WRONG);
    if (playerLives <= 0) {
        currentGameState = GAME_OVER;
        feedbackState = FB_NONE;
    }
}

void applyTimeout() {
    playerLives--;
    sprintf(livesString, "Lives: %d", playerLives);
    gameStartTime -= PENALTY_WRONG;
    feedbackState = FB_WRONG;
    feedbackTimer = (float)glfwGetTime();
    printf("[QUIZ] Waktu habis! -%d nyawa\n", 1);
    if (playerLives <= 0) {
        currentGameState = GAME_OVER;
        feedbackState = FB_NONE;
    }
}

// =============================================
// PROCESS INPUT
// =============================================
void processInput(GLFWwindow* window) {
    float currentTime = (float)glfwGetTime();
    static float lastTime = 0.0f;
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    // --- TITLE SCREEN ---
    if (currentGameState == TITLE_SCREEN) {
        if (glfwGetKey(window,GLFW_KEY_P)==GLFW_PRESS && !keyPressedLastFrame[GLFW_KEY_P]) {
            currentGameState = PLAYING;
            gameStarted = false; gameCompleted = false;
            playerScore = 0; playerLives = MAX_LIVES;
            feedbackState = FB_NONE;
            quizJustExited = false;
            gameStartTime = currentTime;
            sprintf(timeString,  "Time: 0.00");
            sprintf(scoreString, "Kertas: 0");
            sprintf(livesString, "Lives: %d", playerLives);
            playerX=1.5f; playerY=0.0f; playerZ=1.5f; playerAngle=0.0f;
            shuffleQuestions();
            keyPressedLastFrame[GLFW_KEY_P]=true;
        } else if (glfwGetKey(window,GLFW_KEY_P)==GLFW_RELEASE) keyPressedLastFrame[GLFW_KEY_P]=false;

        bool shift = glfwGetKey(window,GLFW_KEY_LEFT_SHIFT)==GLFW_PRESS ||
                     glfwGetKey(window,GLFW_KEY_RIGHT_SHIFT)==GLFW_PRESS;
        if (glfwGetKey(window,GLFW_KEY_SLASH)==GLFW_PRESS && shift && !keyPressedLastFrame[GLFW_KEY_SLASH]) {
            currentGameState=RULES_SCREEN; keyPressedLastFrame[GLFW_KEY_SLASH]=true;
        } else if (glfwGetKey(window,GLFW_KEY_SLASH)==GLFW_RELEASE) keyPressedLastFrame[GLFW_KEY_SLASH]=false;

        if (glfwGetKey(window,GLFW_KEY_ESCAPE)==GLFW_PRESS) glfwSetWindowShouldClose(window,GL_TRUE);
        return;
    }

    // --- RULES SCREEN ---
    if (currentGameState == RULES_SCREEN) {
        if (glfwGetKey(window,GLFW_KEY_B)==GLFW_PRESS && !keyPressedLastFrame[GLFW_KEY_B]) {
            currentGameState=TITLE_SCREEN; keyPressedLastFrame[GLFW_KEY_B]=true;
        } else if (glfwGetKey(window,GLFW_KEY_B)==GLFW_RELEASE) keyPressedLastFrame[GLFW_KEY_B]=false;
        if (glfwGetKey(window,GLFW_KEY_ESCAPE)==GLFW_PRESS) glfwSetWindowShouldClose(window,GL_TRUE);
        return;
    }

    // --- GAME OVER ---
    if (currentGameState == GAME_OVER) {
        if (glfwGetKey(window,GLFW_KEY_R)==GLFW_PRESS && !keyPressedLastFrame[GLFW_KEY_R]) {
            playerX=1.5f; playerY=0.0f; playerZ=1.5f; playerAngle=0.0f;
            gameStarted=false; gameCompleted=false;
            playerScore=0; playerLives=MAX_LIVES;
            feedbackState=FB_NONE; quizJustExited=false;
            currentQuestionIndex=0;
            currentGameState=PLAYING;
            gameStartTime=currentTime;
            sprintf(timeString,"Time: 0.00");
            sprintf(scoreString,"Kertas: 0");
            sprintf(livesString,"Lives: %d",playerLives);
            copyMazeArray(maze,mazeCopy);
            shuffleQuestions();
            keyPressedLastFrame[GLFW_KEY_R]=true;
        } else if (glfwGetKey(window,GLFW_KEY_R)==GLFW_RELEASE) keyPressedLastFrame[GLFW_KEY_R]=false;

        if (glfwGetKey(window,GLFW_KEY_B)==GLFW_PRESS && !keyPressedLastFrame[GLFW_KEY_B]) {
            currentGameState=TITLE_SCREEN; keyPressedLastFrame[GLFW_KEY_B]=true;
        } else if (glfwGetKey(window,GLFW_KEY_B)==GLFW_RELEASE) keyPressedLastFrame[GLFW_KEY_B]=false;
        if (glfwGetKey(window,GLFW_KEY_ESCAPE)==GLFW_PRESS) glfwSetWindowShouldClose(window,GL_TRUE);
        return;
    }

    // --- COMPLETED ---
    if (currentGameState == COMPLETED || gameCompleted) {
        if (glfwGetKey(window,GLFW_KEY_R)==GLFW_PRESS && !keyPressedLastFrame[GLFW_KEY_R]) {
            playerX=1.5f; playerY=0.0f; playerZ=1.5f; playerAngle=0.0f;
            gameStarted=false; gameCompleted=false;
            playerScore=0; playerLives=MAX_LIVES;
            feedbackState=FB_NONE; quizJustExited=false;
            currentQuestionIndex=0;
            currentGameState=PLAYING;
            gameStartTime=currentTime;
            sprintf(timeString,"Time: 0.00");
            sprintf(scoreString,"Kertas: 0");
            sprintf(livesString,"Lives: %d",playerLives);
            copyMazeArray(maze,mazeCopy);
            shuffleQuestions();
            keyPressedLastFrame[GLFW_KEY_R]=true;
        } else if (glfwGetKey(window,GLFW_KEY_R)==GLFW_RELEASE) keyPressedLastFrame[GLFW_KEY_R]=false;

        if (glfwGetKey(window,GLFW_KEY_B)==GLFW_PRESS && !keyPressedLastFrame[GLFW_KEY_B]) {
            currentGameState=TITLE_SCREEN; keyPressedLastFrame[GLFW_KEY_B]=true;
        } else if (glfwGetKey(window,GLFW_KEY_B)==GLFW_RELEASE) keyPressedLastFrame[GLFW_KEY_B]=false;
        if (glfwGetKey(window,GLFW_KEY_ESCAPE)==GLFW_PRESS) glfwSetWindowShouldClose(window,GL_TRUE);
        return;
    }

    // --- QUIZ SCREEN (movement LOCKED) ---
    if (currentGameState == QUIZ_SCREEN) {
        Question& q = questions[activeQuestion];

        // Timer check
        float remaining = quizTimeLimit - (currentTime - quizStartTime);
        if (remaining <= 0.0f) {
            applyTimeout();
            quizJustExited = true;
            quizCooldownTimer = currentTime;
            if (currentGameState != GAME_OVER) currentGameState = PLAYING;
            return;
        }

        // Wait for feedback animation before accepting new input
        if (feedbackState != FB_NONE) {
            if (currentTime - feedbackTimer >= FEEDBACK_DURATION) {
                feedbackState = FB_NONE;
                if (currentGameState == QUIZ_SCREEN) currentGameState = PLAYING;
            }
            return;
        }

        // Answer A
        if (glfwGetKey(window,GLFW_KEY_A)==GLFW_PRESS && !keyPressedLastFrame[GLFW_KEY_A]) {
            if (q.correct==0) applyCorrectAnswer(); else applyWrongAnswer();
            quizJustExited=true; quizCooldownTimer=currentTime;
            if (currentGameState!=GAME_OVER && feedbackState==FB_NONE) currentGameState=PLAYING;
            keyPressedLastFrame[GLFW_KEY_A]=true;
        } else if (glfwGetKey(window,GLFW_KEY_A)==GLFW_RELEASE) keyPressedLastFrame[GLFW_KEY_A]=false;

        // Answer B
        if (glfwGetKey(window,GLFW_KEY_B)==GLFW_PRESS && !keyPressedLastFrame[GLFW_KEY_B]) {
            if (q.correct==1) applyCorrectAnswer(); else applyWrongAnswer();
            quizJustExited=true; quizCooldownTimer=currentTime;
            if (currentGameState!=GAME_OVER && feedbackState==FB_NONE) currentGameState=PLAYING;
            keyPressedLastFrame[GLFW_KEY_B]=true;
        } else if (glfwGetKey(window,GLFW_KEY_B)==GLFW_RELEASE) keyPressedLastFrame[GLFW_KEY_B]=false;

        // Answer C
        if (glfwGetKey(window,GLFW_KEY_C)==GLFW_PRESS && !keyPressedLastFrame[GLFW_KEY_C]) {
            if (q.correct==2) applyCorrectAnswer(); else applyWrongAnswer();
            quizJustExited=true; quizCooldownTimer=currentTime;
            if (currentGameState!=GAME_OVER && feedbackState==FB_NONE) currentGameState=PLAYING;
            keyPressedLastFrame[GLFW_KEY_C]=true;
        } else if (glfwGetKey(window,GLFW_KEY_C)==GLFW_RELEASE) keyPressedLastFrame[GLFW_KEY_C]=false;

        // Answer D
        if (glfwGetKey(window,GLFW_KEY_D)==GLFW_PRESS && !keyPressedLastFrame[GLFW_KEY_D]) {
            if (q.correct==3) applyCorrectAnswer(); else applyWrongAnswer();
            quizJustExited=true; quizCooldownTimer=currentTime;
            if (currentGameState!=GAME_OVER && feedbackState==FB_NONE) currentGameState=PLAYING;
            keyPressedLastFrame[GLFW_KEY_D]=true;
        } else if (glfwGetKey(window,GLFW_KEY_D)==GLFW_RELEASE) keyPressedLastFrame[GLFW_KEY_D]=false;

        // Skip (E) - small penalty, NO life loss
        if (glfwGetKey(window,GLFW_KEY_E)==GLFW_PRESS && !keyPressedLastFrame[GLFW_KEY_E]) {
            gameStartTime -= PENALTY_SKIP;
            printf("[QUIZ] Dilewati. Penalti +%.0f detik\n", PENALTY_SKIP);
            quizJustExited=true; quizCooldownTimer=currentTime;
            currentGameState=PLAYING;
            keyPressedLastFrame[GLFW_KEY_E]=true;
        } else if (glfwGetKey(window,GLFW_KEY_E)==GLFW_RELEASE) keyPressedLastFrame[GLFW_KEY_E]=false;

        return; // movement LOCKED in quiz
    }

    // --- PLAYING ---
    float adjSpd    = playerSpeed    * deltaTime * 60.0f;
    float adjRotSpd = rotationSpeed  * deltaTime * 60.0f;

    // Feedback overlay timer (shown during PLAYING after returning from quiz)
    if (feedbackState != FB_NONE && currentTime - feedbackTimer >= FEEDBACK_DURATION)
        feedbackState = FB_NONE;

    // Quiz cooldown (prevent re-trigger)
    if (quizJustExited && currentTime - quizCooldownTimer >= QUIZ_COOLDOWN)
        quizJustExited = false;

    if (!gameStarted && (
        glfwGetKey(window,GLFW_KEY_W)==GLFW_PRESS || glfwGetKey(window,GLFW_KEY_S)==GLFW_PRESS ||
        glfwGetKey(window,GLFW_KEY_A)==GLFW_PRESS || glfwGetKey(window,GLFW_KEY_D)==GLFW_PRESS)) {
        gameStarted=true; gameStartTime=currentTime;
    }

    if (glfwGetKey(window,GLFW_KEY_W)==GLFW_PRESS) {
        float nx=playerX+sinf(playerAngle)*adjSpd, nz=playerZ-cosf(playerAngle)*adjSpd;
        if (!checkCollision(nx,nz)){playerX=nx;playerZ=nz;}
        else if (!checkCollision(nx,playerZ)) playerX=nx;
        else if (!checkCollision(playerX,nz)) playerZ=nz;
    }
    if (glfwGetKey(window,GLFW_KEY_S)==GLFW_PRESS) {
        float nx=playerX-sinf(playerAngle)*adjSpd, nz=playerZ+cosf(playerAngle)*adjSpd;
        if (!checkCollision(nx,nz)){playerX=nx;playerZ=nz;}
        else if (!checkCollision(nx,playerZ)) playerX=nx;
        else if (!checkCollision(playerX,nz)) playerZ=nz;
    }
    if (glfwGetKey(window,GLFW_KEY_A)==GLFW_PRESS) playerAngle -= adjRotSpd;
    if (glfwGetKey(window,GLFW_KEY_D)==GLFW_PRESS) playerAngle += adjRotSpd;

    int cellX=(int)playerX, cellZ=(int)playerZ;
    if (cellX>=0 && cellX<MAZE_WIDTH && cellZ>=0 && cellZ<MAZE_HEIGHT) {
        // Goal reached
        if (maze[cellZ][cellX]==2 && !gameCompleted) {
            gameCompleted=true; currentGameState=COMPLETED;
            float finalT = currentGameTime - (REWARD_CORRECT * playerScore);
            sprintf(congratsMessage, "Selesai dalam %.2f detik (raw)", currentGameTime);
            sprintf(finalscore,      "Kertas Dikumpulkan: %d", playerScore);
            sprintf(finaltime,       "Waktu Final: %.2f detik", finalT);
        }

        // Quiz board trigger — PREVENT RE-TRIGGER
        if (maze[cellZ][cellX]==3 && isNearBoard(playerX,playerZ,cellX,cellZ) && !quizJustExited) {
            paperCellX=cellX; paperCellZ=cellZ;
            activeQuestion=getNextQuestion();
            quizStartTime=currentTime;
            currentGameState=QUIZ_SCREEN;
            feedbackState=FB_NONE;
        }
    }

    if (glfwGetKey(window,GLFW_KEY_ESCAPE)==GLFW_PRESS && !keyPressedLastFrame[GLFW_KEY_ESCAPE]) {
        currentGameState=TITLE_SCREEN; keyPressedLastFrame[GLFW_KEY_ESCAPE]=true;
    } else if (glfwGetKey(window,GLFW_KEY_ESCAPE)==GLFW_RELEASE) keyPressedLastFrame[GLFW_KEY_ESCAPE]=false;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0,0,width,height);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluPerspective(60.0,(GLfloat)width/(GLfloat)height,0.1,100.0);
    glMatrixMode(GL_MODELVIEW);
}

// =============================================
// RENDER TITLE SCREEN
// =============================================
void renderTitleScreen() {
    glDisable(GL_LIGHTING); glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    glOrtho(0,WIDTH,0,HEIGHT,-1,1);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();

    glBegin(GL_QUADS);
    glColor3f(0.05f,0.05f,0.2f); glVertex2f(0,0); glVertex2f(WIDTH,0);
    glColor3f(0.2f,0.2f,0.5f);   glVertex2f(WIDTH,HEIGHT); glVertex2f(0,HEIGHT);
    glEnd();

    int cx=WIDTH/2, cy=HEIGHT/2;

    glColor3f(1.0f,0.8f,0.0f);
    char t1[]="MAZE3D - QUIZ CHALLENGE";
    glRasterPos2i(cx-(int)(strlen(t1)*7), cy+90);
    for(int i=0;t1[i];i++) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,t1[i]);

    glColor3f(0.7f,0.9f,1.0f);
    char t2[]="10 Soal | 3 Nyawa | Easy -> Hard";
    glRasterPos2i(cx-(int)(strlen(t2)*5), cy+55);
    for(int i=0;t2[i];i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,t2[i]);

    glColor3f(1,1,1);
    const char* items[]={"Press P to Play","Press ? for Rules","Press ESC to Quit"};
    for(int i=0;i<3;i++) {
        glRasterPos2i(cx-(int)(strlen(items[i])*4), cy-10-i*30);
        for(int j=0;items[i][j];j++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,items[i][j]);
    }

    float pulse=(sinf((float)glfwGetTime()*2.0f)+1.0f)*0.5f;
    glLineWidth(3.0f); glColor3f(0.5f+pulse*0.5f,0.4f,0.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(cx-250,cy-110); glVertex2f(cx+250,cy-110);
    glVertex2f(cx+250,cy+120); glVertex2f(cx-250,cy+120);
    glEnd(); glLineWidth(1.0f);

    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);  glPopMatrix();
    glEnable(GL_DEPTH_TEST); glEnable(GL_LIGHTING);
}

// =============================================
// RENDER RULES SCREEN
// =============================================
void renderRulesScreen() {
    glDisable(GL_LIGHTING); glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    glOrtho(0,WIDTH,0,HEIGHT,-1,1);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();

    glBegin(GL_QUADS);
    glColor3f(0.1f,0.1f,0.3f);
    glVertex2f(0,0); glVertex2f(WIDTH,0); glVertex2f(WIDTH,HEIGHT); glVertex2f(0,HEIGHT);
    glEnd();

    glColor3f(1.0f,0.8f,0.0f);
    char title[]="HOW TO PLAY";
    glRasterPos2i(WIDTH/2-(int)(strlen(title)*6),HEIGHT-45);
    for(int i=0;title[i];i++) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,title[i]);

    glColor3f(1,1,1);
    const char* lines[]={
        "GERAKAN: W=Maju  S=Mundur  A=Putar Kiri  D=Putar Kanan",
        "",
        "QUIZ BOARD (papan kayu berputar):",
        "  Dekati board => soal muncul, gerak TERKUNCI",
        "  A/B/C/D = jawab pilihan",
        "  E = lewati soal (penalti kecil +2 detik, tidak kehilangan nyawa)",
        "",
        "SISTEM NYAWA: Mulai dengan 3 nyawa (hati)",
        "  Jawab salah / waktu habis => -1 nyawa + penalti +5 detik",
        "  Jawab benar => +1 skor + bonus -3 detik",
        "  0 nyawa => GAME OVER",
        "",
        "TUJUAN: Temukan blok MERAH secepat mungkin!",
        "",
        "Press B untuk kembali"
    };
    for(int i=0;i<15;i++) {
        if(i==7) glColor3f(1.0f,0.5f,0.5f);
        else if(i==12) glColor3f(0.5f,1.0f,0.5f);
        else glColor3f(1,1,1);
        glRasterPos2i(60, HEIGHT-90-i*28);
        for(int j=0;lines[i][j];j++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,lines[i][j]);
    }

    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);  glPopMatrix();
    glEnable(GL_DEPTH_TEST); glEnable(GL_LIGHTING);
}

// =============================================
// RENDER HUD (timer + score + lives)
// =============================================
void renderHUD() {
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    glOrtho(0,WIDTH,HEIGHT,0,-1,1);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();

    // Background strip
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
    glColor4f(0,0,0,0.5f);
    glVertex2f(0,0); glVertex2f(WIDTH,0); glVertex2f(WIDTH,55); glVertex2f(0,55);
    glEnd(); glDisable(GL_BLEND);

    glColor3f(1,1,1);
    glRasterPos2i(10,18);
    for(int i=0;timeString[i];i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,timeString[i]);

    glColor3f(0.4f,1.0f,0.4f);
    glRasterPos2i(10,38);
    for(int i=0;scoreString[i];i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,scoreString[i]);

    // Lives as hearts
    glColor3f(1.0f,0.3f,0.3f);
    char heartFull[]  = "[♥]";
    char heartEmpty[] = "[♡]";
    int lx = WIDTH - 160;
    for(int i=0;i<MAX_LIVES;i++) {
        glRasterPos2i(lx + i*50, 28);
        const char* h = (i < playerLives) ? heartFull : heartEmpty;
        for(int j=0;h[j];j++) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,h[j]);
    }

    glMatrixMode(GL_MODELVIEW); glPopMatrix();
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_LIGHTING);
}

// =============================================
// RENDER VISUAL FEEDBACK OVERLAY
// =============================================
void renderFeedbackOverlay() {
    if (feedbackState == FB_NONE) return;
    float elapsed = (float)glfwGetTime() - feedbackTimer;
    if (elapsed >= FEEDBACK_DURATION) { feedbackState=FB_NONE; return; }

    float alpha = 0.45f * (1.0f - elapsed/FEEDBACK_DURATION); // fade out

    glDisable(GL_LIGHTING); glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    glOrtho(0,WIDTH,0,HEIGHT,-1,1);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();

    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
    if (feedbackState==FB_CORRECT) glColor4f(0.0f,1.0f,0.2f,alpha);
    else                            glColor4f(1.0f,0.1f,0.1f,alpha);
    glVertex2f(0,0); glVertex2f(WIDTH,0); glVertex2f(WIDTH,HEIGHT); glVertex2f(0,HEIGHT);
    glEnd();
    glDisable(GL_BLEND);

    // Big text
    glColor3f(1,1,1);
    const char* msg = (feedbackState==FB_CORRECT) ? "BENAR! +1 Skor" : "SALAH! -1 Nyawa";
    int len = strlen(msg);
    glRasterPos2i(WIDTH/2 - len*7, HEIGHT/2+10);
    for(int i=0;msg[i];i++) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,msg[i]);

    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);  glPopMatrix();
    glEnable(GL_DEPTH_TEST); glEnable(GL_LIGHTING);
}

// =============================================
// RENDER QUIZ SCREEN
// =============================================
void renderQuizScreen() {
    glDisable(GL_LIGHTING); glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    glOrtho(0,WIDTH,0,HEIGHT,-1,1);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();

    // Semi-transparent dark overlay
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
    glColor4f(0.03f,0.03f,0.15f,0.94f);
    glVertex2f(50,50); glVertex2f(750,50); glVertex2f(750,550); glVertex2f(50,550);
    glEnd(); glDisable(GL_BLEND);

    // Border (color by difficulty)
    Question& q=questions[activeQuestion];
    if(q.difficulty==0)      glColor3f(0.3f,1.0f,0.3f);   // easy=green
    else if(q.difficulty==1) glColor3f(1.0f,0.7f,0.1f);   // medium=orange
    else                     glColor3f(1.0f,0.2f,0.2f);   // hard=red
    glLineWidth(4.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(50,50); glVertex2f(750,50); glVertex2f(750,550); glVertex2f(50,550);
    glEnd(); glLineWidth(1.0f);

    // Difficulty label
    const char* diffLabel[]={"[ MUDAH ]","[ SEDANG ]","[ SULIT ]"};
    GLfloat diffClr[3][3]={{0.3f,1.0f,0.3f},{1.0f,0.7f,0.1f},{1.0f,0.3f,0.3f}};
    glColor3fv(diffClr[q.difficulty]);
    glRasterPos2i(WIDTH/2-(int)(strlen(diffLabel[q.difficulty])*6),525);
    for(int i=0;diffLabel[q.difficulty][i];i++) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,diffLabel[q.difficulty][i]);

    // Question number
    char numText[32];
    sprintf(numText,"Soal %d / %d",currentQuestionIndex+1,TOTAL_QUESTIONS);
    glColor3f(0.7f,0.7f,0.7f);
    glRasterPos2i(590,525);
    for(int i=0;numText[i];i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,numText[i]);

    // Question text (word-wrap simple: just display)
    glColor3f(1,1,1);
    glRasterPos2i(80,475);
    for(int i=0;q.question[i];i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,q.question[i]);

    // Answer options
    GLfloat optClr[4][3]={{0.3f,1.0f,0.4f},{0.3f,0.6f,1.0f},{1.0f,0.85f,0.2f},{1.0f,0.4f,0.5f}};
    const char* keys[]={"[A]","[B]","[C]","[D]"};
    for(int i=0;i<4;i++) {
        // Option background box
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glBegin(GL_QUADS);
        glColor4f(optClr[i][0]*0.3f, optClr[i][1]*0.3f, optClr[i][2]*0.3f, 0.6f);
        int oy=390-i*65;
        glVertex2f(75,oy-5); glVertex2f(725,oy-5); glVertex2f(725,oy+28); glVertex2f(75,oy+28);
        glEnd(); glDisable(GL_BLEND);

        glColor3fv(optClr[i]);
        glRasterPos2i(85,390-i*65);
        for(int j=0;keys[i][j];j++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,keys[i][j]);
        glColor3f(1,1,1);
        glRasterPos2i(125,390-i*65);
        for(int j=0;q.options[i][j];j++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,q.options[i][j]);
    }

    // Timer countdown
    float remaining=quizTimeLimit-((float)glfwGetTime()-quizStartTime);
    if(remaining<0)remaining=0;
    char timerTxt[32]; sprintf(timerTxt,"%.1f detik",remaining);
    if(remaining<5.0f)      glColor3f(1.0f,0.2f,0.2f);
    else if(remaining<8.0f) glColor3f(1.0f,0.7f,0.1f);
    else                    glColor3f(0.3f,1.0f,0.5f);
    glRasterPos2i(570,90);
    for(int i=0;timerTxt[i];i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,timerTxt[i]);

    // Timer bar (gradient)
    float frac=remaining/quizTimeLimit; if(frac<0)frac=0;
    glBegin(GL_QUADS);
    if(frac>0.5f)       glColor3f(0.2f,0.85f,0.3f);
    else if(frac>0.25f) glColor3f(0.9f,0.6f,0.1f);
    else                glColor3f(0.9f,0.1f,0.1f);
    glVertex2f(80,75); glVertex2f(80+frac*610,75);
    glVertex2f(80+frac*610,92); glVertex2f(80,92);
    glEnd();
    glColor3f(0.4f,0.4f,0.4f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(80,75); glVertex2f(690,75); glVertex2f(690,92); glVertex2f(80,92);
    glEnd();

    // Skip hint
    glColor3f(0.7f,0.7f,0.3f);
    char skipTxt[]="[E] Lewati (+2 detik penalti, tanpa kehilangan nyawa)";
    glRasterPos2i(80,60);
    for(int i=0;skipTxt[i];i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,skipTxt[i]);

    // Feedback overlay (within quiz)
    renderFeedbackOverlay();

    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);  glPopMatrix();
    glEnable(GL_DEPTH_TEST); glEnable(GL_LIGHTING);
}

// =============================================
// RENDER GAME OVER SCREEN
// =============================================
void renderGameOverScreen() {
    glDisable(GL_LIGHTING); glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    glOrtho(0,WIDTH,0,HEIGHT,-1,1);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();

    // Pulsing red background
    float pulse=(sinf((float)glfwGetTime()*2.0f)+1.0f)*0.5f;
    glBegin(GL_QUADS);
    glColor3f(0.25f+pulse*0.1f,0.0f,0.0f);
    glVertex2f(0,0); glVertex2f(WIDTH,0);
    glColor3f(0.1f,0.0f,0.0f);
    glVertex2f(WIDTH,HEIGHT); glVertex2f(0,HEIGHT);
    glEnd();

    glLineWidth(5.0f); glColor3f(0.8f,0.0f,0.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(50,150); glVertex2f(750,150); glVertex2f(750,450); glVertex2f(50,450);
    glEnd(); glLineWidth(1.0f);

    int cx=WIDTH/2, cy=HEIGHT/2;

    glColor3f(1.0f,0.1f,0.1f);
    char goText[]="GAME OVER";
    glRasterPos2i(cx-(int)(strlen(goText)*11), cy+100);
    for(int i=0;goText[i];i++) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,goText[i]);

    glColor3f(1.0f,1.0f,1.0f);
    char t1[64]; sprintf(t1,"Waktu: %.2f detik",currentGameTime);
    glRasterPos2i(cx-(int)(strlen(t1)*5), cy+40);
    for(int i=0;t1[i];i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,t1[i]);

    char t2[64]; sprintf(t2,"Kertas Dikumpulkan: %d",playerScore);
    glRasterPos2i(cx-(int)(strlen(t2)*5), cy+15);
    for(int i=0;t2[i];i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,t2[i]);

    glColor3f(0.8f,0.8f,0.8f);
    char hint[]="Press R to Restart  |  Press B for Title";
    glRasterPos2i(cx-(int)(strlen(hint)*5), cy-40);
    for(int i=0;hint[i];i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,hint[i]);

    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);  glPopMatrix();
    glEnable(GL_DEPTH_TEST); glEnable(GL_LIGHTING);
}

// =============================================
// RENDER CONGRATS SCREEN
// =============================================
void renderCongratsScreen() {
    glDisable(GL_LIGHTING); glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    glOrtho(0,WIDTH,0,HEIGHT,-1,1);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();

    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
    glColor4f(0.0f,0.0f,0.3f,0.9f); glVertex2f(0,0); glVertex2f(WIDTH,0);
    glColor4f(0.1f,0.2f,0.5f,0.9f); glVertex2f(WIDTH,HEIGHT); glVertex2f(0,HEIGHT);
    glEnd();

    glLineWidth(5.0f); glColor3f(1.0f,0.8f,0.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(WIDTH*0.1f,HEIGHT*0.2f); glVertex2f(WIDTH*0.9f,HEIGHT*0.2f);
    glVertex2f(WIDTH*0.9f,HEIGHT*0.8f); glVertex2f(WIDTH*0.1f,HEIGHT*0.8f);
    glEnd(); glLineWidth(1.0f);

    int cx=WIDTH/2, cy=HEIGHT/2;
    glColor3f(1.0f,1.0f,0.0f);
    char t[]="MAZE COMPLETED!";
    glRasterPos2i(cx-(int)(strlen(t)*10),cy+100);
    for(int i=0;t[i];i++) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,t[i]);

    glColor3f(1,1,1);
    const char* lines[]={congratsMessage,finalscore,finaltime};
    for(int i=0;i<3;i++) {
        glRasterPos2i(cx-(int)(strlen(lines[i])*5), cy+40-i*28);
        for(int j=0;lines[i][j];j++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,lines[i][j]);
    }

    glColor3f(0.7f,0.7f,0.7f);
    char hint[]="Press R to Play Again  |  B for Title";
    glRasterPos2i(cx-(int)(strlen(hint)*5),cy-80);
    for(int i=0;hint[i];i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,hint[i]);

    static float sp[20][2]; static bool sinit=false;
    if(!sinit){for(int i=0;i<20;i++){sp[i][0]=(float)(rand()%WIDTH);sp[i][1]=(float)(rand()%HEIGHT);}sinit=true;}
    glPointSize(6.0f); glBegin(GL_POINTS);
    for(int i=0;i<20;i++){
        float osc=(sinf((float)glfwGetTime()*3.0f+i)+1.0f)*0.5f;
        glColor3f(1.0f,osc,osc); glVertex2f(sp[i][0],sp[i][1]);
        sp[i][0]+=sinf((float)glfwGetTime()*2.0f+i)*2.0f; sp[i][1]+=cosf((float)glfwGetTime()*2.0f+i)*2.0f;
        if(sp[i][0]<0)sp[i][0]=WIDTH; if(sp[i][0]>WIDTH)sp[i][0]=0;
        if(sp[i][1]<0)sp[i][1]=HEIGHT; if(sp[i][1]>HEIGHT)sp[i][1]=0;
    }
    glEnd(); glPointSize(1.0f);
    glDisable(GL_BLEND);

    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);  glPopMatrix();
    glEnable(GL_DEPTH_TEST); glEnable(GL_LIGHTING);
}

// =============================================
// MAIN
// =============================================
int main() {
    srand((unsigned int)time(NULL));
    int argc=1; char* argv[1]={(char*)"App"};
    glutInit(&argc,argv);

    if (!glfwInit()) { fprintf(stderr,"GLFW init failed\n"); return -1; }

    currentGameState=TITLE_SCREEN;

    GLFWwindow* window=glfwCreateWindow(WIDTH,HEIGHT,"3D Maze Quiz Challenge",NULL,NULL);
    if (!window) { fprintf(stderr,"Window creation failed\n"); glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window,framebuffer_size_callback);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE); glCullFace(GL_BACK);

    copyMazeArray(mazeCopy,maze);
    shuffleQuestions();

    glEnable(GL_LIGHTING); glEnable(GL_LIGHT0);
    GLfloat lpos[]={2.0,3.0,2.0,0.0};
    GLfloat lamb[]={0.25,0.25,0.25,1.0};
    GLfloat ldif[]={1.0,1.0,1.0,1.0};
    GLfloat lspc[]={1.0,1.0,1.0,1.0};
    glLightfv(GL_LIGHT0,GL_POSITION,lpos);
    glLightfv(GL_LIGHT0,GL_AMBIENT, lamb);
    glLightfv(GL_LIGHT0,GL_DIFFUSE, ldif);
    glLightfv(GL_LIGHT0,GL_SPECULAR,lspc);

    GLfloat mAmb[]={0.7,0.7,0.7,1.0};
    GLfloat mDif[]={0.8,0.8,0.8,1.0};
    GLfloat mSpc[]={1.0,1.0,1.0,1.0};
    GLfloat mShn[]={80.0};
    glMaterialfv(GL_FRONT,GL_AMBIENT,  mAmb);
    glMaterialfv(GL_FRONT,GL_DIFFUSE,  mDif);
    glMaterialfv(GL_FRONT,GL_SPECULAR, mSpc);
    glMaterialfv(GL_FRONT,GL_SHININESS,mShn);

    gameStartTime=(float)glfwGetTime();
    sprintf(timeString, "Time: 0.00");
    sprintf(scoreString,"Kertas: 0");
    sprintf(livesString,"Lives: %d",playerLives);

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glClearColor(0.53f,0.81f,0.92f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        switch (currentGameState) {
            case TITLE_SCREEN:
                renderTitleScreen();
                break;

            case RULES_SCREEN:
                renderRulesScreen();
                break;

            case PLAYING:
                if (gameStarted && !gameCompleted) {
                    currentGameTime=(float)glfwGetTime()-gameStartTime;
                    sprintf(timeString,"Time: %.2f",currentGameTime);
                } else if (!gameStarted && (
                    glfwGetKey(window,GLFW_KEY_W)==GLFW_PRESS ||
                    glfwGetKey(window,GLFW_KEY_A)==GLFW_PRESS ||
                    glfwGetKey(window,GLFW_KEY_S)==GLFW_PRESS ||
                    glfwGetKey(window,GLFW_KEY_D)==GLFW_PRESS)) {
                    gameStarted=true;
                    gameStartTime=(float)glfwGetTime();
                }
                setupCamera();
                renderMaze();
                renderHUD();
                renderFeedbackOverlay();
                break;

            case QUIZ_SCREEN:
                setupCamera();
                renderMaze();
                renderHUD();
                renderQuizScreen();
                break;

            case GAME_OVER:
                renderGameOverScreen();
                break;

            case COMPLETED:
                renderCongratsScreen();
                break;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}