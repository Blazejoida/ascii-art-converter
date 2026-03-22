/**
 * @file conversion.cpp
 * @brief Implementacja funkcji konwersji obrazów do ASCII
 * @author B³a¿ej Jamrozik
 * @date Semestr zimowy 2025/2026
 */

#define NOMINMAX
#include "conversion.h"
#include "global_vars.h"
#include "file_handling.h"
#include "gui.h"
#include "tinyfiledialogs.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <sstream>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;

bool zaladujDLL(bool tryb_asm) {
    if (hDll) {
        FreeLibrary(hDll);
        hDll = nullptr;
        dll_get_buffer_size = nullptr;
        dll_konwertuj_ascii = nullptr;
    }

    const char* nazwa_dll = tryb_asm ? "konwersja_asm.dll" : "konwersja_cpp.dll";
    hDll = LoadLibraryA(nazwa_dll);

    if (!hDll) {
        std::cerr << "Blad: Nie mozna zaladowac " << nazwa_dll << "\n";
        return false;
    }

    dll_get_buffer_size = (GetBufferSizeFunc)GetProcAddress(hDll, "get_required_buffer_size");
    dll_konwertuj_ascii = (KonwertujASCIIFunc)GetProcAddress(hDll, "konwertuj_ascii_core");

    if (!dll_get_buffer_size || !dll_konwertuj_ascii) {
        std::cerr << "Blad: Nie znaleziono funkcji w " << nazwa_dll << "!\n";
        FreeLibrary(hDll);
        hDll = nullptr;
        return false;
    }

    aktualna_biblioteka = nazwa_dll;
    return true;
}

double konwertujObrazJednowatkowo(const std::string& sciezka_obrazu, const std::string& podfolder) {

    auto t0_calkowity = std::chrono::high_resolution_clock::now();
    cv::Mat lokalny_obraz = cv::imread(sciezka_obrazu);
    if (lokalny_obraz.empty()) {
        std::cerr << "Blad: nie mozna wczytac obrazu: " << sciezka_obrazu << "\n";
        return 0.0;
    }

    fs::path sciezka_pliku(sciezka_obrazu);
    std::string lokalna_nazwa_obrazu = sciezka_pliku.stem().string();

    std::string wymagana_biblioteka = tryb_asm ? "konwersja_asm.dll" : "konwersja_cpp.dll";
    if (aktualna_biblioteka != wymagana_biblioteka) {
        if (!zaladujDLL(tryb_asm)) {
            return 0.0;
        }
    }

    if (!dll_get_buffer_size || !dll_konwertuj_ascii) {
        return 0.0;
    }

    constexpr double wsp_proporcji = 1.8;
    int wysokosc_ascii = static_cast<int>((szerokosc_ascii * lokalny_obraz.rows) / (lokalny_obraz.cols * wsp_proporcji));

    cv::Mat przeskalowany;
    cv::resize(lokalny_obraz, przeskalowany, cv::Size(szerokosc_ascii, wysokosc_ascii), 0, 0, cv::INTER_AREA);
    cv::Mat szary;
    cv::cvtColor(przeskalowany, szary, cv::COLOR_BGR2GRAY);

  

    // JEDNOW¥TKOWA konwersja - ca³y obraz naraz
    auto t0_dll = std::chrono::high_resolution_clock::now();

    int buf_size = dll_get_buffer_size(szary.cols, szary.rows);
    std::vector<char> buffer(buf_size);

    dll_konwertuj_ascii(
        szary.data,
        szary.cols,
        szary.rows,
        buffer.data(),
        buf_size,
        0,              // start_row = 0
        szary.rows      // end_row = wszystkie wiersze
    );

    auto t1_dll = std::chrono::high_resolution_clock::now();
    double lokalny_czas_dll_ms = std::chrono::duration<double, std::milli>(t1_dll - t0_dll).count();



    if (buf_size > 0) {
        buffer[buf_size - 1] = '\0';
    }

    std::string lokalny_ascii_txt = buffer.data();

    std::vector<std::string> linie;
    std::string linia;
    for (char c : lokalny_ascii_txt) {
        if (c == '\n') { linie.push_back(linia); linia.clear(); }
        else linia += c;
    }

    int szer = linie.empty() ? 0 : (int)linie[0].size();
    int wys = (int)linie.size();

    int char_width = 12;
    int char_height = 22;

    cv::Mat lokalny_ascii_obraz = cv::Mat(wys * char_height, szer * char_width, CV_8UC3, cv::Scalar(255, 255, 255));
    double font_scale = 0.7;
    int font_thickness = 2;

    for (int y = 0; y < wys; y++) {
        for (int x = 0; x < szer; x++) {
            char znak = linie[y][x];
            std::string s(1, znak);
            cv::Point pos(x * char_width, (y + 1) * char_height - 4);
            cv::putText(lokalny_ascii_obraz, s, pos,
                cv::FONT_HERSHEY_SIMPLEX,
                font_scale,
                cv::Scalar(0, 0, 0),
                font_thickness,
                cv::LINE_AA);
        }
    }

    liczba_przetworzonych_obrazow++;

    std::string aktualny_podfolder = podfolder.empty() ? nazwa_folderu_wejscie : podfolder;
    zapiszWyniki(sciezka_obrazu, lokalny_ascii_txt, lokalny_ascii_obraz, aktualny_podfolder);

    auto t1_calkowity = std::chrono::high_resolution_clock::now();
    double lokalny_czas_calkowity_ms = std::chrono::duration<double, std::milli>(t1_calkowity - t0_calkowity).count();

    zapiszPomiar(lokalny_czas_calkowity_ms, lokalny_czas_dll_ms, 1, lokalna_nazwa_obrazu, aktualny_podfolder);


    return lokalny_czas_dll_ms;
}

