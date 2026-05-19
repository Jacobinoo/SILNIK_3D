#include "Texture.h"
#include <cstdio>
#include <cstring>
#include <cmath>
#include <algorithm>

namespace {
    // Deterministyczne pseudo-losowe pole [0,1) z 32-bitowego inta.
    inline float hash01(unsigned int n) {
        n = (n << 13u) ^ n;
        n = n * (n * n * 15731u + 789221u) + 1376312589u;
        return (n & 0x7fffffffu) / (float)0x7fffffff;
    }

    inline float fclamp(float v, float lo, float hi) {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    inline float flerp(float a, float b, float t) { return a + t * (b - a); }
    inline float smoothstep01(float t) { return t * t * (3.0f - 2.0f * t); }

    // Value noise 2D: rogi calkowitej kraty maja losowe wartosci,
    // wnetrze jest interpolacja bilinearna z wygladzaniem (smoothstep).
    float valueNoise2D(float x, float y, unsigned int seed) {
        int xi = (int)std::floor(x);
        int yi = (int)std::floor(y);
        float fx = smoothstep01(x - xi);
        float fy = smoothstep01(y - yi);

        auto cornerVal = [&](int cx, int cy) {
            unsigned int h = (unsigned int)(cx * 374761393)
                           ^ (unsigned int)(cy * 668265263)
                           ^ seed;
            return hash01(h);
        };

        float a = cornerVal(xi,     yi);
        float b = cornerVal(xi + 1, yi);
        float c = cornerVal(xi,     yi + 1);
        float d = cornerVal(xi + 1, yi + 1);
        return flerp(flerp(a, b, fx), flerp(c, d, fx), fy);
    }

    // Fractional Brownian Motion: sumuje kolejne oktawy szumu o coraz wyzszej
    // czestosci i nizszej amplitudzie. Daje naturalny, samopodobny wzor.
    float fbm2D(float x, float y, int octaves, unsigned int seed) {
        float total = 0.0f, amp = 1.0f, freq = 1.0f, maxAmp = 0.0f;
        for (int i = 0; i < octaves; ++i) {
            total += valueNoise2D(x * freq, y * freq,
                                  seed + (unsigned int)i * 1009u) * amp;
            maxAmp += amp;
            amp *= 0.5f;
            freq *= 2.0f;
        }
        return total / maxAmp;
    }

    inline void writePixel(std::vector<unsigned char>& data, int idx,
                           float r, float g, float b) {
        data[idx + 0] = (unsigned char)(fclamp(r, 0.0f, 1.0f) * 255.0f);
        data[idx + 1] = (unsigned char)(fclamp(g, 0.0f, 1.0f) * 255.0f);
        data[idx + 2] = (unsigned char)(fclamp(b, 0.0f, 1.0f) * 255.0f);
    }
}

Texture::Texture() : textureId(0), loaded(false) {}

Texture::~Texture() {
    release();
}

void Texture::release() {
    if (loaded && textureId != 0) {
        glDeleteTextures(1, &textureId);
        textureId = 0;
        loaded = false;
    }
}

void Texture::uploadToGPU(const std::vector<unsigned char>& data, int width, int height) {
    release();
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // Własna generacja mipmap: uśrednianie kwadratów 2x2 dla każdego poziomu
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, data.data());

