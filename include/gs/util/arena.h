#ifndef GS_UTIL_ARENA_H
#define GS_UTIL_ARENA_H

#include <gs/core/platform.h>

#ifdef __cplusplus
extern "C" 
{
#endif

/// Default reservation size for an arena (1GB)
#define GS_ARENA_DEFAULT_RESERVATION (1024 * 1024 * 1024)

typedef void (*GS_ARENA_CLEANUP_FUNC)(_In_ PVOID);

typedef struct _GS_ARENA_CLEANUP_NODE
{
    GS_ARENA_CLEANUP_FUNC           CleanupFunction;
    PVOID                           Argument;
    struct _GS_ARENA_CLEANUP_NODE*  Next;
} GS_ARENA_CLEANUP_NODE, *PGS_ARENA_CLEANUP_NODE;

typedef struct _GS_ARENA
{
    PVOID                   Buffer;
    SIZE_T                  Reserved;
    SIZE_T                  Committed;
    SIZE_T                  Next;
    DWORD                   PageProtection;
    PGS_ARENA_CLEANUP_NODE  CleanupHead;
} GS_ARENA, *PGS_ARENA;

/**
 * @brief Create a new arena with default reservation and page protection options.
 * 
 * @return PGS_ARENA    Created arena or NULL on failure
 */
PGS_ARENA GsArena();

/**
 * @brief Create a new arena with the specified reservation and default page protection.
 * 
 * @param Reservation Amount of memory to reserve, in bytes.
 * @return PGS_ARENA    Created arena or NULL on failure
 */
PGS_ARENA GsArenaWithReservation(
    _In_ SIZE_T Reservation
);

/**
 * @brief Create a new arena with the specified reservation and page protection options.
 * 
 * @param Reservation Amount of memory to reserve, in bytes.
 * @param PageProtection Page protection to apply to reserved memory, see `VirtualAlloc` for options.
 * @return PGS_ARENA Created arena or NULL on failure
 */
PGS_ARENA GsArenaWithReservationAndPageProtection(
    _In_ SIZE_T Reservation,
    _In_ DWORD  PageProtection
);

/**
 * @brief Allocate and return a block of memory of the specified size.
 * 
 * @param Arena Arena from which memory should be allocated
 * @param Bytes Number of contiguous bytes to be allocated.
 * @return PVOID Pointer to the allocated block of memory or NULL on failure.
 */
PVOID GsArenaAlloc(
    _Inout_ PGS_ARENA   Arena,
    _In_ SIZE_T         Bytes
);

/**
 * @brief Reallocate previously-allocated memory to a new size
 * 
 * @param Arena         Arena to use when reallocating memory
 * @param CurrentBuffer Currently-allocated buffer
 * @param CurrentSize   Current size of the buffer
 * @param NewSize       Newly-requested size in bytes
 * @return PVOID        Pointer to the reallocated buffer
 */
PVOID GsArenaRealloc(
    _Inout_ PGS_ARENA   Arena,
    _In_ PVOID          CurrentBuffer,
    _In_ SIZE_T         CurrentSize,
    _In_ SIZE_T         NewSize
);

/**
 * @brief Add a cleanup task that will be executed during arena release, but before
 * arena memory has been released.
 * 
 * @param Arena             Arena to which the task should be added.
 * @param CleanupFunction   Gseanup function executed during arena release
 * @param Argument          Argument passed to the cleanup function
 * @return BOOL             TRUE - if the cleanup task was added successfully, FALSE otherwise.
 */
BOOL GsArenaAddCleanupTask(
    _Inout_ PGS_ARENA           Arena,
    _In_ GS_ARENA_CLEANUP_FUNC  GseanupFunction,
    _In_ PVOID                  Argument
);

/**
 * @brief Release any memory allocated by the given arena.
 * 
 * @param Arena Arena to be released.
 */
VOID GsArenaRelease(
    _Inout_ PGS_ARENA Arena
);

#ifdef __cplusplus
}
#endif

#endif // GS_UTIL_ARENA_H