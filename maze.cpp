/**
 * @file maze.cpp
 * @brief Implementasi fungsi maze: collision detection, utilitas copy,
 *        dan rendering dunia 3D (ground, dinding, board soal).
 */

#include "maze.h"
#include "globals.h"
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <vector>
#if defined(_WIN32) && !defined(CALLBACK)
#define CALLBACK __stdcall
#endif
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

static GLuint skyTexture = 0;
static GLuint wallTexture = 0;
static GLuint faceTexture = 0;

static uint16_t readU16(const unsigned char* p) {
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t readU32(const unsigned char* p) {
    return (uint32_t)p[0] |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static int lowerPowerOfTwo(int value, int limit) {
    int result = 1;
    while (result <= value / 2 && result < limit) result *= 2;
    return result;
}

static bool loadBmpTexture(const char* filename, GLuint* textureId, bool repeatTexture) {
    FILE* file = fopen(filename, "rb");
    if (!file) return false;

    unsigned char header[54];
    if (fread(header, 1, sizeof(header), file) != sizeof(header) ||
        header[0] != 'B' || header[1] != 'M') {
        fclose(file);
        return false;
    }

    uint32_t dataOffset = readU32(header + 10);
    int32_t width       = (int32_t)readU32(header + 18);
    int32_t height      = (int32_t)readU32(header + 22);
    uint16_t planes     = readU16(header + 26);
    uint16_t bits       = readU16(header + 28);
    uint32_t compression = readU32(header + 30);

    if (width <= 0 || height == 0 || planes != 1 || bits != 24 || compression != 0) {
        fclose(file);
        return false;
    }

    int imageHeight = height < 0 ? -height : height;
    size_t rowSize = ((size_t)width * 3 + 3) & ~((size_t)3);
    std::vector<unsigned char> row(rowSize);
    std::vector<unsigned char> pixels((size_t)width * imageHeight * 3);

    if (fseek(file, (long)dataOffset, SEEK_SET) != 0) {
        fclose(file);
        return false;
    }

    for (int fileRow = 0; fileRow < imageHeight; fileRow++) {
        if (fread(row.data(), 1, rowSize, file) != rowSize) {
            fclose(file);
            return false;
        }

        int targetRow = height > 0 ? fileRow : imageHeight - 1 - fileRow;
        unsigned char* target = pixels.data() + (size_t)targetRow * width * 3;
        for (int x = 0; x < width; x++) {
            target[x * 3 + 0] = row[x * 3 + 2];
            target[x * 3 + 1] = row[x * 3 + 1];
            target[x * 3 + 2] = row[x * 3 + 0];
        }
    }
    fclose(file);

    GLint maxTextureSize = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    int textureLimit = maxTextureSize > 0 ? maxTextureSize : 1024;
    if (textureLimit > 2048) textureLimit = 2048;

    int textureWidth = lowerPowerOfTwo(width, textureLimit);
    int textureHeight = lowerPowerOfTwo(imageHeight, textureLimit);
    std::vector<unsigned char> resized;
    const unsigned char* texturePixels = pixels.data();

    if (textureWidth != width || textureHeight != imageHeight) {
        resized.resize((size_t)textureWidth * textureHeight * 3);
        for (int y = 0; y < textureHeight; y++) {
            int sourceY = y * imageHeight / textureHeight;
            for (int x = 0; x < textureWidth; x++) {
                int sourceX = x * width / textureWidth;
                const unsigned char* source =
                    pixels.data() + ((size_t)sourceY * width + sourceX) * 3;
                unsigned char* target =
                    resized.data() + ((size_t)y * textureWidth + x) * 3;
                target[0] = source[0];
                target[1] = source[1];
                target[2] = source[2];
            }
        }
        texturePixels = resized.data();
    }

    if (*textureId) glDeleteTextures(1, textureId);
    glGenTextures(1, textureId);
    glBindTexture(GL_TEXTURE_2D, *textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeatTexture ? GL_REPEAT : GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeatTexture ? GL_REPEAT : GL_CLAMP);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, texturePixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    GLenum textureError = glGetError();
    glBindTexture(GL_TEXTURE_2D, 0);
    if (textureError != GL_NO_ERROR) {
        glDeleteTextures(1, textureId);
        *textureId = 0;
        return false;
    }
    return true;
}

bool loadSkyTexture(const char* filename) {
    return loadBmpTexture(filename, &skyTexture, false);
}

bool loadWallTexture(const char* filename) {
    return loadBmpTexture(filename, &wallTexture, true);
}

bool loadFaceTexture(const char* filename) {
    return loadBmpTexture(filename, &faceTexture, false);
}

static void drawExitFacePoster() {
    if (!faceTexture) return;

    GLboolean lightingEnabled, cullFaceEnabled, texture2DEnabled;
    glGetBooleanv(GL_LIGHTING, &lightingEnabled);
    glGetBooleanv(GL_CULL_FACE, &cullFaceEnabled);
    glGetBooleanv(GL_TEXTURE_2D, &texture2DEnabled);

    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, faceTexture);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.28f, 0.24f, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.28f, 0.24f, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.28f, 0.80f, 0.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.28f, 0.80f, 0.0f);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    if (texture2DEnabled) glEnable(GL_TEXTURE_2D); else glDisable(GL_TEXTURE_2D);
    if (cullFaceEnabled) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
    if (lightingEnabled) glEnable(GL_LIGHTING); else glDisable(GL_LIGHTING);
}

