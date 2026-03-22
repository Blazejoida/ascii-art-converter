/**
 * @file ascii_core.h
 * @brief Deklaracje funkcji konwersji obrazów do ASCII
 * @author B³a¿ej Jamrozik
 * @date Semestr zimowy 2025/2026
 */

#pragma once
#include <cstdint>

#ifdef _WIN32
#define API_CALL __stdcall
#else
#define API_CALL
#endif

extern "C" {

    /**
     * @brief Pobiera wymagany rozmiar bufora wyjœciowego dla konwersji ASCII
     * @param width Szerokoœæ obrazu w pikselach
     * @param height Wysokoœæ obrazu w pikselach
     * @return Wymagany rozmiar bufora w bajtach
     */
    __declspec(dllexport) int API_CALL get_required_buffer_size(int width, int height);

    /**
     * @brief Konwertuje fragment obrazu na ASCII
     * @param dane_szare WskaŸnik do danych obrazu w skali szaroœci
     * @param width Szerokoœæ obrazu w pikselach
     * @param height Wysokoœæ obrazu w pikselach
     * @param out_buf Bufor wyjœciowy dla tekstu ASCII
     * @param out_buf_size Rozmiar bufora wyjœciowego w bajtach
     * @param start_row Pocz¹tkowy wiersz do przetworzenia (w³¹cznie)
     * @param end_row Koñcowy wiersz do przetworzenia (wy³¹cznie)
     * @return Kod b³êdu (0 = sukces, inne wartoœci = b³¹d)
     */
    __declspec(dllexport) int API_CALL konwertuj_ascii_core(
        const uint8_t* dane_szare,
        int width,
        int height,
        char* out_buf,
        int out_buf_size,
        int start_row,
        int end_row
    );

}