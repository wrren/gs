#include <gs/util/arena.h>
#include <gs/util/test.h>

VOID GsArenaTestCleanupTask(_In_ PVOID Argument)
{
    *((PINT) Argument) = 1;
}

int main(int argc, char** argv)
{
    PGS_ARENA Arena = GsArena();
    GS_REQUIRE(Arena != NULL);

    PVOID Buffer = GsArenaAlloc(Arena, 1024);
    GS_REQUIRE(Buffer != NULL);

    PVOID Reallocated = GsArenaRealloc(Arena, Buffer, 1024, 2048);
    GS_REQUIRE(Reallocated == Buffer);

    INT ToCleanUp = 0;
    GsArenaAddCleanupTask(Arena, GsArenaTestCleanupTask, &ToCleanUp);

    GsArenaRelease(Arena);

    GS_REQUIRE(ToCleanUp == 1);

    return EXIT_SUCCESS;
}