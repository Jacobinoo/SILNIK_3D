/**
 * @file Light.h
 * @brief Material Phonga + swiatlo punktowe.
 *
 * Korzystamy z OpenGL fixed-function lighting (GL_LIGHT0..GL_LIGHT7).
 * Material ma 3 komponenty (ambient, diffuse, specular) plus shininess
 * do wzoru Phonga na specular.
 */
#ifndef LIGHT_H
#define LIGHT_H

#include "SceneNode.h"

/**
 * @brief Material powierzchni dla oswietlenia Phonga.
 *
 * Wartosci zwykle w zakresie [0,1] na komponent. Ambient to to co widac
 * w cieniu, diffuse to swiatlo rozproszone (kolor obiektu), specular to
 * blysk na polysku, shininess kontroluje jak ostry/szeroki jest ten blysk.
 */
struct Material {
    Vec3 ambient;    ///< kolor pod swiatlem ambient (zwykle ciemniejsza wersja diffuse)
    Vec3 diffuse;    ///< "wlasciwy" kolor obiektu pod oswietleniem
    Vec3 specular;   ///< kolor odbicia kierunkowego
    float shininess; ///< wykladnik dla specular - im wiekszy tym ostrzejszy punkt blysku

    /** @brief Material domyslny - szare diffuse, bez specular. */
    Material();

    /** @brief Pelne ustawienie wszystkich parametrow. */
    Material(const Vec3& ambientValue, const Vec3& diffuseValue,
             const Vec3& specularValue, float shininessValue);
};

/**
 * @brief Swiatlo punktowe (point light) - emituje rownomiernie we wszystkich kierunkach.
 *
 * Pozycja swiatla = pozycja wezla scene'a. Renderuje sie przez `glLightfv`
 * w eye-space. Mozna miec do 8 takich swiatel jednoczesnie (limit OpenGL
 * fixed-function), kazde z innym lightIndex (0..7 -> GL_LIGHT0..GL_LIGHT7).
 *
 * @note Wazne zeby wszystkie swiatla byly wezlami przed geometria w grafie
 * sceny - bo renderRecursive idzie w kolejnosci dzieci. Inaczej geometria
 * renderowana przed swiatlem nie bedzie oswietlona przez to swiatlo.
 */
class PointLight : public SceneNode {
public:
    /** @brief Konstruktor - bialy diffuse, bez attenuation, lightIndex=0. */
    PointLight();

    void setAmbient(const Vec3& value);   ///< ustawia kolor skladnika ambient
    void setDiffuse(const Vec3& value);   ///< ustawia kolor skladnika diffuse
    void setSpecular(const Vec3& value);  ///< ustawia kolor skladnika specular

    /**
     * @brief Wspolczynniki tlumienia odleglosci.
     *
     * Wzor OpenGL: attenuation = 1 / (kc + kl*d + kq*d^2) gdzie d to odleglosc
     * od swiatla. Im mniejsze kl, kq tym swiatlo siega dalej.
     */
    void setAttenuation(float constantValue, float linearValue, float quadraticValue);

    void setEnabled(bool value);          ///< wlacza/wylacza swiatlo
    /**
     * @brief Ktore GL_LIGHTx ma reprezentowac to swiatlo (0..7).
     *
     * Jak masz wiele swiatel to kazde musi miec inny index, inaczej sie
     * nadpisuja.
     */
    void setLightIndex(int value);

    bool enabled() const;
    int lightIndex() const;

protected:
    /** @brief Ustawia GL state (glLightfv) gdy renderer dotrze do tego wezla. */
    void renderSelf(const Mat4& worldMatrix) const override;

private:
    Vec3 ambientColor;
    Vec3 diffuseColor;
    Vec3 specularColor;
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
    bool isEnabled;
    int activeLightIndex;
};

#endif
