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
// =============================================
// CHECK COLLISION (DENGAN HITBOX KUSTOM)
// =============================================
bool checkCollision(float newX, float newZ) {
    // Memisahkan ketebalan fisik karakter:
    float bufX = 0.32f; // Lebar ke samping (menjaga lengan teal & krem)
    float bufZ = 0.16f; // Tebal ke depan/belakang (biar gak nabrak dinding gaib)

    int cx = (int)newX;
    int cz = (int)newZ;

    // Keluar batas maze = dianggap tabrakan
    if (cx < 0 || cx >= MAZE_WIDTH || cz < 0 || cz >= MAZE_HEIGHT) return true;

    // Tile tempat berdiri adalah dinding
    if (maze[cz][cx] == 1) return true;

    // Cek posisi fraksional dalam tile saat ini
    float fx = newX - cx;
    float fz = newZ - cz;

    // Cek sisi horizontal (Kanan - Kiri) menggunakan bufX
    if (fx > (1 - bufX) && cx + 1 < MAZE_WIDTH  && maze[cz][cx+1]   == 1) return true;
    if (fx < bufX        && cx - 1 >= 0           && maze[cz][cx-1]   == 1) return true;
    
    // Cek sisi vertikal (Depan - Belakang) menggunakan bufZ
    if (fz > (1 - bufZ) && cz + 1 < MAZE_HEIGHT  && maze[cz+1][cx] == 1) return true;
    if (fz < bufZ        && cz - 1 >= 0           && maze[cz-1][cx] == 1) return true;

    // Diagonal sudut kanan-bawah
    if (fx > (1-bufX) && fz > (1-bufZ) && cx+1 < MAZE_WIDTH  && cz+1 < MAZE_HEIGHT && maze[cz+1][cx+1] == 1) return true;
    // Diagonal sudut kiri-atas
    if (fx < bufX     && fz < bufZ     && cx-1 >= 0           && cz-1 >= 0           && maze[cz-1][cx-1] == 1) return true;
    // Diagonal sudut kanan-atas
    if (fx > (1-bufX) && fz < bufZ     && cx+1 < MAZE_WIDTH   && cz-1 >= 0           && maze[cz-1][cx+1] == 1) return true;
    // Diagonal sudut kiri-bawah
    if (fx < bufX     && fz > (1-bufZ) && cx-1 >= 0           && cz+1 < MAZE_HEIGHT  && maze[cz+1][cx-1] == 1) return true;

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
    glDisable(GL_LIGHTING);

    for (int x = 0; x < MAZE_WIDTH; x++) {
        for (int z = 0; z < MAZE_HEIGHT; z++) {

            if ((x + z) % 2 == 0)
                glColor3f(0.78f, 0.78f, 0.78f);
            else
                glColor3f(0.60f, 0.60f, 0.60f);

            glBegin(GL_QUADS);
                glVertex3f((float)x,     -0.01f, (float)z);
                glVertex3f((float)x + 1, -0.01f, (float)z);
                glVertex3f((float)x + 1, -0.01f, (float)z + 1);
                glVertex3f((float)x,     -0.01f, (float)z + 1);
            glEnd();
        }
    }

    glColor3f(0.05f, 0.05f, 0.05f);
    glLineWidth(1.0f);

    glBegin(GL_LINES);
        for (int i = 0; i <= MAZE_WIDTH; i++) {
            glVertex3f((float)i, -0.005f, 0.0f);
            glVertex3f((float)i, -0.005f, (float)MAZE_HEIGHT);
        }

        for (int j = 0; j <= MAZE_HEIGHT; j++) {
            glVertex3f(0.0f, -0.005f, (float)j);
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
                //Warna dasar
                GLfloat brick[] = {0.55f, 0.18f, 0.10f, 1.0f};
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, brick);

                glPushMatrix();
                    glTranslatef(x + 0.5f, 0.5f, z + 0.5f);
                    drawCube(1.0f);
                glPopMatrix();

                //Gamar kotak kotak berbntuk bata
                glDisable(GL_LIGHTING);
                glColor3f(0.05f, 0.02f, 0.01f);

                glPushMatrix();
                glTranslatef((float)x, 0.0f, (float)z);

                // Garis horizontal bata
                for (float y = 0.2f; y < 1.0f; y += 0.2f) {
                    glBegin(GL_LINES);

                    glVertex3f(0.01f, y, -0.001f);
                    glVertex3f(0.99f, y, -0.001f);

                    glVertex3f(0.01f, y, 1.001f);
                    glVertex3f(0.99f, y, 1.001f);

                    glVertex3f(-0.001f, y, 0.01f);
                    glVertex3f(-0.001f, y, 0.99f);

                    glVertex3f(1.001f, y, 0.01f);
                    glVertex3f(1.001f, y, 0.99f);

                    glEnd();
                }

                    // Garis vertikal bata
                    for (float y = 0.0f; y < 1.0f; y += 0.2f) {
                            float offset = ((int)(y * 10) % 4 == 0) ? 0.25f : 0.5f;

                    for (float a = offset; a < 1.0f; a += 0.5f) {
                        glBegin(GL_LINES);

                        glVertex3f(a, y, -0.001f);
                        glVertex3f(a, y + 0.2f, -0.001f);

                        glVertex3f(a, y, 1.001f);
                        glVertex3f(a, y + 0.2f, 1.001f);

                        glVertex3f(-0.001f, y, a);
                        glVertex3f(-0.001f, y + 0.2f, a);

                        glVertex3f(1.001f, y, a);
                        glVertex3f(1.001f, y + 0.2f, a);

                    glEnd();
                    }
                }

            glPopMatrix();
glEnable(GL_LIGHTING);
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
