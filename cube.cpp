#include <iostream>
#include <cmath>
#include <thread>
#include <algorithm>
#include <chrono>

using namespace std;
using namespace chrono;

constexpr int WIDTH = 100;
constexpr int HEIGHT = 50;
constexpr int AREA = WIDTH * HEIGHT;

constexpr float CUBE_WIDTH = 15.0f;
constexpr float DISTANCE_FROM_CAM = 50.0f;
constexpr float K1 = 30.0f; // FOV 
constexpr float INCREMENT_SPEED = 0.1f;
constexpr float INCREMENT_ANGLE = 0.005f;
constexpr int SLEEP_TIME_US = 10000000;

// Buffer
char buffer[AREA];
float z_buffer[AREA]; 

// Rotation state
float angleX = 0.0f, angleY = 0.0f, angleZ = 0.0f;

// Compute sin/cos
struct RotationCache {
    float sinA, cosA, sinB, cosB, sinC, cosC;
    
    void update(float A, float B, float C) {
        sinA = sin(A); cosA = cos(A);
        sinB = sin(B); cosB = cos(B);
        sinC = sin(C); cosC = cos(C);
    }
} rot;

// Rotation matrix calculations
inline float calculateX(float i, float j, float k) {
    return j * rot.sinA * rot.sinB * rot.cosC - k * rot.cosA * rot.sinB * rot.cosC +
           j * rot.cosA * rot.sinC + k * rot.sinA * rot.sinC + i * rot.cosB * rot.cosC;
}

inline float calculateY(float i, float j, float k) {
    return j * rot.cosA * rot.cosC + k * rot.sinA * rot.cosC -
           j * rot.sinA * rot.sinB * rot.sinC + k * rot.cosA * rot.sinB * rot.sinC -
           i * rot.cosB * rot.sinC;
}

inline float calculateZ(float i, float j, float k) {
    return k * rot.cosA * rot.cosB - j * rot.sinA * rot.cosB + i * rot.sinB;
}

// Project and draw single point
void renderPoint(float cube_x, float cube_y, float cube_z, char symbol) {
    // Rotate point in 3D space
    float x = calculateX(cube_x, cube_y, cube_z);
    float y = calculateY(cube_x, cube_y, cube_z);
    float z = calculateZ(cube_x, cube_y, cube_z) + DISTANCE_FROM_CAM;
    
    // Perspective projection
    float ooz = 1.0f / z;  // One over Z
    int xp = static_cast<int>(WIDTH / 2 + K1 * ooz * x * 2);
    int yp = static_cast<int>(HEIGHT / 2 + K1 * ooz * y);
    
    // Bounds check and Z-buffer test
    int idx = xp + yp * WIDTH;
    if (idx >= 0 && idx < AREA && xp >= 0 && xp < WIDTH) {
        if (ooz > z_buffer[idx]) {
            z_buffer[idx] = ooz;
            buffer[idx] = symbol;
        }
    }
}

void renderCube() {
    for (float i = -CUBE_WIDTH; i < CUBE_WIDTH; i += INCREMENT_SPEED) {
        for (float j = -CUBE_WIDTH; j < CUBE_WIDTH; j += INCREMENT_SPEED) {
            renderPoint(i, j, -CUBE_WIDTH, '@');  // Back
            renderPoint(CUBE_WIDTH, j, i, '$');   // Right
            renderPoint(-CUBE_WIDTH, j, -i, '~'); // Left
            renderPoint(-i, j, CUBE_WIDTH, '#');  // Front
            renderPoint(i, -CUBE_WIDTH, -j, ';'); // Bottom
            renderPoint(i, CUBE_WIDTH, j, '+');   // Top
        }
    }
}

int main() {
    // Clear screen and hide cursor
    cout << "\x1b[2J\x1b[?25l";
    
    while (true) {
        // Clear buffers
        fill(buffer, buffer + AREA, ' ');
        fill(z_buffer, z_buffer + AREA, 0.0f);
        
        // Update rotation cache
        rot.update(angleX, angleY, angleZ);
        
        // Render cube
        renderCube();
        
        // Build frame string - improved fps :D
        std::string frame;
        frame.reserve(AREA + HEIGHT); // Pre-allocate for speed
        for (int i = 0; i < AREA; ++i) {
            frame += (i % WIDTH ? buffer[i] : '\n');
        }

        // Display buffer in one call
        cout << "\x1b[H" << frame;

        // Update rotation angles
        angleX += INCREMENT_ANGLE * 3;
        angleY += INCREMENT_ANGLE * 2; // change these to tweak the spinning
        angleZ += INCREMENT_ANGLE / 2;

        this_thread::yield();
    }
    
    return 0;
}

// By Ezi0 