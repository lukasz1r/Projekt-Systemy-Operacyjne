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

int main() {
     //zmienne przechowujące nazwy ścieżek - źródłowej i docelowej
     char src_path[] = "zrodlowy";
     char dest_path[] = "docelowy";

     //utworzenie struktur, które będą zawierać info o typie plików w ścieżce
     struct stat src_stat, dest_stat;
     int sleep_time = 5;

     //petla sprawdzająca na bieżąco ścieżki
     //kończy się jeżeli w ścieżce jest plik lub gdy nie można pobrać informacji o pliku lub katalogu
     while (1) {
          //sprawdzenie informacji o pliku lub katalogu
          if (stat(src_path, &src_stat) == -1 || stat(dest_path, &dest_stat) == -1) {
               printf("Error: Failed to get stats\n");
               exit(1);
          }
          //sprawdzenie czy ścieżka ma typ katalogu lub pliku
          //jeżeli katalog (1) to program czeka podany czas (sleep_time) i zaczyna prace
          //jeżeli plik (0) to program kończy prace 
          if (S_ISDIR(src_stat.st_mode) == 1 && S_ISDIR(dest_stat.st_mode) == 1) {

               //czekanie
               sleep(sleep_time);
      
               //wskaźniki na struktury katalogów
               DIR *src_dir, *dest_dir;
               //utworzenie struktur dirent, które będą zawierać info o wszystkich plikach w katalogach
               struct dirent *src_file_info, *dest_file_info;

               //otworzenie katalogów
               src_dir = opendir(src_path);
               dest_dir = opendir(dest_path);
               //iteracja przez pliki w katalogu źródłowym
               while ((src_file_info = readdir(src_dir)) != NULL) {
                    bool exists = false;
                    // iteracja przez pliki w katalogu docelowym
                    while((dest_file_info = readdir(dest_dir)) != NULL) {
                         // pobranie informacji o plikach za pomocą stat()
                         char path_to_src[128], path_to_dest[128]; 

                         strcpy(path_to_src, src_path);
                         strcat(path_to_src, "/");
                         strcat(path_to_src, src_file_info->d_name);

                         strcpy(path_to_dest, dest_path);
                         strcat(path_to_dest, "/");
                         strcat(path_to_dest, dest_file_info->d_name);

                         struct stat src_file_stat, dest_file_stat;
                         stat(path_to_src, &src_file_stat);
                         stat(path_to_dest, &dest_file_stat);

                         // porównanie nazw plików
                         if (strcmp(src_file_info->d_name, dest_file_info->d_name) == 0) {
                              exists = true;

                              time_t mod_time_f1 = src_file_stat.st_mtime, mod_time_f2 = dest_file_stat.st_mtime;
                              if (difftime(mod_time_f1, mod_time_f2) > 0) {
                                   printf("rozni sie");
                                   remove(path_to_dest);
                                   char buffer[16384];
                                   int src_file = open(path_to_src, O_RDONLY);
                                   int dest_file = open(path_to_dest, O_WRONLY | O_CREAT);
                                   int bytes;

                                   while((bytes = read(src_file, buffer, sizeof(buffer))) > 0)
                                   {
                                        write(dest_file, buffer, bytes);
                                   }
                                   close(src_file);
                                   close(dest_file);
                              } 
                              break;
                         }
                    }

                    // jeżeli plik nie istnieje to jest kopiowany z 1 do 2 katalogu
                    if (exists == false) {
                         char path_to_src[128], path_to_dest[128]; 
                         // połączenie ścieżek w jeden string
                         strcpy(path_to_src, src_path);
                         strcat(path_to_src, "/");
                         strcat(path_to_src, src_file_info->d_name);

                         strcpy(path_to_dest, dest_path);
                         strcat(path_to_dest, "/");
                         strcat(path_to_dest, src_file_info->d_name);

                         // zwykłe kopiowanie z jednej scieżki do drugiej
                         char buffer[16384];
                         int src_file = open(path_to_src, O_RDONLY);
                         int dest_file = open(path_to_dest, O_WRONLY | O_CREAT);
                         int bytes;

                         while((bytes = read(src_file, buffer, sizeof(buffer))) > 0)
                         {
                              write(dest_file, buffer, bytes);
                         }
                         close(src_file);
                         close(dest_file);
                    }
                    rewinddir(dest_dir);
               }

               //iteracja w drugą stronę, tj. porównanie plików z katalogu 2 z plikami z katalogu 1 i ew. usunięcie ich

               //zamknięcie katalogów
               closedir(src_dir);
               closedir(dest_dir);

          } else if (S_ISDIR(src_stat.st_mode) == 0 && S_ISDIR(dest_stat.st_mode) == 0) {
               printf("Error: Given paths are not directories\n");
               exit(1);
          }
     }

     return 0;
}