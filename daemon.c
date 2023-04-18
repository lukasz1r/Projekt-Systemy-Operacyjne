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

//sensowniejsze komenty potem sie powpisuje

void copyFile(char *path_to_src, char *path_to_dest) {
     char *buffer = (char*)malloc(128);
     int src_file;
     int dest_file;
     int bytes;

     if ((src_file = open(path_to_src, O_RDONLY)) == -1) {
          perror("open first file");
          exit(1);
     }

     if ((dest_file = open(path_to_dest, O_WRONLY | O_CREAT)) == -1) {
          perror("open second file");
          exit(1);
     }

     while((bytes = read(src_file, buffer, sizeof(buffer))) > 0) {
          write(dest_file, buffer, bytes);
     }
     
     if (close(src_file) == -1) {
          perror("close first file");
          exit(1);
     }

     if (close(dest_file) == -1) {
          perror("close second file");
          exit(1);
     }

     free(buffer);
}

void removeFiles(char *path_to_dest) {
     DIR *dest_dir;
     struct dirent *dest_path_info;

     if ((dest_dir = opendir(path_to_dest)) == NULL) {
          perror("open destination directory");
          exit(1);
     } 

     while((dest_path_info = readdir(dest_dir)) != NULL) {
          if (strcmp(dest_path_info->d_name, ".") == 0 || strcmp(dest_path_info->d_name, "..") == 0) {
               continue;
          }

          char dest[128];
          snprintf(dest, sizeof(dest), "%s/%s", path_to_dest, dest_path_info->d_name);

          if (dest_path_info->d_type == DT_DIR) {
               removeFiles(dest);
               rmdir(dest);
          }
          else {
               remove(dest);
          }
     }

     if ((dest_dir = opendir(path_to_dest)) != NULL) {
          while((dest_path_info = readdir(dest_dir)) != NULL) {
               if (strcmp(dest_path_info->d_name, ".") != 0 && strcmp(dest_path_info->d_name, "..") != 0) {
                    closedir(dest_dir);
                    return;
               }
          }
          closedir(dest_dir);
          rmdir(path_to_dest);
     }
}

void checkDirectory(char *src_path, char *dest_path) {
      
     DIR *src_dir, *dest_dir;

     struct dirent *src_path_info, *dest_path_info;

     if ((src_dir = opendir(src_path)) == NULL) {
          perror("open source directory");
          exit(1);
     } else if ((dest_dir = opendir(dest_path)) == NULL) {
          perror("open destination directory");
          exit(1);
     }

     while ((src_path_info = readdir(src_dir)) != NULL) {

          char path_to_src[128], path_to_dest[128];
          snprintf(path_to_src, sizeof(path_to_src), "%s/%s", src_path, src_path_info->d_name);
          snprintf(path_to_dest, sizeof(path_to_dest), "%s/%s", dest_path, src_path_info->d_name);

          struct stat src_file_stat, dest_file_stat;
          stat(path_to_src, &src_file_stat);
          stat(path_to_dest, &dest_file_stat);

          if (src_path_info->d_type == DT_DIR /* && opcja -R*/) {

               if (access(path_to_dest, F_OK) != 0) {
                    mkdir(path_to_dest, src_file_stat.st_mode);
               }
               if (strcmp(src_path_info->d_name, ".") != 0 && strcmp(src_path_info->d_name, "..") != 0) {
                    checkDirectory(path_to_src, path_to_dest);
               }

          } else if (src_path_info->d_type == DT_REG) {
               if (access(path_to_src, F_OK) == 0 && access(path_to_dest, F_OK) != 0) {
                    copyFile(path_to_src, path_to_dest);
               } else if (access(path_to_src, F_OK) == 0 && access(path_to_dest, F_OK) == 0) {

                    time_t src_file_mtime = src_file_stat.st_mtime, dest_file_mtime = dest_file_stat.st_mtime;
                    
                    if (difftime(src_file_mtime, dest_file_mtime) > 0) {
                         remove(path_to_dest);   

                         copyFile(path_to_src, path_to_dest);
                    } 
               }
          }
     }

     while ((dest_path_info = readdir(dest_dir)) != NULL) {

          char path_to_src[128], path_to_dest[128];
          snprintf(path_to_src, sizeof(path_to_src), "%s/%s", src_path, dest_path_info->d_name);
          snprintf(path_to_dest, sizeof(path_to_dest), "%s/%s", dest_path, dest_path_info->d_name);

          if(access(path_to_dest, F_OK) == 0 && access(path_to_src, F_OK) != 0) {
               if (dest_path_info->d_type == DT_REG) {
                    remove(path_to_dest); 
               } else if (dest_path_info->d_type == DT_DIR /* && opcja -R*/) {
                    removeFiles(path_to_dest);        
                    rmdir(path_to_dest);
               }
          }
     }

     if (closedir(src_dir) == -1) {
          perror("close source directory");
          exit(1);
     }

     if (closedir(dest_dir) == -1) {
          perror("close destination directory");
          exit(1);
     }
}

int main() {
     char src_path[] = "zrodlowy";
     char dest_path[] = "docelowy";

     struct stat src_stat, dest_stat;
     int sleep_time = 1;

     while (1) {
          if (stat(src_path, &src_stat) == -1 || stat(dest_path, &dest_stat) == -1) {
               printf("Error: Failed to get stats\n");
               exit(1);
          } else if (S_ISDIR(src_stat.st_mode) == 0 && S_ISDIR(dest_stat.st_mode) == 0) {
               printf("Error: Given paths are not directories\n");
               exit(1);
          } else if (S_ISDIR(src_stat.st_mode) == 1 && S_ISDIR(dest_stat.st_mode) == 1) {
               sleep(sleep_time);
               checkDirectory(src_path, dest_path);
          }
     }
     return 0;
}