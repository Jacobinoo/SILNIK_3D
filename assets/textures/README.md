# Tekstury BMP

Pliki **24-bitowych nieskompresowanych BMP** które gra wczytuje przy starcie.

## Wymagane pliki

| Plik          | Powierzchnia                  | Zalecany rozmiar |
|---------------|-------------------------------|------------------|
| `floor.bmp`   | Podloga pokoju                | 256×256 lub 512×512 |
| `target.bmp`  | Cel-sfera (powtarza sie na UV)| 128×128 lub 256×256 |

Jesli plik nie istnieje, gra uzyje proceduralnej tekstury w jego miejsce
(`generateBricks` dla podlogi, `generateCheckerboard` dla celu).

## Wymagania formatu BMP

- **24-bit RGB** (3 bajty na piksel, bez kanalu alfa)
- **Nieskompresowany** (BI_RGB, compression = 0)
- Rozmiar **potega dwojki** dla najlepszej jakosci (mipmapping)

## Jak skonwertowac PNG na BMP

### GIMP (zalecane)
1. `File → Open` -> wybierz plik PNG
2. `Image → Mode → RGB` (jesli nie jest)
3. `File → Export As...`
4. Wpisz nazwe z rozszerzeniem `.bmp`
5. W oknie dialogowym: **Advanced Options → 24 bits R8 G8 B8**
6. Snij "Do not write color space information" jesli pyta

### Microsoft Paint
1. Otworz PNG
2. `File → Save As → BMP picture`
3. Wybierz **"24-bit Bitmap (*.bmp)"**

### ImageMagick (terminal)
```
magick floor.png -type TrueColor BMP3:floor.bmp
```

### Online
- convertio.co
- cloudconvert.com

## Skad pobrac darmowe tekstury

- **[ambientCG.com](https://ambientcg.com)** - tysiace CC0 (do swobodnego uzytku komercyjnie), kategorie: Bricks, Wood, Concrete, Metal itp.
- **[Poly Haven](https://polyhaven.com/textures)** - wysokiej jakosci CC0
- **[OpenGameArt.org](https://opengameart.org)** - assety do gier
- **[textures.com](https://www.textures.com)** - duzy katalog (czesc darmowa)

Sciagnij wersje 1k lub 512px, potem przekonwertuj na BMP.

## Tip: tekstura celu

Bardzo dobrze wyglada **klasyczna tarcza** (czerwono-bialy bullseye)
lub **wzor 3D**: gdy cel sie obraca, widzisz animacje. Przyklady:
- Tekstura tarczy strzelnicznej (target / bullseye)
- Szachownica (jak teraz - dobrze widac obrot)
- Cyfry / wzor (oryginalny look)

## Strukura katalogow

```
pgk/
├── assets/
│   └── textures/
│       ├── README.md       <- ten plik
│       ├── floor.bmp       <- dodaj
│       └── target.bmp      <- dodaj
├── build/
│   ├── Silnik3D.exe
│   └── assets/             <- kopiowane automatycznie przez CMake
│       └── textures/
│           ├── floor.bmp
│           └── target.bmp
└── ...
```