void konwertujFolder() {
    if (pliki_obrazow.empty()) {
        return;
    }

    trwa_konwersja = true;
    laczny_czas_konwersji_ms = 0.0;
    liczba_przetworzonych_obrazow = 0;

    std::string komunikat_folder = "Konwersja folderu: " + std::to_string(pliki_obrazow.size()) + " zdjec";
    ostatni_komunikat_czasu = komunikat_folder;
    pokazOkno();
    cv::waitKey(1);

    auto t0_caly_folder = std::chrono::high_resolution_clock::now();

    unsigned int hw = std::thread::hardware_concurrency();
    if (hw == 0) hw = 2;
    int total_images = static_cast<int>(pliki_obrazow.size());
    int workers = std::max(1, liczba_watkow);

    int base = total_images / workers;
    int extra = total_images % workers;

    std::vector<std::thread> thread_pool;
    thread_pool.reserve(static_cast<size_t>(workers));

    double suma_czasow_dll = 0.0;
    std::mutex dll_mutex;

    // RÓWNOLEG£OŒÆ TYLKO NA POZIOMIE OBRAZÓW 
    for (int w = 0; w < workers; ++w) {
        int start_index = w * base + std::min(w, extra);
        int count = base + (w < extra ? 1 : 0);
        int end_index = start_index + count;

        thread_pool.emplace_back(
            [start_index, end_index, &suma_czasow_dll, &dll_mutex]() {
                double lokalna_suma_dll = 0.0;

                for (int idx = start_index; idx < end_index; ++idx) {
                    const std::string& sciezka = pliki_obrazow[idx];

                    // JEDNOW¥TKOWA konwersja ka¿dego obrazu
                    double czas_dll_obrazu = konwertujObrazJednowatkowo(sciezka, "");
                    lokalna_suma_dll += czas_dll_obrazu;
                }

                std::lock_guard<std::mutex> lock(dll_mutex);
                suma_czasow_dll += lokalna_suma_dll;
            }
        );
    }

    for (auto& th : thread_pool) {
        if (th.joinable()) th.join();
    }

    auto t1_caly_folder = std::chrono::high_resolution_clock::now();
    double czas_calego_folderu_ms = std::chrono::duration<double, std::milli>(t1_caly_folder - t0_caly_folder).count();

    zapiszRaportFolderu(czas_calego_folderu_ms, suma_czasow_dll, total_images, nazwa_folderu_wejscie);

    std::stringstream komunikat;
    komunikat << "Folder - Calkowity: " << std::fixed << std::setprecision(1) << czas_calego_folderu_ms << " ms "
        << "DLL (suma): " << std::fixed << std::setprecision(1) << suma_czasow_dll << " ms "
        << "(" << total_images << " zdjec, " << workers << " watkow)";

    ostatni_komunikat_czasu = komunikat.str();

    if (!pliki_obrazow.empty() && aktualny_plik_index >= 0) {
        obraz = cv::imread(pliki_obrazow[aktualny_plik_index]);
        if (!obraz.empty()) {
            fs::path sciezka_pliku(pliki_obrazow[aktualny_plik_index]);
            std::string nazwa_pliku = sciezka_pliku.stem().string();
            std::string tryb_tekst = tryb_asm ? "ASM" : "CPP";
            std::string sciezka_ascii_png = folder_wyniki + "/" + nazwa_folderu_wejscie + "/" + nazwa_pliku + "/" + nazwa_pliku + "_ASCII_" + tryb_tekst + ".png";

            ascii_obraz = cv::imread(sciezka_ascii_png);
            if (ascii_obraz.empty()) {
                ascii_obraz.release();
                ascii_txt.clear();
            }
            gotowy = !ascii_obraz.empty();
        }
    }

    std::string komunikat_zapis = "Zdjecia zapisano w folderze: " + folder_wyniki + "/" + nazwa_folderu_wejscie;
    tinyfd_messageBox("Konwersja zakonczona", komunikat_zapis.c_str(), "ok", "info", 1);

    trwa_konwersja = false;
    pokazOkno();
}

