/**
 * @file player.cpp
 * @brief Implementasi kamera first-person, pemrosesan input keyboard,
 *        dan logika pergerakan player di dalam maze.
 */

#include "player.h"
#include "globals.h"
#include "maze.h"
#include "quiz.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#if defined(_WIN32) && !defined(CALLBACK)
#define CALLBACK __stdcall
#endif
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

// =============================================
// HELPER: RESET GAME STATE
// =============================================
/**
 * @brief Mereset semua state game ke kondisi awal untuk new game / restart.
 *
 * Fungsi internal (tidak diekspor) yang dipanggil di beberapa
 * tempat dalam processInput: dari TITLE_SCREEN, GAME_OVER, dan COMPLETED.
 * Dipusatkan agar tidak ada yang terlewat saat reset.
 *
 * @param currentTime Timestamp saat ini dari glfwGetTime(), dipakai
 *                    sebagai gameStartTime baru.
 */
static void resetGame(float currentTime) {
    // Restore maze ke kondisi awal (board yang sudah hilang muncul lagi)
    copyMazeArray(maze, mazeCopy);

    // Reset posisi player ke spawn awal (tengah tile 1,1)
    playerX     = 1.5f;
    playerY     = 0.0f;
    playerZ     = 1.5f;
    playerAngle = 0.0f;

    // Reset status gameplay
    gameStarted   = false;
    gameCompleted = false;
    playerScore   = 0;
    playerLives   = MAX_LIVES;
    feedbackState = FB_NONE;
    quizJustExited = false;
    timeOffset    = 0.0f;
    gameStartTime = currentTime;

    // Reset string HUD
    sprintf(timeString,  "Time: 0.00");
    sprintf(scoreString, "Kertas: 0");
    sprintf(livesString, "Lives: %d", playerLives);

    // Kocok ulang urutan soal untuk sesi baru
    shuffleQuestions();

    currentGameState = PLAYING;
}

// =============================================
// DRAW PLAYER (Third-Person Model)
// =============================================
void drawPlayer() {
    glPushMatrix();

    // Tempatkan model di posisi player, rotasi sesuai arah hadap.
    // Konversi playerAngle (radian, sumbu Y) ke derajat untuk glRotatef.
    // Ditambah 180° karena model menghadap +Z saat angle=0, sedangkan
    // konvensi gerak kita adalah -Z = maju saat angle=0.
    glTranslatef(playerX, playerY, playerZ);
    glRotatef(-(playerAngle * 180.0f / 3.14159265f) + 180.0f, 0.0f, 1.0f, 0.0f);

    // Matikan specular agar model terlihat flat/cartoon
    GLfloat no_spec[] = {0, 0, 0, 1};
    GLfloat no_shin[] = {0};
    glMaterialfv(GL_FRONT, GL_SPECULAR,  no_spec);
    glMaterialfv(GL_FRONT, GL_SHININESS, no_shin);

    // --- Badan (putih keabuan) ---
    GLfloat bodyColor[] = {0.85f, 0.85f, 0.9f, 1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, bodyColor);
    glPushMatrix();
        glTranslatef(0.0f, 0.3f, 0.0f); // Pusat badan di y=0.3
        glScalef(0.4f, 0.6f, 0.3f);
        drawCube(1.0f);
    glPopMatrix();

    // --- Kepala (skin-tone) ---
    GLfloat headColor[] = {0.95f, 0.78f, 0.60f, 1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, headColor);
    glPushMatrix();
        glTranslatef(0.0f, 0.72f, 0.0f); // Di atas badan
        glScalef(0.28f, 0.28f, 0.28f);
        drawCube(1.0f);
    glPopMatrix();

    // --- Indikator arah hadap (garis merah ke depan) ---
    // Dirender tanpa lighting agar warnanya solid
    glDisable(GL_LIGHTING);
    glLineWidth(2.5f);
    glColor3f(1.0f, 0.1f, 0.1f);
    glBegin(GL_LINES);
        glVertex3f(0.0f, 0.3f, 0.0f);   // Pusat badan
        glVertex3f(0.0f, 0.3f, 0.35f);  // Ke depan model (+Z lokal)
    glEnd();
    glLineWidth(1.0f);
    glEnable(GL_LIGHTING);

    glPopMatrix();
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

    // Arah hadap player dalam koordinat dunia
    float lookX =  sinf(playerAngle);
    float lookZ = -cosf(playerAngle);

    if (cameraMode == FIRST_PERSON) {
        // -----------------------------------------------
        // FIRST PERSON: mata di kepala player
        // -----------------------------------------------
        gluLookAt(
            playerX,          playerY + 0.5f, playerZ,           // posisi mata
            playerX + lookX,  playerY + 0.5f, playerZ + lookZ,   // titik pandang
            0.0f, 1.0f, 0.0f                                      // vektor atas
        );

    } else {
        // -----------------------------------------------
        // THIRD PERSON: kamera di belakang-atas player
        //
        // eyeX/Z dihitung dengan membalik arah hadap player:
        //   belakang player = -lookX, +lookZ
        // Dikalikan tpDistance untuk jarak, lalu diberi offset Y tpHeight.
        // -----------------------------------------------
        float eyeX = playerX - lookX * 0.8f;
        float eyeZ = playerZ - lookZ * 0.8f;
        float eyeY = playerY + 0.7f;

        gluLookAt(
            eyeX,    eyeY,          eyeZ,           // posisi mata (di belakang-atas)
            playerX, playerY + 0.5f, playerZ,        // titik pandang (kepala player)
            0.0f, 1.0f, 0.0f                          // vektor atas
        );
    }
}

