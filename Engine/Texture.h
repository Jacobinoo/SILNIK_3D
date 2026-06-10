/**
 * @file Texture.h
 * @brief Klasa tekstury 2D z loaderem BMP i generatorami proceduralnymi.
 *
 * Wszystko pisane samodzielnie - parser BMP czyta naglowki bajt po bajcie,
 * generatory proceduralne to wlasne implementacje algorytmow (Perlin noise,
 * slojowe drewno, ceglki, marmur).
 *
 * Po wygenerowaniu/zaladowaniu dane sa uploadowane do GPU jako GL_RGB
 * tekstura wraz z recznie obliczonymi mipmapami (usrednianie blokow 2x2
 * az do 1x1).
 */
#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/freeglut.h>
#include <string>
#include <vector>

/**
 * @brief Tekstura 2D OpenGL.
 *
 * @details Klasa nie-kopiowalna (delete copy ctor) bo trzyma GL handle.
 * Mozna ja trzymac przez std::shared_ptr i wielokrotnie binowac do
 * roznych obiektow.
 */
class Texture {
public:
    Texture();
    ~Texture();

    // Wylaczamy kopiowanie - kopia GL handle zostaweni dwa wezly wskazujace
    // na te sama teksture i destruktor by ja zwolnil dwa razy = problem.
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    /**
     * @brief Wczytuje 24-bitowy nieskompresowany BMP z dysku.
     *
     * Obsluguje tylko BMP 24-bit RGB (BI_RGB, kompresja = 0). Wiersze BMP
     * sa wyrownywane do 4 bajtow i z dolu do gory (chyba ze ujemna wysokosc),
     * dlatego loader robi padding i flip. Konwertuje BGR -> RGB.
     *
     * @param filename sciezka do pliku
     * @return true jesli zaladowano pomyslnie. False przy bledzie - format
     *         zly, plik nie istnieje, naglowek niepoprawny itd. Wtedy
     *         tekstura jest nieoznaczona (isLoaded() = false).
     */
    bool loadBMP(const std::string& filename);

    /**
     * @brief Tekstura proceduralna - szachownica naprzemiennych kolorow.
     * @param size wymiar w pikselach (kwadrat size x size)
     * @param tileCount ile pol szachownicy na bok
     */
    void generateCheckerboard(int size,
                              float r1, float g1, float b1,
                              float r2, float g2, float b2,
                              int tileCount = 8);

    /**
     * @brief Plynny gradient miedzy dwoma kolorami (poziomy lub pionowy).
     * @param horizontal true = lewo->prawo, false = gora->dol
     */
    void generateGradient(int size,
                          float r1, float g1, float b1,
                          float r2, float g2, float b2,
                          bool horizontal = true);

    /** @brief Pionowe paski naprzemiennych kolorow. */
    void generateStripes(int size,
                         float r1, float g1, float b1,
                         float r2, float g2, float b2,
                         int stripeCount = 8);

    /**
     * @brief Fraktalny szum (value noise + FBM).
     *
     * Interpoluje miedzy dwoma kolorami wedlug wartosci szumu (0, 1).
     * Wieksze octaves = wiecej detali na roznych skalach. Wieksze scale =
     * drobniejszy wzor.
     */
    void generatePerlinNoise(int size,
                             float r1, float g1, float b1,
                             float r2, float g2, float b2,
                             int octaves = 4, float scale = 4.0f);

    /**
     * @brief Slojowe drewno - koncentryczne kregi zaburzone szumem.
     * @param rings ile slojow w teksturze
     */
    void generateWood(int size,
                      float r1, float g1, float b1,
                      float r2, float g2, float b2,
                      int rings = 8);

    /**
     * @brief Mur z cegiel z fugami i naturalna wariacja koloru.
     * @param br,bg,bb kolor cegly
     * @param mr,mg,mb kolor fugi (mortar)
     */
    void generateBricks(int size,
                        float br, float bg, float bb,
                        float mr, float mg, float mb,
                        int rowsPerTexture = 6);

    /**
     * @brief Marmur - zyly otrzymane przez turbulencje na funkcji sin.
     * @param turbulence sila zaburzenia (wieksze = bardziej falujace zyly)
     */
    void generateMarble(int size,
                        float r1, float g1, float b1,
                        float r2, float g2, float b2,
                        float turbulence = 5.0f);

    /** @brief Aktywuje teksture w aktualnym GL state (glBindTexture). */
    void bind() const;
    /** @brief Deaktywuje teksture (glBindTexture 0). */
    void unbind() const;

    /** @brief Czy tekstura jest poprawnie zaladowana. */
    bool isLoaded() const { return loaded; }
    /** @brief OpenGL handle (do uzytku zaawansowanego). */
    GLuint id() const { return textureId; }

private:
    /**
     * @brief Wgrywa dane do GPU + generuje wszystkie poziomy mipmap.
     *
     * Mipmapping: kazdy kolejny poziom to obraz 2x mniejszy z usrednionych
     * blokow 2x2 z poprzedniego. Robione dopoki nie zostanie 1x1 piksel.
     */
    void uploadToGPU(const std::vector<unsigned char>& data, int width, int height);
    /** @brief Zwalnia GL handle (glDeleteTextures). */
    void release();

    GLuint textureId;
    bool loaded;
};

#endif
