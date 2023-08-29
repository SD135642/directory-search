#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_DIRLEN 256
#define MAX_PERLEN 10

void retrieve_entry_permissions(mode_t mode, char *entry_permissions) {
    entry_permissions[0] = (mode & S_IRUSR) ? 'r' : '-';
    entry_permissions[1] = (mode & S_IWUSR) ? 'w' : '-';
    entry_permissions[2] = (mode & S_IXUSR) ? 'x' : '-';
    entry_permissions[3] = (mode & S_IRGRP) ? 'r' : '-';
    entry_permissions[4] = (mode & S_IWGRP) ? 'w' : '-';
    entry_permissions[5] = (mode & S_IXGRP) ? 'x' : '-';
    entry_permissions[6] = (mode & S_IROTH) ? 'r' : '-';
    entry_permissions[7] = (mode & S_IWOTH) ? 'w' : '-';
    entry_permissions[8] = (mode & S_IXOTH) ? 'x' : '-';
    entry_permissions[9] = '\0';
}

void process_directory(char *directory, char *permissions) {
    char path[PATH_MAX];
    if (realpath(directory, path) == NULL) {
        fprintf(stderr, "Error: Cannot get full path of directory '%s'. %s.\n", directory, strerror(errno));
        exit(EXIT_FAILURE);
    }
    DIR *dir;
    if ((dir = opendir(path)) == NULL) {
        if (errno == EACCES) {
            fprintf(stderr, "Cannot open directory %s. %s.\n", path, strerror(errno));
            return;
        } else {
            fprintf(stderr, "Error: Cannot open directory '%s'. %s.\n", path, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    char full_path[PATH_MAX + 1];
    full_path[0] = '\0';
    if (strcmp(path, "/")) {
        size_t copy_len = strnlen(path, PATH_MAX);
        memcpy(full_path, path, copy_len);
        full_path[copy_len] = '\0';
    }
    size_t path_len = strlen(full_path) + 1;
    full_path[path_len - 1] = '/';
    full_path[path_len] = '\0';
    struct dirent *entry;
    struct stat entry_stat;
    char entry_permissions[9];
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        strncpy(full_path + path_len, entry->d_name, PATH_MAX - path_len);
        if (lstat(full_path, &entry_stat) < 0) {
            fprintf(stderr, "Error: Cannot lstate '%s'. %s.\n", full_path, strerror(errno));
            exit(EXIT_FAILURE);
        }
        retrieve_entry_permissions(entry_stat.st_mode, entry_permissions);
        if (S_ISDIR(entry_stat.st_mode)) {
            process_directory(full_path, permissions);
        } else if (strcpy(permissions, entry_permissions) == 0) {
            fprintf(stdout, "%s\n", full_path);
        }
    }
    closedir(dir);
    return;
}

char *validate_permissions(char *permissions) {
    char valid = 'Y';
    if (strlen(permissions) == 9) {
        for (int i = 0; i < 9; i += 3) {
            if (permissions[i] != 'r' && permissions[i] != '-') {
                valid = 'N';
            }
            if (permissions[i+1] != 'w' && permissions[i+1] != '-') {
                valid = 'N';
            }
            if (permissions[i+2] != 'x' && permissions[i+2] != '-') {
                valid = 'N';
            }
        }
    }
    if (strlen(permissions) != 9 || valid == 'N') {
        fprintf(stderr, "Error: Permissions string '%s' is invalid.\n", permissions);
        exit(EXIT_FAILURE);
    }
    return permissions;
}

char *validate_directory(char *directory) {
    char path[PATH_MAX];
    struct stat entry_stat;
    if (stat(directory, &entry_stat)) {
        fprintf(stderr, "Error: Cannot stat '%s'. %s. \n", directory, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (!S_ISDIR(entry_stat.st_mode)) {
        fprintf(stderr, "Error: '%s' is not a directory path. %s.\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return directory;
}

void print_usage(char **argv) {
    fprintf(stdout, "Usage: %s -d <directory> -p <permissions string> [-h]\n", argv[0]);
    exit(EXIT_SUCCESS);
}

void process_commandline(int argc, char **argv, char *directory, char *permissions) {
    if (argc == 1) {
        print_usage(argv);
    }
    if (argc > 6) {
        fprintf(stderr, "Error: Too many command line arguments received.\n");
        exit(EXIT_FAILURE);
    }
    int opt;
    char d_seen = 'N';
    char p_seen = 'N';

    while ((opt = getopt(argc, argv, ":d:p:h")) != -1) {
        switch(opt) {
            case 'd':
                validate_directory(optarg);
                strncpy(directory, optarg, MAX_DIRLEN - 1);
                directory[MAX_DIRLEN - 1] = '\0';
                d_seen = 'Y';
                break;
            case 'p':
                validate_permissions(optarg);
                strncpy(permissions, optarg, MAX_PERLEN - 1);
                permissions[MAX_PERLEN - 1] = '\0';
                p_seen = 'Y';
                break;
            case 'h':
                print_usage(argv);
                break;
            case ':':
                if (optopt == 'd') {
                    fprintf(stderr, "Error: Required argument for %c <directory> not found.\n", optopt);
                }
                if (optopt == 'p') {
                    fprintf(stderr, "Error: Required argument for %c <permissions string> not found.\n", optopt);
                }
                exit(EXIT_FAILURE);
                break;
             case '?':
                fprintf(stderr, "Error: Unknown option '-%c' received.\n", optopt);
                exit(EXIT_FAILURE);
                break;
        }
    }
    if (p_seen == 'N') {
        fprintf(stderr, "Error: Required argument -p <permissions string> not found.\n");
        exit(EXIT_FAILURE);
    }
    if (d_seen == 'N') {
        fprintf(stderr, "Error: Required argument -d <directory> not found.\n");
        exit(EXIT_FAILURE);
    }
    return;
}

int main(int argc, char **argv) {
    char directory[MAX_DIRLEN] = {0};
    char permissions[MAX_PERLEN] = {0};
    process_commandline(argc, argv, directory, permissions);
    process_directory(directory, permissions);
    return EXIT_SUCCESS;
}
