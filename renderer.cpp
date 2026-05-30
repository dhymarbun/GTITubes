/**
 * @file renderer.cpp
 * @brief Implementasi fungsi render layar non-gameplay:
 *        title, rules, game over, dan congrats screen.
 *
 * Semua fungsi di sini bekerja dalam koordinat 2D orthographic
 * (origin kiri-bawah, x ke kanan, y ke atas) dengan rentang
 * [0..WIDTH] x [0..HEIGHT]. Setiap fungsi push/pop matrix sendiri
 * sehingga tidak mengotori state matriks game loop.
 */

#include "renderer.h"
#include "globals.h"
#include <cstdio>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/glut.h>

// =============================================
// HELPER MAKRO: Render string bitmap di posisi (x, y)
// =============================================
// Menggunakan glutBitmapCharacter yang render per-karakter.
// Tidak ada built-in word wrap; semua teks diasumsikan satu baris.
#define RENDER_STR(font, x, y, str) do { \
    glRasterPos2i((x), (y)); \
    for (int _i = 0; (str)[_i]; _i++) \
        glutBitmapCharacter((font), (str)[_i]); \
} while(0)

// =============================================
// HELPER: Masuk mode 2D orthographic
// Simpan matriks sebelumnya, nonaktifkan lighting & depth test.
// =============================================
static void enter2D() {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    glOrtho(0, WIDTH, 0, HEIGHT, -1, 1); // Y: 0 = bawah, HEIGHT = atas
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
}

// =============================================
// HELPER: Keluar mode 2D, pulihkan state
// =============================================
static void exit2D() {
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);  glPopMatrix();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

