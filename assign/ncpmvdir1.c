#define _XOPEN_SOURCE 500
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>

static char* destination_dir;
static char** excluded_extensions;
static int num_excluded_extensions;
static int move;  // Declare the 'move' variable globally

static int copy_or_move_entry(const char* fpath, const struct stat* sb, int tflag, struct FTW* ftwbuf) {
    char source_path[PATH_MAX];
    char destination_path[PATH_MAX];

    // Construct the source path relative to the root directory
    snprintf(source_path, sizeof(source_path), "%s/%s", fpath, fpath + ftwbuf->base);

    // Construct the destination path relative to the destination directory
    snprintf(destination_path, sizeof(destination_path), "%s/%s", destination_dir, fpath + ftwbuf->base);

    // Check if the current entry matches any excluded extensions
    int i;
    for (i = 0; i < num_excluded_extensions; i++) {
        if (strcmp(excluded_extensions[i], strrchr(source_path, '.')) == 0) {
            printf("Skipping: %s\n", source_path);
            return 0;  // Skip the current entry
        }
    }

    // Create the destination directory (if necessary) for directory entries
    if (tflag == FTW_D && mkdir(destination_path, sb->st_mode & 0777) != 0) {
        perror("Failed to create directory");
        return -1;
    }

    // Copy or move regular files
    if (tflag == FTW_F) {
        FILE* source_file = fopen(source_path, "rb");
        if (source_file == NULL) {
            perror("Failed to open source file");
            return -1;
        }

        FILE* destination_file = fopen(destination_path, move ? "wb" : "ab");
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

        // Remove the source file if it was moved
        if (move && remove(source_path) != 0) {
            perror("Failed to remove source file");
            return -1;
        }
    }

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printf("Usage: ncpmvdir [source_dir] [destination_dir] [options] <extension list>\n");
        printf("Options: -cp  Copy the directory tree\n");
        printf("         -mv  Move the directory tree\n");
        return 1;
    }

    char* source_dir = argv[1];
    destination_dir = argv[2];

    move = (strcmp(argv[3], "-cp") != 0);

    num_excluded_extensions = argc - 4;
    excluded_extensions = &argv[4];

    int flags = FTW_PHYS;
    if (nftw(source_dir, copy_or_move_entry, 20, flags) == -1) {
        perror("nftw");
        return 1;
    }

    return 0;
}
