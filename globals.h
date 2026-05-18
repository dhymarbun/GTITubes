/**
 * @file globals.h
 * @brief Deklarasi semua variabel global yang dipakai lintas modul.
 *
 * Pola yang digunakan: "extern" di sini (deklarasi),
 * definisi aktual ada di globals.cpp.
 * Setiap .cpp yang butuh variabel ini cukup #include "globals.h".
 *
 * Kenapa extern dan bukan langsung define di header?
 * Karena kalau kamu taruh definisi (`int x = 0;`) di header lalu
 * header itu di-include lebih dari satu .cpp, linker akan error
 * "multiple definition of x". Dengan extern, header hanya bilang
 * "variabel ini ADA di suatu tempat", dan definisinya satu-satunya
 * ada di globals.cpp.
 */

#pragma once

#if defined(_WIN32) && !defined(CALLBACK)
#define CALLBACK __stdcall
#endif
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

// =============================================
// WINDOW DIMENSIONS
// =============================================
extern const int WIDTH;
extern const int HEIGHT;

// =============================================
// GAME TIMER
// Waktu dimulai dari nol saat player pertama bergerak.
// timeOffset dipakai untuk menambah/kurang waktu sebagai reward/penalti
// tanpa menyentuh gameStartTime (agar perhitungan elapsed tetap akurat).
// =============================================
extern float gameStartTime;
extern float timeOffset;
extern float currentGameTime;
extern bool  gameStarted;
extern char  timeString[32];

// =============================================
// GAME STATES
// Enum untuk finite state machine layar game.
// =============================================
enum GameState {
    TITLE_SCREEN,
    PLAYING,
    COMPLETED,
    RULES_SCREEN,
    QUIZ_SCREEN,
    GAME_OVER
};
extern GameState currentGameState;
extern bool rulesDisplayed;

// =============================================
// PLAYER STATE
// =============================================
extern float playerX;       // Posisi X player di dunia maze
extern float playerY;       // Posisi Y (selalu 0, maze flat)
extern float playerZ;       // Posisi Z player di dunia maze
extern float playerAngle;   // Arah hadap player dalam radian
extern float playerSpeed;   // Kecepatan maju/mundur per frame
extern float rotationSpeed; // Kecepatan rotasi kiri/kanan per frame
extern bool  gameCompleted;
extern char  congratsMessage[128];
extern int   playerScore;
extern char  scoreString[32];
extern char  finalscore[32];
extern char  finaltime[32];
extern char  livesString[32];

// =============================================
// BOARD (soal) ANIMATION
// Board adalah papan kayu berputar di maze (tile tipe 3).
// boardYOffset: efek mengambang naik-turun (sinusoidal)
// boardRotation: rotasi Y terus-menerus per frame
// =============================================
extern float boardYOffset;
extern float boardAnimSpeed;
extern float boardRotation;

// =============================================
// TIME PENALTIES & REWARDS
// =============================================
extern const float PENALTY_WRONG;   // Salah/timeout => +5 detik ke waktu
extern const float PENALTY_SKIP;    // Skip         => +2 detik ke waktu
extern const float REWARD_CORRECT;  // Benar        => -3 detik dari waktu

// =============================================
// MAZE DATA
// Tile values:
//   0 = jalan kosong
//   1 = dinding
//   2 = tujuan (blok merah)
//   3 = board soal (papan kayu)
// =============================================
extern const int MAZE_WIDTH;
extern const int MAZE_HEIGHT;
extern int maze[10][10];      // Maze aktif (berubah saat board diambil)
extern int mazeCopy[10][10];  // Salinan maze awal untuk reset game

// =============================================
// QUIZ STATE
// =============================================
// Jumlah total soal dalam pool
extern const int TOTAL_QUESTIONS;

// Urutan soal setelah dikocok (easy -> medium -> hard)
extern int  questionOrder[10];
// Status apakah soal sudah pernah dipakai dalam sesi ini
extern bool questionUsed[10];
// Indeks iterasi getNextQuestion()
extern int  currentQuestionIndex;
// ID soal yang sedang aktif ditampilkan
extern int  activeQuestion;

extern float quizStartTime;   // Timestamp saat soal mulai ditampilkan
extern float quizTimeLimit;   // Batas waktu per soal (detik)
extern int   paperCellX;      // Koordinat tile board yang memicu soal
extern int   paperCellZ;

// =============================================
// LIFE SYSTEM
// =============================================
extern int playerLives;
extern const int MAX_LIVES;

// =============================================
// VISUAL FEEDBACK
// Overlay warna hijau/merah sesaat setelah jawab soal.
// =============================================
enum FeedbackType { FB_NONE, FB_CORRECT, FB_WRONG };
extern FeedbackType feedbackState;
extern float        feedbackTimer;      // Timestamp saat feedback mulai
extern const float  FEEDBACK_DURATION; // Durasi overlay feedback (detik)

// =============================================
// QUIZ RE-TRIGGER PREVENTION
// Mencegah soal langsung muncul lagi setelah player keluar dari QUIZ_SCREEN.
// quizJustExited diset true saat keluar quiz, lalu di-reset setelah
// quizCooldownTimer habis.
// =============================================
extern bool  quizJustExited;
extern float quizCooldownTimer;
extern const float QUIZ_COOLDOWN;

// =============================================
// CAMERA MODE
// Mendukung dua mode kamera yang bisa di-toggle tombol V saat PLAYING.
//
// FIRST_PERSON : kamera tepat di kepala player, pandangan ke depan.
// THIRD_PERSON : kamera mengambang di belakang-atas player, player
//                terlihat sebagai model 3D kecil di tengah layar.
// =============================================
enum CameraMode { FIRST_PERSON, THIRD_PERSON };
extern CameraMode cameraMode;

// Jarak dan ketinggian kamera dari player saat third-person.
// Bisa diubah runtime via globals.cpp untuk fine-tuning.
extern float tpDistance; // Jarak horizontal ke belakang player
extern float tpHeight;   // Ketinggian kamera di atas player

// =============================================
// KEY STATE (debounce)
// Menyimpan state tombol frame sebelumnya untuk deteksi "baru ditekan"
// (rising edge), bukan sekadar "sedang ditekan".
// =============================================
extern bool keyPressedLastFrame[GLFW_KEY_LAST];
