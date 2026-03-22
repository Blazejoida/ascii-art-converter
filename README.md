# 🎨 ASCII ART Converter

> Konwerter obrazów do postaci ASCII Art z implementacją asemblerową SSE4.1 i wielowątkowością

![converse_ascii_art](https://github.com/user-attachments/assets/aecd7aaf-d284-4a44-93b0-56f42d6d8f2f)
  
---

## Opis projektu

Aplikacja okienkowa realizująca konwersję obrazów rastrowych do reprezentacji **ASCII Art**. 
Algorytm mapuje wartości jasności pikseli (skala szarości) na znaki z predefiniowanej **palety 64 symboli ASCII**, 
uporządkowanych według rosnącej jasności - od gęstych znaków takich jak `$` i `@`, przez litery i cyfry, aż do spacji.

### Kluczowe cechy

- **Dwie wersje algorytmu konwersji** — asemblerowa (SSE4.1) oraz referencyjna w C++ (z opcjonalną flagą O2)
- **Wielowątkowe przetwarzanie folderów** — równoległe przetwarzanie wielu obrazów z pulą wątków (1–64)
- **Interfejs graficzny** oparty na OpenCV 
- **Automatyczny zapis wyników** do struktury katalogów `wyniki/` (pliki `.txt`, `.png`, `.csv`)

---

## Działanie algorytmu

Mapowanie jasności piksela na indeks znaku realizowane jest przez arytmetykę stałoprzecinkową:

```
indeks = (jasność × 63) >> 8
```

Wersja asemblerowa (`ascii_core.asm`) przetwarza **16 pikseli jednocześnie** przy użyciu instrukcji SIMD SSE4.1:

```asm
pmovzxbw xmm1, xmm0   ; rozszerza 8 bajtów do 16-bit words
pmullw   xmm1, xmm7   ; mnoży przez 63
psrlw    xmm1, 8      ; >> 8 (dzielenie przez 256)
packuswb xmm1, xmm2   ; spakowanie z powrotem do bajtów
```

---

## Tryby pracy

<img width="500" alt="interfejs_aplikacji" src="https://github.com/user-attachments/assets/0978dbff-cfe9-4983-a116-1af6a58aa0d6" />

### Tryb: Pojedynczy obraz

1. Kliknij **Wybierz** → **Yes - Pojedynczy obraz**
2. Wybierz plik (PNG, JPG, JPEG, BMP)
3. Kliknij **Konwertuj**
4. Wynik pojawi się w prawym panelu, plik zostanie zapisany w `wyniki/[nazwa_obrazu]/`

Po konwersji pojawia się komunikat:
```
Pojedynczy – Calkowity: X ms  DLL: Y ms  (Z watkow, TRYB)
```

### Tryb: Folder obrazów

1. Kliknij **Wybierz** → **No - Cały folder**
2. Wskaż folder (system rekurencyjnie znajdzie wszystkie obrazy PNG/JPG/JPEG/BMP)
3. Kliknij **Konwertuj**
4. Obrazy są przetwarzane równolegle, wyniki trafiają do `wyniki/[nazwa_folderu]/`

Po zakończeniu pojawia się komunikat:
```
Folder – Calkowity: X ms  DLL (suma): Y ms  (Z zdjec, W watkow)
```
