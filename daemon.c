#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <stdbool.h>
#include <fcntl.h>
#include <libgen.h>
#include <time.h>

void copyFiles(char *path_to_src, char *path_to_dest) {
     // kopiowanie
     char *buffer = (char*)malloc(16384);
     int src_file = open(path_to_src, O_RDONLY);
     int dest_file = open(path_to_dest, O_WRONLY | O_CREAT);
     int bytes;

     while((bytes = read(src_file, buffer, sizeof(buffer))) > 0) {
          write(dest_file, buffer, bytes);
     }
     close(src_file);
     close(dest_file);
     free(buffer);
}

int main() {
     // zmienne przechowujące nazwy ścieżek - źródłowej i docelowej
     char src_path[] = "zrodlowy";
     char dest_path[] = "docelowy";

     // utworzenie struktur, które będą zawierać info o typie plików w ścieżce
     struct stat src_stat, dest_stat;
     int sleep_time = 0;

     // petla sprawdzająca na bieżąco ścieżki
     // kończy się jeżeli w ścieżce jest plik lub gdy nie można pobrać informacji o pliku lub katalogu
     while (1) {
          // jeżeli nie można pobrać info o ścieżkach to program kończy pracę
          // jeżeli plik (0) to program kończy prace 
          // jeżeli katalog (1) to program czeka podany czas (sleep_time) i zaczyna prace
          if (stat(src_path, &src_stat) == -1 || stat(dest_path, &dest_stat) == -1) {
               printf("Error: Failed to get stats\n");
               exit(1);
          } else if (S_ISDIR(src_stat.st_mode) == 0 && S_ISDIR(dest_stat.st_mode) == 0) {
               printf("Error: Given paths are not directories\n");
               exit(1);
          } else if (S_ISDIR(src_stat.st_mode) == 1 && S_ISDIR(dest_stat.st_mode) == 1) {
               sleep(sleep_time);
      
               // wskaźniki na struktury katalogów
               DIR *src_dir, *dest_dir;

               // utworzenie struktur dirent, które będą zawierać info o wszystkich plikach w katalogach
               struct dirent *src_file_info, *dest_file_info;

               // otworzenie katalogów
               src_dir = opendir(src_path);
               dest_dir = opendir(dest_path);

               // iteracja przez pliki w katalogu źródłowym
               while ((src_file_info = readdir(src_dir)) != NULL) {
                    bool exists = false;

                    //bufor na ścieżke pliku żródłowego
                    char path_to_src[128];

                    // zapisanie ścieżki w buforze
                    snprintf(path_to_src, sizeof(path_to_src), "%s/%s", src_path, src_file_info->d_name);

                    struct stat src_file_stat;
                    stat(path_to_src, &src_file_stat);

                    // iteracja przez pliki w katalogu docelowym
                    while((dest_file_info = readdir(dest_dir)) != NULL) {
                         //bufor na ścieżke pliku docelowego
                         char path_to_dest[128];

                         // zapisanie ścieżki w buforze
                         snprintf(path_to_dest, sizeof(path_to_dest), "%s/%s", dest_path, src_file_info->d_name);

                         //struktury na informacje o pliku
                         struct stat dest_file_stat;
                         stat(path_to_dest, &dest_file_stat);

                         // porównanie nazw plików
                         if (strcmp(src_file_info->d_name, dest_file_info->d_name) == 0) {
                              exists = true;

                              // zainicjalizowanie dat modyfikacji plików
                              time_t src_file_mtime = src_file_stat.st_mtime, dest_file_mtime = dest_file_stat.st_mtime;

                              if (difftime(src_file_mtime, dest_file_mtime) > 0) {
                                   remove(path_to_dest);

                                   // wywołanie funkcji kopiującej
                                   copyFiles(path_to_src, path_to_dest);
                              } 
                              break;
                         }
                    }

                    // jeżeli plik nie istnieje to jest kopiowany z 1 do 2 katalogu
                    if (exists == false && S_ISDIR(src_file_stat.st_mode) == 0) {
                         // bufory na ścieżki plików
                         char path_to_dest[128]; 

                         // połączenie ścieżek w jeden string
                         snprintf(path_to_dest, sizeof(path_to_dest), "%s/%s", dest_path, src_file_info->d_name);

                         // wywołanie funkcji kopiującej
                         copyFiles(path_to_src, path_to_dest);
                    }
                    rewinddir(dest_dir);
               }
               rewinddir(src_dir);

               // iteracja w drugą stronę, tj. porównanie plików z katalogu 2 z plikami z katalogu 1 i ew. usunięcie ich

               // zamknięcie katalogów
               closedir(src_dir);
               closedir(dest_dir);
          }
     }
     return 0;
}