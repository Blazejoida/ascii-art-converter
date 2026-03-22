/**
 * @file gui.cpp
 * @brief Implementacja funkcji interfejsu u¿ytkownika
 * @author B³a¿ej Jamrozik
 * @date Semestr zimowy 2025/2026
 */

#define NOMINMAX
#include "gui.h"
#include "global_vars.h"
#include "conversion.h"
#include "file_handling.h"
#include "tinyfiledialogs.h"
#include <filesystem>
#include <algorithm>
#include <iostream>

namespace fs = std::filesystem;

void narysujPrzyciski(cv::Mat& gui) {
    cv::rectangle(gui, btn_wybierz, cv::Scalar(200, 200, 255), cv::FILLED);
    cv::rectangle(gui, btn_wybierz, cv::Scalar(0, 0, 0), 2);
    cv::putText(gui, "Wybierz", btn_wybierz.tl() + cv::Point(35, 40),
        cv::FONT_HERSHEY_SIMPLEX, 0.9, cv::Scalar(0, 0, 0), 2);

    cv::rectangle(gui, btn_konwertuj, cv::Scalar(0, 255, 0), cv::FILLED);
    cv::rectangle(gui, btn_konwertuj, cv::Scalar(0, 0, 0), 2);
    cv::rectangle(gui, btn_reset, cv::Scalar(0, 200, 255), cv::FILLED);
    cv::rectangle(gui, btn_reset, cv::Scalar(0, 0, 0), 2);
    cv::rectangle(gui, btn_wyjdz, cv::Scalar(0, 0, 255), cv::FILLED);
    cv::rectangle(gui, btn_wyjdz, cv::Scalar(0, 0, 0), 2);

    cv::putText(gui, "Konwertuj", btn_konwertuj.tl() + cv::Point(25, 40),
        cv::FONT_HERSHEY_SIMPLEX, 0.9, cv::Scalar(0, 0, 0), 2);
    cv::putText(gui, "Reset", btn_reset.tl() + cv::Point(50, 40),
        cv::FONT_HERSHEY_SIMPLEX, 0.9, cv::Scalar(0, 0, 0), 2);
    cv::putText(gui, "Wyjdz", btn_wyjdz.tl() + cv::Point(50, 40),
        cv::FONT_HERSHEY_SIMPLEX, 0.9, cv::Scalar(0, 0, 0), 2);

    cv::rectangle(gui, checkbox_asm, cv::Scalar(200, 200, 200), cv::FILLED);
    cv::rectangle(gui, checkbox_asm, cv::Scalar(0, 0, 0), 2);

    cv::rectangle(gui, checkbox_cpp, cv::Scalar(200, 200, 200), cv::FILLED);
    cv::rectangle(gui, checkbox_cpp, cv::Scalar(0, 0, 0), 2);

    if (tryb_asm) {
        cv::rectangle(gui,
            cv::Point(checkbox_asm.x + 5, checkbox_asm.y + 5),
            cv::Point(checkbox_asm.x + checkbox_asm.width - 5, checkbox_asm.y + checkbox_asm.height - 5),
            cv::Scalar(0, 150, 0), cv::FILLED);
    }
    else {
        cv::rectangle(gui,
            cv::Point(checkbox_cpp.x + 5, checkbox_cpp.y + 5),
            cv::Point(checkbox_cpp.x + checkbox_cpp.width - 5, checkbox_cpp.y + checkbox_cpp.height - 5),
            cv::Scalar(0, 150, 0), cv::FILLED);
    }

    cv::putText(gui, "Tryb ASM", checkbox_asm.tl() + cv::Point(40, 20),
        cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 0), 2);
    cv::putText(gui, "Tryb CPP", checkbox_cpp.tl() + cv::Point(40, 20),
        cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 0), 2);

    if (trwa_konwersja) {
        cv::putText(gui, "TRWA KONWERSJA...", cv::Point(740, 640),
            cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 255), 2);
    }

    if (tryb_folder) {
        std::string info_folder = "FOLDER: " + std::to_string(pliki_obrazow.size()) + " zdjec";
        cv::putText(gui, info_folder, cv::Point(740, 670),
            cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255), 2);

        if (aktualny_plik_index >= 0) {
            std::string info_plik = "Wyswietlam przykladowe zdjecie";
            cv::putText(gui, info_plik, cv::Point(740, 690),
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255), 2);
        }
    }
}

cv::Mat skalujZProporcjami(const cv::Mat& obraz, int max_szer, int max_wys) {
    double skala = std::min((double)max_szer / obraz.cols, (double)max_wys / obraz.rows);
    int nowa_szer = static_cast<int>(obraz.cols * skala);
    int nowa_wys = static_cast<int>(obraz.rows * skala);
    cv::Mat wynik;
    cv::resize(obraz, wynik, cv::Size(nowa_szer, nowa_wys));
    return wynik;
}

void pokazOkno() {
    cv::Mat lewy;
    if (!obraz.empty()) {
        lewy = skalujZProporcjami(obraz, 500, 500);
    }
    else {
        lewy = cv::Mat(500, 500, CV_8UC3, cv::Scalar(220, 220, 220));
        cv::putText(lewy, "BRAK OBRAZU", cv::Point(150, 250),
            cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(100, 100, 100), 2);
    }

    cv::Mat prawy;
    if (!ascii_obraz.empty()) {
        double skala = std::min(500.0 / ascii_obraz.cols, 500.0 / ascii_obraz.rows);
        int nowa_szer = static_cast<int>(ascii_obraz.cols * skala);
        int nowa_wys = static_cast<int>(ascii_obraz.rows * skala);
        cv::resize(ascii_obraz, prawy, cv::Size(nowa_szer, nowa_wys), 0, 0, cv::INTER_AREA);
    }
    else {
        prawy = cv::Mat(500, 500, CV_8UC3, cv::Scalar(220, 220, 220));
        cv::putText(prawy, "ASCII ART", cv::Point(150, 250),
            cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(100, 100, 100), 2);
    }

    cv::Mat gui(800, 1200, CV_8UC3, cv::Scalar(180, 180, 180));
    lewy.copyTo(gui(cv::Rect(50, 50, lewy.cols, lewy.rows)));
    prawy.copyTo(gui(cv::Rect(650, 50, prawy.cols, prawy.rows)));

    if (!ostatni_komunikat_czasu.empty()) {
        cv::putText(gui, ostatni_komunikat_czasu, cv::Point(50, 600),
            cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 0), 2);
    }

    narysujPrzyciski(gui);
    cv::imshow("Konwerter ASCII", gui);
}

