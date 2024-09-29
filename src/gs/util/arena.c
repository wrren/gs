#include <gs/util/arena.h>

/// Growth factor for committed capacity
#define GS_ARENA_CAPACITY_GROWTH_FACTOR 2

PGS_ARENA GsArena()
{
    return GsArenaWithReservationAndPageProtection(GS_ARENA_DEFAULT_RESERVATION, PAGE_READWRITE);
}

PGS_ARENA GsArenaWithReservation(
    _In_ SIZE_T Reservation
)
{
    return GsArenaWithReservationAndPageProtection(Reservation, PAGE_READWRITE);
}

PGS_ARENA GsArenaWithReservationAndPageProtection(
    _In_ SIZE_T Reservation,
    _In_ DWORD  PageProtection
)
{
    PGS_ARENA Arena = (PGS_ARENA) VirtualAlloc(NULL, sizeof(GS_ARENA), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if(Arena == NULL) {
        return NULL;
    }

    Arena->Buffer = VirtualAlloc(NULL, Reservation, MEM_RESERVE, PageProtection);
    if(Arena->Buffer == NULL) {
        VirtualFree(Arena, 0, MEM_RELEASE);
        return NULL;
    }

    Arena->Reserved         = Reservation;
    Arena->Committed        = 0;
    Arena->Next             = 0;
    Arena->PageProtection   = PageProtection;
    Arena->CleanupHead      = NULL;

    return Arena;
}

PVOID GsArenaAlloc(
    _Inout_ PGS_ARENA Arena,
    _In_ SIZE_T Bytes
)
{
    if(Arena->Next + Bytes > Arena->Reserved) {
        return NULL;
    }

    if(Bytes > (Arena->Committed - Arena->Next)) {
        SIZE_T RequiredCommitment = max(Arena->Next + Bytes, Arena->Committed * GS_ARENA_CAPACITY_GROWTH_FACTOR);

        if(VirtualAlloc(Arena->Buffer, RequiredCommitment, MEM_COMMIT, Arena->PageProtection) == NULL) {
            return NULL;
        }

        Arena->Committed = RequiredCommitment;
    }

    PVOID Buffer = ((PUINT8) Arena->Buffer) + Arena->Next;
    Arena->Next += Bytes;

    return Buffer;
}

PVOID GsArenaRealloc(
    _Inout_ PGS_ARENA   Arena,
    _In_ PVOID          CurrentBuffer,
    _In_ SIZE_T         CurrentSize,
    _In_ SIZE_T         NewSize
)
{
    if(NewSize <= CurrentSize) {
        return CurrentBuffer;
    }

    if(CurrentBuffer == NULL || CurrentSize == 0) {
        return GsArenaAlloc(Arena, NewSize);
    }

    // Current buffer was the last to be allocated, we can keep using it and just advance the `Next` index
    if((((PUINT8) CurrentBuffer) + CurrentSize) == ((PUINT8) Arena->Buffer) + Arena->Next) {
        Arena->Next += (NewSize - CurrentSize);
        return CurrentBuffer;
    } 

    // Need to allocate new space
    PVOID NewBuffer = GsArenaAlloc(Arena, NewSize);
    if(NewBuffer == NULL) {
        return NULL;
    }

    // Copy the contents of the old buffer into the new one
    memcpy(NewBuffer, CurrentBuffer, CurrentSize);

    return NewBuffer;
}

BOOL GsArenaAddGseanupTask(
    _Inout_ PGS_ARENA           Arena,
    _In_ GS_ARENA_CLEANUP_FUNC  CleanupFunction,
    _In_ PVOID                  Argument
)
{
    PGS_ARENA_CLEANUP_NODE* Previous = &Arena->CleanupHead;

    while(*Previous != NULL) {
        Previous = &(*Previous)->Next;
    }

    PGS_ARENA_CLEANUP_NODE NewNode = (PGS_ARENA_CLEANUP_NODE) GsArenaAlloc(Arena, sizeof(GS_ARENA_CLEANUP_NODE));
    if(NewNode == NULL) {
        return FALSE;
    }

    NewNode->CleanupFunction    = CleanupFunction;
    NewNode->Argument           = Argument;
    NewNode->Next               = NULL;

    (*Previous) = NewNode;

    return TRUE;
}

VOID GsArenaRelease(
    _Inout_ PGS_ARENA Arena
)
{
    if(Arena == NULL) {
        return;
    }

    PGS_ARENA_CLEANUP_NODE Cleanup = Arena->CleanupHead;

    while(Cleanup != NULL) {
        Cleanup->CleanupFunction(Cleanup->Argument);
        Cleanup = Cleanup->Next;
    }

    VirtualFree(Arena->Buffer, 0, MEM_RELEASE);
    VirtualFree(Arena, 0, MEM_RELEASE);
}