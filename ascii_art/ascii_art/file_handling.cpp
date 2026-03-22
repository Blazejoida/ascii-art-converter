/**
 * @file file_handling.cpp
 * @brief Implementacja funkcji obs³ugi plików i folderów
 * @author B³a¿ej Jamrozik
 * @date Semestr zimowy 2025/2026
 */

#include "file_handling.h"
#include "global_vars.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>

void znajdzPlikiObrazow(const fs::path& folder, std::vector<std::string>& pliki) {
    try {
        for (const auto& entry : fs::recursive_directory_iterator(folder)) {
            if (entry.is_regular_file()) {
                std::string extension = entry.path().extension().string();
                std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

                if (extension == ".png" || extension == ".jpg" ||
                    extension == ".jpeg" || extension == ".bmp") {
                    pliki.push_back(entry.path().string());
                }
            }
        }
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "Blad dostepu do folderu: " << e.what() << "\n";
    }
}

void zapiszPomiar(double czas_calkowity, double czas_dll, int watki, const std::string& nazwa_pliku, const std::string& podfolder) {
    std::string sciezka_podfolder = folder_wyniki + "/" + podfolder;
    if (!fs::exists(sciezka_podfolder)) {
        fs::create_directories(sciezka_podfolder);
    }

    std::string nazwa_pliku_csv = sciezka_podfolder + "/" + (tryb_asm ? "ASM" : "CPP") + "_czasy.csv";

    bool potrzebny_naglowek = !fs::exists(nazwa_pliku_csv) || fs::file_size(nazwa_pliku_csv) == 0;

    std::ofstream out(nazwa_pliku_csv, std::ios::app);
    if (!out.is_open()) {
        std::cerr << "Blad zapisu pliku CSV: " << nazwa_pliku_csv << "\n";
        return;
    }

    if (potrzebny_naglowek) {
        out << "Watki,CzasCalkowity[ms],CzasDLL[ms],NazwaPliku\n";
    }

    out << watki << "," << czas_calkowity << "," << czas_dll << "," << nazwa_pliku << "\n";
    out.close();
}

void zapiszWyniki(const std::string& sciezka_obrazu, const std::string& lokalny_ascii_txt, const cv::Mat& lokalny_ascii_obraz, const std::string& podfolder) {
    if (lokalny_ascii_txt.empty()) {
        return;
    }

    fs::path sciezka_pliku(sciezka_obrazu);
    std::string nazwa_pliku = sciezka_pliku.stem().string();
    std::string tryb_tekst = tryb_asm ? "ASM" : "CPP";

    std::string sciezka_podfolder_obrazu = folder_wyniki + "/" + podfolder + "/" + nazwa_pliku;
    if (!fs::exists(sciezka_podfolder_obrazu)) {
        fs::create_directories(sciezka_podfolder_obrazu);
    }

    std::string nazwa_txt = sciezka_podfolder_obrazu + "/" + nazwa_pliku + "_ASCII_" + tryb_tekst + ".txt";
    std::string nazwa_png = sciezka_podfolder_obrazu + "/" + nazwa_pliku + "_ASCII_" + tryb_tekst + ".png";

    std::ofstream plik_txt(nazwa_txt);
    if (plik_txt.is_open()) {
        plik_txt << lokalny_ascii_txt;
        plik_txt.close();
        std::cout << "Zapisano: " << nazwa_txt << "\n";
    }
    else {
        std::cerr << "Blad zapisu: " << nazwa_txt << "\n";
    }

    if (!lokalny_ascii_obraz.empty()) {
        if (cv::imwrite(nazwa_png, lokalny_ascii_obraz)) {
            std::cout << "Zapisano: " << nazwa_png << "\n";
        }
        else {
            std::cerr << "Blad zapisu: " << nazwa_png << "\n";
        }
    }
}

void zapiszRaportFolderu(double czas_calkowity, double czas_dll, int liczba_plikow, const std::string& nazwa_folderu) {
    std::string nazwa_pliku_csv = folder_wyniki + "/raport_konwersji_folderow_" + (tryb_asm ? "ASM" : "CPP") + ".csv";

    bool potrzebny_naglowek = !fs::exists(nazwa_pliku_csv) || fs::file_size(nazwa_pliku_csv) == 0;

    std::ofstream out(nazwa_pliku_csv, std::ios::app);
    if (!out.is_open()) {
        std::cerr << "Blad zapisu pliku raportu: " << nazwa_pliku_csv << "\n";
        return;
    }

    if (potrzebny_naglowek) {
        out << "Data,Watki,CzasCalkowity[ms],CzasDLL[ms],LiczbaPlikow,NazwaFolderu\n";
    }

    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
    localtime_s(&tm, &time_t);

    std::stringstream data_ss;
    data_ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

    out << data_ss.str() << "," << liczba_watkow << "," << czas_calkowity << "," << czas_dll << ","
        << liczba_plikow << "," << nazwa_folderu << "\n";
    out.close();
    std::cout << "Zapisano raport folderu: " << nazwa_pliku_csv << "\n";
}