void onMouse(int event, int x, int y, int, void*) {
    if (event != cv::EVENT_LBUTTONDOWN) return;

    if (btn_wybierz.contains(cv::Point(x, y))) {
        int wybor = tinyfd_messageBox("Wybierz tryb importu",
            "Wybierz co chcesz zaladowac:\n\nYes - Pojedynczy obraz\nNo - Caly folder",
            "yesno", "question", 1);

        if (wybor == 1) { // YES - pojedynczy obraz
            const char* filtr[] = { "*.png", "*.jpg", "*.jpeg", "*.bmp" };
            const char* sciezka = tinyfd_openFileDialog("Wybierz obraz", "", 4, filtr, "Pliki graficzne", 0);

            if (sciezka) {
                obraz = cv::imread(sciezka);
                if (obraz.empty()) {
                    std::cerr << "Blad: nie mozna wczytac obrazu!\n";
                    return;
                }

                fs::path sciezka_pliku(sciezka);
                nazwa_obrazu = sciezka_pliku.stem().string();
                sciezka_obrazu = sciezka;

                tryb_folder = false;
                pliki_obrazow.clear();
                aktualny_plik_index = -1;
                nazwa_folderu_wejscie = "";

                gotowy = false;
                ascii_obraz.release();
                ascii_txt.clear();
                ostatni_komunikat_czasu.clear();
                pokazOkno();
            }
        }
        else if (wybor == 0) { // NO - ca³y folder
            const char* sciezka_folder = tinyfd_selectFolderDialog("Wybierz folder z obrazami", "");

            if (sciezka_folder) {
                pliki_obrazow.clear();
                znajdzPlikiObrazow(sciezka_folder, pliki_obrazow);

                if (pliki_obrazow.empty()) {
                    tinyfd_messageBox("Blad", "W wybranym folderze nie znaleziono obrazow!", "ok", "error", 1);
                    return;
                }

                std::sort(pliki_obrazow.begin(), pliki_obrazow.end());

                int random_index = rand() % pliki_obrazow.size();
                obraz = cv::imread(pliki_obrazow[random_index]);
                if (obraz.empty()) {
                    std::cerr << "Blad: nie mozna wczytac obrazu!\n";
                    return;
                }

                fs::path sciezka_pliku(pliki_obrazow[random_index]);
                nazwa_obrazu = sciezka_pliku.stem().string();
                sciezka_obrazu = pliki_obrazow[random_index];

                fs::path folder_path(sciezka_folder);
                nazwa_folderu_wejscie = folder_path.filename().string();
                if (nazwa_folderu_wejscie.empty()) {
                    nazwa_folderu_wejscie = "folder";
                }

                tryb_folder = true;
                aktualny_plik_index = random_index;

                gotowy = false;
                ascii_obraz.release();
                ascii_txt.clear();
                ostatni_komunikat_czasu = "Folder: " + std::to_string(pliki_obrazow.size()) + " zdjec";
                pokazOkno();
            }
        }
    }

    if (btn_konwertuj.contains(cv::Point(x, y))) {
        if (tryb_folder && !pliki_obrazow.empty()) {
            konwertujFolder();
        }
        else if (!obraz.empty()) {
            trwa_konwersja = true;
            pokazOkno();
            cv::waitKey(1);

            if (wykonajKonwersjePojedynczegoObrazu(sciezka_obrazu, nazwa_obrazu)) {
                std::string komunikat_zapis = "Plik zapisano w folderze: wyniki/" + nazwa_obrazu;
                tinyfd_messageBox("Konwersja zakonczona", komunikat_zapis.c_str(), "ok", "info", 1);
            }

            trwa_konwersja = false;
            pokazOkno();
        }
    }

    if (btn_reset.contains(cv::Point(x, y))) {
        obraz.release();
        ascii_obraz.release();
        ascii_txt.clear();
        gotowy = false;
        ostatni_komunikat_czasu.clear();
        tryb_folder = false;
        pliki_obrazow.clear();
        aktualny_plik_index = -1;
        laczny_czas_konwersji_ms = 0.0;
        liczba_przetworzonych_obrazow = 0;
        nazwa_folderu_wejscie = "";
        sciezka_obrazu = "";
        pokazOkno();
    }

    if (btn_wyjdz.contains(cv::Point(x, y))) {
        if (hDll) FreeLibrary(hDll);
        cv::destroyAllWindows();
        exit(0);
    }

    if (checkbox_asm.contains(cv::Point(x, y))) {
        if (!tryb_asm) {
            if (zaladujDLL(true)) {
                tryb_asm = true;
            }
            else {
                zaladujDLL(false);
            }
            pokazOkno();
        }
    }

    if (checkbox_cpp.contains(cv::Point(x, y))) {
        if (tryb_asm) {
            if (zaladujDLL(false)) {
                tryb_asm = false;
            }
            else {
                zaladujDLL(true);
            }
            pokazOkno();
        }
    }
}