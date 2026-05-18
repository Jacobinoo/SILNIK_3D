#include "Math3D.h"

Vec3::Vec3() : x(0.0f), y(0.0f), z(0.0f) {}

Vec3::Vec3(float xValue, float yValue, float zValue) : x(xValue), y(yValue), z(zValue) {}

Vec3 Vec3::operator+(const Vec3& other) const { return Vec3(x + other.x, y + other.y, z + other.z); }
Vec3 Vec3::operator-(const Vec3& other) const { return Vec3(x - other.x, y - other.y, z - other.z); }
Vec3 Vec3::operator*(float scalar) const { return Vec3(x * scalar, y * scalar, z * scalar); }
Vec3 Vec3::operator/(float scalar) const { return Vec3(x / scalar, y / scalar, z / scalar); }
Vec3& Vec3::operator+=(const Vec3& other) { x += other.x; y += other.y; z += other.z; return *this; }
Vec3& Vec3::operator-=(const Vec3& other) { x -= other.x; y -= other.y; z -= other.z; return *this; }
Vec3& Vec3::operator*=(float scalar) { x *= scalar; y *= scalar; z *= scalar; return *this; }

Vec3 operator*(float scalar, const Vec3& vector) { return vector * scalar; }

float dot(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

Vec3 cross(const Vec3& a, const Vec3& b) {
    return Vec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

float length(const Vec3& vector) { return std::sqrt(dot(vector, vector)); }

Vec3 normalize(const Vec3& vector) {
    float len = length(vector);
    if (len <= 0.000001f) {
        return Vec3(0.0f, 0.0f, 0.0f);
    }
    return vector / len;
}

Mat4::Mat4() {
    for (float& value : m) {
        value = 0.0f;
    }
}

Mat4::Mat4(float diagonalValue) : Mat4() {
    at(0, 0) = diagonalValue;
    at(1, 1) = diagonalValue;
    at(2, 2) = diagonalValue;
    at(3, 3) = diagonalValue;
}

float& Mat4::at(int row, int col) { return m[col * 4 + row]; }
float Mat4::at(int row, int col) const { return m[col * 4 + row]; }

Mat4 Mat4::identity() { return Mat4(1.0f); }

Mat4 Mat4::translation(const Vec3& offset) {
    Mat4 result = Mat4::identity();
    result.at(0, 3) = offset.x;
    result.at(1, 3) = offset.y;
    result.at(2, 3) = offset.z;
    return result;
}

Mat4 Mat4::scale(const Vec3& factor) {
    Mat4 result;
    result.at(0, 0) = factor.x;
    result.at(1, 1) = factor.y;
    result.at(2, 2) = factor.z;
    result.at(3, 3) = 1.0f;
    return result;
}

Mat4 Mat4::rotationX(float radians) {
    Mat4 result = Mat4::identity();
    float c = std::cos(radians);
    float s = std::sin(radians);
    result.at(1, 1) = c;
    result.at(1, 2) = -s;
    result.at(2, 1) = s;
    result.at(2, 2) = c;
    return result;
}

Mat4 Mat4::rotationY(float radians) {
    Mat4 result = Mat4::identity();
    float c = std::cos(radians);
    float s = std::sin(radians);
    result.at(0, 0) = c;
    result.at(0, 2) = s;
    result.at(2, 0) = -s;
    result.at(2, 2) = c;
    return result;
}

Mat4 Mat4::rotationZ(float radians) {
    Mat4 result = Mat4::identity();
    float c = std::cos(radians);
    float s = std::sin(radians);
    result.at(0, 0) = c;
    result.at(0, 1) = -s;
    result.at(1, 0) = s;
    result.at(1, 1) = c;
    return result;
}

Mat4 Mat4::perspective(float fovRadians, float aspect, float nearPlane, float farPlane) {
    Mat4 result;
    float f = 1.0f / std::tan(fovRadians * 0.5f);
    result.at(0, 0) = f / aspect;
    result.at(1, 1) = f;
    result.at(2, 2) = (farPlane + nearPlane) / (nearPlane - farPlane);
    result.at(2, 3) = (2.0f * farPlane * nearPlane) / (nearPlane - farPlane);
    result.at(3, 2) = -1.0f;
    return result;
}

Mat4 Mat4::orthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
    Mat4 result = Mat4::identity();
    result.at(0, 0) = 2.0f / (right - left);
    result.at(1, 1) = 2.0f / (top - bottom);
    result.at(2, 2) = -2.0f / (farPlane - nearPlane);
    result.at(0, 3) = -(right + left) / (right - left);
    result.at(1, 3) = -(top + bottom) / (top - bottom);
    result.at(2, 3) = -(farPlane + nearPlane) / (farPlane - nearPlane);
    return result;
}

Mat4 Mat4::lookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
    Vec3 forward = normalize(center - eye);
    Vec3 side    = normalize(cross(forward, up));
    Vec3 trueUp  = cross(side, forward);

    // Macierz widoku: kolejne WIERSZE to bazowe wektory kamery w przestrzeni
    // swiata (right, up, -forward). Czwarta kolumna to translacja eye do origin.
    // (Wczesniejsza wersja zapisywala wektory jako KOLUMNY, czyli macierz
    //  byla transponowana = wrenderowanie przesuniete wzgledem rzeczywistych
    //  pozycji w przestrzeni 3D.)
    Mat4 result = Mat4::identity();
    // Wiersz 0: prawo kamery (X po transformacji)
    result.at(0, 0) = side.x;
    result.at(0, 1) = side.y;
    result.at(0, 2) = side.z;
    // Wiersz 1: gora kamery (Y po transformacji)
    result.at(1, 0) = trueUp.x;
    result.at(1, 1) = trueUp.y;
    result.at(1, 2) = trueUp.z;
    // Wiersz 2: -forward (Z po transformacji - OpenGL patrzy w -Z)
    result.at(2, 0) = -forward.x;
    result.at(2, 1) = -forward.y;
    result.at(2, 2) = -forward.z;
    // Kolumna 3: -R * eye (przesuniecie aby eye trafil w origin kamery)
    result.at(0, 3) = -dot(side,    eye);
    result.at(1, 3) = -dot(trueUp,  eye);
    result.at(2, 3) =  dot(forward, eye);
    return result;
}

