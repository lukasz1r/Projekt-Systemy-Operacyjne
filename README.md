# Projekt-Systemy-Operacyjne
Informacje ogólne

<p>1. Projekty pisane są w grupach 2-4 osobowych.</p>
<p>2. Odbiór projektu polega na dostarczeniu kodu źródłowego (z komentarzami) oraz
sprawozdania w postaci pliku PDF.</p>
<p>3. Sprawozdanie powinno zawierać:</p>
<ol type="a">
<li>dane wykonawców projektu i prowadzącego zajęcia + treść zadania,</li>
<li>opis poszczególnych modułów programu (funkcji i zmiennych globalnych),</li>
<li>zrzuty ekranu działającego programu (demonstracja poszczególnych funkcjonalności),</li>
<li>należy jasno opisać podział pracy.</li>
</ol>
<hr>
<br>
<h2>Temat 1 – Demon synchronizujący dwa podkatalogi</h2>
<hr>

<p>[12p.] Program który otrzymuje co najmniej dwa argumenty: ścieżkę źródłową
oraz ścieżkę docelową . Jeżeli któraś ze ścieżek nie jest katalogiem program
powraca natychmiast z komunikatem błędu. W przeciwnym wypadku staje się
demonem. Demon wykonuje następujące czynności: śpi przez pięć minut (czas
spania można zmieniać przy pomocy dodatkowego opcjonalnego argumentu), po
czym po obudzeniu się porównuje katalog źródłowy z katalogiem docelowym.
Pozycje które nie są zwykłymi plikami są ignorowane (np. katalogi i dowiązania
symboliczne). Jeżeli demon (a) napotka na nowy plik w katalogu źródłowym, i
tego pliku brak w katalogu docelowym lub (b) plik w katalogu źrodłowym ma
późniejszą datę ostatniej modyfikacji demon wykonuje kopię pliku z katalogu
źródłowego do katalogu docelowego - ustawiając w katalogu docelowym datę
modyfikacji tak aby przy kolejnym obudzeniu nie trzeba było wykonać kopii
(chyba ze plik w katalogu źródłowym zostanie ponownie zmieniony). Jeżeli zaś
odnajdzie plik w katalogu docelowym, którego nie ma w katalogu źródłowym to
usuwa ten plik z katalogu docelowego. Możliwe jest również natychmiastowe
obudzenie się demona poprzez wysłanie mu sygnału SIGUSR1. Wyczerpująca
informacja o każdej akcji typu uśpienie/obudzenie się demona (naturalne lub w
wyniku sygnału), wykonanie kopii lub usunięcie pliku jest przesłana do logu
systemowego. Informacja ta powinna zawierać aktualną datę.</p>
<br><br><hr>
<p><b>Dodatkowo:</b></p>
<br>
<p>a) [10p.] Dodatkowa opcja -R pozwalająca na rekurencyjną synchronizację
katalogów (teraz pozycje będące katalogami nie są ignorowane). W szczególności
jeżeli demon stwierdzi w katalogu docelowym podkatalog którego brak w
katalogu źródłowym powinien usunąć go wraz z zawartością.</p>
<br>
<p>b) [12p.] W zależności od rozmiaru plików dla małych plików wykonywane jest
kopiowanie przy pomocy read/write a w przypadku dużych przy pomocy
mmap/write (plik źródłowy) zostaje zamapowany w całości w pamięci. Próg
dzielący pliki małe od dużych może być przekazywany jako opcjonalny argument.
Uwagi: (a) Wszelkie operacje na plikach należy wykonywać przy pomocy API
Linuksa a nie standardowej biblioteki języka C (b) kopiowanie za każdym
obudzeniem całego drzewa katalogów zostanie potraktowane jako poważny błąd</p>
<br>
<p>(c) podobnie jak przerzucenie części zadań na shell systemowy (funkcja system). </p>
<br><hr>

<h2>Projekt nr 1 – materiały pomocnicze</h2>
<br>
<p>a) literatura</p>
1. R. Love – „Linux. Programowanie systemowe”
2. M. Mitchell – „Linux. Programowanie dla zaawansowanych”
<br>
<p>b) przydatne funkcje</p>
fork(), setsid(), chdir(), exit(), signal(), kill(), openlog(), syslog(), closelog(),
open(), read(), write(), close(), lseek(), mmap(), memcpy(), munmap(), strcmp(),
strcpy(), strcat(), strtoul(), strstr(), strlen(), atoi(), stat(), opendir(), readdir(),
closedir(), rewinddir(), remove(), unlink(), rmdir(), mkdir(), chmod(), access(),
sleep()
<br>
<p>c) użyteczne struktury i typy</p>
struct stat, struct dirent, DIR, mode_t, off_t, time_t
<br>
<p>d) wysyłanie sygnałów</p>
Sygnał można wysłać za pomocą wywołania systemowego kill lub polecenia kill
i podając żądany pid procesu.
<br>
<p>e) inne</p>
argumenty funkcji main() 



 
<!-- Informacje ogólne

1. Projekty pisane są w grupach 2-4 osobowych.
2. Odbiór projektu polega na dostarczeniu kodu źródłowego (z komentarzami) oraz
sprawozdania w postaci pliku PDF.
3. Sprawozdanie powinno zawierać:
a) dane wykonawców projektu i prowadzącego zajęcia + treść zadania,
b) opis poszczególnych modułów programu (funkcji i zmiennych globalnych),
c) zrzuty ekranu działającego programu (demonstracja poszczególnych
funkcjonalności),
d) należy jasno opisać podział pracy.  -->