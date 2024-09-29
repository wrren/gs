#include <gs/util/list.h>
#include <gs/util/test.h>

BOOL GsListTestEvaluator(_In_ PVOID Element, _In_ PVOID Context)
{
    return *((PUINT64) Element) == *((PUINT64) Context);
}

int main(int argc, char** argv)
{
    PGS_ARENA Arena = GsArena();

    PGS_LIST Empty = GsListInit(Arena, sizeof(UINT64));
    GS_REQUIRE(Empty != NULL);
    GS_REQUIRE(Empty->ElementSize == sizeof(UINT64));

    PGS_LIST List = GsListInit(Arena, sizeof(UINT64));
    GS_REQUIRE(List != NULL);
    GS_REQUIRE(List->ElementSize == sizeof(UINT64));

    UINT64 A = 1;
    UINT64 B = 2;
    UINT64 C = 3;
    UINT64 D = 4;

    GS_REQUIRE(GsListLength(List) == 0);

    GS_REQUIRE(GsListInsert(List, &A) == GsListSuccess);
    GS_REQUIRE(GsListInsert(List, &B) == GsListSuccess);
    GS_REQUIRE(GsListInsert(List, &C) == GsListSuccess);

    GS_REQUIRE(*((PUINT64) GsListAt(List, 0)) == A);
    GS_REQUIRE(*((PUINT64) GsListAt(List, 1)) == B);
    GS_REQUIRE(*((PUINT64) GsListAt(List, 2)) == C);

    GS_REQUIRE(GsListLength(List) == 3);

    GS_REQUIRE(GsListRemoveIf(List, GsListTestEvaluator, &A) == 1);
    GS_REQUIRE(GsListLength(List) == 2);
    GS_REQUIRE(GsListRemoveIf(List, GsListTestEvaluator, &B) == 1);
    GS_REQUIRE(GsListLength(List) == 1);
    GS_REQUIRE(GsListRemoveIf(List, GsListTestEvaluator, &C) == 1);
    GS_REQUIRE(GsListLength(List) == 0);

    GS_REQUIRE(GsListInsert(List, &A) == GsListSuccess);
    GS_REQUIRE(GsListInsert(List, &B) == GsListSuccess);
    GS_REQUIRE(GsListInsert(List, &C) == GsListSuccess);

    GS_REQUIRE(GsListFindIf(List, GsListTestEvaluator, &A) != NULL);
    GS_REQUIRE(GsListFindIf(List, GsListTestEvaluator, &B) != NULL);
    GS_REQUIRE(GsListFindIf(List, GsListTestEvaluator, &C) != NULL);
    GS_REQUIRE(GsListFindIf(List, GsListTestEvaluator, &D) == NULL);

    GsArenaRelease(Arena);

    return EXIT_SUCCESS;
}