    std::vector<unsigned char> src = data;
    int mipW = width, mipH = height, level = 1;
    while (mipW > 1 || mipH > 1) {
        int newW = std::max(1, mipW / 2);
        int newH = std::max(1, mipH / 2);
        std::vector<unsigned char> dst((size_t)(newW * newH * 3));
        for (int y = 0; y < newH; ++y) {
            for (int x = 0; x < newW; ++x) {
                int sx = x * 2, sy = y * 2;
                for (int c = 0; c < 3; ++c) {
                    // Uśredniamy do 4 pikseli (lub mniej na krawędzi)
                    unsigned int sum = 0, count = 0;
                    for (int dy = 0; dy < 2 && (sy + dy) < mipH; ++dy)
                        for (int dx = 0; dx < 2 && (sx + dx) < mipW; ++dx) {
                            sum += src[((sy + dy) * mipW + (sx + dx)) * 3 + c];
                            ++count;
                        }
                    dst[(y * newW + x) * 3 + c] = (unsigned char)(sum / count);
                }
            }
        }
        glTexImage2D(GL_TEXTURE_2D, level, GL_RGB, newW, newH, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, dst.data());
        src = std::move(dst);
        mipW = newW; mipH = newH; ++level;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    loaded = true;
}

// ---------- BMP loader ----------
// Obsługuje wyłącznie 24-bitowe nieskompresowane pliki BMP (BI_RGB).
// Piksele BMP są zapisane w kolejności BGR; ta funkcja konwertuje do RGB.
// Wiersze obrazu są zapisane od dołu (jeśli height > 0) – odwracamy je.
bool Texture::loadBMP(const std::string& filename) {
    FILE* f = fopen(filename.c_str(), "rb");
    if (!f) return false;

    // Nagłówek pliku BMP: 14 bajtów
    unsigned char fhdr[14];
    if (fread(fhdr, 1, 14, f) != 14) { fclose(f); return false; }
    if (fhdr[0] != 'B' || fhdr[1] != 'M') { fclose(f); return false; }

    unsigned int dataOffset =
        (unsigned int)fhdr[10] |
        ((unsigned int)fhdr[11] << 8) |
        ((unsigned int)fhdr[12] << 16) |
        ((unsigned int)fhdr[13] << 24);

    // Nagłówek DIB (BITMAPINFOHEADER): 40 bajtów
    unsigned char dib[40];
    if (fread(dib, 1, 40, f) != 40) { fclose(f); return false; }

    int w = (int)((unsigned int)dib[4]  | ((unsigned int)dib[5]  << 8) |
                  ((unsigned int)dib[6]  << 16) | ((unsigned int)dib[7]  << 24));
    unsigned int uh = (unsigned int)dib[8]  | ((unsigned int)dib[9]  << 8) |
                      ((unsigned int)dib[10] << 16) | ((unsigned int)dib[11] << 24);
    int h = (int)uh;

    unsigned short bpp = (unsigned short)(dib[14] | (dib[15] << 8));
    unsigned int compression = (unsigned int)dib[16] | ((unsigned int)dib[17] << 8) |
                               ((unsigned int)dib[18] << 16) | ((unsigned int)dib[19] << 24);

    if (bpp != 24 || compression != 0) { fclose(f); return false; }
    if (w <= 0) { fclose(f); return false; }

    // Ujemna wysokość oznacza obraz zapisany od góry (top-down)
    bool topDown = (h < 0);
    if (h < 0) h = -h;
    if (h == 0) { fclose(f); return false; }

    // Każdy wiersz BMP jest wyrównany do 4 bajtów
    int rowStride = (w * 3 + 3) & ~3;

    fseek(f, (long)dataOffset, SEEK_SET);
    std::vector<unsigned char> raw((size_t)(rowStride * h));
    if (fread(raw.data(), 1, raw.size(), f) != raw.size()) { fclose(f); return false; }
    fclose(f);

    // Konwersja BGR -> RGB z opcjonalnym odwróceniem wierszy
    std::vector<unsigned char> pixels((size_t)(w * h * 3));
    for (int row = 0; row < h; ++row) {
        int srcRow = topDown ? row : (h - 1 - row);
        for (int col = 0; col < w; ++col) {
            int src = srcRow * rowStride + col * 3;
            int dst = (row * w + col) * 3;
            pixels[dst + 0] = raw[src + 2]; // R
            pixels[dst + 1] = raw[src + 1]; // G
            pixels[dst + 2] = raw[src + 0]; // B
        }
    }

    uploadToGPU(pixels, w, h);
    return true;
}

// ---------- Procedural generators ----------

void Texture::generateCheckerboard(int size,
                                   float r1, float g1, float b1,
                                   float r2, float g2, float b2,
                                   int tileCount) {
    std::vector<unsigned char> data((size_t)(size * size * 3));
    int tileSize = std::max(1, size / tileCount);
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            bool first = (((x / tileSize) + (y / tileSize)) % 2) == 0;
            int idx = (y * size + x) * 3;
            data[idx + 0] = (unsigned char)((first ? r1 : r2) * 255.0f);
            data[idx + 1] = (unsigned char)((first ? g1 : g2) * 255.0f);
            data[idx + 2] = (unsigned char)((first ? b1 : b2) * 255.0f);
        }
    }
    uploadToGPU(data, size, size);
}

void Texture::generateGradient(int size,
                               float r1, float g1, float b1,
                               float r2, float g2, float b2,
                               bool horizontal) {
    std::vector<unsigned char> data((size_t)(size * size * 3));
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            float t = horizontal
                ? (float)x / (float)(size - 1)
                : (float)y / (float)(size - 1);
            int idx = (y * size + x) * 3;
            data[idx + 0] = (unsigned char)((r1 + t * (r2 - r1)) * 255.0f);
            data[idx + 1] = (unsigned char)((g1 + t * (g2 - g1)) * 255.0f);
            data[idx + 2] = (unsigned char)((b1 + t * (b2 - b1)) * 255.0f);
        }
    }
    uploadToGPU(data, size, size);
}

void Texture::generateStripes(int size,
                              float r1, float g1, float b1,
                              float r2, float g2, float b2,
                              int stripeCount) {
    std::vector<unsigned char> data((size_t)(size * size * 3));
    int stripeSize = std::max(1, size / stripeCount);
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            bool first = ((x / stripeSize) % 2) == 0;
            int idx = (y * size + x) * 3;
            data[idx + 0] = (unsigned char)((first ? r1 : r2) * 255.0f);
            data[idx + 1] = (unsigned char)((first ? g1 : g2) * 255.0f);
            data[idx + 2] = (unsigned char)((first ? b1 : b2) * 255.0f);
        }
    }
    uploadToGPU(data, size, size);
}

// ---------- Procedural: noise / wood / bricks / marble ----------

