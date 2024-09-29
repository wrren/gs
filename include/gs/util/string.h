#ifndef GS_CORE_STRING_H
#define GS_CORE_STRING_H

#ifdef __cplusplus
extern "C" 
{
#endif

#include <gs/core/platform.h>
#include <gs/util/arena.h>

typedef struct _GS_STRING
{
    PGS_ARENA   Arena;
    LPSTR       Content;
    SIZE_T      Length;
    SIZE_T      Capacity;
} GS_STRING, *PGS_STRING;

typedef enum
{
    GsStringSuccess,
    GsStringAllocationError,
    GsStringConcatenationError,
    GsStringCopyError
} GsStringError;

/**
 * @brief Initialize a new empty GS_STRING
 * 
 * @param Arena             Arena to use for memory allocations
 * @return PGS_STRING       Pointer to newly-created string or NULL on failure.
 */
PGS_STRING GsStringInit(
    _In_ PGS_ARENA Arena
);

/**
 * @brief Initialize a new GS_STRING that holds the given content.
 * 
 * @param Arena             Arena to use for memory allocations
 * @param Content           Content to be copied into the new GS_STRING
 * @return PGS_STRING       Pointer to newly-created string or NULL on failure.
 */
PGS_STRING GsStringInitWithContent(
    _In_ PGS_ARENA  Arena,
    _In_z_ LPCSTR   Content
);

/**
 * @brief Initialize a new GS_STRING that holds the given content.
 * 
 * @param Arena             Arena to use for memory allocations
 * @param Content           Wide string content to be converted and copied into the new GS_STRING
 * @return PGS_STRING       Pointer to newly-created string or NULL on failure.
 */
PGS_STRING GsStringInitWithWideContent(
    _In_ PGS_ARENA  Arena,
    _In_z_ LPCWSTR  Content
);

/**
 * @brief Initialize a new GS_STRING that holds up to N characters of the given content.
 * 
 * @param Arena             Arena to use for memory allocations
 * @param Content           Content to be copied into the new GS_STRING
 * @param ContentLength     Maximum number of characters to copy from content
 * @return PGS_STRING       Pointer to newly-created string or NULL on failure.
 */
PGS_STRING GsStringInitWithContentN(
    _In_ PGS_ARENA  Arena,
    _In_z_ LPCSTR   Content,
    _In_ SIZE_T     ContentLength
);

/**
 * @brief Initialize a new GS_STRING that holds up to N characters the given content.
 * 
 * @param Arena             Arena to use for memory allocations
 * @param Content           Wide string content to be converted and copied into the new GS_STRING
 * @param ContentLength     Maximum number of characters to copy from content
 * @return PGS_STRING       Pointer to newly-created string or NULL on failure.
 */
PGS_STRING GsStringInitWithWideContentN(
    _In_ PGS_ARENA  Arena,
    _In_z_ LPCWSTR  Content,
    _In_ SIZE_T     ContentLength
);

/**
 * @brief Append new content onto the given string.
 * 
 * @param String            String onto which the given content should be concatenated
 * @param Content           Content to be concatenated onto the string
 * @return GsStringError    GsStringSuccess on success
 */
GsStringError GsStringConcat(
    _In_ PGS_STRING String,
    _In_z_ LPCSTR   Content
);

/**
 * @brief Append new content onto the given string up to the specified number of characters.
 * 
 * @param String            String onto which the given content should be concatenated
 * @param Content           Content to be concatenated onto the string
 * @param ContentLength     Maximum number of characters to concatenate
 * @return GsStringError    GsStringSuccess on success
 */
GsStringError GsStringConcatN(
    _In_ PGS_STRING String,
    _In_z_ LPCSTR   Content,
    _In_ SIZE_T     ContentLength
);

/**
 * @brief Ensure that the given string has at least the specified capacity.
 * 
 * @param String            String whose capacity is to be modified
 * @param Capacity          New minimum capacity in number of characters.
 * @return GsStringError    GsStringSuccess on success
 */
_Success_(return == GsStringSuccess)
GsStringError GsStringReserve(
    _Inout_ PGS_STRING String,
    _In_ SIZE_T        Capacity
);

/**
 * @brief Clear the contents of the given string by resettings its length to zero.
 * 
 * @param String String to be cleared
 * @return VOID 
 */
VOID GsStringClear(
    _Inout_ PGS_STRING String
);

/**
 * @brief Search the given string for any of of the given characters and return the
 * index of the first match found.
 * 
 * @param String        String to be searched
 * @param Offset        Offset into the string where searhing should begin
 * @param Characters    Set of characters to search for
 * @return SIZE_T       Index of the first match or SIZE_MAX on failure.
 */
SIZE_T GsStringFindFirstOf(
    _In_ PGS_STRING String,
    _In_ SIZE_T     Offset,
    _In_ LPCSTR     Characters
);

/**
 * @brief Search the given string in reverse for any of of the given characters and return the
 * index of the first match found.
 * 
 * @param String        String to be searched
 * @param Offset        Offset from the end of the string where searhing should begin
 * @param Characters    Set of characters to search for
 * @return SIZE_T       Index of the first match or SIZE_MAX on failure.
 */
SIZE_T GsStringFindLastOf(
    _In_ PGS_STRING String,
    _In_ SIZE_T     Offset,
    _In_ LPCSTR     Characters
);

#ifdef __cplusplus
}
#endif

#endif // GS_CORE_STRING_H