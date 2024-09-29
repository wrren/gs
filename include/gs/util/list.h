#ifndef GS_UTIL_LIST_H
#define GS_UTIL_LIST_H

#ifdef __cplusplus
extern "C" 
{
#endif

#include <gs/util/arena.h>

typedef struct _GS_LIST_LINK
{
    PVOID                   Data;
    struct _GS_LIST_LINK*   Next;
} GS_LIST_LINK, *PGS_LIST_LINK;

typedef struct _GS_LIST
{
    PGS_ARENA       Arena;
    PGS_LIST_LINK   Head;
    SIZE_T          ElementSize;
} GS_LIST, *PGS_LIST;

typedef BOOL (*GsListEvaluationFunc)(
    _In_ PVOID Element,
    _In_ PVOID Context
);

typedef enum
{
    GsListSuccess,
    GsListAllocationError
} GsListError;

/**
 * @brief Initialize a new GS_LIST.
 * 
 * @param Arena         Arena used to manage allocations
 * @param ElementSize   Size of an individual element in bytes
 * @return PGS_LIST     Pointer to the initialized list or NULL on failure
 */
PGS_LIST GsListInit(
    _In_ PGS_ARENA  Arena,
    _In_ SIZE_T     ElementSize
);

/**
 * @brief Determine the length of the given list.
 * 
 * @param List          List whose length is to be checked.
 * @return SIZE_T       Size of the list
 */
SIZE_T GsListLength(
    _In_ PGS_LIST   List
);

/**
 * @brief Insert a new element at the tail of the given list.
 * 
 * @param List          List into which the element should be inserted
 * @param Element       Element to be inserted
 * @return GsListError  GsListSuccess on success
 */
GsListError GsListInsert(
    _Inout_ PGS_LIST    List,
    _In_ PVOID          Element
);

/**
 * @brief Retrieve the element at the specified list index.
 * 
 * @param List          List from which the element is to be retrieved.
 * @param Index         Index into the list from which the element is to be reetrieved.
 * @return PVOID        Pointer to retrieved element or NULL if there is no element at the specified index.
 */
PVOID GsListAt(
    _In_ PGS_LIST   List,
    _In_ SIZE_T     Index
);

/**
 * @brief Pop the head element off the given list.
 * 
 * @param List          List to pop the head element off.
 * @return PVOID        Pointer to the head element or NULL if the list is empty
 */
PVOID GsListPop(
    _In_ PGS_LIST   List
);

/**
 * @brief Pop the tail element off the given list.
 * 
 * @param List          List to pop the tail element off.
 * @return PVOID        Pointer to the tail element or NULL if the list is empty
 */
PVOID GsListPopBack(
    _In_ PGS_LIST   List
);

/**
 * @brief Iterates over list elements and removes any for which the given evaluation function returns TRUE.
 * 
 * @param List          List from which elements are to be removed
 * @param Evaluator     Comparator used to determine whether list elements should be removed.
 * @param Context       Context pointer passed as the second parameter to the evaluation function
 * @return SIZE_T       Number of elements removed.
 */
SIZE_T GsListRemoveIf(
    _Inout_ PGS_LIST                List,
    _In_    GsListEvaluationFunc    Evaluator,
    _In_opt_ PVOID                  Context
);

/**
 * @brief Iterates over list elements and returns the first element for which the given evaluation function returns TRUE.
 * 
 * @param List          List to be searched
 * @param Evaluator     Comparator used to determine whether list elements should be returned.
 * @param Context       Context pointer passed as the second parameter to the evaluation function
 * @return PVOID        Pointer to the matching list element or NULL on failure.
 */
_Success_(return != NULL)
PVOID GsListFindIf(
    _Inout_ PGS_LIST                List,
    _In_    GsListEvaluationFunc    Evaluator,
    _In_opt_ PVOID                  Context
);

#ifdef __cplusplus
}
#endif

#endif // GS_UTIL_LIST_H