// =============================================
// PROCESS INPUT
// =============================================
void processInput(GLFWwindow* window) {
    float currentTime = (float)glfwGetTime();

    // Delta time: selisih waktu sejak frame terakhir.
    // Dipakai untuk membuat kecepatan gerak frame-rate independent.
    static float lastTime = 0.0f;
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    // -----------------------------------------------
    // TITLE SCREEN
    // -----------------------------------------------
    if (currentGameState == TITLE_SCREEN) {
        // P: Mulai game baru
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !keyPressedLastFrame[GLFW_KEY_P]) {
            resetGame(currentTime);
            keyPressedLastFrame[GLFW_KEY_P] = true;
        } else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {
            keyPressedLastFrame[GLFW_KEY_P] = false;
        }

        // Shift + /  (= ?) : Tampilkan layar aturan
        bool shift = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)  == GLFW_PRESS ||
                     glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
        if (glfwGetKey(window, GLFW_KEY_SLASH) == GLFW_PRESS && shift && !keyPressedLastFrame[GLFW_KEY_SLASH]) {
            currentGameState = RULES_SCREEN;
            keyPressedLastFrame[GLFW_KEY_SLASH] = true;
        } else if (glfwGetKey(window, GLFW_KEY_SLASH) == GLFW_RELEASE) {
            keyPressedLastFrame[GLFW_KEY_SLASH] = false;
        }

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);

        return; // Tidak ada input lain yang diproses di title screen
    }

    // -----------------------------------------------
    // RULES SCREEN
    // -----------------------------------------------
    if (currentGameState == RULES_SCREEN) {
        if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !keyPressedLastFrame[GLFW_KEY_B]) {
            currentGameState = TITLE_SCREEN;
            keyPressedLastFrame[GLFW_KEY_B] = true;
        } else if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE) {
            keyPressedLastFrame[GLFW_KEY_B] = false;
        }

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);

        return;
    }

    // -----------------------------------------------
    // GAME OVER
    // -----------------------------------------------
    if (currentGameState == GAME_OVER) {
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !keyPressedLastFrame[GLFW_KEY_R]) {
            resetGame(currentTime);
            keyPressedLastFrame[GLFW_KEY_R] = true;
        } else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) {
            keyPressedLastFrame[GLFW_KEY_R] = false;
        }

        if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !keyPressedLastFrame[GLFW_KEY_B]) {
            currentGameState = TITLE_SCREEN;
            keyPressedLastFrame[GLFW_KEY_B] = true;
        } else if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE) {
            keyPressedLastFrame[GLFW_KEY_B] = false;
        }

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);

        return;
    }

    // -----------------------------------------------
    // COMPLETED
    // -----------------------------------------------
    if (currentGameState == COMPLETED || gameCompleted) {
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !keyPressedLastFrame[GLFW_KEY_R]) {
            resetGame(currentTime);
            keyPressedLastFrame[GLFW_KEY_R] = true;
        } else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) {
            keyPressedLastFrame[GLFW_KEY_R] = false;
        }

        if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !keyPressedLastFrame[GLFW_KEY_B]) {
            currentGameState = TITLE_SCREEN;
            keyPressedLastFrame[GLFW_KEY_B] = true;
        } else if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE) {
            keyPressedLastFrame[GLFW_KEY_B] = false;
        }

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);

        return;
    }

    // -----------------------------------------------
    // QUIZ SCREEN — movement DIBLOKIR
    // -----------------------------------------------
    if (currentGameState == QUIZ_SCREEN) {

        // Jika feedback sedang ditampilkan, blok semua input
        // tapi state sudah PLAYING — return saja.
        if (feedbackState != FB_NONE) {
            return;
        }

        // Cek timer soal — jika habis, terapkan timeout
        float remaining = quizTimeLimit - (currentTime - quizStartTime);
        if (remaining <= 0.0f) {
            applyTimeout();
            quizJustExited    = true;
            quizCooldownTimer = currentTime;
            if (currentGameState != GAME_OVER) currentGameState = PLAYING;
            return;
        }

        Question& q = questions[activeQuestion];

        // Jawaban 1
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && !keyPressedLastFrame[GLFW_KEY_1]) {
            if (q.correct == 0) applyCorrectAnswer(); else applyWrongAnswer();
            quizJustExited = true; quizCooldownTimer = currentTime;
            if (currentGameState != GAME_OVER) currentGameState = PLAYING;
            keyPressedLastFrame[GLFW_KEY_1] = true;
        } else if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE) {
            keyPressedLastFrame[GLFW_KEY_1] = false;
        }

        // Jawaban 2
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && !keyPressedLastFrame[GLFW_KEY_2]) {
            if (q.correct == 1) applyCorrectAnswer(); else applyWrongAnswer();
            quizJustExited = true; quizCooldownTimer = currentTime;
            if (currentGameState != GAME_OVER) currentGameState = PLAYING;
            keyPressedLastFrame[GLFW_KEY_2] = true;
        } else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE) {
            keyPressedLastFrame[GLFW_KEY_2] = false;
        }

        // Jawaban 3
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && !keyPressedLastFrame[GLFW_KEY_3]) {
            if (q.correct == 2) applyCorrectAnswer(); else applyWrongAnswer();
            quizJustExited = true; quizCooldownTimer = currentTime;
            if (currentGameState != GAME_OVER) currentGameState = PLAYING;
            keyPressedLastFrame[GLFW_KEY_3] = true;
        } else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE) {
            keyPressedLastFrame[GLFW_KEY_3] = false;
        }

        // Jawaban 4
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && !keyPressedLastFrame[GLFW_KEY_4]) {
            if (q.correct == 3) applyCorrectAnswer(); else applyWrongAnswer();
            quizJustExited = true; quizCooldownTimer = currentTime;
            if (currentGameState != GAME_OVER) currentGameState = PLAYING;
            keyPressedLastFrame[GLFW_KEY_4] = true;
        } else if (glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE) {
            keyPressedLastFrame[GLFW_KEY_4] = false;
        }

        // Skip (E) — tidak ada feedback overlay, transisi langsung ke PLAYING
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && !keyPressedLastFrame[GLFW_KEY_E]) {
            applySkip();
            quizJustExited    = true;
            quizCooldownTimer = currentTime;
            activeQuestion    = getNextQuestion(); // siapkan soal berikutnya
            currentGameState  = PLAYING;
            keyPressedLastFrame[GLFW_KEY_E] = true;
        } else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE) {
            keyPressedLastFrame[GLFW_KEY_E] = false;
        }

        return; // Tidak ada movement selama di QUIZ_SCREEN
    }

    // -----------------------------------------------
    // PLAYING — pergerakan normal
    // -----------------------------------------------

    // Normalisasi kecepatan ke frame rate ~60fps agar speed konsisten
    float adjSpd    = playerSpeed    * deltaTime * 60.0f;
    float adjRotSpd = rotationSpeed  * deltaTime * 60.0f;

    // Bersihkan feedback overlay kalau sudah habis durasinya
    if (feedbackState != FB_NONE && currentTime - feedbackTimer >= FEEDBACK_DURATION)
        feedbackState = FB_NONE;

    // Reset flag cooldown soal kalau sudah lewat QUIZ_COOLDOWN
    if (quizJustExited && currentTime - quizCooldownTimer >= QUIZ_COOLDOWN)
        quizJustExited = false;

    // Timer mulai saat player pertama kali bergerak (bukan saat tekan P)
    if (!gameStarted && (
        glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)) {
        gameStarted   = true;
        gameStartTime = currentTime;
        timeOffset    = 0.0f;
    }

    // W: Maju ke arah hadap player
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        float nx = playerX + sinf(playerAngle) * adjSpd;
        float nz = playerZ - cosf(playerAngle) * adjSpd;
        // Wall sliding: coba gerak diagonal, lalu fallback ke sumbu tunggal
        if      (!checkCollision(nx, nz)) { playerX = nx; playerZ = nz; }
        else if (!checkCollision(nx, playerZ)) playerX = nx;
        else if (!checkCollision(playerX, nz)) playerZ = nz;
    }

    // S: Mundur
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        float nx = playerX - sinf(playerAngle) * adjSpd;
        float nz = playerZ + cosf(playerAngle) * adjSpd;
        if      (!checkCollision(nx, nz)) { playerX = nx; playerZ = nz; }
        else if (!checkCollision(nx, playerZ)) playerX = nx;
        else if (!checkCollision(playerX, nz)) playerZ = nz;
    }

    // A/D: Rotasi kamera (bukan strafe)
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) playerAngle -= adjRotSpd;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) playerAngle += adjRotSpd;

    // Deteksi tile yang sedang diinjak player
    int cellX = (int)playerX;
    int cellZ = (int)playerZ;

    if (cellX >= 0 && cellX < MAZE_WIDTH && cellZ >= 0 && cellZ < MAZE_HEIGHT) {

        // Tile 2 = GOAL: game selesai
        if (maze[cellZ][cellX] == 2 && !gameCompleted) {
            gameCompleted    = true;
            currentGameState = COMPLETED;
            float rawTime = currentTime - gameStartTime;
            float finalT  = rawTime + timeOffset;
            sprintf(congratsMessage, "Selesai dalam %.2f detik (raw)", rawTime);
            sprintf(finalscore,      "Kertas Dikumpulkan: %d", playerScore);
            sprintf(finaltime,       "Waktu Final: %.2f detik", finalT);
        }

        // Tile 3 = BOARD: trigger quiz jika cukup dekat dan cooldown habis
        if (maze[cellZ][cellX] == 3 &&
            isNearBoard(playerX, playerZ, cellX, cellZ) &&
            !quizJustExited) {
            paperCellX       = cellX;
            paperCellZ       = cellZ;
            activeQuestion   = getNextQuestion();
            quizStartTime    = currentTime;
            currentGameState = QUIZ_SCREEN;
            feedbackState    = FB_NONE;
        }
    }

    // V: Toggle antara First-Person dan Third-Person
    // Hanya bisa di-toggle saat PLAYING, bukan saat quiz/menu.
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS && !keyPressedLastFrame[GLFW_KEY_V]) {
        cameraMode = (cameraMode == FIRST_PERSON) ? THIRD_PERSON : FIRST_PERSON;
        printf("[CAM] Mode: %s\n", cameraMode == FIRST_PERSON ? "First-Person" : "Third-Person");
        keyPressedLastFrame[GLFW_KEY_V] = true;
    } else if (glfwGetKey(window, GLFW_KEY_V) == GLFW_RELEASE) {
        keyPressedLastFrame[GLFW_KEY_V] = false;
    }

    // ESC: Kembali ke title screen
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && !keyPressedLastFrame[GLFW_KEY_ESCAPE]) {
        currentGameState = TITLE_SCREEN;
        keyPressedLastFrame[GLFW_KEY_ESCAPE] = true;
    } else if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE) {
        keyPressedLastFrame[GLFW_KEY_ESCAPE] = false;
    }
}

// =============================================
// FRAMEBUFFER SIZE CALLBACK
// =============================================
void framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height) {
    // Update viewport agar sesuai ukuran window baru
    glViewport(0, 0, width, height);

    // Update proyeksi agar aspek rasio tidak terdistorsi
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat)width / (GLfloat)height, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}