; =============================================================================
; TEMAT PROJEKTU: Konwerter obrazu do ASCII Art
; 
; OPIS ALGORYTMU:
; Program konwertuje obraz w skali szaroœci na reprezentacjê ASCII
; wykorzystuj¹c¹ 64 znaki o ró¿nej gêstoœci. Algorytm mapuje wartoœæ
; jasnoœci piksela (0-255) na indeks znaku ASCII (0-63).
;
; DATA WYKONANIA: Semestr zimowy 2025/2026
; AUTOR: B³a¿ej Jamrozik
;
; WERSJA: 2.0
; HISTORIA ZMIAN:
; v1.0 - Podstawowa implementacja bez u¿ycia SSE4.1
; v2.0 - Optymalizacja z wykorzystaniem SSE4.1
; =============================================================================

option casemap:none

.data
; =============================================================================
; ZNAKI - tablica 64 znaków ASCII uporz¹dkowanych wed³ug rosn¹cej jasnoœci
; Zakres: 64 znaki od najciemniejszego ($) do najjaœniejszego (spacja)
; =============================================================================
ZNAKI BYTE "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/()1{}[]?-_+~<>i!lI;:,^ ",0

; =============================================================================
; fixed_scale - sta³a fixed-point do mapowania jasnoœci 0-255 na indeksy 0-63
; =============================================================================
ALIGN 16
fixed_scale WORD 8 DUP (63)

; =============================================================================
; tmp_idx - bufor pomocniczy do przechowywania 16 tymczasowych indeksów
; Przeznaczenie: Przechowuje wyniki konwersji przed mapowaniem na znaki ASCII
; =============================================================================
ALIGN 16
tmp_idx BYTE 16 DUP (0)

.code

; =============================================================================
; PROCEDURA: get_required_buffer_size
; 
; OPIS: Oblicza minimalny rozmiar bufora potrzebny do przechowania
;       wyniku konwersji ASCII uwzglêdniaj¹c znaki nowej linii
;
; PARAMETRY WEJŒCIOWE:
;   rcx - width  (szerokoœæ obrazu w pikselach, zakres: > 0)
;   rdx - height (wysokoœæ obrazu w pikselach, zakres: > 0)
;
; PARAMETRY WYJŒCIOWE:
;   rax - rozmiar bufora w bajtach (0 w przypadku b³êdu)
;
; ZMIENIANE REJESTRY: rax, rflags
; =============================================================================
PUBLIC get_required_buffer_size
get_required_buffer_size PROC
    push rbp                   ; zapisuje wskaŸnik ramki stosu
    mov  rbp, rsp              ; ustawia now¹ ramkê stosu

    ; --- Walidacja parametru width ---
    mov  eax, ecx              ; kopiuje width do eax
    test eax, eax              ; sprawdza czy width > 0
    jle  ret_zero              ; jeœli <= 0, ustawia wynik na 0

    ; --- Walidacja parametru height ---
    mov  eax, edx              ; kopiuje height do eax  
    test eax, eax              ; sprawdza czy height > 0
    jle  ret_zero              ; jeœli <= 0, ustawia wynik na 0

    ; --- Obliczanie rozmiaru bufora: (width * height) + height + 1 ---
    mov  rax, rcx              ; rax = width
    imul rax, rdx              ; rax = width * height
    add  rax, rdx              ; dodaje height
    add  rax, 1                ; dodaje 1 dla znaku koñca bufora
    
    ; --- Sprawdzenie przepe³nienia INT32_MAX ---
    cmp  rax, 7FFFFFFFh        ; sprawdza czy nie przekracza INT32_MAX
    ja   ret_zero              ; jeœli tak, ustawia wynik na 0

    mov  eax, eax              ; obcina do 32-bit (zgodnoœæ z C++)
    jmp  exit_grs              ; przechodzi do wyjœcia

ret_zero:
    xor  eax, eax              ; ustawia wynik na 0 w przypadku b³êdu

exit_grs:
    pop  rbp                   ; przywraca poprzedni wskaŸnik ramki
    ret                        ; koñczy procedurê
get_required_buffer_size ENDP

