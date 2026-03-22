/**
 * @file global_vars.cpp
 * @brief Definicje zmiennych globalnych
 * @author B³a¿ej Jamrozik
 * @date Semestr zimowy 2025/2026
 */

#include "global_vars.h"
#include <thread>

 // ================== ZMIENNE DLL ==================
HMODULE hDll = nullptr;
GetBufferSizeFunc dll_get_buffer_size = nullptr;
KonwertujASCIIFunc dll_konwertuj_ascii = nullptr;
std::string aktualna_biblioteka = "";

// ================== ZMIENNE OBRAZÓW ==================
cv::Mat obraz;
cv::Mat ascii_obraz;
std::string ascii_txt;
bool gotowy = false;
int szerokosc_ascii = 400;

// ================== ZMIENNE CZASU ==================
double czas_konwersja_ms = 0.0;
std::string ostatni_komunikat_czasu = "";

// ================== ZMIENNE PLIKÓW ==================
std::string nazwa_obrazu;
std::string sciezka_obrazu;
const std::string folder_wyniki = "wyniki";

// ================== ZMIENNE KONFIGURACJI ==================
int liczba_watkow = std::thread::hardware_concurrency();
bool tryb_asm = false;
bool trwa_konwersja = false;

// ================== ZMIENNE FOLDERU ==================
std::vector<std::string> pliki_obrazow;
int aktualny_plik_index = -1;
bool tryb_folder = false;
double laczny_czas_konwersji_ms = 0.0;
int liczba_przetworzonych_obrazow = 0;
std::string nazwa_folderu_wejscie = "";

// ================== DEFINICJE PRZYCISKÓW ==================
cv::Rect btn_wybierz(50, 620, 180, 60);
cv::Rect btn_konwertuj(50, 700, 180, 60);
cv::Rect btn_reset(280, 700, 180, 60);
cv::Rect btn_wyjdz(510, 700, 180, 60);
cv::Rect checkbox_asm(280, 620, 200, 30);
cv::Rect checkbox_cpp(500, 620, 200, 30);