/**
 * @file main.cpp
 * @brief Główny plik programu - konwerter obrazów do ASCII Art
 *
 * OPIS ALGORYTMU:
 * Program konwertuje obrazy do postaci ASCII Art przy użyciu dwóch trybów:
 * - Tryb C++ (domyślny) - wykorzystuje bibliotekę DLL napisaną w C++
 * - Tryb ASM - wykorzystuje bibliotekę DLL zoptymalizowaną w asemblerze
 *
 * @author Błażej Jamrozik
 * @date Semestr zimowy 2025/2026
 * @version 3.5
 *
 * HISTORIA ZMIAN:
 * v1.0 - Podstawowa funkcjonalność konwersji obrazów
 * v1.5 - Dodanie obsługi wielowątkowości
 * v2.0 - Rozbudowany pomiar czasu, poprawki GUI, optymalizacja zapisu CSV
 * v3.0 - Dodanie obsługi całych folderów, automatyczny zapis wszystkich wyników
 * v3.1 - Poprawki GUI, struktura katalogów, raporty czasu
 * v3.2 - Naprawa konwersji pojedynczych obrazów, poprawa struktury katalogów
 * v3.3 - Rekurencyjne przeszukiwanie folderów, poprawa struktury katalogów wynikowych
 * v3.4 - Naprawa zapisu wyników, uproszczenie CSV, poprawa struktury katalogów
 * v3.5 - Usunięcie niepotrzebnych zmiennych i pomiarów czasu, uproszczenie kodu
 */

#define NOMINMAX
#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>
#include "global_vars.h"
#include "gui.h"
#include "conversion.h"
#include "file_handling.h"

namespace fs = std::filesystem;

int main() {
    tryb_asm = false;
    if (!zaladujDLL(false)) {
        std::cerr << "Nie mozna zaladowac biblioteki CPP! Koncze program.\n";
        return -1;
    }

    if (!fs::exists(folder_wyniki)) {
        fs::create_directories(folder_wyniki);
    }

    cv::namedWindow("Konwerter ASCII", cv::WINDOW_GUI_EXPANDED);
    cv::resizeWindow("Konwerter ASCII", 1200, 800);
    cv::setMouseCallback("Konwerter ASCII", onMouse);
    cv::createTrackbar("Watki", "Konwerter ASCII", &liczba_watkow, 64);

    pokazOkno();

    while (true) {
        int key = cv::waitKey(30);
        if (key == 27) break;
        if (liczba_watkow < 1) liczba_watkow = 1;
        if (cv::getWindowProperty("Konwerter ASCII", cv::WND_PROP_VISIBLE) < 1) break;
        pokazOkno();
    }

    if (hDll) FreeLibrary(hDll);
    return 0;
}