; =============================================================================
; PROCEDURA: jasnoscNaASCII
;
; OPIS: Mapuje wartoœæ jasnoœci piksela (0-255) na odpowiedni znak ASCII
;       z tablicy ZNAKI u¿ywaj¹c fixed-point arithmetic
;
; PARAMETRY WEJŒCIOWE:
;   rcx - jasnosc (wartoœæ jasnoœci piksela, zakres: 0-255)
;
; PARAMETRY WYJŒCIOWE:
;   al - znak ASCII odpowiadaj¹cy jasnoœci
;
; ZMIENIANE REJESTRY: rax, rdx, rflags
; =============================================================================
PUBLIC jasnoscNaASCII
jasnoscNaASCII PROC
    push rdx                   ; zapisuje oryginalny rdx na stosie

    ; --- Fixed-point calculation: (jasnosc * 63) >> 8 ---
    movzx eax, cl              ; rozszerza bajt jasnoœci do 32-bit
    imul eax, 63               ; mno¿y przez 63
    shr  eax, 8                ; dzieli przez 256 (fixed-point >> 8)

    ; --- Clampowanie wyniku do zakresu 0-63 ---
    cmp eax, 63                ; sprawdza czy > 63
    ja  set63                  ; jeœli tak, ustawia wynik na 63
    cmp eax, 0                 ; sprawdza czy < 0
    jl  set0                   ; jeœli tak, ustawia wynik na 0
    jmp get_char               ; jeœli w zakresie, pobiera znak

set63:
    mov  rdx, OFFSET ZNAKI     ; ³aduje adres tablicy znaków
    mov  al, BYTE PTR [rdx + 63] ; pobiera ostatni znak z tablicy
    jmp  done                  ; przechodzi do koñca procedury

set0:
    mov  rdx, OFFSET ZNAKI     ; ³aduje adres tablicy znaków
    mov  al, BYTE PTR [rdx + 0]  ; pobiera pierwszy znak z tablicy
    jmp  done                  ; przechodzi do koñca procedury

get_char:
    mov  rdx, OFFSET ZNAKI     ; ³aduje adres tablicy znaków
    mov  dl, BYTE PTR [rdx + rax] ; pobiera znak na podstawie indeksu
    mov  al, dl                ; przenosi wynik do al

done:
    pop  rdx                   ; przywraca oryginalny rdx
    ret                        ; koñczy procedurê
jasnoscNaASCII ENDP

; =============================================================================
; PROCEDURA: konwertuj_ascii_core
;
; OPIS: G³ówna procedura konwersji obrazu w skali szaroœci na ASCII art
;       wykorzystuj¹ca instrukcje SSE do przetwarzania wektorowego
;
; PARAMETRY WEJŒCIOWE:
;   rcx - dane_szare   (wskaŸnik do danych obrazu w skali szaroœci)
;   rdx - width        (szerokoœæ obrazu w pikselach, zakres: > 0)
;   r8  - height       (wysokoœæ obrazu w pikselach, zakres: > 0)  
;   r9  - out_buf      (wskaŸnik do bufora wyjœciowego)
;   [rsp+40] - out_buf_size (rozmiar bufora wyjœciowego, zakres: > 0)
;   [rsp+48] - start_row    (pierwszy wiersz do przetworzenia, zakres: 0 do height-1)
;   [rsp+56] - end_row      (wiersz koñcowy + 1, zakres: 1 do height)
;
; PARAMETRY WYJŒCIOWE:
;   eax - kod wyniku (0 = sukces, 1 = NULL pointer, 2 = nieprawid³owy rozmiar, 4 = nieprawid³owy zakres)
;
; ZMIENIANE REJESTRY: 
;   rax, rbx, rsi, rdi, r8-r15, xmm0-xmm7, rflags
; =============================================================================
PUBLIC konwertuj_ascii_core
konwertuj_ascii_core PROC
    ; --- Pobiera argumenty ze stosu ---
    mov  eax, DWORD PTR [rsp + 40]   ; out_buf_size
    mov  r14d, DWORD PTR [rsp + 48]  ; start_row
    mov  r15d, DWORD PTR [rsp + 56]  ; end_row

    ; --- Zapisuje nieulotne rejestry ---
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15

    ; --- Inicjalizacja parametrów do rejestrów roboczych ---
    mov  rsi, rcx          ; rsi = wskaŸnik do danych szaroœci
    mov  ebx, edx          ; ebx = szerokoœæ obrazu
    mov  r12d, r8d         ; r12 = wysokoœæ obrazu  
    mov  rdi, r9           ; rdi = wskaŸnik do bufora wyjœciowego
    mov  r13d, eax         ; r13 = rozmiar bufora wyjœciowego

    ; --- Walidacja parametrów wejœciowych ---
    cmp  rsi, 0            ; sprawdza czy dane_szare != NULL
    je   err_1             ; b³¹d 1: NULL pointer
    cmp  rdi, 0            ; sprawdza czy out_buf != NULL  
    je   err_1             ; b³¹d 1: NULL pointer
    test ebx, ebx          ; sprawdza czy width > 0
    jle  err_2             ; b³¹d 2: nieprawid³owy rozmiar
    test r12d, r12d        ; sprawdza czy height > 0
    jle  err_2             ; b³¹d 2: nieprawid³owy rozmiar
    cmp  r14d, 0           ; sprawdza czy start_row >= 0
    jl   err_4             ; b³¹d 4: nieprawid³owy zakres wierszy
    cmp  r15d, r12d        ; sprawdza czy end_row <= height
    ja   err_4             ; b³¹d 4: nieprawid³owy zakres wierszy
    cmp  r14d, r15d        ; sprawdza czy start_row < end_row
    jge  err_4             ; b³¹d 4: nieprawid³owy zakres wierszy

    ; --- Inicjalizacja sta³ych SSE dla fixed-point ---
    movdqa xmm7, XMMWORD PTR [fixed_scale] ; ³aduje sta³¹ fixed-point [63,63,...]

    mov  r10d, r14d        ; r10 = licznik wierszy (y = start_row)

