#include <stdlib.h>
#include <stdio.h>

#include <gs/loader/lib.h>

int main(int argc, char** argv) {
    if(GsLibraryInit() == FALSE) {
        printf("Library System Initialization Failed!\n");
        return EXIT_FAILURE;
    }

    PGS_LIBRARY Library = GsLibraryLoad("pots.dll");
    if(Library == NULL) {
        printf("Load Library Failed!\n");
        GsLibraryRelease();
        return EXIT_FAILURE;
    }

    GsLibraryRelease();

    return EXIT_SUCCESS;
}