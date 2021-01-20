#include <linux/seccomp.h>
#include <linux/filter.h>
#include <linux/audit.h>
#include <stddef.h>
#include <linux/unistd.h>
#include <linux/signal.h>
#include <linux/bpf.h>
#include <sys/ptrace.h>
#include <sys/prctl.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <err.h>
#include <error.h>

void install_filter(void) {
    struct sock_filter filter[] = {
        // Verify architecture
        //        LoaD        Word    ABSolute addressing --> A register
        BPF_STMT(BPF_LD + BPF_W + BPF_ABS, offsetof(struct seccomp_data, arch)),
        //           JuMP    if EQual   immediate value
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, AUDIT_ARCH_X86_64, 1, 0),
        //           RETurn immediate value
        BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_KILL),

        // Load syscall number
        BPF_STMT(BPF_LD + BPF_W + BPF_ABS, offsetof(struct seccomp_data, nr)),

        // If syscall is exit, jump to return (allow)
        // else jump to next check
        BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, __NR_exit, 0, 1),
        BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_ALLOW),

        // Allow exit_group
        BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, __NR_exit_group, 0, 1),
        BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_ALLOW),

        // Allow read
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_read, 0, 1),
        BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_ALLOW),

        // Allow write
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_write, 0, 1),
        BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_ALLOW),

        // Allow open and openat if the only flag is O_RDONLY
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_open, 0, 3),
        BPF_STMT(BPF_LD + BPF_W + BPF_ABS, offsetof(struct seccomp_data, args[1])),
        BPF_JUMP(BPF_JMP + BPF_JEQ+ BPF_K, O_RDONLY, 0, 1),
        BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_ALLOW),

        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_openat, 0, 3),
        BPF_STMT(BPF_LD + BPF_W + BPF_ABS, offsetof(struct seccomp_data, args[2])),
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, O_RDONLY, 0, 1),
        BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_ALLOW),

        // If syscall is none of the above, set errno to EPERM
        BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_ERRNO | (EPERM & SECCOMP_RET_DATA)),
    };

    struct sock_fprog prog = {
        .len = sizeof(filter) / sizeof(filter[0]),
        .filter = filter,
    };

    prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0);
    if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog) != 0)
        err(1, "seccomp failed");
}

// Try to open this file
void open_source(void) {
    int fd = open(__FILE__, O_RDONLY);
    if (fd < 0)
        err(1, "Failed to open file");
    printf("%s opened with fd %d\n", __FILE__, fd);
    close(fd);
}

void open_log(void) {
    int fd = open("log", O_RDWR | O_APPEND | O_CREAT, 0644);
    if (fd < 0)
        err(1, "Failed to open log");
    printf("log opened with fd %d\n", fd);
    close(fd);
}

int main(void) {
    open_source();
    open_log();
    printf("Installing filter\n");
    install_filter();
    open_source();
    open_log();
}