bool wykonajKonwersjePojedynczegoObrazu(const std::string& sciezka_obrazu, const std::string& podfolder) {
    cv::Mat lokalny_obraz = cv::imread(sciezka_obrazu);
    if (lokalny_obraz.empty()) {
        std::cerr << "Blad: nie mozna wczytac obrazu: " << sciezka_obrazu << "\n";
        return false;
    }

    fs::path sciezka_pliku(sciezka_obrazu);
    std::string lokalna_nazwa_obrazu = sciezka_pliku.stem().string();

    std::string wymagana_biblioteka = tryb_asm ? "konwersja_asm.dll" : "konwersja_cpp.dll";
    if (aktualna_biblioteka != wymagana_biblioteka) {
        if (!zaladujDLL(tryb_asm)) {
            return false;
        }
    }

    if (!dll_get_buffer_size || !dll_konwertuj_ascii) {
        return false;
    }

    auto t0_calkowity = std::chrono::high_resolution_clock::now();

    constexpr double wsp_proporcji = 1.8;
    int wysokosc_ascii = static_cast<int>((szerokosc_ascii * lokalny_obraz.rows) / (lokalny_obraz.cols * wsp_proporcji));

    cv::Mat przeskalowany;
    cv::resize(lokalny_obraz, przeskalowany, cv::Size(szerokosc_ascii, wysokosc_ascii), 0, 0, cv::INTER_AREA);
    cv::Mat szary;
    cv::cvtColor(przeskalowany, szary, cv::COLOR_BGR2GRAY);

    // JEDNOW¥TKOWA konwersja - ca³y obraz naraz
    auto t0_dll = std::chrono::high_resolution_clock::now();

    int buf_size = dll_get_buffer_size(szary.cols, szary.rows);
    std::vector<char> buffer(buf_size);

    dll_konwertuj_ascii(
        szary.data,
        szary.cols,
        szary.rows,
        buffer.data(),
        buf_size,
        0,              // start_row = 0
        szary.rows      // end_row = wszystkie wiersze
    );

    auto t1_dll = std::chrono::high_resolution_clock::now();
    double lokalny_czas_dll_ms = std::chrono::duration<double, std::milli>(t1_dll - t0_dll).count();

    if (buf_size > 0) {
        buffer[buf_size - 1] = '\0';
    }

    std::string lokalny_ascii_txt = buffer.data();

    std::vector<std::string> linie;
    std::string linia;
    for (char c : lokalny_ascii_txt) {
        if (c == '\n') { linie.push_back(linia); linia.clear(); }
        else linia += c;
    }

    int szer = linie.empty() ? 0 : (int)linie[0].size();
    int wys = (int)linie.size();

    int char_width = 12;
    int char_height = 22;

    cv::Mat lokalny_ascii_obraz = cv::Mat(wys * char_height, szer * char_width, CV_8UC3, cv::Scalar(255, 255, 255));
    double font_scale = 0.7;
    int font_thickness = 2;

    for (int y = 0; y < wys; y++) {
        for (int x = 0; x < szer; x++) {
            char znak = linie[y][x];
            std::string s(1, znak);
            cv::Point pos(x * char_width, (y + 1) * char_height - 4);
            cv::putText(lokalny_ascii_obraz, s, pos,
                cv::FONT_HERSHEY_SIMPLEX,
                font_scale,
                cv::Scalar(0, 0, 0),
                font_thickness,
                cv::LINE_AA);
        }
    }

    obraz = lokalny_obraz.clone();
    ascii_txt = lokalny_ascii_txt;
    ascii_obraz = lokalny_ascii_obraz.clone();
    nazwa_obrazu = lokalna_nazwa_obrazu;
    czas_konwersja_ms = lokalny_czas_dll_ms;
    gotowy = true;

    std::string aktualny_podfolder = podfolder.empty() ? nazwa_obrazu : podfolder;
    zapiszWyniki(sciezka_obrazu, lokalny_ascii_txt, lokalny_ascii_obraz, aktualny_podfolder);

    auto t1_calkowity = std::chrono::high_resolution_clock::now();
    double lokalny_czas_calkowity_ms = std::chrono::duration<double, std::milli>(t1_calkowity - t0_calkowity).count();

    zapiszPomiar(lokalny_czas_calkowity_ms, lokalny_czas_dll_ms, 1, lokalna_nazwa_obrazu, aktualny_podfolder);

    std::string tryb_tekst_display = tryb_asm ? "ASM" : "CPP";
    std::stringstream komunikat;
    komunikat << "Pojedynczy - Calkowity: " << std::fixed << std::setprecision(1) << lokalny_czas_calkowity_ms << " ms "
        << "DLL: " << std::fixed << std::setprecision(4) << lokalny_czas_dll_ms << " ms "
        << "(1 watek, " << tryb_tekst_display << ")";
    ostatni_komunikat_czasu = komunikat.str();

    return true;
}