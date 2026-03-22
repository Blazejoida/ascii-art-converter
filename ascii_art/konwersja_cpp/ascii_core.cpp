/*
 * Temat projektu: Konwerter obrazów do ASCII Art (DLL C++)
 * Krótki opis algorytmu: Konwersja obrazu szaroœci na ASCII z fixed-point arithmetic
 * Data wykonania: Semestr zimowy 2025/2026
 * Autor: B³a¿ej Jamrozik
 * Wersja: 3.1
 * Historia zmian:
 * - v1.0: Podstawowa implementacja
 * - v2.0: Wielow¹tkowoœæ
 * - v3.0: Fixed-point arithmetic
 * - v3.1: Ujednolicenie typów z ASM
 */

#include "pch.h"
#include "ascii_core.h"
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdint>

 /**
  * @brief Paleta 64 znaków ASCII od najciemniejszego do najjaœniejszego
  */
static const std::string ZNAKI = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/()1{}[]?-_+~<>i!lI;:,^ ";

/**
 * @brief Skala dla arytmetyki sta³oprzecinkowej. Wartoœæ 63 odpowiada maksymalnemu indeksowi (rozmiar palety - 1).
 */
constexpr uint32_t FIXED_SCALE = 63;  

/**
 * @brief Przesuniêcie bitowe dla arytmetyki sta³oprzecinkowej (dzielenie).
 */
constexpr uint32_t FIXED_SHIFT = 8;  

/**
 * @brief Konwertuje jasnoœæ piksela na znak ASCII u¿ywaj¹c fixed-point
 * @param jasnosc Wartoœæ jasnoœci w zakresie 0-255
 * @return Znak ASCII odpowiadaj¹cy jasnoœci
 */
inline char jasnoscNaASCII(uint8_t jasnosc) {
    uint32_t idx = (static_cast<uint32_t>(jasnosc) * FIXED_SCALE) >> FIXED_SHIFT;
    idx = std::clamp(idx, 0U, 63U);
    return ZNAKI[idx];
}


/**
 * @brief Oblicza wymagany rozmiar bufora wyjœciowego
 * @param width Szerokoœæ obrazu w pikselach (> 0)
 * @param height Wysokoœæ obrazu w pikselach (> 0)
 * @return Rozmiar bufora w bajtach lub 0 przy b³êdzie
 */
extern "C" __declspec(dllexport) int API_CALL get_required_buffer_size(int width, int height) {
    if (width <= 0 || height <= 0) return 0;

    int64_t needed = static_cast<int64_t>(width) * height + height + 1;
    if (needed > INT32_MAX) return 0;

    return static_cast<int>(needed);
}

/**
 * @brief G³ówna funkcja konwersji obrazu na ASCII Art
 * @param dane_szare WskaŸnik do danych obrazu w skali szaroœci
 * @param width Szerokoœæ obrazu w pikselach (> 0)
 * @param height Wysokoœæ obrazu w pikselach (> 0)
 * @param out_buf WskaŸnik do bufora wyjœciowego
 * @param out_buf_size Rozmiar bufora wyjœciowego (> 0)
 * @param start_row Pierwszy wiersz do przetworzenia (0 do height-1)
 * @param end_row Wiersz koñcowy + 1 (1 do height)
 * @return Kod b³êdu: 0 = sukces, 1-4 = b³êdy
 */
extern "C" __declspec(dllexport) int API_CALL konwertuj_ascii_core(
    const uint8_t* dane_szare,
    int width,
    int height,
    char* out_buf,
    int out_buf_size,
    int start_row,
    int end_row
) {
    if (!dane_szare || !out_buf) return 1;
    if (width <= 0 || height <= 0) return 2;
    if (out_buf_size <= 0) return 3;
    if (start_row < 0 || end_row > height || start_row >= end_row) return 4;

    for (int y = start_row; y < end_row; ++y) {
        const uint8_t* src_row = dane_szare + static_cast<size_t>(y) * width;
        char* dst_row = out_buf + static_cast<size_t>(y) * (width + 1);

        for (int x = 0; x < width; ++x) {
            dst_row[x] = jasnoscNaASCII(src_row[x]);
        }
        dst_row[width] = '\n';
    }

    return 0;
}