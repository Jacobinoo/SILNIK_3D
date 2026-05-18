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
