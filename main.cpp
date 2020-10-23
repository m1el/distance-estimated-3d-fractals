#include <stdio.h>
#include <math.h>
#include <stdint.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "./stb_image_write.h"

struct vec3
{
    float x, y, z;
    vec3(float x, float y, float z):
        x(x), y(y), z(z)
    {}
};

vec3 operator+(vec3 a, vec3 b)
{
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

vec3 operator-(vec3 a, vec3 b)
{
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

vec3 operator*(float s, vec3 a)
{
    return {a.x * s, a.y * s, a.z * s};
}

template <typename T>
T max(T x, T y)
{
    return x < y ? y : x;
}

template <typename T>
T min(T x, T y)
{
    return x > y ? y : x;
}

template <typename T>
T ilerp(T start, T end, T x)
{
    if (x < start) {
        return 0.0;
    } else if (x > end) {
        return 1.0;
    } else {
        return (x - start) / (end - start);
    }
}


float length(vec3 a)
{
    return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
}

vec3 normalize(vec3 a)
{
    const float l = length(a);
    return vec3(a.x / l, a.y / l, a.z / l);
}

const float TAU = 6.28318530718f;

const int MAXIMUM_RAY_STEPS = 69 * 2;
const float MINIMUM_DISTANCE = 0.5f;

const float EYE_DISTANCE = 150.0f;

const int PIXEL_COLUMNS = 800;
const int PIXEL_ROWS = 600;
const float PIXEL_WIDTH = 0.2f;
const float PIXEL_HEIGHT = 0.2f;
const float SCREEN_WIDTH = (float) PIXEL_COLUMNS * PIXEL_WIDTH;
const float SCREEN_HEIGHT = (float) PIXEL_ROWS * PIXEL_HEIGHT;

const float R = (float) PIXEL_COLUMNS / 2.0f;
const vec3 C = {0.0f, 0.0f, EYE_DISTANCE * 5.0f + R};

float solid_sphere(vec3 p)
{
    return max(0.0f, length(p - C) - R);
}

float hollow_sphere(vec3 p)
{
    return fabsf(length(p) - R);
}

vec3 offset = { 0.0, 0.0, 0.0 };
float combined_spheres(vec3 p)
{
    return min(
            length(p - C - offset) - R,
            length(p - C + offset) - R);
}

struct RGBA
{
    uint8_t r, g, b, a;
};

template <typename DistanceEstimator>
RGBA trace(vec3 from, vec3 direction, DistanceEstimator distance_estimator)
{
    float totalDistance = 0.0f;

    int steps;
    vec3 point = from;
    for (steps = 0; steps < MAXIMUM_RAY_STEPS; ++steps) {
        float distance = distance_estimator(point);
        if (distance < MINIMUM_DISTANCE) break;
        point = point + distance * direction;
        totalDistance += distance;
    }

    float maxDistance = R * 3.0f;
    float minDistance = C.z - R * 1.5f;
    float t = 1.0f - ilerp(minDistance, maxDistance, totalDistance);
    return {
        (uint8_t)(255.0f * t),
        (uint8_t)(255.0f * t),
        (uint8_t)(255.0f * t),
        255
    };
}

const RGBA RED = {255, 0, 0, 255};

RGBA canvas[PIXEL_COLUMNS * PIXEL_ROWS] = {};

template <typename DistanceEstimator>
void render_to_file(const char *filepath, DistanceEstimator distance_estimator)
{
    for (int row = 0; row < PIXEL_ROWS; ++row) {
        for (int column = 0; column < PIXEL_COLUMNS; ++column) {
            float x = column * PIXEL_WIDTH  - SCREEN_WIDTH  * 0.5f + PIXEL_WIDTH  * 0.5f;
            float y = row    * PIXEL_HEIGHT - SCREEN_HEIGHT * 0.5f + PIXEL_HEIGHT * 0.5f;
            float z = EYE_DISTANCE;
            RGBA t = trace(vec3(0.0, 0.0, 0.0), normalize(vec3(x, y, z)), distance_estimator);
            canvas[row * PIXEL_COLUMNS + column] = t;
        }
    }

    stbi_write_png(filepath, PIXEL_COLUMNS, PIXEL_ROWS, 4, canvas, sizeof(RGBA) * PIXEL_COLUMNS);
}

int main(int argc, char *argv[])
{
    for (int i = 0; i < 20; i++) {
        float angle = ((float)i) / 20.0f * TAU;
        offset = vec3(sinf(angle) * (R / 2.0f), 0.0, cosf(angle) * (R / 2.0f));
        char buf[128];
        sprintf(&buf[0], "output-%02d.png", i);
        render_to_file(buf, combined_spheres);
    }

    return 0;
}
