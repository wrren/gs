#include <gs/util/buffer.h>
#include <gs/util/test.h>

INT GsTestComparator(_In_ PVOID Element, _In_ PVOID Context)
{
    return *((PINT) Element) - *((PINT) Context);
}

PVOID GsTestRetriever(_In_ PVOID Buffer, _In_ SIZE_T Index, _In_ PVOID Context) {
    return (PVOID)((UINT_PTR) Buffer + (Index * sizeof(INT)));
}

INT GsTestEvaluator(_In_ PVOID Element, _In_ PVOID Context)
{
    PINT ElementInt = (PINT) Element;
    PINT ContextInt = (PINT) Context;

    return *ElementInt - *ContextInt;
}

int main(int argc, char** argv)
{
    INT Buffer[] = {
        3, 5, 1, 2, 9, 8, 4, 6, 7
    };
    INT Sorted[] = {
        1, 2, 3, 4, 5, 6, 7, 8, 9
    };

    GsBufferSort(Buffer, sizeof(Buffer), sizeof(INT), GsTestComparator);

    GS_REQUIRE(memcmp(Buffer, Sorted, sizeof(Buffer)) == 0);

    INT Target = 4;

    GS_REQUIRE(GsBufferBinarySearch(Buffer, sizeof(Buffer), sizeof(INT), GsTestEvaluator, &Target) == 3);
    GS_REQUIRE(GsBufferBinarySearchWithRetriever(Buffer, sizeof(Buffer) / sizeof(INT), GsTestEvaluator, &Target, GsTestRetriever, NULL) == 3);

    return EXIT_SUCCESS;
}