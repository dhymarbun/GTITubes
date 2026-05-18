/**
 * @file quiz.h
 * @brief Deklarasi sistem kuis: struktur soal, pool soal, dan logika jawaban.
 *
 * Modul ini menangani:
 *  - Definisi struct Question
 *  - Pool soal (easy / medium / hard)
 *  - Pengacakan urutan soal
 *  - Pengambilan soal berikutnya
 *  - Aplikasi hasil jawaban (benar / salah / skip / timeout)
 *  - Penghapusan board dari maze setelah soal selesai
 */

#pragma once

// =============================================
// QUESTION STRUCT
// =============================================
/**
 * @brief Satu entri soal pilihan ganda.
 *
 * @param question   Teks pertanyaan.
 * @param options    Array 4 pilihan jawaban (A-D).
 * @param correct    Index pilihan yang benar (0=A, 1=B, 2=C, 3=D).
 * @param difficulty Tingkat kesulitan: 0=mudah, 1=sedang, 2=sulit.
 */
struct Question {
    const char* question;
    const char* options[4];
    int correct;
    int difficulty;
};

/**
 * @brief Pool semua soal yang tersedia.
 *
 * Soal dikelompokkan berdasarkan difficulty:
 *   Index 0-2  : easy   (difficulty=0)
 *   Index 3-6  : medium (difficulty=1)
 *   Index 7-9  : hard   (difficulty=2)
 *
 * Pengelompokan ini penting untuk shuffleQuestions() yang mengocok
 * tiap kelompok secara terpisah agar urutan easy->medium->hard terjaga.
 */
extern Question questions[];

// =============================================
// FUNCTION DECLARATIONS
// =============================================

/**
 * @brief Mengocok urutan soal dalam tiga kelompok (easy, medium, hard).
 *
 * Menggunakan Fisher-Yates shuffle di dalam setiap kelompok.
 * Setelah dikocok, hasilnya disimpan ke questionOrder[].
 * Semua status questionUsed[] di-reset ke false.
 * currentQuestionIndex di-reset ke 0.
 */
void shuffleQuestions();

/**
 * @brief Mengambil ID soal berikutnya yang belum dipakai.
 *
 * Iterasi maju dari currentQuestionIndex sampai menemukan soal
 * yang belum dipakai (questionUsed[id] == false).
 * Jika semua soal habis, memanggil shuffleQuestions() dan mulai ulang.
 *
 * @return Index soal dalam array questions[] yang akan ditampilkan.
 */
int getNextQuestion();

/**
 * @brief Menghapus board (tile 3) dari maze di posisi (paperCellX, paperCellZ).
 *
 * Dipanggil setiap kali soal selesai (benar/salah/skip/timeout).
 * Tujuannya agar board tidak bisa men-trigger soal lagi setelah selesai.
 * Kalau paperCellX/Z negatif (belum ada board aktif), fungsi ini no-op.
 */
void removeBoard();

/**
 * @brief Menangani jawaban benar.
 *
 * Efek:
 *  - Hapus board dari maze
 *  - playerScore++
 *  - timeOffset -= REWARD_CORRECT (bonus waktu)
 *  - Set feedback overlay hijau
 *  - Tandai soal sebagai sudah dipakai
 */
void applyCorrectAnswer();

/**
 * @brief Menangani jawaban salah.
 *
 * Efek:
 *  - Hapus board dari maze
 *  - playerLives--
 *  - timeOffset += PENALTY_WRONG (penalti waktu)
 *  - Set feedback overlay merah
 *  - Tandai soal sebagai sudah dipakai
 *  - Jika lives <= 0, transisi ke GAME_OVER
 */
void applyWrongAnswer();

/**
 * @brief Menangani skip soal (tombol E).
 *
 * Efek:
 *  - Hapus board dari maze
 *  - timeOffset += PENALTY_SKIP (penalti kecil)
 *  - Tandai soal sebagai sudah dipakai
 *  - TIDAK mengurangi nyawa, TIDAK ada feedback overlay
 */
void applySkip();

/**
 * @brief Menangani timeout (waktu soal habis).
 *
 * Efek:
 *  - Hapus board dari maze
 *  - playerLives--
 *  - timeOffset += PENALTY_WRONG
 *  - Set feedback overlay merah
 *  - Tandai soal sebagai sudah dipakai
 *  - Jika lives <= 0, transisi ke GAME_OVER
 */
void applyTimeout();
