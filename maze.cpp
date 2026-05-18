/**
 * @file maze.cpp
 * @brief Implementasi fungsi maze: collision detection, utilitas copy,
 *        dan rendering dunia 3D (ground, dinding, board soal).
 */

#include "maze.h"
#include "globals.h"
#include <cmath>
#if defined(_WIN32) && !defined(CALLBACK)
#define CALLBACK __stdcall
#endif
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

// =============================================
// COPY MAZE ARRAY
// =============================================
void copyMazeArray(int dest[10][10], int src[10][10]) {
    for (int z = 0; z < MAZE_HEIGHT; z++)
        for (int x = 0; x < MAZE_WIDTH; x++)
            dest[z][x] = src[z][x];
}

// =============================================
// DRAW CUBE
// =============================================
void drawCube(float size) {
    float h = size / 2.0f; // Setengah sisi — kubus dipusat di origin

    // Normal tiap sisi: dipakai OpenGL fixed-function lighting untuk
    // menghitung diffuse shading. Urutan: depan, belakang, kanan,
    // kiri, atas, bawah.
    GLfloat normals[][3] = {
        { 0, 0, 1}, { 0, 0,-1},
        { 1, 0, 0}, {-1, 0, 0},
        { 0, 1, 0}, { 0,-1, 0}
    };

    // 24 vertex (4 per sisi × 6 sisi), disusun searah jarum jam
    // bila dilihat dari luar kubus (untuk face culling GL_BACK).
    GLfloat verts[][3] = {
        // Depan  (+Z)
        {-h,-h, h},{ h,-h, h},{ h, h, h},{-h, h, h},
        // Belakang (-Z)
        {-h,-h,-h},{-h, h,-h},{ h, h,-h},{ h,-h,-h},
        // Kanan (+X)
        { h,-h, h},{ h,-h,-h},{ h, h,-h},{ h, h, h},
        // Kiri (-X)
        {-h,-h, h},{-h, h, h},{-h, h,-h},{-h,-h,-h},
        // Atas (+Y)
        {-h, h, h},{ h, h, h},{ h, h,-h},{-h, h,-h},
        // Bawah (-Y)
        {-h,-h, h},{-h,-h,-h},{ h,-h,-h},{ h,-h, h}
    };

    glBegin(GL_QUADS);
    for (int f = 0; f < 6; f++) {
        glNormal3fv(normals[f]);
        for (int v = 0; v < 4; v++)
            glVertex3fv(verts[f * 4 + v]);
    }
    glEnd();
}

// =============================================
// CHECK COLLISION
// =============================================
bool checkCollision(float newX, float newZ) {
    float buf = 0.15f; // Radius buffer agar player tidak "masuk" ke dinding

    int cx = (int)newX;
    int cz = (int)newZ;

    // Keluar batas maze = dianggap tabrakan
    if (cx < 0 || cx >= MAZE_WIDTH || cz < 0 || cz >= MAZE_HEIGHT) return true;

    // Tile tempat berdiri adalah dinding
    if (maze[cz][cx] == 1) return true;

    // Cek 4 sisi dan 4 sudut diagonal berdasarkan posisi fraksional
    // dalam tile saat ini. Ini memungkinkan player "sliding" sepanjang dinding.
    float fx = newX - cx;
    float fz = newZ - cz;

    if (fx > (1 - buf) && cx + 1 < MAZE_WIDTH  && maze[cz][cx+1]   == 1) return true;
    if (fx < buf        && cx - 1 >= 0           && maze[cz][cx-1]   == 1) return true;
    if (fz > (1 - buf) && cz + 1 < MAZE_HEIGHT  && maze[cz+1][cx]   == 1) return true;
    if (fz < buf        && cz - 1 >= 0           && maze[cz-1][cx]   == 1) return true;

    // Diagonal sudut kanan-bawah
    if (fx > (1-buf) && fz > (1-buf) && cx+1 < MAZE_WIDTH  && cz+1 < MAZE_HEIGHT && maze[cz+1][cx+1] == 1) return true;
    // Diagonal sudut kiri-atas
    if (fx < buf     && fz < buf     && cx-1 >= 0           && cz-1 >= 0           && maze[cz-1][cx-1] == 1) return true;
    // Diagonal sudut kanan-atas
    if (fx > (1-buf) && fz < buf     && cx+1 < MAZE_WIDTH   && cz-1 >= 0           && maze[cz-1][cx+1] == 1) return true;
    // Diagonal sudut kiri-bawah
    if (fx < buf     && fz > (1-buf) && cx-1 >= 0           && cz+1 < MAZE_HEIGHT  && maze[cz+1][cx-1] == 1) return true;

    return false;
}

// =============================================
// IS NEAR BOARD
// =============================================
bool isNearBoard(float px, float pz, int x, int z) {
    // Pusat tile ada di (x+0.5, z+0.5)
    float dx = px - (x + 0.5f);
    float dz = pz - (z + 0.5f);
    return sqrtf(dx * dx + dz * dz) < 0.9f;
}