// =============================================
// RENDER TITLE SCREEN
// =============================================
void renderTitleScreen() {
    enter2D();

    // Latar belakang gradien biru gelap (bawah lebih gelap, atas lebih terang)
    glBegin(GL_QUADS);
        glColor3f(0.05f, 0.05f, 0.2f);  glVertex2f(0, 0);
        glColor3f(0.05f, 0.05f, 0.2f);  glVertex2f(WIDTH, 0);
        glColor3f(0.2f,  0.2f,  0.5f);  glVertex2f(WIDTH, HEIGHT);
        glColor3f(0.2f,  0.2f,  0.5f);  glVertex2f(0, HEIGHT);
    glEnd();

    int cx = WIDTH / 2, cy = HEIGHT / 2;

    // Judul game
    glColor3f(1.0f, 0.8f, 0.0f);
    char t1[] = "MAZE3D - QUIZ CHALLENGE";
    RENDER_STR(GLUT_BITMAP_TIMES_ROMAN_24, cx - (int)(strlen(t1) * 7), cy + 90, t1);

    // Subtitle
    glColor3f(0.7f, 0.9f, 1.0f);
    char t2[] = "10 Soal | 3 Nyawa | Easy -> Hard";
    RENDER_STR(GLUT_BITMAP_HELVETICA_18, cx - (int)(strlen(t2) * 5), cy + 55, t2);

    // Daftar kontrol
    glColor3f(1, 1, 1);
    const char* items[] = {"Press P to Play", "Press ? for Rules", "Press ESC to Quit"};
    for (int i = 0; i < 3; i++)
        RENDER_STR(GLUT_BITMAP_HELVETICA_18, cx - (int)(strlen(items[i]) * 4), cy - 10 - i * 30, items[i]);

    // Keterangan camera toggle
    glColor3f(0.6f, 0.9f, 0.6f);
    char camTip[] = "[V] Toggle First / Third Person (saat bermain)";
    RENDER_STR(GLUT_BITMAP_HELVETICA_12, cx - (int)(strlen(camTip) * 3), cy - 105, camTip);

    // Bingkai beranimasi pulse (sin wave pada brightness)
    float pulse = (sinf((float)glfwGetTime() * 2.0f) + 1.0f) * 0.5f;
    glLineWidth(3.0f);
    glColor3f(0.5f + pulse * 0.5f, 0.4f, 0.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(cx - 250, cy - 115);
        glVertex2f(cx + 250, cy - 115);
        glVertex2f(cx + 250, cy + 120);
        glVertex2f(cx - 250, cy + 120);
    glEnd();
    glLineWidth(1.0f);

    // Credits di bawah layar
    glColor3f(0.65f, 0.65f, 0.65f);
    const char* credits[] = {
        "Adhyaksa M. Banjar Nahor - 24060124140152",
        "Raihan Lazuardi           - 24060124140178",
        "Ganendra Satya Sindhunata - 24060124120025"
    };
    for (int i = 0; i < 3; i++) {
        int tw = (int)(strlen(credits[i]) * 6);
        RENDER_STR(GLUT_BITMAP_HELVETICA_12, cx - tw / 2, 14 + i * 16, credits[i]);
    }

    exit2D();
}

// =============================================
// RENDER RULES SCREEN
// =============================================
void renderRulesScreen() {
    enter2D();

    // Latar belakang gelap solid
    glColor3f(0.1f, 0.1f, 0.3f);
    glBegin(GL_QUADS);
        glVertex2f(0, 0); glVertex2f(WIDTH, 0);
        glVertex2f(WIDTH, HEIGHT); glVertex2f(0, HEIGHT);
    glEnd();

    // Judul
    glColor3f(1.0f, 0.8f, 0.0f);
    char title[] = "HOW TO PLAY";
    RENDER_STR(GLUT_BITMAP_TIMES_ROMAN_24, WIDTH / 2 - (int)(strlen(title) * 6), HEIGHT - 45, title);

    // Daftar aturan — indeks 7 & 12 diberi warna khusus untuk penekanan
    glColor3f(1, 1, 1);
    const char* lines[] = {
        "GERAKAN: W=Maju  S=Mundur  A=Putar Kiri  D=Putar Kanan",
        "[V] = Toggle First-Person / Third-Person View",
        "",
        "QUIZ BOARD (papan kayu berputar):",
        "  Dekati board => soal muncul, gerak TERKUNCI",
        "  1/2/3/4 = jawab pilihan",
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
    int lineCount = 16;
    for (int i = 0; i < lineCount; i++) {
        if      (i == 8)  glColor3f(1.0f, 0.5f, 0.5f); // Sistem nyawa — merah
        else if (i == 13) glColor3f(0.5f, 1.0f, 0.5f); // Tujuan — hijau
        else              glColor3f(1, 1, 1);
        RENDER_STR(GLUT_BITMAP_HELVETICA_18, 60, HEIGHT - 90 - i * 28, lines[i]);
    }

    exit2D();
}

// =============================================
// RENDER GAME OVER SCREEN
// =============================================
void renderGameOverScreen() {
    enter2D();

    // Latar merah beranimasi pulse — intensity ikut gelombang sin
    float pulse = (sinf((float)glfwGetTime() * 2.0f) + 1.0f) * 0.5f;
    glBegin(GL_QUADS);
        glColor3f(0.25f + pulse * 0.1f, 0.0f, 0.0f); glVertex2f(0, 0);
        glColor3f(0.25f + pulse * 0.1f, 0.0f, 0.0f); glVertex2f(WIDTH, 0);
        glColor3f(0.1f,  0.0f, 0.0f);                glVertex2f(WIDTH, HEIGHT);
        glColor3f(0.1f,  0.0f, 0.0f);                glVertex2f(0, HEIGHT);
    glEnd();

    // Bingkai merah tebal
    glLineWidth(5.0f);
    glColor3f(0.8f, 0.0f, 0.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(50, 150); glVertex2f(750, 150);
        glVertex2f(750, 450); glVertex2f(50, 450);
    glEnd();
    glLineWidth(1.0f);

    int cx = WIDTH / 2, cy = HEIGHT / 2;

    // Teks "GAME OVER"
    glColor3f(1.0f, 0.1f, 0.1f);
    char goText[] = "GAME OVER";
    RENDER_STR(GLUT_BITMAP_TIMES_ROMAN_24, cx - (int)(strlen(goText) * 11), cy + 100, goText);

    // Statistik akhir
    glColor3f(1.0f, 1.0f, 1.0f);
    char t1[64]; sprintf(t1, "Waktu: %.2f detik", currentGameTime);
    RENDER_STR(GLUT_BITMAP_HELVETICA_18, cx - (int)(strlen(t1) * 5), cy + 40, t1);

    char t2[64]; sprintf(t2, "Kertas Dikumpulkan: %d", playerScore);
    RENDER_STR(GLUT_BITMAP_HELVETICA_18, cx - (int)(strlen(t2) * 5), cy + 15, t2);

    // Hint kontrol
    glColor3f(0.8f, 0.8f, 0.8f);
    char hint[] = "Press R to Restart  |  Press B for Title";
    RENDER_STR(GLUT_BITMAP_HELVETICA_18, cx - (int)(strlen(hint) * 5), cy - 40, hint);

    exit2D();
}

// =============================================
// RENDER CONGRATS SCREEN
// =============================================
void renderCongratsScreen() {
    enter2D();

    // Latar biru dengan alpha blend
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
        glColor4f(0.0f, 0.0f, 0.3f, 0.9f); glVertex2f(0, 0);
        glColor4f(0.0f, 0.0f, 0.3f, 0.9f); glVertex2f(WIDTH, 0);
        glColor4f(0.1f, 0.2f, 0.5f, 0.9f); glVertex2f(WIDTH, HEIGHT);
        glColor4f(0.1f, 0.2f, 0.5f, 0.9f); glVertex2f(0, HEIGHT);
    glEnd();

    // Bingkai emas
    glLineWidth(5.0f);
    glColor3f(1.0f, 0.8f, 0.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(WIDTH * 0.1f, HEIGHT * 0.2f);
        glVertex2f(WIDTH * 0.9f, HEIGHT * 0.2f);
        glVertex2f(WIDTH * 0.9f, HEIGHT * 0.8f);
        glVertex2f(WIDTH * 0.1f, HEIGHT * 0.8f);
    glEnd();
    glLineWidth(1.0f);

    int cx = WIDTH / 2, cy = HEIGHT / 2;

    // Judul selamat
    glColor3f(1.0f, 1.0f, 0.0f);
    char t[] = "MAZE COMPLETED!";
    RENDER_STR(GLUT_BITMAP_TIMES_ROMAN_24, cx - (int)(strlen(t) * 10), cy + 100, t);

    // Tiga baris statistik: raw time, skor, waktu final
    glColor3f(1, 1, 1);
    const char* lines[] = {congratsMessage, finalscore, finaltime};
    for (int i = 0; i < 3; i++)
        RENDER_STR(GLUT_BITMAP_HELVETICA_18, cx - (int)(strlen(lines[i]) * 5), cy + 40 - i * 28, lines[i]);

    // Hint kontrol
    glColor3f(0.7f, 0.7f, 0.7f);
    char hint[] = "Press R to Play Again  |  B for Title";
    RENDER_STR(GLUT_BITMAP_HELVETICA_18, cx - (int)(strlen(hint) * 5), cy - 80, hint);

    // ---- Partikel bintang beranimasi ----
    // 20 titik bergerak masing-masing dengan lintasan sinusoidal unik
    // berdasarkan index i sebagai phase offset.
    // Posisi disimpan di static array agar konsisten antar frame.
    static float sp[20][2];
    static bool sinit = false;
    if (!sinit) {
        for (int i = 0; i < 20; i++) {
            sp[i][0] = (float)(rand() % WIDTH);
            sp[i][1] = (float)(rand() % HEIGHT);
        }
        sinit = true;
    }

    glPointSize(6.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < 20; i++) {
        // Warna berosilasi antara merah dan kuning menggunakan sin
        float osc = (sinf((float)glfwGetTime() * 3.0f + i) + 1.0f) * 0.5f;
        glColor3f(1.0f, osc, osc);
        glVertex2f(sp[i][0], sp[i][1]);

        // Gerakkan partikel per frame, wrap di tepi layar
        sp[i][0] += sinf((float)glfwGetTime() * 2.0f + i) * 2.0f;
        sp[i][1] += cosf((float)glfwGetTime() * 2.0f + i) * 2.0f;
        if (sp[i][0] < 0)      sp[i][0] = (float)WIDTH;
        if (sp[i][0] > WIDTH)  sp[i][0] = 0;
        if (sp[i][1] < 0)      sp[i][1] = (float)HEIGHT;
        if (sp[i][1] > HEIGHT) sp[i][1] = 0;
    }
    glEnd();
    glPointSize(1.0f);
    glDisable(GL_BLEND);

    exit2D();
}