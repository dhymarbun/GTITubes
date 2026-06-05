/**
 * @file hud.cpp
 * @brief Implementasi HUD, overlay feedback, dan layar soal kuis.
 */

#include "hud.h"
#include "globals.h"
#include "quiz.h"
#include <cstdio>
#include <cstring>
#include <cmath>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/glut.h>

// =============================================
// RENDER HUD
// =============================================
// =============================================
// RENDER HUD
// =============================================
void renderHUD() {
    glDisable(GL_LIGHTING);

    // Koordinat 2D dengan Y=0 di ATAS layar (beda dari renderer.cpp yang Y=0 di bawah).
    // Ini lebih intuitif untuk HUD karena kita taruh elemen dari ujung atas.
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    glOrtho(0, WIDTH, HEIGHT, 0, -1, 1); // Y dibalik: 0=atas, HEIGHT=bawah
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();

    // Strip transparan hitam sebagai background HUD (atas layar)
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
        glColor4f(0, 0, 0, 0.5f);
        glVertex2f(0, 0); glVertex2f(WIDTH, 0);
        glVertex2f(WIDTH, 55); glVertex2f(0, 55);
    glEnd();
    glDisable(GL_BLEND);

    // Timer (putih, kiri atas baris 1)
    glColor3f(1, 1, 1);
    glRasterPos2i(10, 18);
    for (int i = 0; timeString[i]; i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, timeString[i]);

    // Skor (hijau, kiri atas baris 2)
    glColor3f(0.4f, 1.0f, 0.4f);
    glRasterPos2i(10, 38);
    for (int i = 0; scoreString[i]; i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, scoreString[i]);

    // Label mode kamera (tengah atas) — dikembalikan ke tengah bawaan asli kelompok lo
    const char* camLabel = (cameraMode == FIRST_PERSON) ? "[FP]" : (cameraMode == THIRD_PERSON ? "[TP]" : "[GTA]");
    glColor3f(0.7f, 0.9f, 1.0f);
    glRasterPos2i(WIDTH / 2 - 14, 20);
    for (int i = 0; camLabel[i]; i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, camLabel[i]);

    // --------------------------------------------------------
    // KUNCI PERUBAHAN: TEKS PETUNJUK TOMBOL V (POJOK KANAN BAWAH HUD)
    // --------------------------------------------------------
    const char* hintText = "Press [V] to change camera mode";
    glColor3f(0.9f, 0.9f, 0.5f); // Warna kuning pastel lembut biar kelihatan sebagai petunjuk
    glRasterPos2i(WIDTH - 250, 42); // Diposisikan di bawah kotak nyawa, pas di dalam bar hitam HUD
    for (int i = 0; hintText[i]; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, hintText[i]);
    }

    // Indikator nyawa [v] = isi, [ ] = kosong (kanan atas)
    glColor3f(1.0f, 0.3f, 0.3f);
    char heartFull[]  = "[v]";
    char heartEmpty[] = "[ ]";
    int lx = WIDTH - 160;
    for (int i = 0; i < MAX_LIVES; i++) {
        glRasterPos2i(lx + i * 50, 24); // Diangkat naik dikit ke Y=24 biar gak tabrakan sama teks petunjuk
        const char* h = (i < playerLives) ? heartFull : heartEmpty;
        for (int j = 0; h[j]; j++) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, h[j]);
    }

    glMatrixMode(GL_MODELVIEW);  glPopMatrix();
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_LIGHTING);
}
// =============================================
// RENDER FEEDBACK OVERLAY
// =============================================
void renderFeedbackOverlay() {
    if (feedbackState == FB_NONE) return;

    float elapsed = (float)glfwGetTime() - feedbackTimer;
    if (elapsed >= FEEDBACK_DURATION) {
        feedbackState = FB_NONE;
        return;
    }

    // Alpha fade-out linear: mulai dari 0.45 lalu menuju 0 seiring waktu
    float alpha = 0.45f * (1.0f - elapsed / FEEDBACK_DURATION);

    glDisable(GL_LIGHTING); glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    glOrtho(0, WIDTH, 0, HEIGHT, -1, 1); // Y=0 bawah untuk overlay ini
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();

    // Quad penuh layar — hijau untuk benar, merah untuk salah
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
        if (feedbackState == FB_CORRECT) glColor4f(0.0f, 1.0f, 0.2f, alpha);
        else                              glColor4f(1.0f, 0.1f, 0.1f, alpha);
        glVertex2f(0, 0); glVertex2f(WIDTH, 0);
        glVertex2f(WIDTH, HEIGHT); glVertex2f(0, HEIGHT);
    glEnd();
    glDisable(GL_BLEND);

    // Teks utama dan sub-teks feedback
    const char* msg    = (feedbackState == FB_CORRECT) ? "BENAR! +1 Skor" : "SALAH! -1 Nyawa";
    const char* submsg = (feedbackState == FB_CORRECT) ? "Bonus -3 detik!" : "Penalti +5 detik";

    int len    = (int)strlen(msg);
    int sublen = (int)strlen(submsg);

    // Shadow hitam (geser +2px) untuk keterbacaan di atas warna overlay
    glColor3f(0.0f, 0.0f, 0.0f);
    glRasterPos2i(WIDTH / 2 - len * 11 + 2, HEIGHT / 2 + 12);
    for (int i = 0; msg[i]; i++) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, msg[i]);

    // Teks utama putih
    glColor3f(1, 1, 1);
    glRasterPos2i(WIDTH / 2 - len * 11, HEIGHT / 2 + 14);
    for (int i = 0; msg[i]; i++) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, msg[i]);

    // Sub-teks (bonus/penalti)
    glColor3f(0.9f, 0.9f, 0.9f);
    glRasterPos2i(WIDTH / 2 - sublen * 5, HEIGHT / 2 - 12);
    for (int i = 0; submsg[i]; i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, submsg[i]);

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
    glOrtho(0, WIDTH, 0, HEIGHT, -1, 1);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();

    // Panel background semi-transparan di tengah layar
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
        glColor4f(0.03f, 0.03f, 0.15f, 0.96f);
        glVertex2f(50, 50); glVertex2f(750, 50);
        glVertex2f(750, 550); glVertex2f(50, 550);
    glEnd();
    glDisable(GL_BLEND);

    // Bingkai panel berwarna sesuai difficulty aktif
    Question& q = questions[activeQuestion];
    if      (q.difficulty == 0) glColor3f(0.3f, 1.0f, 0.3f);  // Mudah  = hijau
    else if (q.difficulty == 1) glColor3f(1.0f, 0.7f, 0.1f);  // Sedang = kuning
    else                        glColor3f(1.0f, 0.2f, 0.2f);  // Sulit  = merah
    glLineWidth(4.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(50, 50); glVertex2f(750, 50);
        glVertex2f(750, 550); glVertex2f(50, 550);
    glEnd();
    glLineWidth(1.0f);

    // Label difficulty di atas-kiri panel
    const char* diffLabel[] = {"[ MUDAH ]", "[ SEDANG ]", "[ SULIT ]"};
    GLfloat diffClr[3][3] = {
        {0.3f, 1.0f, 0.3f},
        {1.0f, 0.7f, 0.1f},
        {1.0f, 0.3f, 0.3f}
    };
    glColor3fv(diffClr[q.difficulty]);
    glRasterPos2i(WIDTH / 2 - (int)(strlen(diffLabel[q.difficulty]) * 6), 525);
    for (int i = 0; diffLabel[q.difficulty][i]; i++)
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, diffLabel[q.difficulty][i]);

    // Nomor soal di kanan atas panel
    char numText[32];
    sprintf(numText, "Soal %d / %d", currentQuestionIndex + 1, TOTAL_QUESTIONS);
    glColor3f(0.7f, 0.7f, 0.7f);
    glRasterPos2i(590, 525);
    for (int i = 0; numText[i]; i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, numText[i]);

    // Teks pertanyaan (putih terang)
    glColor3f(1.0f, 1.0f, 0.9f);
    glRasterPos2i(80, 475);
    for (int i = 0; q.question[i]; i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, q.question[i]);

    // ---- 4 opsi jawaban ----
    // Tiap opsi punya warna sendiri: A=hijau, B=biru, C=kuning, D=merah
    GLfloat optClr[4][3] = {
        {0.3f, 1.0f, 0.4f}, // A - hijau
        {0.3f, 0.6f, 1.0f}, // B - biru
        {1.0f, 0.85f,0.2f}, // C - kuning
        {1.0f, 0.4f, 0.5f}  // D - merah muda
    };
    const char* keys[] = {"[1]", "[2]", "[3]", "[4]"};

    for (int i = 0; i < 4; i++) {
        int oy = 395 - i * 68; // Y tiap opsi, dari bawah ke atas

        // Background opsi (gelap, semi-transparan)
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBegin(GL_QUADS);
            glColor4f(optClr[i][0] * 0.25f, optClr[i][1] * 0.25f, optClr[i][2] * 0.25f, 0.75f);
            glVertex2f(75, oy - 7); glVertex2f(725, oy - 7);
            glVertex2f(725, oy + 30); glVertex2f(75, oy + 30);
        glEnd();

        // Aksen bar kiri (penuh warna opsi)
        glBegin(GL_QUADS);
            glColor4f(optClr[i][0], optClr[i][1], optClr[i][2], 0.9f);
            glVertex2f(75, oy - 7); glVertex2f(82, oy - 7);
            glVertex2f(82, oy + 30); glVertex2f(75, oy + 30);
        glEnd();
        glDisable(GL_BLEND);

        // Kunci jawaban (warna opsi)
        glColor3fv(optClr[i]);
        glRasterPos2i(90, oy);
        for (int j = 0; keys[i][j]; j++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, keys[i][j]);

        // Teks opsi (putih)
        glColor3f(1.0f, 1.0f, 1.0f);
        glRasterPos2i(132, oy);
        for (int j = 0; q.options[i][j]; j++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, q.options[i][j]);
    }

    // ---- Timer countdown ----
    float remaining = quizTimeLimit - ((float)glfwGetTime() - quizStartTime);
    if (remaining < 0) remaining = 0;

    // Warna teks timer berubah sesuai urgensi
    char timerTxt[32]; sprintf(timerTxt, "%.1f detik", remaining);
    if      (remaining < 5.0f) glColor3f(1.0f, 0.2f, 0.2f);  // Kritis: merah
    else if (remaining < 8.0f) glColor3f(1.0f, 0.7f, 0.1f);  // Warning: kuning
    else                        glColor3f(0.3f, 1.0f, 0.5f);  // Aman: hijau
    glRasterPos2i(570, 90);
    for (int i = 0; timerTxt[i]; i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, timerTxt[i]);

    // Progress bar timer — lebar proporsional dengan sisa waktu
    float frac = remaining / quizTimeLimit;
    if (frac < 0) frac = 0;

    glBegin(GL_QUADS);
        if      (frac > 0.5f)  glColor3f(0.2f, 0.85f, 0.3f); // > 50% : hijau
        else if (frac > 0.25f) glColor3f(0.9f, 0.6f,  0.1f); // > 25% : kuning
        else                   glColor3f(0.9f, 0.1f,  0.1f); // <= 25%: merah
        glVertex2f(80, 75); glVertex2f(80 + frac * 610, 75);
        glVertex2f(80 + frac * 610, 92); glVertex2f(80, 92);
    glEnd();

    // Border progress bar
    glColor3f(0.4f, 0.4f, 0.4f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(80, 75); glVertex2f(690, 75);
        glVertex2f(690, 92); glVertex2f(80, 92);
    glEnd();

    // Petunjuk skip
    glColor3f(0.7f, 0.7f, 0.3f);
    char skipTxt[] = "[E] Lewati (+2 detik penalti, tanpa kehilangan nyawa)";
    glRasterPos2i(80, 60);
    for (int i = 0; skipTxt[i]; i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, skipTxt[i]);

    // Overlay feedback ditaruh di atas panel soal
    renderFeedbackOverlay();

    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);  glPopMatrix();
    glEnable(GL_DEPTH_TEST); glEnable(GL_LIGHTING);
}