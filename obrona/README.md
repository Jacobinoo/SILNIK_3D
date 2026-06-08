# Materialy do obrony projektu (WEWNETRZNE)

Ten folder zawiera osobny plik PDF dla kazdej osoby z zespolu - sciaga do
obrony projektu. **To NIE jest czesc dokumentacji Doxygen** - to prywatne
materialy zespolu do przygotowania sie na pytania wykladowcy.

## Pliki PDF (gotowe do wydruku / czytania)

| Plik                 | Osoba   | Zakres                                        |
|----------------------|---------|-----------------------------------------------|
| `Lukasz_Osoba1.pdf`  | Lukasz  | Math3D, SceneNode, Room.cpp                    |
| `Rogert_Osoba2.pdf`  | Rogert  | PrimitiveNode, Obstacles.cpp                   |
| `Kacper_Osoba3.pdf`  | Kacper  | Camera, Light, Texture, Targets.cpp           |
| `Jakub_Osoba4.pdf`   | Jakub   | Engine, main, Gameplay, Constants, CMake      |

Kazdy PDF ma:
1. **Mape plikow** (drzewo) z zaznaczeniem wlasnych plikow - do szybkiego
   znalezienia podczas prezentacji.
2. **Opis "po ludzku"** - co i dlaczego robia pliki (od podstaw).
3. **Kluczowe fragmenty kodu** z wyjasnieniem.
4. **Pytania i odpowiedzi** - przewidziane pytania wykladowcy + gotowe
   odpowiedzi.

## Jak wygenerowac ponownie

Wymaga Pythona 3.9+ z biblioteka reportlab:

```
python -m pip install reportlab
python gen_lukasz.py
python gen_rogert.py
python gen_kacper.py
python gen_jakub.py
```

- `_pdfkit.py` - wspolne narzedzia (style, czcionki, helpery) uzywane przez
  wszystkie 4 skrypty.
- `gen_*.py` - tresc poszczegolnych PDF-ow.
