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
#include <syslog.h>
#include <signal.h>

void copyFiles(char *path_to_src, char *path_to_dest) {
     // kopiowanie
     char *buffer = (char*)malloc(16384);
     int src_file = open(path_to_src, O_RDONLY);
     int dest_file = open(path_to_dest, O_WRONLY | O_CREAT);
     int bytes;

     while((bytes = read(src_file, buffer, sizeof(buffer))) > 0) {
          write(dest_file, buffer, bytes);
     }
     chmod(path_to_dest, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
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
     int sleep_time = 10;

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
               
               // utworzenie procesu potomnego
               pid_t process_pid = fork();

               if (process_pid == 0) {
                    // iteracja przez pliki w katalogu źródłowym
                    while ((src_file_info = readdir(src_dir)) != NULL) {
                         // inicjalizacja ścieżek plików w dwóch katalogach
                         char path_to_src[128], path_to_dest[128];
                         snprintf(path_to_src, sizeof(path_to_src), "%s/%s", src_path, src_file_info->d_name);
                         snprintf(path_to_dest, sizeof(path_to_dest), "%s/%s", dest_path, src_file_info->d_name);

                         // struktury na informacje o pliku
                         struct stat src_file_stat, dest_file_stat;
                         stat(path_to_src, &src_file_stat);
                         stat(path_to_dest, &dest_file_stat);

                         // jeżeli ścieżka jest plikiem 
                         // sprawdzenie czy jest dostęp do pliku w katalogu 1 i jednocześnie tego samego pliku w katalogu 2, jezeli nie - skopiuj
                         // jeżeli tak, sprawdź datę modyfikacji; jeżeli nowsza pliku w kat. 1 - usuń z kat. 2 ten plik i skopiuj z kat. źródłowego
                         if (S_ISDIR(src_file_stat.st_mode) == 0) {
                              if (access(path_to_src, F_OK) == 0 && access(path_to_dest, F_OK) != 0) {
                                   copyFiles(path_to_src, path_to_dest);
                              } else if (access(path_to_src, F_OK) == 0 && access(path_to_dest, F_OK) == 0) {
                                   // zainicjalizowanie dat modyfikacji plików
                                   time_t src_file_mtime = src_file_stat.st_mtime, dest_file_mtime = dest_file_stat.st_mtime;
                                   
                                   if (difftime(src_file_mtime, dest_file_mtime) > 0) {
                                        remove(path_to_dest);                 
                                             // wywołanie funkcji kopiującej
                                        copyFiles(path_to_src, path_to_dest);
                                   } 
                                   continue;
                              }
                         }
                    }
                    exit(0);
               }

               // jeżeli proces bazowy
               if (process_pid != 0) {
                    // iteracja w drugą stronę, tj. porównanie plików z katalogu 2 z plikami z katalogu 1 i ew. usunięcie ich
                    while ((dest_file_info = readdir(dest_dir)) != NULL) {
                         // inicjalizacja ścieżek plików w dwóch katalogach
                         char path_to_src[128], path_to_dest[128];
                         snprintf(path_to_src, sizeof(path_to_src), "%s/%s", src_path, dest_file_info->d_name);
                         snprintf(path_to_dest, sizeof(path_to_dest), "%s/%s", dest_path, dest_file_info->d_name);

                         // sprawdzenie, jest dostęp katalogu 2 i jednocześnie tego saemgo pliku w katalogu 1, jezeli nie - usuń
                         if(access(path_to_dest, F_OK) == 0 && access(path_to_src, F_OK) != 0) {
                              remove(path_to_dest);
                         } else {
                              continue;
                         }
                    }
               }

               // czekanie aż proces potomny zakończy pracę
               wait(NULL);

               //zamknięcie katalogów
               closedir(src_dir);
               closedir(dest_dir);
          }
     }
     return 0;
}