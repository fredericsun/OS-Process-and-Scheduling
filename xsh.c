#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#define maxinputlen 129
#define home "HOME"

char **path;
int pathlen = 1;
int innum = 0;
int out = 0;
void printerror() {
  char error_message[30] = "An error has occurred\n";
  write(STDERR_FILENO, error_message, strlen(error_message));
}

// check if there is any redirect sign
int rdtst(char **list, int count) {
  for (int i = 0; i < count; i++) {
      if (strcmp(list[i], ">") == 0) {
         if (i == count - 1) {
            return -1;
         } else if (count - i -1 == 1) {
            innum = 0;
            out = 1;
            return i;
         } else if ((count - i - 1) == 3) {
            if (strcmp(list[i + 2], "<<<") == 0) {
               innum = 1;
               out = 1;
               return i;
            } else {
               return -1;
            }
         } else {
               return -1;
         }
      } else if (strcmp(list[i], "<<<") == 0) {
         if ((i == count -1) || (count - 1 - i > 1)) {
            return -1;
         } else {
            innum = 1;
            out = 0;
            return i;
         }
      }
  }
return 0;
}

int main(int argc, char *argv[]) {
  if (argc != 1) {
  printerror();
  exit(1);
  }
  int ret = 0;

  path = malloc(sizeof(char*));
  if (path == NULL) {
     printerror();
     exit(1);
  }
  path[0] = strdup("/bin");
  char *cwd;
  char buff[PATH_MAX + 1];
  while (1) {
     cwd = getcwd(buff, PATH_MAX + 1);
     if (cwd != NULL) {
        printf("[%s]\n", cwd);
     }
     fflush(stdout);
     printf("%d> ", ret);
     fflush(stdout);
     size_t inlen = 0;
     char *in = NULL;
     if (getline(&in, &inlen, stdin) == -1) {
        free(in);
        printerror();
        exit(1);
     }
     int count = 0;
     char **list = NULL;

     if (strlen(in) > maxinputlen) {
        printerror();
        free(in);
        ret = 1;
        continue;
     }
     char *token = strtok(in, " \n\t");
     if (token == NULL) {
        free(in);
        continue;
     }
     while (token != NULL) {
        count++;
        list = realloc(list, sizeof(char*) * count);
        list[count - 1] = token;
        token = strtok(NULL, " \n\t");
     }
     list = realloc(list, sizeof(char*) * (count + 1));
     list[count] = NULL;

     if (strcmp(list[0], "exit") == 0) {
        for (int i = 0; i < count; i++) {
           free(path[i]);
        }
        free(path);
        free(list);
        free(in);
        exit(0);
     } else if (strcmp(list[0], "cd") == 0) {
        int cdc = 0;
        char *HOME = getenv(home);
        if (count == 1) {
           cdc = chdir(HOME);
           ret = 0;
           if (cdc != 0) {
              ret = 1;
              printerror();
           }
        } else if (count == 2) {
           cdc = chdir(list[1]);
           ret = 0;
           if (cdc != 0) {
              ret = 1;
              printerror();
           }
        }
     } else if (strcmp(list[0], "path") == 0) {
        if (count == 1) {
           for (int i = 0; i < pathlen; i++) {
              printf("%s\n", path[i]);
           }
           ret = 0;
        } else {
           for (int i = 0; i < pathlen; i++) {
              free(path[i]);
           }
           path = realloc(path, sizeof(char*)* (count -1));
           if (path == NULL) {
              printerror();
              ret = 1;
           }
           for (int i = 0; i < count - 1; i++) {
              path[i] = strdup(list[i + 1]);
           }
           pathlen = count - 1;
           ret = 0;
        }
     } else if (strcmp(list[0], "type") == 0) {
        if (count != 2) {
           printerror();
           ret = 1;
        } else if (strcmp(list[1], "type") == 0
        ||strcmp(list[1], "cd") == 0
        ||strcmp(list[1], "path") == 0
        ||strcmp(list[1], "exit") == 0) {
           printf("%s is a shell builtin\n", list[1]);
           ret = 0;
        } else {
           struct stat buf;
           char *dir[pathlen];
           for (int i = 0; i< pathlen; i++) {
              int len = strlen(list[0]) + strlen(path[i]) + 2;
              dir[i] = malloc(sizeof(char*) * len);
              strcpy(dir[i], path[i]);
              strcat(dir[i], "/");
              strcat(dir[i], list[1]);
           }
           int yep;
           for (int i = 0; i < pathlen; i++) {
              yep = stat(dir[i], &buf);
              if (yep == 0) {
                 printf("%s is %s\n", list[1], dir[i]);
                 ret = 0;
                 break;
              }
           }
           for (int i = 0; i < pathlen; i++) {
              free(dir[i]);
           }
              if (yep == 0) {
                 free(in);
                 free(list);
                 continue;
              } else {
                 ret = 1;
                 printerror();
              }
        }
     } else {
        int p[2];
        int rt = rdtst(list, count);
        if (pipe(p) == -1) {
           printerror();
           ret = -1;
        }
        pid_t fc;
        int status;
        fc = fork();
        if (fc < 0) {
           printerror();
           ret = 1;
        } else if (fc == 0) {
           if (rt == -1) {
              printerror();
              exit(1);
           } else if (rt == 0) {
              // do nothing
           } else if (rt > 0 && out == 1) {
              int length = strlen(list[rt + 1]) + strlen(".out") + 1;
              char *output = malloc(sizeof(char) * length);
              strcpy(output, list[rt + 1]);
              strcat(output, ".out");
              // out file
              int newfd_out = open(output, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
              if (newfd_out == -1) {
                 printerror();
                 exit(1);
              }
              if (dup2(newfd_out, STDOUT_FILENO) == -1) {
                 printerror();
                 exit(1);
              }
              // error file
              char *error = malloc(sizeof(char) * length);
              strcpy(error, list[rt + 1]);
              strcat(error, ".err");

              int newfd_error = open(error, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
              if (newfd_error == -1) {
                 printerror();
                 exit(1);
              }
              if (dup2(newfd_error, STDERR_FILENO) == -1) {
                 printerror();
                 exit(1);
              }
              free(output);
              free(error);
              list[rt] = NULL;
           }
            if (innum) {
              close(p[1]);
              if (dup2(p[0], STDIN_FILENO) == -1) {
                 printerror();
                 exit(1);
              } else {}
              close(p[0]);
              list[rt] = NULL;
           } else {
              close(p[0]);
              close(p[1]);
           }

           struct stat buf;
           char *dir[pathlen];
           for (int i = 0; i< pathlen; i++) {
              int len = strlen(list[0]) + strlen(path[i]) + 2;
              dir[i] = malloc(sizeof(char*) * len);
              strcpy(dir[i], path[i]);
              strcat(dir[i], "/");
              strcat(dir[i], list[0]);
              }
           int yep;
           for (int i = 0; i < pathlen; i++) {
              yep = stat(dir[i], &buf);
               if (yep == 0) {
                 list[0] = dir[i];
                 if (execvp(dir[i], list) == -1) {
                    printerror();
                    exit(1);
                 }
              }
           }
           for (int i = 0; i < pathlen; i++) {
              free(dir[i]);
           }
              if (yep == 0) {
              } else {
                 printerror();
                 ret = 1;
                 exit(1);
              }
        } else {
            if (innum) {
               write(p[1], list[count - 1], strlen(list[count - 1]));
               write(p[1], "\n", 1);
            }
            close(p[1]);
            close(p[0]);
            pid_t pid;
            pid = wait(&status);
            if (pid == -1) {
              ret = 1;
              printerror();
              continue;
            }
            ret = WEXITSTATUS(status);
        }
     }
  free(in);
  free(list);
  innum = 0;
  out = 0;
  }
exit(0);
}
