/**
 * @file hud.h
 * @brief Deklarasi fungsi HUD (heads-up display) dan overlay gameplay:
 *        indikator status, feedback jawaban, dan layar soal kuis.
 *
 * Semua fungsi di sini di-render dalam mode 2D orthographic
 * di atas scene 3D yang sudah dirender sebelumnya.
 */

#pragma once

/**
 * @brief Menggambar HUD atas layar saat gameplay.
 *
 * Konten strip transparan hitam di bagian atas:
 *  - Timer permainan (putih)   — kiri atas
 *  - Skor kertas (hijau)       — kiri atas, baris kedua
 *  - Indikator nyawa [v]/[ ]   — kanan atas
 *  - Label mode kamera         — tengah atas
 *
 * Menggunakan glOrtho dengan Y terbalik (0=atas, HEIGHT=bawah)
 * agar koordinat teks mudah dihitung dari ujung atas layar.
 */
void renderHUD();

/**
 * @brief Menggambar overlay warna feedback setelah menjawab soal.
 *
 * Overlay berupa quad penuh layar berwarna:
 *  - Hijau transparan  : jawaban benar  (FB_CORRECT)
 *  - Merah transparan  : jawaban salah  (FB_WRONG)
 *
 * Alpha di-fade out secara linear selama FEEDBACK_DURATION detik.
 * Jika feedbackState == FB_NONE atau durasi sudah habis, fungsi ini
 * langsung return tanpa render apapun.
 *
 * Teks feedback (BENAR! / SALAH!) dan sub-teks bonus/penalti
 * ditampilkan di tengah layar dengan shadow hitam untuk keterbacaan.
 */
void renderFeedbackOverlay();

/**
 * @brief Menggambar layar soal kuis (QUIZ_SCREEN state).
 *
 * Layout panel:
 *  - Background panel gelap (semi-transparan) ditengah layar
 *  - Bingkai berwarna sesuai difficulty (hijau/kuning/merah)
 *  - Label difficulty dan nomor soal di baris atas
 *  - Teks pertanyaan
 *  - 4 pilihan jawaban dengan latar berwarna per opsi (A=hijau, B=biru, C=kuning, D=merah)
 *  - Timer countdown: teks + progress bar berwarna (hijau->kuning->merah)
 *  - Petunjuk skip (E) di bawah
 *
 * renderFeedbackOverlay() dipanggil di dalam fungsi ini agar overlay
 * feedback tampil di atas panel soal saat transisi jawaban benar/salah.
 */
void renderQuizScreen();
