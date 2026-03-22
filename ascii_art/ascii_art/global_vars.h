
/**
 * @file global_vars.h
 * @brief Deklaracje zmiennych globalnych u¿ywanych w ca³ym programie
 * @author B³a¿ej Jamrozik
 * @date Semestr zimowy 2025/2026
 */

#ifndef GLOBAL_VARS_H
#define GLOBAL_VARS_H

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <windows.h>
#include "conversion.h"

// ================== ZMIENNE DLL ==================
extern HMODULE hDll;
extern GetBufferSizeFunc dll_get_buffer_size;
extern KonwertujASCIIFunc dll_konwertuj_ascii;
extern std::string aktualna_biblioteka;

// ================== ZMIENNE OBRAZÓW ==================
extern cv::Mat obraz;
extern cv::Mat ascii_obraz;
extern std::string ascii_txt;
extern bool gotowy;
extern int szerokosc_ascii;

// ================== ZMIENNE CZASU ==================
extern double czas_konwersja_ms;
extern std::string ostatni_komunikat_czasu;

// ================== ZMIENNE PLIKÓW ==================
extern std::string nazwa_obrazu;
extern std::string sciezka_obrazu;
extern const std::string folder_wyniki;

// ================== ZMIENNE KONFIGURACJI ==================
extern int liczba_watkow;
extern bool tryb_asm;
extern bool trwa_konwersja;

// ================== ZMIENNE FOLDERU ==================
extern std::vector<std::string> pliki_obrazow;
extern int aktualny_plik_index;
extern bool tryb_folder;
extern double laczny_czas_konwersji_ms;
extern int liczba_przetworzonych_obrazow;
extern std::string nazwa_folderu_wejscie;

// ================== DEFINICJE PRZYCISKÓW ==================
extern cv::Rect btn_wybierz;
extern cv::Rect btn_konwertuj;
extern cv::Rect btn_reset;
extern cv::Rect btn_wyjdz;
extern cv::Rect checkbox_asm;
extern cv::Rect checkbox_cpp;

#endif 