; =============================================================================
; PÊTLA PO WIERSZACH
; Przetwarza wiersze obrazu od start_row do end_row-1
; =============================================================================
row_loop:
    cmp  r10d, r15d        ; sprawdza czy y < end_row
    jge  all_done          ; jeœli nie, koñczy przetwarzanie

    ; --- Oblicza wskaŸniki do bie¿¹cego wiersza ---
    ; src_row = dane_szare + y * width
    mov  eax, r10d         ; eax = y
    imul eax, ebx          ; eax = y * width  
    mov  r8, rax           ; r8 = offset wiersza
    add  r8, rsi           ; r8 = wskaŸnik do pocz¹tku wiersza Ÿród³owego

    ; dst_row = out_buf + y * (width + 1)
    mov  eax, r10d         ; eax = y
    mov  edx, ebx          ; edx = width
    inc  edx               ; edx = width + 1 (znak nowej linii)
    imul eax, edx          ; eax = y * (width + 1)
    mov  r9, rax           ; r9 = offset wiersza
    add  r9, rdi           ; r9 = wskaŸnik do pocz¹tku wiersza docelowego

    mov  r11d, ebx         ; r11 = licznik kolumn w bie¿¹cym wierszu

; =============================================================================
; PÊTLA WEKTOROWA (SSE) - FIXED-POINT
; Przetwarza 16 pikseli jednoczeœnie
; =============================================================================
pixel_loop_vector:
    cmp  r11d, 16          ; sprawdza czy pozosta³o >= 16 pikseli
    jl   pixel_loop_scalar ; jeœli mniej, przechodzi do przetwarzania skalarnego

    ; --- Za³adowanie 16 pikseli do rejestru SSE ---
    movdqu xmm0, XMMWORD PTR [r8] ; ³aduje 16 bajtów jasnoœci do xmm0
    lea    rax, tmp_idx    ; rax = adres bufora pomocniczego

    ; --- Rozszerzenie bajtów 0-7 do words i przetworzenie ---
    pmovzxbw xmm1, xmm0    ; rozszerza bajty 0-7 do 8 words (16-bit)
    pmullw xmm1, xmm7      ; mno¿y przez 63 (fixed-point)
    psrlw xmm1, 8          ; dzieli przez 256 (>> 8)

    ; --- Rozszerzenie bajtów 8-15 do words i przetworzenie ---
    psrldq xmm0, 8         ; przesuwa o 8 bajtów w prawo (bajty 8-15)
    pmovzxbw xmm2, xmm0    ; rozszerza bajty 8-15 do 8 words (16-bit)
    pmullw xmm2, xmm7      ; mno¿y przez 63 (fixed-point)
    psrlw xmm2, 8          ; dzieli przez 256 (>> 8)

    ; --- Spakowanie wyników z powrotem do bajtów ---
    packuswb xmm1, xmm2    ; spakuje 16 words do 16 bajtów
    movdqu XMMWORD PTR [rax], xmm1 ; zapisuje wszystkie 16 indeksów do bufora

    ; --- Mapowanie indeksów na znaki ASCII ---
    push r13               ; zapisuje r13 (rozmiar bufora) na stosie
    push r14               ; zapisuje r14 (start_row/end_row) na stosie
    lea  r13, ZNAKI        ; r13 = adres tablicy znaków ASCII
    xor  r14, r14          ; r14 = licznik pêtli (0-15)