// =============================================
// RENDER GROUND
// =============================================
void renderGround() {
    // Matikan lighting agar ground tampil flat (warna solid, tidak shading)
    glDisable(GL_LIGHTING);

    // Ground plane sedikit di bawah y=0 (-0.01) agar tidak z-fighting
    // dengan dinding yang lantainya juga di y=0
    glBegin(GL_QUADS);
    glColor3f(0.22f, 0.32f, 0.18f); // Hijau gelap seperti rerumputan
    glVertex3f(0.0f,            -0.01f, 0.0f);
    glVertex3f((float)MAZE_WIDTH, -0.01f, 0.0f);
    glVertex3f((float)MAZE_WIDTH, -0.01f, (float)MAZE_HEIGHT);
    glVertex3f(0.0f,            -0.01f, (float)MAZE_HEIGHT);
    glEnd();

    // Grid lines untuk memberi kesan grid lantai / depth cue
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    glColor3f(0.15f, 0.23f, 0.12f);
    for (int i = 0; i <= MAZE_WIDTH; i++) {
        glVertex3f((float)i, -0.005f, 0.0f);
        glVertex3f((float)i, -0.005f, (float)MAZE_HEIGHT);
    }
    for (int j = 0; j <= MAZE_HEIGHT; j++) {
        glVertex3f(0.0f,            -0.005f, (float)j);
        glVertex3f((float)MAZE_WIDTH, -0.005f, (float)j);
    }
    glEnd();

    glEnable(GL_LIGHTING);
}

// =============================================
// RENDER MAZE
// =============================================
void renderMaze() {
    // Hitung animasi board: mengambang sinusoidal + rotasi terus-menerus
    boardYOffset   = sinf((float)glfwGetTime() * boardAnimSpeed) * 0.06f;
    boardRotation += 0.4f;
    if (boardRotation > 360.0f) boardRotation -= 360.0f;

    for (int x = 0; x < MAZE_WIDTH; x++) {
        for (int z = 0; z < MAZE_HEIGHT; z++) {

            // --- TILE 1: Dinding ---
            if (maze[z][x] == 1) {
                GLfloat grey[] = {0.5f, 0.5f, 0.5f, 1.0f};
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, grey);
                glPushMatrix();
                    glTranslatef(x + 0.5f, 0.5f, z + 0.5f);
                    drawCube(1.0f);
                glPopMatrix();
            }

            // --- TILE 2: Goal (blok merah) ---
            else if (maze[z][x] == 2) {
                GLfloat red[] = {1.0f, 0.0f, 0.0f, 1.0f};
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
                glPushMatrix();
                    glTranslatef(x + 0.5f, 0.5f, z + 0.5f);
                    drawCube(1.0f);
                glPopMatrix();
            }

            // --- TILE 3: Board Soal ---
            // Model terdiri dari:
            //   - Tiang kayu (coklat, skala tipis memanjang)
            //   - Papan hitam (kotak lebar di atas tiang)
            //   - Permukaan putih (layar soal)
            //   - 4 garis biru (dekorasi layar)
            //   - 1 aksen kuning (pointer/marker)
            else if (maze[z][x] == 3) {
                glPushMatrix();
                    // Terapkan animasi mengambang dan rotasi Y
                    glTranslatef(x + 0.5f, boardYOffset, z + 0.5f);
                    glRotatef(boardRotation, 0.0f, 1.0f, 0.0f);

                    // Matikan specular highlight pada material kayu agar
                    // terlihat matte (tidak mengkilap)
                    GLfloat no_spec[] = {0, 0, 0, 1};
                    GLfloat no_shin[] = {0};

                    // Tiang kayu
                    GLfloat wood[] = {0.55f, 0.27f, 0.07f, 1.0f};
                    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, wood);
                    glMaterialfv(GL_FRONT, GL_SPECULAR,  no_spec);
                    glMaterialfv(GL_FRONT, GL_SHININESS, no_shin);
                    glPushMatrix();
                        glTranslatef(0.0f, 0.1f, 0.0f);
                        glScalef(0.08f, 1.0f, 0.08f); // Tipis tinggi
                        drawCube(1.0f);
                    glPopMatrix();

                    // Bingkai papan (lebih gelap dari tiang)
                    GLfloat darkwood[] = {0.4f, 0.2f, 0.05f, 1.0f};
                    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, darkwood);
                    glPushMatrix();
                        glTranslatef(0.0f, 0.72f, 0.04f);
                        glScalef(0.7f, 0.45f, 0.06f);
                        drawCube(1.0f);
                    glPopMatrix();

                    // Permukaan putih (layar soal)
                    GLfloat white[] = {1.0f, 1.0f, 1.0f, 1.0f};
                    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
                    glPushMatrix();
                        glTranslatef(0.0f, 0.72f, 0.075f);
                        glScalef(0.58f, 0.35f, 0.01f);
                        drawCube(1.0f);
                    glPopMatrix();

                    // Garis-garis biru sebagai dekorasi "tulisan" di layar
                    GLfloat lineblue[] = {0.1f, 0.2f, 0.8f, 1.0f};
                    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, lineblue);
                    for (int ln = 0; ln < 4; ln++) {
                        glPushMatrix();
                            // Setiap garis digeser 0.07 ke bawah
                            glTranslatef(0.0f, 0.82f - ln * 0.07f, 0.082f);
                            glScalef(0.4f, 0.012f, 0.005f);
                            drawCube(1.0f);
                        glPopMatrix();
                    }

                    // Aksen kuning (pointer kiri layar)
                    GLfloat yellow[] = {1.0f, 0.9f, 0.0f, 1.0f};
                    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, yellow);
                    glPushMatrix();
                        glTranslatef(0.18f, 0.72f, 0.083f);
                        glScalef(0.06f, 0.18f, 0.005f);
                        drawCube(1.0f);
                    glPopMatrix();

                glPopMatrix(); // Akhir transformasi board
            }
        }
    }
}
