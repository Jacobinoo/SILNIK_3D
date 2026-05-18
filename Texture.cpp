#include "Texture.h"
#include <cstdio>
#include <cstring>
#include <algorithm>

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

    // gluBuild2DMipmaps generuje wszystkie poziomy mipmap ręcznie
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height,
                      GL_RGB, GL_UNSIGNED_BYTE, data.data());

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

void Texture::bind() const {
    glBindTexture(GL_TEXTURE_2D, textureId);
}

void Texture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}