void renderSky() {
    if (!skyTexture) return;

    const int slices = 48;
    const int stacks = 16;
    const float radius = 45.0f;
    const float horizonY = 0.7f;
    const float halfPi = 1.57079632679f;
    const float twoPi = 6.28318530718f;

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, skyTexture);
    glColor3f(1.0f, 1.0f, 1.0f);

    glPushMatrix();
        glTranslatef(playerX, horizonY, playerZ);

        // Kubah setengah bola: tidak memiliki sisi vertikal atau sudut kotak.
        for (int stack = 0; stack < stacks; stack++) {
            float v0 = (float)stack / stacks;
            float v1 = (float)(stack + 1) / stacks;
            float elevation0 = v0 * halfPi;
            float elevation1 = v1 * halfPi;
            float y0 = sinf(elevation0) * radius;
            float y1 = sinf(elevation1) * radius;
            float ring0 = cosf(elevation0) * radius;
            float ring1 = cosf(elevation1) * radius;

            glBegin(GL_QUAD_STRIP);
            for (int slice = 0; slice <= slices; slice++) {
                float u = (float)slice / slices;
                float angle = u * twoPi;
                float sn = sinf(angle);
                float cs = cosf(angle);

                glTexCoord2f(u, v1);
                glVertex3f(sn * ring1, y1, -cs * ring1);
                glTexCoord2f(u, v0);
                glVertex3f(sn * ring0, y0, -cs * ring0);
            }
            glEnd();
        }
    glPopMatrix();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void cleanupSkyTexture() {
    if (skyTexture) {
        glDeleteTextures(1, &skyTexture);
        skyTexture = 0;
    }
    if (wallTexture) {
        glDeleteTextures(1, &wallTexture);
        wallTexture = 0;
    }
    if (faceTexture) {
        glDeleteTextures(1, &faceTexture);
        faceTexture = 0;
    }
}