void Texture::generatePerlinNoise(int size,
                                  float r1, float g1, float b1,
                                  float r2, float g2, float b2,
                                  int octaves, float scale) {
    std::vector<unsigned char> data((size_t)(size * size * 3));
    const unsigned int seed = 1337u;

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            float fx = (float)x / (float)size * scale;
            float fy = (float)y / (float)size * scale;
            float n  = fbm2D(fx, fy, octaves, seed);  // wartosc w [0,1]
            int idx = (y * size + x) * 3;
            writePixel(data, idx,
                       r1 + n * (r2 - r1),
                       g1 + n * (g2 - g1),
                       b1 + n * (b2 - b1));
        }
    }
    uploadToGPU(data, size, size);
}

void Texture::generateWood(int size,
                           float r1, float g1, float b1,
                           float r2, float g2, float b2,
                           int rings) {
    std::vector<unsigned char> data((size_t)(size * size * 3));
    const unsigned int seed = 31u;

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            // Pozycja wzgledem srodka tekstury
            float fx = ((float)x / size - 0.5f) * 2.0f;
            float fy = ((float)y / size - 0.5f) * 2.0f;

            // Zaburzenie sloji szumem (turbulencja)
            float turb = fbm2D(fx * 2.5f, fy * 2.5f, 4, seed) - 0.5f;
            float dist = std::sqrt(fx * fx + fy * fy) + turb * 0.20f;

            // Wzor sloji: pila z dist * rings
            float ring  = dist * (float)rings;
            float fract = ring - std::floor(ring);
            // Wzmacniamy kontrast - jasne tlo, ciemne sloje
            float t = (fract < 0.75f) ? (fract / 0.75f) : (1.0f - (fract - 0.75f) / 0.25f);

            int idx = (y * size + x) * 3;
            writePixel(data, idx,
                       r1 + t * (r2 - r1),
                       g1 + t * (g2 - g1),
                       b1 + t * (b2 - b1));
        }
    }
    uploadToGPU(data, size, size);
}

void Texture::generateBricks(int size,
                             float br, float bg, float bb,
                             float mr, float mg, float mb,
                             int rowsPerTexture) {
    std::vector<unsigned char> data((size_t)(size * size * 3));
    const unsigned int seed = 4099u;

    int rowHeight = std::max(2, size / rowsPerTexture);
    int brickWidth = rowHeight * 2;        // proporcje 2:1
    int mortarPx = std::max(1, rowHeight / 8);

    for (int y = 0; y < size; ++y) {
        int row    = y / rowHeight;
        int yInRow = y % rowHeight;
        int xOff   = (row % 2) * (brickWidth / 2);   // przesuniecie naprzemienne

        for (int x = 0; x < size; ++x) {
            int xAdj  = (x + xOff) % brickWidth;
            bool mortar = (yInRow < mortarPx) || (xAdj < mortarPx);

            // Drobna wariacja koloru per cegla (deterministyczny hash z indeksu)
            int brickCol = (x + xOff) / brickWidth;
            float jitter = (hash01((unsigned int)row * 7919u
                                 + (unsigned int)brickCol * 17389u
                                 + seed) - 0.5f) * 0.18f;

            float r, g, b;
            if (mortar) {
                r = mr; g = mg; b = mb;
            } else {
                // Dodatkowy mikro-szum wewnatrz cegly
                float n = fbm2D((float)x * 0.10f, (float)y * 0.10f, 3, seed + 7u) - 0.5f;
                r = br + jitter + n * 0.08f;
                g = bg + jitter + n * 0.08f;
                b = bb + jitter + n * 0.08f;
            }

            writePixel(data, (y * size + x) * 3, r, g, b);
        }
    }
    uploadToGPU(data, size, size);
}

void Texture::generateMarble(int size,
                             float r1, float g1, float b1,
                             float r2, float g2, float b2,
                             float turbulence) {
    std::vector<unsigned char> data((size_t)(size * size * 3));
    const unsigned int seed = 2027u;

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            float fx = (float)x / size;
            float fy = (float)y / size;

            // Turbulencja: suma modulow szumu (zamiast samego sumowania) - daje wzor zyl
            float t = 0.0f, amp = 1.0f, freq = 4.0f;
            for (int i = 0; i < 5; ++i) {
                t += std::abs(valueNoise2D(fx * freq, fy * freq,
                                           seed + (unsigned int)i * 1000u) - 0.5f) * amp;
                amp  *= 0.5f;
                freq *= 2.0f;
            }

            // Zyly = wartosc sin(x + turb) - ostre przejscia
            float v = std::sin((fx + t * turbulence) * 6.2831853f);
            v = std::pow(std::abs(v), 0.5f);  // wyostrzenie

            int idx = (y * size + x) * 3;
            writePixel(data, idx,
                       r1 + v * (r2 - r1),
                       g1 + v * (g2 - g1),
                       b1 + v * (b2 - b1));
        }
    }
    uploadToGPU(data, size, size);
}

void Texture::bind() const {
    glBindTexture(GL_TEXTURE_2D, textureId);
}

void Texture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}
