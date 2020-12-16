#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


extern char **environ;

#define NS_NB 5
#define STR_MAX 1024

char fmt_ns[NS_NB][STR_MAX] = {
  "/proc/%d/ns/mnt",
  "/proc/%d/ns/uts",
  "/proc/%d/ns/net",
  "/proc/%d/ns/ipc",
  "/proc/%d/ns/pid"
};
char str_ns[NS_NB][STR_MAX];
int fd_ns[NS_NB];

char fmt_root[STR_MAX] = "/proc/%d/root";
char path_root[STR_MAX];
int fd_root;


void create_ns_paths(int pid) {
  for (size_t i = 0; i < NS_NB; i++)
  {
    sprintf(str_ns[i], fmt_ns[i], pid);
  }
  sprintf(path_root, fmt_root, pid);

#ifdef DEBUG
  for (size_t i = 0; i < NS_NB; i++)
  {
    printf("%s\n", str_ns[i]);
  }
  printf("%s\n", path_root);
#endif
}

void set_all_ns() { //FIXME split in two func get / set
  for (int i = 0; i < NS_NB; i++)
  {
#ifdef DEBUG
    printf("open(%s, O_RDONLY)\n", str_ns[i]);
#endif
    fd_ns[i] = open(str_ns[i], O_RDONLY);
    if (fd_ns[i] < 0) {
      printf("Error opening %s.\n", str_ns[i]);
      perror("open");
      exit(1);
    }
  }
  fd_root = open(path_root, O_RDONLY); //FIXME check error 
  for (int i = 0; i < NS_NB; i++) {
#ifdef DEBUG
    printf("setns(%s, 0)\n", str_ns[i]);
#endif
    if (setns(fd_ns[i], 0) == -1) {
      printf("Error setns(%s, 0).\n", str_ns[i]);
      perror("setns");
      exit(1);
    }
    close(fd_ns[i]);
  }
}

void change_root() {
#ifdef DEBUG
  printf("fchdir(%s)\n", path_root);
#endif
  if (fchdir(fd_root) == -1) {
    printf("Error fchdir(%s)\n", path_root);
    perror("fchdir");
    exit(1);
  }
#ifdef DEBUG
  printf("chroot(\".\")\n");
#endif
  if (chroot(".") == -1) {
    printf("Error chroot(\".\")");
    perror("chroot");
    exit(1);
  }
}

void exec_command(char *cmd, char *args[], char *envp[]) {
  if (execve(cmd, args, envp) == -1) {
    perror("execve");
    exit(1);
  }
}

// argv[1] == pid argv[2+n] is command
int main(int argc, char **argv) {
  (void)argc;
  char *cmd = argv[2];
  char **args = argv + 2;
  int target_pid = atoi(argv[1]);

  create_ns_paths(target_pid);

  set_all_ns();

  change_root();

  exec_command(cmd, args, environ);

  return 0;
}
