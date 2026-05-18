/**
 * @file renderer.h
 * @brief Deklarasi fungsi render layar non-gameplay:
 *        title, rules, game over, dan congrats screen.
 *
 * Semua fungsi di sini menggunakan mode 2D orthographic sementara
 * (push/pop matrix) sehingga tidak mengganggu proyeksi 3D yang
 * sedang aktif di game loop.
 */

#pragma once

/**
 * @brief Menggambar layar judul (title screen).
 *
 * Konten:
 *  - Latar belakang gradien biru gelap (GL_QUADS dua warna)
 *  - Judul game dengan warna emas
 *  - Subtitle info soal/nyawa
 *  - Daftar kontrol (P, ?, ESC)
 *  - Bingkai berkilap dengan animasi pulse berbasis sinusoidal
 *  - Credits tiga baris di bagian bawah layar
 *
 * Lighting dan depth test dimatikan sementara selama render 2D.
 */
void renderTitleScreen();

/**
 * @brief Menggambar layar aturan (rules/how-to-play screen).
 *
 * Konten:
 *  - Latar belakang gelap penuh
 *  - Judul "HOW TO PLAY"
 *  - Daftar aturan lengkap: gerakan, mekanik quiz, sistem nyawa, tujuan
 *  - Tombol kembali: B
 */
void renderRulesScreen();

/**
 * @brief Menggambar layar game over.
 *
 * Konten:
 *  - Latar gradien merah beranimasi pulse
 *  - Teks "GAME OVER" besar
 *  - Waktu total dan skor kertas yang berhasil dikumpulkan
 *  - Petunjuk restart (R) dan kembali ke title (B)
 *  - Bingkai merah tebal
 */
void renderGameOverScreen();

/**
 * @brief Menggambar layar selamat (congrats/completed screen).
 *
 * Konten:
 *  - Latar biru dengan alpha blend
 *  - Teks "MAZE COMPLETED!"
 *  - Tiga baris statistik: pesan selesai, skor, waktu final
 *  - Petunjuk play again (R) dan title (B)
 *  - 20 partikel bintang beranimasi bergerak random di layar
 *  - Bingkai emas tebal
 */
void renderCongratsScreen();
