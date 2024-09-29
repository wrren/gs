#include <gs/util/list.h>

PGS_LIST GsListInit(
    _In_ PGS_ARENA  Arena,
    _In_ SIZE_T     ElementSize
)
{
    PGS_LIST List = (PGS_LIST) GsArenaAlloc(Arena, sizeof(GS_LIST));
    if(List == NULL) {
        return NULL;
    }

    List->Arena         = Arena;
    List->Head          = NULL;
    List->ElementSize   = ElementSize;

    return List;
}

SIZE_T GsListLength(
    _In_ PGS_LIST   List
)
{
    PGS_LIST_LINK* Previous = &(List->Head);
    SIZE_T Length = 0;

    while((*Previous) != NULL) {
        Previous = &(*Previous)->Next;
        ++Length;
    }

    return Length;
}

GsListError GsListInsert(
    _Inout_ PGS_LIST    List,
    _In_ PVOID          Element
)
{
    PGS_LIST_LINK* Previous = &(List->Head);

    while((*Previous) != NULL) {
        Previous = &(*Previous)->Next;
    }

    PGS_LIST_LINK NewLink = (PGS_LIST_LINK) GsArenaAlloc(List->Arena, sizeof(GS_LIST_LINK));
    if(NewLink == NULL) {
        return GsListAllocationError;
    }

    NewLink->Next = NULL;
    NewLink->Data = GsArenaAlloc(List->Arena, List->ElementSize);
    if(NewLink->Data == NULL) {
        return GsListAllocationError;
    }

    memcpy(NewLink->Data, Element, List->ElementSize);

    (*Previous) = NewLink;

    return GsListSuccess;
}

PVOID GsListAt(
    _In_ PGS_LIST   List,
    _In_ SIZE_T     Index
)
{
    PGS_LIST_LINK* Previous = &(List->Head);

    while((*Previous) != NULL && Index > 0) {
        Previous = &(*Previous)->Next;
        --Index;
    }

    if(Index != 0 || (*Previous) == NULL) {
        return NULL;
    }

    return (*Previous)->Data;
}

PVOID GsListPop(
    _In_ PGS_LIST   List
)
{
    if(List->Head == NULL) {
        return NULL;
    }

    PVOID Data = List->Head->Data;
    List->Head = List->Head->Next;

    return Data;
}

PVOID GsListPopBack(
    _In_ PGS_LIST   List
)
{
    if(List->Head == NULL) {
        return NULL;
    }

    PGS_LIST_LINK* Previous = &(List->Head);

    while((*Previous)->Next != NULL) {
        Previous = &(*Previous)->Next;
    }

    PVOID Tail = (*Previous)->Data;
    (*Previous) = NULL;

    return Tail;
}

SIZE_T GsListRemoveIf(
    _Inout_ PGS_LIST                List,
    _In_    GsListEvaluationFunc    Evaluator,
    _In_opt_ PVOID                  Context
)
{
    SIZE_T Removed = 0;

    PGS_LIST_LINK* Previous = &(List->Head);

    while((*Previous) != NULL) {
        PGS_LIST_LINK Next = (*Previous)->Next;

        if(Evaluator((*Previous)->Data, Context)) {
            (*Previous) = Next;
            ++Removed;
        } else {
            Previous = &(*Previous)->Next;
        }        
    }

    return Removed;
}

_Success_(return != NULL)
PVOID GsListFindIf(
    _Inout_ PGS_LIST                List,
    _In_    GsListEvaluationFunc    Evaluator,
    _In_opt_ PVOID                  Context
)
{
    PGS_LIST_LINK* Previous = &(List->Head);

    while((*Previous) != NULL) {
        PGS_LIST_LINK Next = (*Previous)->Next;

        if(Evaluator((*Previous)->Data, Context)) {
            return (*Previous)->Data;
        } else {
            Previous = &(*Previous)->Next;
        }        
    }

    return NULL;
}