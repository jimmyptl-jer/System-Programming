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

static int copy_or_move_directory(const char* source_path, const struct stat* sb, int tflag, struct FTW* ftwbuf) {
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

    if (!excluded) {
        // Create the destination directory (if necessary)
        if (tflag == FTW_D) {
            if (mkdir(destination_path, sb->st_mode & 0777) != 0) {
                perror("Failed to create directory");
                return -1;
            }
        } else if (tflag == FTW_F) {
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
    if (argc < 4) {
        printf("Usage: ncpmvdir [source_dir] [destination_dir] [options] <extension list>\n");
        return 1;
    }

    source_dir = argv[1];
    destination_dir = argv[2];
    int move = 0;

    // Parse the options and extension list
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "-cp") == 0) {
            // Copy option selected
            move = 0;
        } else if (strcmp(argv[i], "-mv") == 0) {
            // Move option selected
            move = 1;
        } else {
            // Extension list
            if (i - 3 < 6) {
                extension_list[i - 3] = argv[i];
            }
        }
    }

    int flags = 0;
    if (nftw(source_dir, copy_or_move_directory, 20, flags) == -1) {
        perror("nftw");
        return 1;
    }

        // Delete the source directory after moving
        if (move && remove(source_dir) != 0) {
            perror("Failed to remove source directory");
            return 1;
        }

    return 0;
}
