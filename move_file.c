#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s [source_file] [dest_file]\n", argv[0]);
        return 1;
    }

    const char* source_file = argv[1];
    const char* dest_file = argv[2];

    if (rename(source_file, dest_file) != 0) {
        perror("Failed to move file");
        return 1;
    }

    printf("File moved successfully!\n");

    return 0;
}
