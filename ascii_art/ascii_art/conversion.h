/**
 * @file conversion.h
 * @brief Deklaracje funkcji konwersji obrazów do ASCII
 * @author B³a¿ej Jamrozik
 * @date Semestr zimowy 2025/2026
 */

#ifndef CONVERSION_H
#define CONVERSION_H

#include <string>
#include <vector>
#include <windows.h>

 /**
  * @brief Typ funkcji pobieraj¹cej wymagany rozmiar bufora wyjœciowego
  * @param width Szerokoœæ obrazu w pikselach
  * @param height Wysokoœæ obrazu w pikselach
  * @return Wymagany rozmiar bufora w bajtach
  */
typedef int(__stdcall* GetBufferSizeFunc)(int width, int height);

/**
 * @brief Typ funkcji konwertuj¹cej obraz na ASCII
 * @param dane_szare WskaŸnik do danych obrazu w skali szaroœci
 * @param width Szerokoœæ obrazu
 * @param height Wysokoœæ obrazu
 * @param out_buf Bufer wyjœciowy dla tekstu ASCII
 * @param out_buf_size Rozmiar bufora wyjœciowego
 * @param start_row Pocz¹tkowy wiersz do przetworzenia
 * @param end_row Koñcowy wiersz do przetworzenia (nieinkl.)
 * @return Kod b³êdu (0 = sukces)
 */
typedef int(__stdcall* KonwertujASCIIFunc)(const uint8_t* dane_szare, int width, int height,
    char* out_buf, int out_buf_size, int start_row, int end_row);

/**
 * @brief £aduje odpowiedni¹ bibliotekê DLL w zale¿noœci od trybu
 * @param tryb_asm True = ³aduj ASM DLL, false = ³aduj CPP DLL
 * @return True jeœli ³adowanie siê powiod³o, false w przeciwnym razie
 */
bool zaladujDLL(bool tryb_asm);

/**
 * @brief Wykonuje konwersjê pojedynczego obrazu JEDNOW¥TKOWO (dla trybu folderu)
 * @param sciezka_obrazu Œcie¿ka do obrazu do konwersji
 * @param podfolder Podfolder dla wyników
 * @return Czas konwersji DLL w ms
 */
double konwertujObrazJednowatkowo(const std::string& sciezka_obrazu, const std::string& podfolder);

/**
 * @brief Konwertuje wszystkie obrazy w folderze
 */
void konwertujFolder();

/**
 * @brief Wykonuje konwersjê pojedynczego obrazu na ASCII (JEDNOW¥TKOWO)
 * @param sciezka_obrazu Œcie¿ka do obrazu do konwersji
 * @param podfolder Podfolder dla wyników
 * @return True jeœli konwersja siê powiod³a, false w przeciwnym razie
 */
bool wykonajKonwersjePojedynczegoObrazu(const std::string& sciezka_obrazu, const std::string& podfolder = "");

#endif