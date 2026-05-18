/**
 * @file player.h
 * @brief Deklarasi fungsi player: setup kamera, pemrosesan input,
 *        dan callback resize window.
 */

#pragma once

#include <GLFW/glfw3.h>

// =============================================
// CAMERA
// =============================================

/**
 * @brief Mengatur proyeksi perspektif dan posisi kamera sesuai cameraMode.
 *
 * === FIRST_PERSON ===
 * Kamera ditempatkan tepat di kepala player (playerY + 0.5) dan mengarah
 * ke depan berdasarkan playerAngle. Player tidak terlihat sebagai objek.
 *
 * === THIRD_PERSON ===
 * Kamera mengambang di belakang dan atas player:
 *   eyeX = playerX - sin(playerAngle) * tpDistance
 *   eyeZ = playerZ + cos(playerAngle) * tpDistance
 *   eyeY = playerY + tpHeight
 * Target pandang tetap ke playerX, playerY+0.5, playerZ.
 * Model player (drawPlayer) dirender setelah setupCamera di mode ini.
 *
 * Keduanya memakai FOV 60°, near=0.1, far=100.
 */
void setupCamera();

/**
 * @brief Menggambar model player 3D sederhana untuk third-person view.
 *
 * Model terdiri dari:
 *   - Badan: kubus putih agak gepeng (0.4 x 0.6 x 0.3)
 *   - Kepala: kubus skin-tone lebih kecil di atas badan
 *   - Arah hadap: garis merah pendek menunjuk ke depan player
 *
 * Transformasi: ditaruh di posisi player (playerX, 0, playerZ),
 * dirotasi sesuai playerAngle. Fungsi ini hanya dipanggil saat
 * cameraMode == THIRD_PERSON.
 */
void drawPlayer();

// =============================================
// INPUT
// =============================================

/**
 * @brief Memproses semua input keyboard sesuai GameState aktif.
 *
 * Fungsi ini dipanggil setiap frame dari game loop.
 * Alur penanganan berdasarkan state:
 *
 *   TITLE_SCREEN  : P=mulai game, ?=rules, ESC=quit
 *   RULES_SCREEN  : B=kembali, ESC=quit
 *   GAME_OVER     : R=restart, B=title, ESC=quit
 *   COMPLETED     : R=restart, B=title, ESC=quit
 *   QUIZ_SCREEN   : A/B/C/D=jawab, E=skip; movement DIBLOKIR
 *   PLAYING       : W/S=maju/mundur, A/D=rotasi; deteksi trigger board/goal
 *
 * Semua tombol yang perlu deteksi "baru ditekan" (rising edge)
 * menggunakan array keyPressedLastFrame[] untuk debounce manual.
 * Ini mencegah satu tekan fisik terhitung berkali-kali dalam loop.
 *
 * Delta time dipakai untuk normalisasi kecepatan gerak agar konsisten
 * di berbagai frame rate (adjSpd = playerSpeed * deltaTime * 60).
 *
 * @param window Pointer ke window GLFW aktif.
 */
void processInput(GLFWwindow* window);

// =============================================
// WINDOW CALLBACK
// =============================================

/**
 * @brief Callback yang dipanggil GLFW saat ukuran framebuffer berubah.
 *
 * Memperbarui viewport OpenGL dan matriks proyeksi agar aspek rasio
 * gambar tetap benar saat jendela di-resize.
 *
 * @param window Pointer ke window GLFW (tidak dipakai, tapi wajib ada).
 * @param width  Lebar framebuffer baru dalam piksel.
 * @param height Tinggi framebuffer baru dalam piksel.
 */
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
