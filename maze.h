/**
 * @file maze.h
 * @brief Deklarasi fungsi-fungsi terkait maze: collision detection,
 *        salinan maze, dan rendering dunia 3D.
 */

#pragma once

// =============================================
// MAZE UTILITY
// =============================================

/**
 * @brief Menyalin isi array maze 2D dari src ke dest.
 *
 * Dipakai untuk:
 *  1. Menyimpan snapshot maze awal ke mazeCopy sebelum gameplay dimulai.
 *  2. Merestore maze ke kondisi awal saat restart/new game.
 *
 * @param dest Array tujuan [MAZE_WIDTH][MAZE_HEIGHT].
 * @param src  Array sumber [MAZE_WIDTH][MAZE_HEIGHT].
 */
void copyMazeArray(int dest[20][20], int src[20][20]);

// =============================================
// COLLISION DETECTION
// =============================================

/**
 * @brief Memeriksa apakah posisi (newX, newZ) menabrak dinding.
 *
 * Menggunakan buffer tipis (0.15f) di sekeliling player sehingga
 * player tidak bisa "menancap" ke dalam dinding walau bergerak
 * diagonal. Dicek 8 titik batas: kiri, kanan, atas, bawah, dan
 * empat sudut diagonal.
 *
 * @param newX Koordinat X yang ingin dituju.
 * @param newZ Koordinat Z yang ingin dituju.
 * @return true jika ada tabrakan (tidak boleh bergerak ke sana),
 *         false jika aman.
 */
bool checkCollision(float newX, float newZ);

/**
 * @brief Memeriksa apakah player cukup dekat dengan sebuah board.
 *
 * Jarak dihitung dari pusat tile board ke posisi player.
 * Threshold: 0.9f unit (sekitar setengah lebar tile).
 *
 * @param px Posisi X player.
 * @param pz Posisi Z player.
 * @param x  Koordinat tile X board.
 * @param z  Koordinat tile Z board.
 * @return true jika player dalam radius trigger board.
 */
bool isNearBoard(float px, float pz, int x, int z);

// =============================================
// RENDERING
// =============================================

/**
 * @brief Menggambar geometri satu kubus berpusat di origin.
 *
 * Kubus digambar dengan 6 sisi (GL_QUADS) beserta normal tiap sisi
 * agar lighting OpenGL bekerja dengan benar. Ukuran diatur lewat
 * parameter `size`; pusat kubus selalu di (0, 0, 0) sehingga
 * penempatan dilakukan dengan glTranslatef sebelum memanggil fungsi ini.
 *
 * @param size Panjang sisi kubus dalam unit dunia.
 */
void drawCube(float size);

/**
 * @brief Menggambar bidang tanah (ground plane) di seluruh area maze.
 *
 * Ground berupa quad hijau gelap yang menutupi seluruh grid maze,
 * ditambah grid lines untuk memberi kedalaman visual (depth cues).
 * Lighting dimatikan sementara selama render ground agar warna
 * tampil flat dan tidak terpengaruh shading dinding.
 */
void renderGround();

/**
 * @brief Memuat tekstur langit BMP 24-bit dari file.
 *
 * @return true jika tekstur berhasil dimuat.
 */
bool loadSkyTexture(const char* filename);

/**
 * @brief Menggambar skybox bertekstur mengelilingi player.
 */
void renderSky();

/**
 * @brief Melepas texture OpenGL milik skybox.
 */
void cleanupSkyTexture();

/**
 * @brief Menggambar semua elemen maze: dinding, goal, dan board soal.
 *
 * Iterasi seluruh array maze[z][x] dan render berdasarkan tipe tile:
 *   - Tile 1: kubus abu-abu (dinding)
 *   - Tile 2: kubus merah  (tujuan/goal)
 *   - Tile 3: model papan kayu dengan animasi mengambang dan rotasi
 *
 * Animasi board dihitung dari glfwGetTime() sehingga berjalan
 * secara real-time terlepas dari frame rate.
 */
void renderMaze();
