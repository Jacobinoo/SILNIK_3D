# Tekstury BMP

Pliki **24-bitowych nieskompresowanych BMP** które gra wczytuje przy starcie.

## Pliki opcjonalne

| Plik          | Powierzchnia                       | Zalecany rozmiar | Fallback gdy brak |
|---------------|------------------------------------|------------------|-------------------|
| `floor.bmp`   | Podloga pokoju                     | 256×256 / 512×512| cegly (generateBricks) |
| `wall.bmp`    | Wszystkie 4 sciany pokoju          | 256×256 / 512×512| drewno (generateWood) |
| `target.bmp`  | Cel-sfera                          | 128×128 / 256×256| szachownica (generateCheckerboard) |
| `cone.bmp`    | Stozki-przeszkody (3 sztuki)       | 128×128 / 256×256| paski (generateStripes) |
| `box.bmp`     | Szescian-skrzynie (2 sztuki)       | 256×256 / 512×512| drewno (generateWood) |

Wszystkie pliki sa **opcjonalne** - gra dziala bez nich (uzyje proceduralnych).
Dodawaj te ktorych potrzebujesz.

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
│       ├── floor.bmp       <- opcjonalny
│       ├── target.bmp      <- opcjonalny
│       ├── cone.bmp        <- opcjonalny
│       └── box.bmp         <- opcjonalny
├── build/
│   ├── Silnik3D.exe
│   └── assets/             <- kopiowane automatycznie przez CMake
│       └── textures/
│           └── *.bmp
└── ...
```

## Co jakiej tekstury szukac

- **floor.bmp** - drewno, kafelki, parquet, beton, dlazka (`floor`, `wood planks`, `tile`)
- **wall.bmp** - tynk, cegly, kamien, panele, plytki (`wall`, `plaster`, `brick wall`, `stone`)
- **target.bmp** - tarcza strzelnicza, bullseye, dartboard, lub wzor 3D
- **cone.bmp** - paski drogowe / hazard, pasy ostrzegawcze (`traffic cone`, `hazard stripes`)
- **box.bmp** - drewno, deski, skrzynia, paleta (`wood crate`, `wooden box`, `pallet`)
