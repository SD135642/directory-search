#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

void error_exit(const char *str1, const char *str2, const char *str3) {
    char error[100];
    snprintf(error, sizeof(error), "%s%s%s", str1, str2, str3);
    perror(error);
    exit(EXIT_FAILURE);
}

void parent_waits(pid_t pid, const char *pipe_name, int *child_status) {
    int status;
    pid_t wait_pid = waitpid(pid, &status, WUNTRACED | WCONTINUED);
    if (wait_pid == -1) {
        error_exit("Error: Failed to wait for ", pipe_name, ".\n");
    }
    if ((!WIFEXITED(status) || WEXITSTATUS(status) != 0) && child_status != NULL) {
        *child_status = 1;
    }
    return;
}

void write_stdout(int argc, char **argv, int *sort, int child_status) {
    int line;
    int count = 0;
    char buffer[BUFSIZ];
    while ((line = read(sort[0], buffer, BUFSIZ)) > 0) {
        if (write(STDOUT_FILENO, buffer, line) < 0) {
            perror("Error: Failed to write from sort read end to STDOUT_FILENO.\n");
            exit(EXIT_FAILURE);
        }
        for (int i = 0; i < line; i++) {
            if (buffer[i] == '\n') {
                count++;
            }
        }
    }
    if (line < 0) {
        perror("Error: Failed to read from sort pipe.\n");
        exit(EXIT_FAILURE);
    }
    close(sort[0]);
    char boolean = 'Y';
    for (int j = 1; j < argc; j++) {
        if (strcmp(argv[j], "-h") == 0) {
            boolean = 'N';
        }
    }
    if (child_status == 1) {
        boolean = 'N';
    }
    if (boolean == 'Y') {
        printf("Total Matches: %d\n", count);
    }
    return;
}

void process_children(char **argv, pid_t pid, int *pfind, int *sort, const char *pipe_name) {
    if (pid == -1) {
        error_exit("Error: Failed to fork ", pipe_name, ".\n");
    } else if (pid == 0) {
        if (sort == NULL) {
            close(pfind[0]);
            dup2(pfind[1], STDOUT_FILENO);
            close(pfind[1]);
            if (execvp("./pfind", argv) == -1) {
                perror("Error: pfind failed.\n");
                exit(EXIT_FAILURE);
            }
        } else {
            close(pfind[1]);
            close(sort[0]);
            dup2(pfind[0], STDIN_FILENO);
            dup2(sort[1], STDOUT_FILENO);
            close(pfind[0]);
            close(sort[1]);
            if (execlp("sort", "sort", NULL) == -1) {
                perror("Error: sort failed.\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    return;  
}

void create_pipe(int *pipe_array, const char *pipe_name) {
    if (pipe(pipe_array) == -1) {
        error_exit("Error: Failed to create ", pipe_name, " pipe.\n");
    }
    return;
}

int main(int argc, char **argv) {
    int pfind[2];
    int child_status = 0;
    create_pipe(pfind, "pfind");
    pid_t pfind_pid = fork();
    process_children(argv, pfind_pid, pfind, NULL, "pfind");
    parent_waits(pfind_pid, "pfind", &child_status);

    int sort[2];
    create_pipe(sort, "sort");
    pid_t sort_pid = fork();
    process_children(argv, sort_pid, pfind, sort, "sort");
    close(pfind[0]);
    close(pfind[1]);
    close(sort[1]);
    parent_waits(sort_pid, "sort", NULL);
    write_stdout(argc, argv, sort, child_status);
    return EXIT_SUCCESS;
}
