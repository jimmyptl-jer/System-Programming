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

static int move_files(const char* source_path, const struct stat* sb, int tflag, struct FTW* ftwbuf) {
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
        // Move the file by renaming it
        if (rename(source_path, destination_path) != 0) {
            perror("Failed to move file");
            return -1;
        }
        
        // Delete the file from the source directory
        if (unlink(source_path) != 0) {
            perror("Failed to delete file from source directory");
            return -1;
        }
    }

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 4 || argc > 10) {
        printf("Usage: %s [source_dir] [destination_dir] [extension list]\n", argv[0]);
        return 1;
    }

    source_dir = argv[1];
    destination_dir = argv[2];

    // Parse the extension list
    for (int i = 3; i < argc && i <= 9; i++) {
        extension_list[i - 3] = argv[i];
    }

    int flags = FTW_PHYS; // Perform a physical walk, avoiding symbolic links
    if (nftw(source_dir, move_files, 20, flags) == -1) {
        perror("nftw");
        return 1;
    }

    return 0;
}

