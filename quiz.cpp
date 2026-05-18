/**
 * @file quiz.cpp
 * @brief Implementasi sistem kuis: pool soal, pengacakan, dan logika jawaban.
 */

#include "quiz.h"
#include "globals.h"
#include <cstdio>
#include <cstdlib>

// =============================================
// POOL SOAL
// =============================================
// Soal dibagi tiga tingkat:
//   Index 0-2 : Easy   (matematika & CS dasar)
//   Index 3-6 : Medium (kalkulus, struktur data, konversi)
//   Index 7-9 : Hard   (integral, kompleksitas, turunan lanjut)
Question questions[] = {
    // --- EASY ---
    {"[MATH] Berapa hasil dari 5 + 3 x 2?",
     {"A. 16", "B. 11", "C. 13", "D. 10"}, 1, 0},
    {"[CS] Big-O dari pencarian linear array N?",
     {"A. O(log N)", "B. O(N^2)", "C. O(N)", "D. O(1)"}, 2, 0},
    {"[MATH] Berapa nilai dari 2^8?",
     {"A. 128", "B. 512", "C. 64", "D. 256"}, 3, 0},

    // --- MEDIUM ---
    {"[MATH] Limit x->0 dari (sin x)/x = ?",
     {"A. 0", "B. Tak terdefinisi", "C. 1", "D. Infinity"}, 2, 1},
    {"[CS] Struktur data LIFO adalah?",
     {"A. Queue", "B. Stack", "C. Tree", "D. Graph"}, 1, 1},
    {"[MATH] Turunan dari f(x) = x^3 + 2x adalah?",
     {"A. 3x + 2", "B. x^2 + 2", "C. 3x^2 + 2", "D. 3x^2"}, 2, 1},
    {"[CS] Binary dari desimal 42 adalah?",
     {"A. 101010", "B. 100110", "C. 110100", "D. 101100"}, 0, 1},

    // --- HARD ---
    {"[MATH] Integral dari 1/x dx adalah?",
     {"A. x + C", "B. ln|x| + C", "C. 1/x^2 + C", "D. e^x + C"}, 1, 2},
    {"[CS] Kompleksitas waktu worst-case QuickSort?",
     {"A. O(N log N)", "B. O(log N)", "C. O(N^2)", "D. O(N)"}, 2, 2},
    {"[MATH] Jika f(x)=e^(2x), maka f''(x)=?",
     {"A. e^(2x)", "B. 2e^(2x)", "C. 4e^(2x)", "D. e^(4x)"}, 2, 2},
};

// =============================================
// SHUFFLE QUESTIONS
// =============================================
void shuffleQuestions() {
    // Tiap kelompok dikocok secara independen agar urutan
    // easy -> medium -> hard selalu terjaga.
    int easy[3]   = {0, 1, 2};
    int medium[4] = {3, 4, 5, 6};
    int hard[3]   = {7, 8, 9};

    // Fisher-Yates shuffle untuk tiap kelompok.
    // Algoritma: mulai dari elemen terakhir, swap dengan elemen random
    // di posisi 0..i. Kompleksitas O(n), distribusi seragam.
    for (int i = 2; i > 0; i--) {
        int j = rand() % (i + 1);
        int t = easy[i]; easy[i] = easy[j]; easy[j] = t;
    }
    for (int i = 3; i > 0; i--) {
        int j = rand() % (i + 1);
        int t = medium[i]; medium[i] = medium[j]; medium[j] = t;
    }
    for (int i = 2; i > 0; i--) {
        int j = rand() % (i + 1);
        int t = hard[i]; hard[i] = hard[j]; hard[j] = t;
    }

    // Gabungkan hasil shuffle ke dalam questionOrder[]
    for (int i = 0; i < 3; i++) questionOrder[i]     = easy[i];
    for (int i = 0; i < 4; i++) questionOrder[3 + i]  = medium[i];
    for (int i = 0; i < 3; i++) questionOrder[7 + i]  = hard[i];

    // Reset status pemakaian semua soal
    for (int i = 0; i < TOTAL_QUESTIONS; i++) questionUsed[i] = false;

    // Reset indeks ke titik awal (satu-satunya tempat reset dilakukan)
    currentQuestionIndex = 0;
}

