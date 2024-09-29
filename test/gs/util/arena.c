#include <gs/util/arena.h>
#include <gs/util/test.h>

int main(int argc, char** argv)
{
    PGS_ARENA Arena = GsArena();
    GS_REQUIRE(Arena != NULL);

    PVOID Buffer = GsArenaAlloc(Arena, 1024);
    GS_REQUIRE(Buffer != NULL);

    PVOID Reallocated = GsArenaRealloc(Arena, Buffer, 1024, 2048);
    GS_REQUIRE(Reallocated == Buffer);

    GsArenaRelease(Arena);

    return EXIT_SUCCESS;
}