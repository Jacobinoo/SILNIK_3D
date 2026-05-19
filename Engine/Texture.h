#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/freeglut.h>
#include <string>
#include <vector>

// Klasa do zarządzania teksturami 2D w OpenGL.
// Obsługuje ręczne wczytywanie plików BMP (24-bit, nieskompresowany)
// oraz generowanie tekstur proceduralnych.
class Texture {
public:
    Texture();
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    // Wczytuje 24-bitowy nieskompresowany plik BMP.
    // Zwraca true jeśli sukces, false przy błędzie (np. zły format, brak pliku).
    bool loadBMP(const std::string& filename);

    // Generuje teksturę szachownicy (dwa naprzemienne kolory).
    void generateCheckerboard(int size,
                              float r1, float g1, float b1,
                              float r2, float g2, float b2,
                              int tileCount = 8);

    // Generuje płynny gradient między dwoma kolorami.
    void generateGradient(int size,
                          float r1, float g1, float b1,
                          float r2, float g2, float b2,
                          bool horizontal = true);

    // Generuje pionowe paski naprzemiennych kolorów.
    void generateStripes(int size,
                         float r1, float g1, float b1,
                         float r2, float g2, float b2,
                         int stripeCount = 8);

    // Fraktalny szum (value noise + FBM) - naturalna chaotyczna tekstura.
    // Interpoluje miedzy dwoma kolorami wedlug wartosci szumu w (0, 1).
    // octaves: ile poziomow szumu sumowac (wiecej = wiecej detali).
    // scale: ile cykli szumu na cala teksture (wiecej = drobniejszy wzor).
    void generatePerlinNoise(int size,
                             float r1, float g1, float b1,
                             float r2, float g2, float b2,
                             int octaves = 4, float scale = 4.0f);

    // Slojowe drewno: koncentryczne kregi zaburzone szumem.
    // r1,g1,b1 = ciemne sloje; r2,g2,b2 = jasne tlo. rings = ilosc slojow.
    void generateWood(int size,
                      float r1, float g1, float b1,
                      float r2, float g2, float b2,
                      int rings = 8);

    // Mur z cegiel z fugami i naturalna wariacja koloru ceglek.
    // br,bg,bb = kolor ceglek; mr,mg,mb = kolor fugi.
    void generateBricks(int size,
                        float br, float bg, float bb,
                        float mr, float mg, float mb,
                        int rowsPerTexture = 6);

    // Marmur: zyly otrzymane przez turbulencje na funkcji sin.
    // r1,g1,b1 = ciemne zyly; r2,g2,b2 = jasny kamien.
    // turbulence = sila zaburzenia (wieksze = bardziej falujace zyly).
    void generateMarble(int size,
                        float r1, float g1, float b1,
                        float r2, float g2, float b2,
                        float turbulence = 5.0f);

    void bind() const;
    void unbind() const;

    bool isLoaded() const { return loaded; }
    GLuint id() const { return textureId; }

private:
    void uploadToGPU(const std::vector<unsigned char>& data, int width, int height);
    void release();

    GLuint textureId;
    bool loaded;
};

#endif