// =============================================
// GET NEXT QUESTION
// =============================================
int getNextQuestion() {
    // Cari soal berikutnya yang belum dipakai mulai dari currentQuestionIndex.
    // Tidak melompat-lompat acak: selalu maju linear agar mudah di-trace.
    for (int i = currentQuestionIndex; i < TOTAL_QUESTIONS; i++) {
        if (!questionUsed[questionOrder[i]]) {
            currentQuestionIndex = i;
            return questionOrder[i];
        }
    }
    // Semua soal sudah terpakai — kocok ulang dan mulai dari awal
    shuffleQuestions();
    return questionOrder[0];
}

// =============================================
// REMOVE BOARD
// =============================================
void removeBoard() {
    // Hanya bekerja kalau paperCellX/Z sudah diset (>= 0).
    // Mengubah tile 3 menjadi 0 (jalan kosong) di array maze[].
    if (paperCellZ >= 0 && paperCellX >= 0) {
        maze[paperCellZ][paperCellX] = 0;
        printf("[BOARD] Removed board at (%d,%d)\n", paperCellX, paperCellZ);
    }
}

// =============================================
// APPLY CORRECT ANSWER
// =============================================
void applyCorrectAnswer() {
    removeBoard();

    playerScore++;
    sprintf(scoreString, "Kertas: %d", playerScore);

    // Kurangi waktu sebagai reward. Kita tidak mengubah gameStartTime
    // langsung karena itu akan mengacaukan perhitungan elapsed.
    // Sebagai gantinya, timeOffset dikurangi sehingga currentGameTime
    // (= elapsed + timeOffset) menjadi lebih kecil.
    timeOffset -= REWARD_CORRECT;

    feedbackState = FB_CORRECT;
    feedbackTimer = (float)glfwGetTime();

    // Tandai soal ini sudah dipakai, tepat satu kali, di sini.
    questionUsed[activeQuestion] = true;
    printf("[QUIZ] Benar! +1 skor, bonus -%.0f detik\n", REWARD_CORRECT);
}

// =============================================
// APPLY WRONG ANSWER
// =============================================
void applyWrongAnswer() {
    removeBoard();

    playerLives--;
    sprintf(livesString, "Lives: %d", playerLives);

    // Tambah timeOffset sebagai penalti (waktu tampil jadi lebih besar)
    timeOffset += PENALTY_WRONG;

    feedbackState = FB_WRONG;
    feedbackTimer = (float)glfwGetTime();

    questionUsed[activeQuestion] = true;
    printf("[QUIZ] Salah! -1 nyawa, penalti +%.0f detik\n", PENALTY_WRONG);

    // Cek game over setelah pengurangan nyawa
    if (playerLives <= 0) {
        currentGameState = GAME_OVER;
        // Bersihkan feedback agar layar game over tampil bersih
        feedbackState = FB_NONE;
    }
}

// =============================================
// APPLY SKIP
// =============================================
void applySkip() {
    removeBoard();

    // Penalti kecil — tidak ada pengurangan nyawa, tidak ada feedback overlay.
    // Soal tetap ditandai sudah dipakai agar tidak muncul lagi di sesi ini.
    timeOffset += PENALTY_SKIP;
    questionUsed[activeQuestion] = true;
    printf("[QUIZ] Dilewati. Penalti +%.0f detik\n", PENALTY_SKIP);
}

// =============================================
// APPLY TIMEOUT
// =============================================
void applyTimeout() {
    removeBoard();

    playerLives--;
    sprintf(livesString, "Lives: %d", playerLives);
    timeOffset += PENALTY_WRONG;

    feedbackState = FB_WRONG;
    feedbackTimer = (float)glfwGetTime();

    questionUsed[activeQuestion] = true;
    printf("[QUIZ] Waktu habis! -1 nyawa\n");

    if (playerLives <= 0) {
        currentGameState = GAME_OVER;
        feedbackState    = FB_NONE;
    }
}