// =============================================
// COPY MAZE ARRAY
// =============================================
void copyMazeArray(int dest[20][20], int src[20][20]) {
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
        glTexCoord2f(0.0f, 0.0f);
        glVertex3fv(verts[f * 4 + 0]);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3fv(verts[f * 4 + 1]);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3fv(verts[f * 4 + 2]);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3fv(verts[f * 4 + 3]);
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
    glEnable(GL_LIGHTING);

    GLfloat floorMat[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat floorSpec[] = {0.12f, 0.12f, 0.12f, 1.0f};
    GLfloat floorShin[] = {12.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, floorMat);
    glMaterialfv(GL_FRONT, GL_SPECULAR, floorSpec);
    glMaterialfv(GL_FRONT, GL_SHININESS, floorShin);

    for (int x = 0; x < MAZE_WIDTH; x++) {
        for (int z = 0; z < MAZE_HEIGHT; z++) {
            glBegin(GL_QUADS);
                glNormal3f(0.0f, 1.0f, 0.0f);
                glVertex3f((float)x,     -0.01f, (float)z);
                glVertex3f((float)x + 1, -0.01f, (float)z);
                glVertex3f((float)x + 1, -0.01f, (float)z + 1);
                glVertex3f((float)x,     -0.01f, (float)z + 1);
            glEnd();
        }
    }

    glDisable(GL_LIGHTING);
    glColor3f(0.86f, 0.86f, 0.86f);
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
                GLfloat brick[] = {wallTexture ? 1.0f : 0.55f, wallTexture ? 1.0f : 0.18f, wallTexture ? 1.0f : 0.10f, 1.0f};
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, brick);

                if (wallTexture) {
                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, wallTexture);
                }

                glPushMatrix();
                    glTranslatef(x + 0.5f, 0.5f, z + 0.5f);
                    drawCube(1.0f);
                glPopMatrix();

                if (wallTexture) {
                    glBindTexture(GL_TEXTURE_2D, 0);
                    glDisable(GL_TEXTURE_2D);
                }

                if (!wallTexture) {
                    // Gambar garis bata manual hanya saat tekstur wall tidak tersedia.
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
            }

            // --- TILE 2: Goal (pintu keluar terbuka) ---
            else if (maze[z][x] == 2) {
                // Arahkan pintu melintang terhadap koridor yang menuju goal.
                bool corridorAlongZ =
                    (z > 0 && maze[z - 1][x] != 1) ||
                    (z + 1 < MAZE_HEIGHT && maze[z + 1][x] != 1);

                glPushMatrix();
                    glTranslatef(x + 0.5f, 0.0f, z + 0.5f);
                    if (!corridorAlongZ) glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

                    // Frame emas-merah: pusat sengaja dibiarkan kosong.
                    GLfloat frame[] = {0.75f, 0.12f, 0.04f, 1.0f};
                    GLfloat gold[]  = {1.00f, 0.62f, 0.08f, 1.0f};
                    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, frame);

                    float doorSides[] = {-0.43f, 0.43f};
                    for (float side : doorSides) {
                        glPushMatrix();
                            glTranslatef(side, 0.42f, 0.0f);
                            glScalef(0.14f, 0.84f, 0.14f);
                            drawCube(1.0f);
                        glPopMatrix();
                    }

                    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, gold);
                    glPushMatrix();
                        glTranslatef(0.0f, 0.92f, 0.0f);
                        glScalef(1.0f, 0.16f, 0.16f);
                        drawCube(1.0f);
                    glPopMatrix();

                    // Ambang rendah sebagai penanda bahwa pintu ini adalah exit.
                    glPushMatrix();
                        glTranslatef(0.0f, 0.025f, 0.0f);
                        glScalef(0.72f, 0.05f, 0.24f);
                        drawCube(1.0f);
                    glPopMatrix();

                    glPushMatrix();
                        glTranslatef(0.0f, 0.0f, 0.485f);
                        drawExitFacePoster();
                    glPopMatrix();
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
                        glTranslatef(0.0f, 0.34f, 0.0f);
                        glScalef(0.05f, 0.56f, 0.05f);
                        drawCube(1.0f);
                    glPopMatrix();

                    // Bingkai papan (lebih gelap dari tiang)
                    GLfloat darkwood[] = {0.4f, 0.2f, 0.05f, 1.0f};
                    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, darkwood);
                    glPushMatrix();
                        glTranslatef(0.0f, 0.66f, 0.035f);
                        glScalef(0.48f, 0.30f, 0.05f);
                        drawCube(1.0f);
                    glPopMatrix();

                    // Permukaan putih (layar soal)
                    GLfloat white[] = {1.0f, 1.0f, 1.0f, 1.0f};
                    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
                    glPushMatrix();
                        glTranslatef(0.0f, 0.66f, 0.063f);
                        glScalef(0.38f, 0.22f, 0.01f);
                        drawCube(1.0f);
                    glPopMatrix();

                    // Garis-garis biru sebagai dekorasi "tulisan" di layar
                    GLfloat lineblue[] = {0.1f, 0.2f, 0.8f, 1.0f};
                    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, lineblue);
                    for (int ln = 0; ln < 4; ln++) {
                        glPushMatrix();
                            // Setiap garis digeser 0.07 ke bawah
                            glTranslatef(0.0f, 0.72f - ln * 0.04f, 0.069f);
                            glScalef(0.26f, 0.008f, 0.005f);
                            drawCube(1.0f);
                        glPopMatrix();
                    }

                    // Aksen kuning (pointer kiri layar)
                    GLfloat yellow[] = {1.0f, 0.9f, 0.0f, 1.0f};
                    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, yellow);
                    glPushMatrix();
                        glTranslatef(0.12f, 0.66f, 0.070f);
                        glScalef(0.04f, 0.12f, 0.005f);
                        drawCube(1.0f);
                    glPopMatrix();

                glPopMatrix(); // Akhir transformasi board
            }
        }
    }
}
