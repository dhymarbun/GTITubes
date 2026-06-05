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
// DRAW PLAYER (GTA Perspective Model)
// =============================================
// =============================================
// DRAW PLAYER (Minecraft Steve Style with Feet)
// =============================================
// =============================================
// DRAW PLAYER (Minecraft Steve Style with Feet)
// =============================================
void drawPlayer() {
    // Kunci: Jika kamera lagi mode First Person, jangan gambar orangnya biar gak bug di mata
    if (cameraMode == FIRST_PERSON) {
        return; 
    }

    glPushMatrix();

    // Tempatkan model di posisi player, rotasi sesuai arah hadap.
    glTranslatef(playerX, playerY, playerZ);
    glRotatef(-(playerAngle * 180.0f / 3.14159265f) + 180.0f, 0.0f, 1.0f, 0.0f);

    // ========================================================
    // LOGIKA AYUNAN LENGAN & KAKI (SAAT BERGERAK MAJU/MUNDUR)
    // ========================================================
    float swingAngle = 0.0f;
    
    GLFWwindow* currentWin = glfwGetCurrentContext();
    if (currentWin && (glfwGetKey(currentWin, GLFW_KEY_W) == GLFW_PRESS || 
                       glfwGetKey(currentWin, GLFW_KEY_S) == GLFW_PRESS)) {
        swingAngle = sinf((float)glfwGetTime() * 12.0f) * 35.0f;
    }

    // ========================================================
    // DEFINISI WARNA (Teal, Krem Salem, Rambut Hitam, Celana Ungu, Sepatu)
    // ========================================================
    GLfloat bodyColor[]   = {0.0f, 0.5f, 0.5f, 1.0f};   // Baju Teal
    GLfloat headColor[]   = {1.0f, 0.8f, 0.67f, 1.0f};  // Kulit Krem Salem
    GLfloat hairColor[]   = {0.0f, 0.0f, 0.0f, 1.0f};   // Rambut Hitam
    GLfloat legColor[]    = {0.5f, 0.0f, 0.5f, 1.0f};   // Celana Ungu
    GLfloat shoeColor[]   = {0.3f, 0.2f, 0.1f, 1.0f};   // Sepatu Cokelat Tua

    glEnable(GL_LIGHTING);
    GLfloat no_mat[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glMaterialfv(GL_FRONT, GL_SPECULAR, no_mat);
    glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);

    // --------------------------------------------------------
    // 1. BALOK BADAN / TORSO (Diangkat ke atas memberi ruang kaki)
    // --------------------------------------------------------
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, bodyColor);
    glPushMatrix();
        glTranslatef(0.0f, 0.45f, 0.0f); 
        glScalef(0.4f, 0.45f, 0.25f);    
        drawCube(1.0f);
    glPopMatrix();

    // --------------------------------------------------------
    // 2. KEPALA UTAMA + RAMBUT HITAM (1/3 ATAS)
    // --------------------------------------------------------
    // A. Kulit Muka (2/3 bawah kepala)
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, headColor);
    glPushMatrix();
        glTranslatef(0.0f, 0.73f, 0.0f); 
        glScalef(0.26f, 0.2f, 0.26f); 
        drawCube(1.0f);
    glPopMatrix();

    // B. Rambut Hitam (1/3 atas kubus kepala)
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, hairColor);
    glPushMatrix();
        glTranslatef(0.0f, 0.845f, 0.0f); 
        glScalef(0.26f, 0.08f, 0.26f);   
        drawCube(1.0f);
    glPopMatrix();

    // --------------------------------------------------------
    // 3. LENGAN KANAN & LIRI (Ikut naik menyesuaikan torso baju)
    // --------------------------------------------------------
    // Lengan Kanan
    glPushMatrix();
        glTranslatef(-0.24f, 0.6f, 0.0f);       
        glRotatef(swingAngle, 1.0f, 0.0f, 0.0f); 
        glPushMatrix();
            glTranslatef(0.0f, -0.15f, 0.0f);  
            glScalef(0.08f, 0.3f, 0.08f);       
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, bodyColor); 
            drawCube(1.0f);
        glPopMatrix();
    glPopMatrix();

    // Lengan Kiri
    glPushMatrix();
        glTranslatef(0.24f, 0.6f, 0.0f);         
        glRotatef(-swingAngle, 1.0f, 0.0f, 0.0f); 
        glPushMatrix();
            glTranslatef(0.0f, -0.15f, 0.0f);
            glScalef(0.08f, 0.3f, 0.08f);
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, bodyColor);
            drawCube(1.0f);
        glPopMatrix();
    glPopMatrix();

    // --------------------------------------------------------
    // 4. SEPASANG KAKI UNGU + SEPATU (Menyentuh Lantai & Melangkah)
    // --------------------------------------------------------
    // --- KAKI KANAN ---
    glPushMatrix();
        glTranslatef(-0.1f, 0.25f, 0.0f); 
        glRotatef(-swingAngle, 1.0f, 0.0f, 0.0f); 
        
        // Celana Ungu Kanan
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, legColor);
        glPushMatrix();
            glTranslatef(0.0f, -0.08f, 0.0f);
            glScalef(0.14f, 0.2f, 0.16f);
            drawCube(1.0f);
        glPopMatrix();
        
        // Sepatu Kanan
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, shoeColor);
        glPushMatrix();
            glTranslatef(0.0f, -0.21f, 0.02f); 
            glScalef(0.14f, 0.06f, 0.2f);
            drawCube(1.0f);
        glPopMatrix();
    glPopMatrix();

    // --- KAKI KIRI ---
    glPushMatrix();
        glTranslatef(0.1f, 0.25f, 0.0f); 
        glRotatef(swingAngle, 1.0f, 0.0f, 0.0f); 
        
        // Celana Ungu Kiri
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, legColor);
        glPushMatrix();
            glTranslatef(0.0f, -0.08f, 0.0f);
            glScalef(0.14f, 0.2f, 0.16f);
            drawCube(1.0f);
        glPopMatrix();
        
        // Sepatu Kiri
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, shoeColor);
        glPushMatrix();
            glTranslatef(0.0f, -0.21f, 0.02f);
            glScalef(0.14f, 0.06f, 0.2f);
            drawCube(1.0f);
        glPopMatrix();
    glPopMatrix();

    // Indikator arah hadap merah (Disembunyikan tipis di bawah lantai kaki)
    glDisable(GL_LIGHTING);
    glLineWidth(2.5f);
    glColor3f(1.0f, 0.1f, 0.1f);
    glBegin(GL_LINES);
        glVertex3f(0.0f, 0.02f, 0.0f);
        glVertex3f(0.0f, 0.02f, 0.2f);
    glEnd();
    glLineWidth(1.0f);
    glEnable(GL_LIGHTING);

    glPopMatrix(); // 
}
// =============================================
// SETUP CAMERA
// =============================================
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
        // FIRST PERSON: mata di kepala player (Bawaan Asli)
        // -----------------------------------------------
        gluLookAt(
            playerX,          playerY + 0.5f, playerZ,           // posisi mata
            playerX + lookX,  playerY + 0.5f, playerZ + lookZ,   // titik pandang
            0.0f, 1.0f, 0.0f                                      // vektor atas
        );

    } else if (cameraMode == GTA_PERSPECTIVE) {
        // -----------------------------------------------
        // GTA PERSPECTIVE: Sudut Pandang Franklin GTA V (Mundur & Naik Pas)
        // -----------------------------------------------
        // KUNCI COBA 1: Jarak dimundurin ke 1.4f (Biar karakter kelihatan utuh dari kepala sampai kaki)
        float targetDistance = 1.4f; 
        
        // KUNCI COBA 2: Tinggi kamera dinaikkan tipis ke +0.9f (Melewati pundak tapi tidak membocorkan rute labirin)
        float eyeX = playerX - lookX * targetDistance;
        float eyeZ = playerZ - lookZ * targetDistance;
        float eyeY = playerY + 0.9f; 

        // Loop pengecekan tabrakan kamera dengan dinding (Menggunakan sistem anti-nembus andalan lo)
        for (int i = 0; i < 10; i++) {
            if (checkCollision(eyeX, eyeZ)) {
                targetDistance -= 0.07f; // Otomatis maju ngedeket kalau bagian belakang kamera mentok tembok
                eyeX = playerX - lookX * targetDistance;
                eyeZ = playerZ - lookZ * targetDistance;
            } else {
                break;
            }
        }

        if (targetDistance < 0.2f) {
            targetDistance = 0.2f;
            eyeX = playerX - lookX * targetDistance;
            eyeZ = playerZ - lookZ * targetDistance;
        }

        gluLookAt(
            eyeX,    eyeY,          eyeZ,           // Posisi kamera ala GTA V
            playerX, playerY + 0.43f, playerZ,       // KUNCI COBA 3: Pandangan mengunci sedikit di bawah kepala (torso atas) agar komposisi objek pas di tengah layar
            0.0f, 1.0f, 0.0f                        
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

    // V: Toggle antara First-Person dan GTA Perspective
    // Hanya bisa di-toggle saat PLAYING, bukan saat quiz/menu.
   if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS && !keyPressedLastFrame[GLFW_KEY_V]) {
        if (cameraMode == FIRST_PERSON) {
            cameraMode = GTA_PERSPECTIVE;
            printf("[CAM] Mode: GTA Style (Atas Kepala)\n");
        } else {
            cameraMode = FIRST_PERSON;
            printf("[CAM] Mode: First-Person\n");
        }
        keyPressedLastFrame[GLFW_KEY_V] = true;
    } else if (glfwGetKey(window, GLFW_KEY_V) == GLFW_RELEASE) {
        keyPressedLastFrame[GLFW_KEY_V] = false;
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