lookup_loop16:
    cmp  r14, 16           ; sprawdza czy przetworzono 16 znaków
    jge  lookup_done       ; jeœli tak, koñczy pêtlê

    movzx ecx, BYTE PTR [rax + r14] ; odczytuje indeks z bufora tymczasowego
    mov   dl, BYTE PTR [r13 + rcx]  ; pobiera znak ASCII na podstawie indeksu
    mov   BYTE PTR [r9 + r14], dl   ; zapisuje znak do bufora wyjœciowego

    inc   r14              ; zwiêksza licznik
    jmp   lookup_loop16    ; kontynuuje pêtlê

lookup_done:
    pop  r14               ; odczytuje poprzedni¹ wartoœæ r14
    pop  r13               ; odczytuje poprzedni¹ wartoœæ r13

    ; --- Aktualizacja wskaŸników i liczników ---
    add  r8, 16            ; przesuwa wskaŸnik Ÿród³owy o 16 bajtów
    add  r9, 16            ; przesuwa wskaŸnik docelowy o 16 bajtów
    sub  r11d, 16          ; zmniejsza licznik kolumn o 16
    jmp  pixel_loop_vector ; kontynuuje pêtlê wektorow¹

; =============================================================================
; PÊTLA SKALARNA
; Przetwarza pozosta³e piksele (<16) u¿ywaj¹c procedury jasnoscNaASCII
; =============================================================================
pixel_loop_scalar:
    test r11d, r11d        ; sprawdza czy pozosta³y jakieœ piksele
    jz   store_newline      ; jeœli nie, przechodzi do zapisu nowej linii

scalar_loop:
    movzx ecx, BYTE PTR [r8] ; odczytuje bajt jasnoœci z bie¿¹cej pozycji
    push r8                ; zapisuje wskaŸnik Ÿród³owy na stosie
    push r9                ; zapisuje wskaŸnik docelowy na stosie  
    push r10               ; zapisuje licznik wierszy na stosie
    push r11               ; zapisuje licznik kolumn na stosie
    call jasnoscNaASCII    ; wywo³uje konwersjê na znak ASCII
    pop  r11               ; odczytuje licznik kolumn
    pop  r10               ; odczytuje licznik wierszy
    pop  r9                ; odczytuje wskaŸnik docelowy
    pop  r8                ; odczytuje wskaŸnik Ÿród³owy
    mov  BYTE PTR [r9], al ; zapisuje znak ASCII do bufora wyjœciowego
    inc  r8                ; zwiêksza wskaŸnik Ÿród³owy
    inc  r9                ; zwiêksza wskaŸnik docelowy
    dec  r11d              ; zmniejsza licznik kolumn
    jnz  scalar_loop       ; kontynuuje pêtlê jeœli pozosta³y piksele

store_newline:
    mov  BYTE PTR [r9], 0Ah ; dodaje znak nowej linii (LF) na koniec wiersza
    inc  r10d              ; zwiêksza licznik wierszy
    jmp  row_loop          ; przechodzi do nastêpnego wiersza

; =============================================================================
; OBS£UGA B£ÊDÓW
; =============================================================================
err_1:
    mov eax, 1             ; ustawia kod b³êdu 1: NULL pointer
    jmp cleanup            ; przechodzi do sprz¹tania

err_2:
    mov eax, 2             ; ustawia kod b³êdu 2: nieprawid³owy rozmiar
    jmp cleanup            ; przechodzi do sprz¹tania

err_4:
    mov eax, 4             ; ustawia kod b³êdu 4: nieprawid³owy zakres wierszy
    jmp cleanup            ; przechodzi do sprz¹tania

all_done:
    xor eax, eax           ; ustawia kod sukcesu: 0

; =============================================================================
; SPRZ¥TANIE
; Przywraca rejestry i koñczy procedurê
; =============================================================================
cleanup:
    pop r15                ; odczytuje r15 ze stosu
    pop r14                ; odczytuje r14 ze stosu  
    pop r13                ; odczytuje r13 ze stosu
    pop r12                ; odczytuje r12 ze stosu
    pop rdi                ; odczytuje rdi ze stosu
    pop rsi                ; odczytuje rsi ze stosu
    pop rbx                ; odczytuje rbx ze stosu
    ret                    ; koñczy procedurê
konwertuj_ascii_core ENDP

END
