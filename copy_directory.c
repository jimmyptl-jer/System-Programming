#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

void copy_file(const char* source_path, const char* dest_path) {
    FILE* source_file = fopen(source_path, "rb");
    FILE* dest_file = fopen(dest_path, "wb");
    if (source_file == NULL || dest_file == NULL) {
        perror("Failed to open file");
        return;
    }

    char buffer[1024];
    size_t read_size;

    while ((read_size = fread(buffer, 1, sizeof(buffer), source_file)) > 0) {
        fwrite(buffer, 1, read_size, dest_file);
    }

    fclose(source_file);
    fclose(dest_file);
}

void copy_directory(const char* source_dir, const char* dest_dir) {
    DIR* dir = opendir(source_dir);
    if (dir == NULL) {
        perror("Failed to open directory");
        return;
    }

    struct dirent* entry;

    while ((entry = readdir(dir)) != NULL) {
        // Skip "." and ".." directories
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char source_path[1024];
        char dest_path[1024];
        snprintf(source_path, sizeof(source_path), "%s/%s", source_dir, entry->d_name);
        snprintf(dest_path, sizeof(dest_path), "%s/%s", dest_dir, entry->d_name);

        struct stat st;
        if (stat(source_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                // Recursively copy subdirectories
                mkdir(dest_path, st.st_mode);
                copy_directory(source_path, dest_path);
            } else if (S_ISREG(st.st_mode)) {
                // Copy regular files
                copy_file(source_path, dest_path);
            }
        }
    }

    closedir(dir);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s [source_dir] [dest_dir]\n", argv[0]);
        return 1;
    }

    const char* source_dir = argv[1];
    const char* dest_dir = argv[2];

    copy_directory(source_dir, dest_dir);

    printf("Directory contents copied successfully!\n");

    return 0;
}
