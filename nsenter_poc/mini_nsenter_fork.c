#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

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

#define MAX_ENV_SIZE (1024 * STR_MAX)
#define MAX_ENV_VARS 1024
char fmt_env[STR_MAX] = "/proc/%d/environ";
char path_env[STR_MAX];
int fd_env;
char buf_env[MAX_ENV_SIZE + 2];
char *envp[MAX_ENV_VARS];


void create_ns_paths(int pid) {
    for (size_t i = 0; i < NS_NB; i++) {
        sprintf(str_ns[i], fmt_ns[i], pid);
    }
    sprintf(path_root, fmt_root, pid);
    sprintf(path_env, fmt_env, pid);

#ifdef DEBUG
    for (size_t i = 0; i < NS_NB; i++) {
        printf("%s\n", str_ns[i]);
    }
    printf("%s\n", path_root);
    printf("%s\n", path_env);
#endif
}

void get_fd_ns() {
    for (int i = 0; i < NS_NB; i++) {
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
    fd_root = open(path_root, O_RDONLY);
    if (fd_root < 0) {
        printf("Error opening %s.\n", path_root);
        perror("open");
        exit(1);
    }
    fd_env = open(path_env, O_RDONLY);
    if (fd_env < 0) {
        printf("Error opening %s.\n", path_env);
        perror("open");
        exit(1);
    }
}

void get_env() {
#ifdef DEBUG
    printf("Reading env from %s\n", path_env);
#endif
    int env_size = read(fd_env, buf_env, MAX_ENV_SIZE);
    if (-1 == env_size) {
        printf("Error reading %s\n", path_env);
        perror("read");
        exit(1);
    }
    close(fd_env);
    if (env_size == MAX_ENV_SIZE) {
        printf("WARNING: environment bigger than %d bytes. It has been truncated.\n", MAX_ENV_SIZE);
    }
    int i;
    char *c;
    for (i = 0, c = buf_env; i < MAX_ENV_VARS && *c; i++, c += strlen(c) + 1) {
        envp[i] = c;
    }
    if (i == MAX_ENV_VARS) {
        printf("WARNING: more than %d vars. Some have been clobbered.\n", MAX_ENV_VARS);
    }
}

void change_ns() {
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
    close(fd_root);
#ifdef DEBUG
    printf("chroot(\".\")\n");
#endif
    if (chroot(".") == -1) {
        printf("Error chroot(\".\")");
        perror("chroot");
        exit(1);
    }
}

void exec_command(char *cmd, char *args[], char *env[]) {
    if (execve(cmd, args, env) == -1) {
        perror("execve");
        exit(1);
    }
}

void spawn_in_ns(char *cmd, char *args[], char *env[]) {
    pid_t f_pid = fork();
    if (f_pid == 0) { // in child
        exec_command(cmd, args, env);
    }
    else {
        waitpid(f_pid, NULL, 0);
    }
}

// argv[1] == pid argv[2+n] is command
int main(int argc, char **argv) {
    (void) argc;
    char *cmd = argv[2];
    char **args = argv + 2;
    int target_pid = atoi(argv[1]);

    create_ns_paths(target_pid);

    get_fd_ns();

    get_env();

    change_ns();

    change_root();

    spawn_in_ns(cmd, args, envp);

    return 0;
}
