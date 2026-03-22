/**
 * @file gui.h
 * @brief Deklaracje funkcji interfejsu u¿ytkownika
 * @author B³a¿ej Jamrozik
 * @date Semestr zimowy 2025/2026
 */

#ifndef GUI_H
#define GUI_H

#include <opencv2/opencv.hpp>
#include <string>

 /**
  * @brief Rysuje interfejs u¿ytkownika z przyciskami i kontrolkami
  * @param gui Referencja do obrazu GUI
  */
void narysujPrzyciski(cv::Mat& gui);

/**
 * @brief Skaluje obraz zachowuj¹c proporcje
 * @param obraz Obraz wejœciowy do skalowania
 * @param max_szer Maksymalna szerokoœæ wyniku
 * @param max_wys Maksymalna wysokoœæ wyniku
 * @return Przeskalowany obraz
 */
cv::Mat skalujZProporcjami(const cv::Mat& obraz, int max_szer, int max_wys);

/**
 * @brief Aktualizuje i wyœwietla g³ówne okno aplikacji
 */
void pokazOkno();

/**
 * @brief Obs³uguje zdarzenia myszy w interfejsie u¿ytkownika
 * @param event Typ zdarzenia myszy
 * @param x Wspó³rzêdna x klikniêcia
 * @param y Wspó³rzêdna y klikniêcia
 * @param flags Dodatkowe flagi zdarzenia
 * @param userdata Dane u¿ytkownika (nieu¿ywane)
 */
void onMouse(int event, int x, int y, int flags, void* userdata);

#endif 