const float* Mat4::data() const { return m; }

Mat4 operator*(const Mat4& left, const Mat4& right) {
    Mat4 result;
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            float sum = 0.0f;
            for (int i = 0; i < 4; ++i) {
                sum += left.at(row, i) * right.at(i, col);
            }
            result.at(row, col) = sum;
        }
    }
    return result;
}

Vec3 transformPoint(const Mat4& matrix, const Vec3& point) {
    float x = matrix.at(0, 0) * point.x + matrix.at(0, 1) * point.y + matrix.at(0, 2) * point.z + matrix.at(0, 3);
    float y = matrix.at(1, 0) * point.x + matrix.at(1, 1) * point.y + matrix.at(1, 2) * point.z + matrix.at(1, 3);
    float z = matrix.at(2, 0) * point.x + matrix.at(2, 1) * point.y + matrix.at(2, 2) * point.z + matrix.at(2, 3);
    float w = matrix.at(3, 0) * point.x + matrix.at(3, 1) * point.y + matrix.at(3, 2) * point.z + matrix.at(3, 3);
    if (std::fabs(w) > 0.000001f) {
        return Vec3(x / w, y / w, z / w);
    }
    return Vec3(x, y, z);
}

Vec3 transformVector(const Mat4& matrix, const Vec3& vector) {
    return Vec3(
        matrix.at(0, 0) * vector.x + matrix.at(0, 1) * vector.y + matrix.at(0, 2) * vector.z,
        matrix.at(1, 0) * vector.x + matrix.at(1, 1) * vector.y + matrix.at(1, 2) * vector.z,
        matrix.at(2, 0) * vector.x + matrix.at(2, 1) * vector.y + matrix.at(2, 2) * vector.z
    );
}

bool raySphereIntersect(const Vec3& origin, const Vec3& direction,
                        const Vec3& center, float radius, float& outT) {
    // |origin + t*dir - center|^2 = radius^2
    // (t*dir + oc).(t*dir + oc) = r^2   gdzie oc = origin - center
    // a*t^2 + 2*b*t + c = 0
    Vec3  oc   = origin - center;
    float a    = dot(direction, direction);
    float b    = dot(oc, direction);
    float c    = dot(oc, oc) - radius * radius;
    float disc = b * b - a * c;
    if (disc < 0.0f) return false;

    float sqrtD = std::sqrt(disc);
    float t1 = (-b - sqrtD) / a;
    float t2 = (-b + sqrtD) / a;

    if (t1 > 0.0001f) { outT = t1; return true; }
    if (t2 > 0.0001f) { outT = t2; return true; }
    return false;
}
