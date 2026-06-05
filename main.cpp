/**
 * @file main.cpp
 * @brief Entry point aplikasi 3D Maze Quiz Challenge.
 *
 * Tanggung jawab file ini:
 *  1. Inisialisasi GLFW dan GLUT
 *  2. Pembuatan window dan OpenGL context
 *  3. Setup OpenGL state awal (depth test, face culling, lighting)
 *  4. Inisialisasi data game (maze copy, shuffle soal, string HUD)
 *  5. Game loop utama: input -> update timer -> render -> swap buffer
 *
 * Semua logika gameplay, rendering, dan input ada di modul terpisah.
 * main.cpp hanya mengatur urutan pemanggilan dan state OpenGL global.
 */

#include <cstdio>
#include <cstdlib>
#include <ctime>
#if defined(_WIN32) && !defined(CALLBACK)
#define CALLBACK __stdcall
#endif
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include "globals.h"
#include "maze.h"
#include "quiz.h"
#include "player.h"
#include "renderer.h"
#include "hud.h"

// =============================================
// HELPER: Update timer string setiap frame
// =============================================
/**
 * @brief Menghitung currentGameTime dan memperbarui timeString.
 *
 * currentGameTime = (glfwGetTime() - gameStartTime) + timeOffset
 * di-clamp ke >= 0 karena reward -3 detik bisa membuat nilai negatif.
 *
 * Hanya berjalan kalau game sudah dimulai (player bergerak pertama kali)
 * dan belum selesai.
 */
static void updateTimer() {
    if (gameStarted && !gameCompleted) {
        float rawElapsed = (float)glfwGetTime() - gameStartTime;
        currentGameTime  = rawElapsed + timeOffset;
        if (currentGameTime < 0.0f) currentGameTime = 0.0f;
        sprintf(timeString, "Time: %.2f", currentGameTime);
    }
}

// =============================================
// MAIN
// =============================================
int main() {
    // Seed RNG untuk shuffle soal dan partikel congrats screen
    srand((unsigned int)time(NULL));

    // GLUT harus diinit sebelum glutBitmapCharacter bisa dipakai.
    // Kita tidak pakai glutMainLoop, tapi init tetap dibutuhkan
    // untuk font bitmap GLUT.
    int argc = 1;
    char* argv[1] = {(char*)"MazeQuiz"};
    glutInit(&argc, argv);

    // ---- Init GLFW ----
    if (!glfwInit()) {
        fprintf(stderr, "[ERROR] GLFW init gagal\n");
        return -1;
    }

    currentGameState = TITLE_SCREEN;

    // ---- Buat window ----
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "3D Maze Quiz Challenge", NULL, NULL);
    if (!window) {
        fprintf(stderr, "[ERROR] Gagal membuat window GLFW\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Callback resize: update viewport dan matriks proyeksi saat window di-resize
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // ---- OpenGL state awal ----
    glEnable(GL_DEPTH_TEST);  // Depth buffer agar objek dekat menutupi yang jauh
    glEnable(GL_CULL_FACE);   // Culling face belakang untuk performa
    glCullFace(GL_BACK);

    // ---- Inisialisasi data game ----
    // Simpan snapshot maze awal sebelum gameplay mengubahnya (board dihapus saat soal selesai)
    copyMazeArray(mazeCopy, maze);
    // Kocok urutan soal untuk sesi pertama
    shuffleQuestions();

    // ---- Setup pencahayaan OpenGL fixed-function ----
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Sumber cahaya: directional light dari arah (2, 3, 2)
    // w=0.0 berarti directional (bukan point light)
    GLfloat lpos[] = {2.0f, 3.0f, 2.0f, 0.0f};
    GLfloat lamb[] = {0.25f, 0.25f, 0.25f, 1.0f}; // Ambient: cahaya baur lemah
    GLfloat ldif[] = {1.0f,  1.0f,  1.0f,  1.0f}; // Diffuse: cahaya putih penuh
    GLfloat lspc[] = {1.0f,  1.0f,  1.0f,  1.0f}; // Specular: highlight putih
    glLightfv(GL_LIGHT0, GL_POSITION, lpos);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  lamb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  ldif);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lspc);

    // Material default (bisa di-override oleh tiap fungsi render per-objek)
    GLfloat mAmb[] = {0.7f, 0.7f, 0.7f, 1.0f};
    GLfloat mDif[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat mSpc[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat mShn[] = {80.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT,   mAmb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   mDif);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  mSpc);
    glMaterialfv(GL_FRONT, GL_SHININESS, mShn);

    // ---- Init HUD string ----
    gameStartTime = (float)glfwGetTime();
    timeOffset    = 0.0f;
    sprintf(timeString,  "Time: 0.00");
    sprintf(scoreString, "Kertas: 0");
    sprintf(livesString, "Lives: %d", playerLives);

    printf("[INFO] Game dimulai. Tekan V untuk toggle kamera.\n");

    // =============================================
    // GAME LOOP UTAMA
    // =============================================
    while (!glfwWindowShouldClose(window)) {

        // 1. Proses input keyboard sesuai state aktif
        processInput(window);

        // 2. Bersihkan color buffer dan depth buffer
        //    Warna langit biru (sky color) sebagai clear color agar
        //    atap maze yang terbuka terlihat seperti langit
        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 3. Render sesuai state
        switch (currentGameState) {

            case TITLE_SCREEN:
                renderTitleScreen();
                break;

            case RULES_SCREEN:
                renderRulesScreen();
                break;

            case PLAYING:
                updateTimer();
                setupCamera();
                renderGround();
                renderMaze();
                // KUNCI PERUBAHAN 1: Ditambahkan syarat || cameraMode == GTA_PERSPECTIVE
                if (cameraMode == THIRD_PERSON || cameraMode == GTA_PERSPECTIVE) drawPlayer();
                renderHUD();
                renderFeedbackOverlay();
                break;

            case QUIZ_SCREEN:
                updateTimer();
                setupCamera();
                renderGround();
                renderMaze();
                // KUNCI PERUBAHAN 2: Ditambahkan syarat || cameraMode == GTA_PERSPECTIVE juga di sini
                if (cameraMode == THIRD_PERSON || cameraMode == GTA_PERSPECTIVE) drawPlayer();
                renderHUD();
                renderQuizScreen(); // renderQuizScreen juga memanggil renderFeedbackOverlay di dalamnya
                break;

            case GAME_OVER:
                renderGameOverScreen();
                break;

            case COMPLETED:
                renderCongratsScreen();
                break;
        }

        // 4. Swap front/back buffer dan poll event GLFW
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
