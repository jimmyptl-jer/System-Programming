#define _XOPEN_SOURCE 500
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>

static int copy_file(const char* source, const char* destination) {
    FILE* source_file = fopen(source, "rb");
    if (!source_file) {
        perror("Failed to open source file");
        return -1;
    }

    FILE* dest_file = fopen(destination, "wb");
    if (!dest_file) {
        fclose(source_file);
        perror("Failed to create destination file");
        return -1;
    }

    char buffer[BUFSIZ];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, BUFSIZ, source_file)) > 0) {
        fwrite(buffer, 1, bytesRead, dest_file);
    }

    fclose(source_file);
    fclose(dest_file);

    return 0;
}

static int copy_directory(const char* source, const char* destination) {
    int result = mkdir(destination, 0777);
    if (result != 0) {
        perror("Failed to create destination directory");
        return -1;
    }

    return 0;
}

static int display_info(const char* fpath, const struct stat* sb,
                        int tflag, struct FTW* ftwbuf) {
    const char* source_path = fpath;
    const char* destination_path = fpath + ftwbuf->base;

    if (tflag == FTW_D) {
        return copy_directory(source_path, destination_path);
    } else if (tflag == FTW_F) {
        return copy_file(source_path, destination_path);
    }

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s [source_dir] [destination_dir]\n", argv[0]);
        return 1;
    }

    int flags = FTW_PHYS;  // Disable following symbolic links

    if (nftw(argv[1], display_info, 20, flags) == -1) {
        perror("nftw");
        exit(EXIT_FAILURE);
    }

    return 0;
}
