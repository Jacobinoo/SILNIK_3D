/**
 * @file PrimitiveNode.h
 * @brief Prymitywy 3D (sześcian, walec, sfera, stożek, torus, plaszczyzna).
 *
 * Wszystkie sa wezlami sceny i wszystkie rysuja sie wlasnym kodem
 * (glBegin/glEnd), z wlasnoreznie liczonymi normalnymi i UV. Brak gluSphere
 * czy glutSolidCube - to bylo wymaganie projektu, ze grupa pisze geometrie
 * sama.
 */
#ifndef PRIMITIVENODE_H
#define PRIMITIVENODE_H

#include "Light.h"
#include "Texture.h"
#include <memory>

/**
 * @brief Klasa bazowa dla wszystkich rysowanych prymitywow.
 *
 * Trzyma material Phonga i opcjonalna teksture. Zaimplementowany
 * renderSelf() ustawia GL state (material, glMultMatrixf z worldMatrix,
 * binduje teksture jezeli jest) a potem wola czysto wirtualne
 * drawGeometry() ktore podklasa implementuje wlasnym glBegin/glEnd.
 */
class PrimitiveNode : public SceneNode {
public:
    /** @brief Konstruktor - przekazuje nazwe do SceneNode. */
    explicit PrimitiveNode(const std::string& name);

    void setMaterial(const Material& value);
    const Material& material() const;

    /** @brief Ustawia teksture (moze byc nullptr - wtedy bez tekstury). */
    void setTexture(const std::shared_ptr<Texture>& tex);
    std::shared_ptr<Texture> texture() const;

protected:
    /** @brief Ustawia material/teksture/transformacje i wola drawGeometry. */
    void renderSelf(const Mat4& worldMatrix) const override;
    /** @brief Czysto wirtualna - kazda podklasa musi wlasnoreznie rysowac geometrie. */
    virtual void drawGeometry() const = 0;

private:
    Material surfaceMaterial;
    std::shared_ptr<Texture> surfaceTexture;
};

/**
 * @brief Sześcian. Centrowany w origin, bok 'size'.
 *
 * Renderuje 6 quadow przez GL_QUADS, kazda sciana ma wlasna normalna
 * i pelne UV od (0,0) do (1,1).
 */
class CubeNode : public PrimitiveNode {
public:
    /** @brief Tworzy sześcian o boku 'size'. */
    explicit CubeNode(float size);
    void setSize(float value);
    float size() const;

protected:
    void drawGeometry() const override;

private:
    float cubeSize;
};

/**
 * @brief Walec. Oś == Y, podstawa w y=-h/2, gora w y=+h/2.
 *
 * Boczna powierzchnia: GL_TRIANGLE_STRIP po obwodzie, slices to liczba
 * podzialow obwodu (wiecej = gladsze). Dyski (gora i dol) jako TRIANGLE_FAN.
 * Normalne sciany bocznej sa prostopadle do osi Y a UV: u=kat, v=wysokosc.
 */
class CylinderNode : public PrimitiveNode {
public:
    /**
     * @brief Tworzy walec.
     * @param radius promien
     * @param height wysokosc
     * @param slices liczba podzialow obwodu (default 24)
     */
    CylinderNode(float radius, float height, int slices = 24);
    void setRadius(float value);
    void setHeight(float value);
    void setSlices(int value);

    float radius() const;
    float height() const;
    int slices() const;

protected:
    void drawGeometry() const override;

private:
    float cylinderRadius;
    float cylinderHeight;
    int cylinderSlices;
};

/**
 * @brief Sfera. Sparametryzowana stacks (rownoleznikow) x slices (poludnikow).
 *
 * Normalne to znormalizowana pozycja (bo sfera jednostkowa). UV w
 * ukladzie sferycznym: u = phi/(2pi), v = theta/pi.
 */
class SphereNode : public PrimitiveNode {
public:
    SphereNode(float radius, int stacks = 18, int slices = 36);
    void setRadius(float value);
    float radius() const;

protected:
    void drawGeometry() const override;

private:
    float sphereRadius;
    int sphereStacks;
    int sphereSlices;
};

/**
 * @brief Stożek. Podstawa w y=-h/2, wierzcholek w y=+h/2.
 *
 * Powierzchnia boczna jako N osobnych trojkatow (apex + 2 wierzcholki
 * podstawy) - takie podejscie bo TRIANGLE_FAN z jednym apexem nie daje
 * dobrych normalnych (apex musialby miec jedna usredniona normalna co
 * sprawia ze cieniowanie wyglada na plaskie). Tu apex w kazdym trojkacie
 * dostaje normalna usredniona z dwoch sasiednich slope'ow.
 *
 * Normalne boczne nachylone zgodnie z katem zwezania (slope angle).
 */
class ConeNode : public PrimitiveNode {
public:
    ConeNode(float radius, float height, int slices = 24);
    void setRadius(float value);
    void setHeight(float value);
    void setSlices(int value);

    float radius() const;
    float height() const;
    int   slices() const;

protected:
    void drawGeometry() const override;

private:
    float coneRadius;
    float coneHeight;
    int   coneSlices;
};

/**
 * @brief Torus (donut).
 *
 * Sparametryzowany dwoma promieniami:
 * - majorRadius (R) - odleglosc srodka rurki od srodka calego torusa
 * - minorRadius (r) - promien samej rurki
 *
 * Powierzchnia:  (x,y,z) = ((R+r*cos(v))*cos(u), r*sin(v), (R+r*cos(v))*sin(u))
 * gdzie u,v ∈ [0, 2pi]. Normalna w danym punkcie to znormalizowany wektor
 * od osrodka rurki przy katcie u do tego punktu.
 *
 * U nas torus jest tylko dekoracyjny (zyrandol pod sufitem).
 */
class TorusNode : public PrimitiveNode {
public:
    TorusNode(float majorRadius, float minorRadius,
              int majorSlices = 24, int minorSlices = 12);
    void setMajorRadius(float v);
    void setMinorRadius(float v);
    float majorRadius() const;
    float minorRadius() const;

protected:
    void drawGeometry() const override;

private:
    float torusMajor;
    float torusMinor;
    int   torusMajorSlices;
    int   torusMinorSlices;
};

/**
 * @brief Plaski czworokat (quad) w plaszczyznie XZ, normalna w +Y.
 *
 * Uzywany na sciany pokoju, podloge, sufit. Po rotacji moze byc dowolnie
 * obrocony.
 */
class PlaneNode : public PrimitiveNode {
public:
    PlaneNode(float width, float depth);
    void setSize(float w, float d);

    /**
     * @brief Skala UV - kontroluje jak duze sa pojedyncze tile tekstury.
     *
     * Domyslnie 1.0 = 1 powtorzenie tekstury na 1 jednostke swiata. Mniejsze
     * wartosci -> wieksze pojedyncze powtorzenia (rzadziej powtarza sie).
     * np. setUVScale(0.2) na scianie 30m daje 6 powtorzen zamiast 30 (bo
     * 30 powtorzen wyglada okropnie - mikrosokojkie tile).
     */
    void setUVScale(float scale);

protected:
    void drawGeometry() const override;

private:
    float planeWidth;
    float planeDepth;
    float planeUVScale;
};

#endif
