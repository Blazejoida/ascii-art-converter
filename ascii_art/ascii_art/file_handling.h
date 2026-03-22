/**
 * @file file_handling.h
 * @brief Deklaracje funkcji obs³ugi plików i folderów
 * @author B³a¿ej Jamrozik
 * @date Semestr zimowy 2025/2026
 */

#ifndef FILE_HANDLING_H
#define FILE_HANDLING_H

#include <string>
#include <vector>
#include <filesystem>
#include <opencv2/opencv.hpp>

namespace fs = std::filesystem;

/**
 * @brief Rekurencyjnie znajdŸ wszystkie pliki obrazów w folderze i podfolderach
 * @param folder Œcie¿ka do folderu do przeszukania
 * @param pliki Wektor do przechowywania znalezionych plików
 */
void znajdzPlikiObrazow(const fs::path& folder, std::vector<std::string>& pliki);

/**
 * @brief Zapisuje pomiary czasu do pliku CSV w podfolderze obrazu
 * @param czas_calkowity Ca³kowity czas konwersji [ms]
 * @param czas_dll Czas wykonania funkcji DLL [ms]
 * @param watki Liczba u¿ytych w¹tków
 * @param nazwa_pliku Nazwa pliku obrazu
 * @param podfolder Podfolder dla wyników
 */
void zapiszPomiar(double czas_calkowity, double czas_dll, int watki, const std::string& nazwa_pliku, const std::string& podfolder);

/**
 * @brief Zapisuje wyniki konwersji (tekst ASCII i obraz) do plików w podfolderze
 * @param sciezka_obrazu Oryginalna œcie¿ka do obrazu
 * @param lokalny_ascii_txt Tekst ASCII do zapisania
 * @param lokalny_ascii_obraz Obraz ASCII do zapisania
 * @param podfolder Podfolder dla wyników
 */
void zapiszWyniki(const std::string& sciezka_obrazu, const std::string& lokalny_ascii_txt, const cv::Mat& lokalny_ascii_obraz, const std::string& podfolder);

/**
 * @brief Zapisuje raport konwersji ca³ego folderu
 * @param czas_calkowity Ca³kowity czas przetwarzania [ms]
 * @param czas_dll Suma czasów wykonania funkcji DLL [ms]
 * @param liczba_plikow Liczba przetworzonych plików
 * @param nazwa_folderu Nazwa folderu wejœciowego
 */
void zapiszRaportFolderu(double czas_calkowity, double czas_dll, int liczba_plikow, const std::string& nazwa_folderu);

#endif 