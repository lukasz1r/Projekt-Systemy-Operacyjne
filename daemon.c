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
#include <sys/wait.h>

void copyFile(char *path_to_src, char *path_to_dest /*jakiś treshold dodać i w zależności od wielkości wybiera sposób kopiowania*/)
{
     char *buffer = malloc(1024);
     int src_file;
     int dest_file;
     int bytes;

     if ((src_file = open(path_to_src, O_RDONLY)) == -1)
     {
          perror("Otwieranie pierwszego pliku");
          return;
     }

     if ((dest_file = open(path_to_dest, O_WRONLY | O_CREAT)) == -1)
     {
          perror("Otwieranie drugiego pliku");
          return;
     }

     while ((bytes = read(src_file, buffer, sizeof(buffer))) > 0)
     {
          write(dest_file, buffer, bytes);
     }

     if (close(src_file) == -1)
     {
          perror("Zamykanie pierwszego pliku");
          return;
     }

     if (close(dest_file) == -1)
     {
          perror("Zamykanie drugiego pliku");
          return;
     }
}

void removeFiles(char *path_to_dest)
{
     DIR *dest_dir;
     struct dirent *dest_path_info;

     if ((dest_dir = opendir(path_to_dest)) == NULL)
     {
          perror("Otwieranie katalogu docelowego");
          return;
     }

     while ((dest_path_info = readdir(dest_dir)) != NULL)
     {
          if (strcmp(dest_path_info->d_name, ".") == 0 || strcmp(dest_path_info->d_name, "..") == 0)
          {
               continue;
          }

          char dest[128];
          snprintf(dest, sizeof(dest), "%s/%s", path_to_dest, dest_path_info->d_name);

          if (dest_path_info->d_type == DT_DIR)
          {
               removeFiles(dest);
               rmdir(dest);
               openlog("DAEMON_DELETE", LOG_PID | LOG_CONS, LOG_USER);
               syslog(LOG_INFO, "Usunieto katalog: %s", dest);
               closelog();
          }
          else if (dest_path_info->d_type == DT_REG)
          {
               remove(dest);
               openlog("DAEMON_DELETE", LOG_PID | LOG_CONS, LOG_USER);
               syslog(LOG_INFO, "Usunieto plik: %s", dest);
               closelog();
          }
     }
}

void syncDirectory(char *src_path, char *dest_path)
{
     DIR *src_dir, *dest_dir;
     struct dirent *src_path_info, *dest_path_info;

     if ((src_dir = opendir(src_path)) == NULL)
     {
          perror("Otwieranie katalogu zrodlowego");
          return;
     }

     if ((dest_dir = opendir(dest_path)) == NULL)
     {
          perror("Otwieranie katalogu docelowego");
          return;
     }

     openlog("DAEMON_CHECK_DIR", LOG_PID | LOG_CONS, LOG_USER);
     syslog(LOG_INFO, "Sprawdzono katalogi");
     closelog();

     while ((src_path_info = readdir(src_dir)) != NULL)
     {
          char path_to_src[128], path_to_dest[128];
          snprintf(path_to_src, sizeof(path_to_src), "%s/%s", src_path, src_path_info->d_name);
          snprintf(path_to_dest, sizeof(path_to_dest), "%s/%s", dest_path, src_path_info->d_name);

          struct stat src_path_stat, dest_path_stat;
          stat(path_to_src, &src_path_stat);
          stat(path_to_dest, &dest_path_stat);

          if (src_path_info->d_type == DT_DIR /* && opcja -R*/)
          {
               if (access(path_to_dest, F_OK) != 0)
               {
                    mkdir(path_to_dest, src_path_stat.st_mode);
               }

               if (strcmp(src_path_info->d_name, ".") != 0 && strcmp(src_path_info->d_name, "..") != 0)
               {
                    syncDirectory(path_to_src, path_to_dest);
               }
          }
          else if (src_path_info->d_type == DT_REG)
          {
               if (access(path_to_src, F_OK) == 0 && access(path_to_dest, F_OK) != 0)
               {
                    copyFile(path_to_src, path_to_dest /*src_path_stat.st_size (treshold)*/);
                    openlog("DAEMON_COPY", LOG_PID | LOG_CONS, LOG_USER);
                    syslog(LOG_INFO, "Skopiowano plik: %s", path_to_src);
                    closelog();
               }
               else if (access(path_to_src, F_OK) == 0 && access(path_to_dest, F_OK) == 0)
               {
                    time_t src_file_mtime = src_path_stat.st_mtime, dest_file_mtime = dest_path_stat.st_mtime;

                    if (difftime(src_file_mtime, dest_file_mtime) > 0 || src_path_stat.st_size != dest_path_stat.st_size)
                    {
                         remove(path_to_dest);
                         copyFile(path_to_src, path_to_dest /*src_path_stat.st_size (treshold)*/);
                         openlog("DAEMON_COPY", LOG_PID | LOG_CONS, LOG_USER);
                         syslog(LOG_INFO, "Zaktualizowano plik: %s", path_to_dest);
                         closelog();
                    }
               }
          }
     }

     while ((dest_path_info = readdir(dest_dir)) != NULL)
     {
          char path_to_src[128], path_to_dest[128];
          snprintf(path_to_src, sizeof(path_to_src), "%s/%s", src_path, dest_path_info->d_name);
          snprintf(path_to_dest, sizeof(path_to_dest), "%s/%s", dest_path, dest_path_info->d_name);

          if (access(path_to_dest, F_OK) == 0 && access(path_to_src, F_OK) != 0)
          {
               if (dest_path_info->d_type == DT_REG)
               {
                    remove(path_to_dest);
                    openlog("DAEMON_DELETE", LOG_PID | LOG_CONS, LOG_USER);
                    syslog(LOG_INFO, "Usunieto plik: %s", path_to_dest);
                    closelog();
               }
               else if (dest_path_info->d_type == DT_DIR /* && opcja -R*/)
               {
                    removeFiles(path_to_dest);
                    rmdir(path_to_dest);
                    openlog("DAEMON_DELETE", LOG_PID | LOG_CONS, LOG_USER);
                    syslog(LOG_INFO, "Usunieto katalog: %s", path_to_dest);
                    closelog();
               }
          }
     }

     if (closedir(src_dir) == -1)
     {
          perror("Zamykanie katalogu zrodlowego");
          return;
     }

     if (closedir(dest_dir) == -1)
     {
          perror("Zamykanie katalogu docelowego");
          return;
     }
}

int main()
{
     char src_path[] = "zrodlowy";
     char dest_path[] = "docelowy";

     struct stat src_stat, dest_stat;
     int sleep_time = 1;

     while (1)
     {
          sleep(sleep_time);
          if (stat(src_path, &src_stat) == -1 || stat(dest_path, &dest_stat) == -1)
          {
               perror("Pobranie informacji o sciezce\n");
               continue;
          }
          else if (S_ISDIR(src_stat.st_mode) == 0 && S_ISDIR(dest_stat.st_mode) == 0)
          {
               perror("Sprawdzenie poprawnosci katalogow\n");
               continue;
          }
          else if (S_ISDIR(src_stat.st_mode) == 1 && S_ISDIR(dest_stat.st_mode) == 1)
          {
               syncDirectory(src_path, dest_path);
          }
     }
     return 0;
}
