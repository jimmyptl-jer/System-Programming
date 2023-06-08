#define _XOPEN_SOURCE 500
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>

static char* source_dir;
static char* destination_dir;
static char* extension_list[6] = {NULL};
static int move_operation = 0;

static int copy_or_move_files(const char* source_path, const struct stat* sb, int tflag, struct FTW* ftwbuf) {
    char destination_path[PATH_MAX];
    snprintf(destination_path, sizeof(destination_path), "%s/%s", destination_dir, source_path + ftwbuf->base);

    // Check if the file extension is in the exclusion list
    int excluded = 0;
    if (tflag == FTW_F) {
        char* extension = strrchr(source_path, '.');
        if (extension != NULL) {
            for (int i = 0; i < 6; i++) {
                if (extension_list[i] != NULL && strcmp(extension_list[i], extension) == 0) {
                    excluded = 1;
                    break;
                }
            }
        }
    }

    if (!excluded && tflag == FTW_F) {
        // Perform the appropriate operation based on the user's choice
        if (move_operation) {
            // Move the file by renaming it
            if (rename(source_path, destination_path) != 0) {
                perror("Failed to move file");
                return -1;
            }
        } else {
            // Copy the file
            FILE* source_file = fopen(source_path, "rb");
            if (source_file == NULL) {
                perror("Failed to open source file");
                return -1;
            }

            FILE* destination_file = fopen(destination_path, "wb");
            if (destination_file == NULL) {
                perror("Failed to open destination file");
                fclose(source_file);
                return -1;
            }

            // Copy the file contents
            char buffer[4096];
            size_t num_read;
            while ((num_read = fread(buffer, 1, sizeof(buffer), source_file)) > 0) {
                if (fwrite(buffer, 1, num_read, destination_file) != num_read) {
                    perror("Failed to write to destination file");
                    fclose(source_file);
                    fclose(destination_file);
                    return -1;
                }
            }

            fclose(source_file);
            fclose(destination_file);
        }
    }

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 4 || argc > 10) {
        printf("Usage: %s [source_dir] [destination_dir] [cp/mv] [extension list]\n", argv[0]);
        return 1;
    }

    source_dir = argv[1];
    destination_dir = argv[2];

    // Check the user's choice of operation
    if (strcmp(argv[3], "mv") == 0) {
        move_operation = 1;
    } else if (strcmp(argv[3], "cp") != 0) {
        printf("Invalid operation. Please choose 'cp' or 'mv'.\n");
        return 1;
    }

    // Parse the extension list
    for (int i = 4; i < argc && i <= 9; i++) {
        extension_list[i - 4] = argv[i];
    }

    int flags = FTW_PHYS; // Perform a physical walk, avoiding symbolic links
    if (nftw(source_dir, copy_or_move_files, 20, flags) == -1) {
        perror("nftw");
        return 1;
    }

    return 0;
}
