/**
 * @file Math3D.h
 * @brief Biblioteka matematyczna dla silnika - wektory, macierze, intersekcje.
 *
 * Wszystko jest pisane od zera. Macierze 4x4 trzymane sa w column-major
 * tak jak OpenGL tego oczekuje (bo glLoadMatrixf czyta wlasnie column-major).
 * Funkcje ray-X-intersect sluza do raycastingu w grze przy strzelaniu.
 */
#ifndef MATH3D_H
#define MATH3D_H

#include <cmath>

/**
 * @brief Wektor 3D (x, y, z). Reprezentuje punkt albo kierunek.
 *
 * Plain struct z floatami, bez padding'u, dzieki temu mozna castowac
 * na float[3] gdyby trzeba bylo wysylac do OpenGL jako tablice.
 */
struct Vec3 {
    float x; ///< skladowa X
    float y; ///< skladowa Y
    float z; ///< skladowa Z

    /** @brief Konstruktor domyslny - wektor zerowy (0, 0, 0). */
    Vec3();
    /** @brief Konstruktor z trzema floatami. */
    Vec3(float xValue, float yValue, float zValue);

    Vec3 operator+(const Vec3& other) const;  ///< dodawanie skladnikowe
    Vec3 operator-(const Vec3& other) const;  ///< odejmowanie skladnikowe
    Vec3 operator*(float scalar) const;       ///< mnozenie przez skalar
    Vec3 operator/(float scalar) const;       ///< dzielenie przez skalar
    Vec3& operator+=(const Vec3& other);
    Vec3& operator-=(const Vec3& other);
    Vec3& operator*=(float scalar);
};

/** @brief Mnozenie skalar*wektor (forma scalar*vec dla wygody). */
Vec3 operator*(float scalar, const Vec3& vector);

/** @brief Iloczyn skalarny (dot product) dwoch wektorow. */
float dot(const Vec3& a, const Vec3& b);

/** @brief Iloczyn wektorowy (cross product). Daje wektor prostopadly do obu argumentow. */
Vec3 cross(const Vec3& a, const Vec3& b);

/** @brief Dlugosc (norma euklidesowa) wektora. */
float length(const Vec3& vector);

/** @brief Wektor o tej samej kierunku ale dlugosci 1. Wektor zerowy zwroci (0,0,0). */
Vec3 normalize(const Vec3& vector);

/**
 * @brief Macierz 4x4 (column-major, kompatybilna z OpenGL).
 *
 * Element (row, col) jest pod indeksem m[col*4 + row]. To wymagane przez
 * OpenGL bo glLoadMatrixf zaklada wlasnie taki uklad - jak by sie pomylilo
 * to widok bylby kompletnie przekrecony (mielismy z tym bug w lookAt zreszta).
 */
struct Mat4 {
    float m[16]; ///< 16 floatow column-major, lewa-gora -> prawa-dol

    Mat4();                              ///< wypelnia zerami
    explicit Mat4(float diagonalValue);  ///< macierz diagonalna (np. identity gdy =1)

    /** @brief Dostep modyfikujacy do elementu (row, col). */
    float& at(int row, int col);
    /** @brief Dostep do odczytu elementu (row, col). */
    float at(int row, int col) const;

    static Mat4 identity();                       ///< macierz jednostkowa
    static Mat4 translation(const Vec3& offset);  ///< macierz translacji
    static Mat4 scale(const Vec3& factor);        ///< macierz skalowania (nieuniformnego)
    static Mat4 rotationX(float radians);         ///< rotacja wokol osi X
    static Mat4 rotationY(float radians);         ///< rotacja wokol osi Y
    static Mat4 rotationZ(float radians);         ///< rotacja wokol osi Z

    /**
     * @brief Macierz projekcji perspektywicznej.
     * @param fovRadians pole widzenia w pionie (w radianach)
     * @param aspect stosunek width/height okna
     * @param nearPlane odleglosc do bliskiej plaszczyzny clipping
     * @param farPlane odleglosc do dalekiej
     */
    static Mat4 perspective(float fovRadians, float aspect, float nearPlane, float farPlane);

    /** @brief Macierz projekcji ortogonalnej (do HUD i podobnych). */
    static Mat4 orthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);

    /**
     * @brief Macierz widoku - kamera w punkcie eye patrzy na center, gora "up".
     *
     * Wiersze sa wektorami bazowymi kamery (side, up, -forward) - kluczowe
     * zeby tu nie pomylic kolumn z wierszami bo dostaniemy odwrotnosc.
     */
    static Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up);

    /** @brief Wskaznik do tablicy 16 floatow (do glLoadMatrixf / glMultMatrixf). */
    const float* data() const;
};

/** @brief Mnozenie macierzy 4x4. result[r][c] = sum_i left[r][i] * right[i][c]. */
Mat4 operator*(const Mat4& left, const Mat4& right);

/** @brief Transformacja punktu macierza (uwzglednia translacje, dzielenie przez w). */
Vec3 transformPoint(const Mat4& matrix, const Vec3& point);

/** @brief Transformacja wektora (kierunku - bez translacji, w=0). */
Vec3 transformVector(const Mat4& matrix, const Vec3& vector);

/**
 * @brief Test przeciecia promienia ze sfera.
 *
 * Wyprowadzenie: |origin + t*dir - center|^2 = radius^2, to daje rownanie
 * kwadratowe w t. Bierzemy najmniejsze dodatnie rozwiazanie.
 *
 * @param origin punkt startowy promienia
 * @param direction kierunek (powinien byc znormalizowany)
 * @param center srodek sfery
 * @param radius promien sfery
 * @param[out] outT parametr t przeciecia jesli zwroci true
 * @return true gdy istnieje przeciecie z t > 0
 */
bool raySphereIntersect(const Vec3& origin, const Vec3& direction,
                        const Vec3& center, float radius, float& outT);

/**
 * @brief Test przeciecia promienia z pionowym walcem (os walca || Y).
 *
 * Walec stoi na podstawie 'base' i rozciaga sie do base.y + height.
 * @param base srodek dolnej podstawy walca
 * @param radius promien walca
 * @param height wysokosc walca
 */
bool rayCylinderIntersect(const Vec3& origin, const Vec3& direction,
                          const Vec3& base, float radius, float height,
                          float& outT);

/**
 * @brief Test przeciecia promienia ze stozkiem (os || Y, wierzcholek u gory).
 *
 * Promien stozka przy wysokosci dy nad podstawa wynosi r(dy) = baseRadius * (1 - dy/height).
 * Po podstawieniu do (x^2 + z^2 = r(dy)^2) wychodzi rownanie kwadratowe w t.
 */
bool rayConeIntersect(const Vec3& origin, const Vec3& direction,
                      const Vec3& base, float baseRadius, float height,
                      float& outT);

/**
 * @brief Test przeciecia promienia z prostopadloscianem osi-aligned (AABB).
 *
 * Metoda "slab" - dla kazdej z 3 osi liczymy przedzial t [tmin, tmax]
 * gdzie promien jest miedzy plaszczyznami slab'a. Przeciecia = intersect
 * wszystkich 3 przedzialow.
 */
bool rayAABBIntersect(const Vec3& origin, const Vec3& direction,
                      const Vec3& boxMin, const Vec3& boxMax,
                      float& outT);

#endif
