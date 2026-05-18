#ifndef MATH3D_H
#define MATH3D_H

#include <cmath>

struct Vec3 {
    float x;
    float y;
    float z;

    Vec3();
    Vec3(float xValue, float yValue, float zValue);

    Vec3 operator+(const Vec3& other) const;
    Vec3 operator-(const Vec3& other) const;
    Vec3 operator*(float scalar) const;
    Vec3 operator/(float scalar) const;
    Vec3& operator+=(const Vec3& other);
    Vec3& operator-=(const Vec3& other);
    Vec3& operator*=(float scalar);
};

Vec3 operator*(float scalar, const Vec3& vector);
float dot(const Vec3& a, const Vec3& b);
Vec3 cross(const Vec3& a, const Vec3& b);
float length(const Vec3& vector);
Vec3 normalize(const Vec3& vector);

struct Mat4 {
    float m[16];

    Mat4();
    explicit Mat4(float diagonalValue);

    float& at(int row, int col);
    float at(int row, int col) const;

    static Mat4 identity();
    static Mat4 translation(const Vec3& offset);
    static Mat4 scale(const Vec3& factor);
    static Mat4 rotationX(float radians);
    static Mat4 rotationY(float radians);
    static Mat4 rotationZ(float radians);
    static Mat4 perspective(float fovRadians, float aspect, float nearPlane, float farPlane);
    static Mat4 orthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);
    static Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up);

    const float* data() const;
};

Mat4 operator*(const Mat4& left, const Mat4& right);
Vec3 transformPoint(const Mat4& matrix, const Vec3& point);
Vec3 transformVector(const Mat4& matrix, const Vec3& vector);

// Test przeciecia promienia ze sfera (ray-sphere intersection).
// Promien: P(t) = origin + t * direction, t >= 0.
// Zwraca true jesli istnieje przeciecie z t > 0; outT = najblizsze.
bool raySphereIntersect(const Vec3& origin, const Vec3& direction,
                        const Vec3& center, float radius, float& outT);

#endif