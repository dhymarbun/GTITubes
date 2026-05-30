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
//   Index 0-5  : Easy   (6 soal)
//   Index 6-13 : Medium (8 soal)
//   Index 14-19: Hard   (6 soal)
Question questions[] = {
    // --- EASY (0-5) ---
    {"[MATH] Berapa hasil dari 5 + 3 x 2?",
     {"16", "11", "13", "10"}, 1, 0},
    {"[CS] Big-O dari pencarian linear array N?",
     {"O(log N)", "O(N^2)", "O(N)", "O(1)"}, 2, 0},
    {"[MATH] Berapa nilai dari 2^8?",
     {"128", "512", "64", "256"}, 3, 0},
    {"[CS] Kepanjangan dari CPU adalah?",
     {"Central Processing Unit", "Core Processing Unit", "Central Program Unit", "Computer Power Unit"}, 0, 0},
    {"[MATH] Berapa hasil dari 12 x 12?",
     {"132", "144", "124", "148"}, 1, 0},
    {"[CS] Ekstensi file gambar yang umum adalah?",
     {".mp3", ".exe", ".jpg", ".docx"}, 2, 0},

    // --- MEDIUM (6-13) ---
    {"[MATH] Limit x->0 dari (sin x)/x = ?",
     {"0", "Tak terdefinisi", "1", "Infinity"}, 2, 1},
    {"[CS] Struktur data LIFO adalah?",
     {"Queue", "Stack", "Tree", "Graph"}, 1, 1},
    {"[MATH] Turunan dari f(x) = x^3 + 2x adalah?",
     {"3x + 2", "x^2 + 2", "3x^2 + 2", "3x^2"}, 2, 1},
    {"[CS] Binary dari desimal 42 adalah?",
     {"101010", "100110", "110100", "101100"}, 0, 1},
    {"[MATH] Nilai dari log2(64) adalah?",
     {"4", "8", "5", "6"}, 3, 1},
    {"[CS] RAM adalah singkatan dari?",
     {"Read Access Memory", "Random Access Memory", "Rapid Access Module", "Read Assign Memory"}, 1, 1},
    {"[MATH] Berapa akar kuadrat dari 169?",
     {"11", "14", "13", "12"}, 2, 1},
    {"[CS] Protokol pengiriman email yang umum digunakan adalah?",
     {"FTP", "HTTP", "SMTP", "SSH"}, 2, 1},

    // --- HARD (14-19) ---
    {"[MATH] Integral dari 1/x dx adalah?",
     {"x + C", "ln|x| + C", "1/x^2 + C", "e^x + C"}, 1, 2},
    {"[CS] Kompleksitas waktu worst-case QuickSort?",
     {"O(N log N)", "O(log N)", "O(N^2)", "O(N)"}, 2, 2},
    {"[MATH] Jika f(x)=e^(2x), maka f''(x)=?",
     {"e^(2x)", "2e^(2x)", "4e^(2x)", "e^(4x)"}, 2, 2},
    {"[CS] Dalam notasi Big-O, O(2^N) disebut?",
     {"Polynomial", "Logarithmic", "Linear", "Exponential"}, 3, 2},
    {"[MATH] Nilai dari integral tentu integral_0^1 x^2 dx adalah?",
     {"1/2", "1/4", "1/3", "1"}, 2, 2},
    {"[CS] AVL Tree adalah jenis Binary Search Tree yang?",
     {"Tidak seimbang", "Self-balancing", "Hanya bisa insert", "Selalu penuh"}, 1, 2},
};

// =============================================
// SHUFFLE QUESTIONS
// =============================================
void shuffleQuestions() {
    // Easy: index 0-5 (6 soal)
    int easy[6]   = {0, 1, 2, 3, 4, 5};
    // Medium: index 6-13 (8 soal)
    int medium[8] = {6, 7, 8, 9, 10, 11, 12, 13};
    // Hard: index 14-19 (6 soal)
    int hard[6]   = {14, 15, 16, 17, 18, 19};

    // Fisher-Yates shuffle tiap kelompok
    for (int i = 5; i > 0; i--) {
        int j = rand() % (i + 1);
        int t = easy[i]; easy[i] = easy[j]; easy[j] = t;
    }
    for (int i = 7; i > 0; i--) {
        int j = rand() % (i + 1);
        int t = medium[i]; medium[i] = medium[j]; medium[j] = t;
    }
    for (int i = 5; i > 0; i--) {
        int j = rand() % (i + 1);
        int t = hard[i]; hard[i] = hard[j]; hard[j] = t;
    }

    // Gabungkan ke questionOrder[]: 6 easy + 8 medium + 6 hard = 20
    for (int i = 0; i < 6; i++) questionOrder[i]      = easy[i];
    for (int i = 0; i < 8; i++) questionOrder[6 + i]  = medium[i];
    for (int i = 0; i < 6; i++) questionOrder[14 + i] = hard[i];

    // Reset status pemakaian semua soal
    for (int i = 0; i < TOTAL_QUESTIONS; i++) questionUsed[i] = false;

    // Reset indeks ke titik awal
    currentQuestionIndex = 0;
}

// =============================================
// GET NEXT QUESTION
// =============================================
int getNextQuestion() {
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

    timeOffset -= REWARD_CORRECT;

    feedbackState = FB_CORRECT;
    feedbackTimer = (float)glfwGetTime();

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

    timeOffset += PENALTY_WRONG;

    feedbackState = FB_WRONG;
    feedbackTimer = (float)glfwGetTime();

    questionUsed[activeQuestion] = true;
    printf("[QUIZ] Salah! -1 nyawa, penalti +%.0f detik\n", PENALTY_WRONG);

    if (playerLives <= 0) {
        currentGameState = GAME_OVER;
        feedbackState    = FB_NONE;
    }
}

// =============================================
// APPLY SKIP
// =============================================
void applySkip() {
    removeBoard();

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