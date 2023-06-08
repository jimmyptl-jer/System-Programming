#define _XOPEN_SOURCE 500
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static char* extension_list[6] = {NULL};

static int display_info(const char* fpath, const struct stat* sb, int tflag, struct FTW* ftwbuf) {
    // Check if the file extension is in the exclusion list
    int excluded = 0;
    if (tflag == FTW_F) {
        char* extension = strrchr(fpath, '.');
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
        printf("%-3s %2d %7jd   %-40s %d %s\n",
               (tflag == FTW_D) ? "d" : (tflag == FTW_DNR) ? "dnr"
                                                             : (tflag == FTW_DP) ? "dp"
                                                                                 : (tflag == FTW_F) ? "f"
                                                                                                   : (tflag == FTW_NS) ? "ns"
                                                                                                                      : (tflag == FTW_SL) ? "sl"
                                                                                                                                          : (tflag == FTW_SLN) ? "sln"
                                                                                                                                                               : "???",
               ftwbuf->level, (intmax_t)sb->st_size,
               fpath, ftwbuf->base, fpath + ftwbuf->base);
    }

    return 0; /* To tell nftw() to continue */
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s [extension list]\n", argv[0]);
        return 1;
    }

    // Parse the extension list
    for (int i = 1; i < argc && i <= 6; i++) {
        extension_list[i - 1] = argv[i];
    }

    int flags = 0;
    if (nftw(".", display_info, 20, flags) == -1) {
        perror("nftw");
        return 1;
    }

    return 0;
}
