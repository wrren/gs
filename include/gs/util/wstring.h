#ifndef GS_UTIL_WSTRING_H
#define GS_UTIL_WSTRING_H

#ifdef __cplusplus
extern "C" 
{
#endif

#include <gs/core/platform.h>
#include <gs/nt/api.h>
#include <gs/util/arena.h>

typedef struct _GS_STRING *PGS_STRING;

typedef struct _GS_WSTRING
{
    PGS_ARENA   Arena;
    LPWSTR      Content;
    SIZE_T      Length;
    SIZE_T      Capacity;
} GS_WSTRING, *PGS_WSTRING;

typedef enum
{
    GsWStringSuccess,
    GsWStringAllocationError,
    GsWStringConcatenationError,
    GsWStringCopyError
} GsWStringError;

/**
 * @brief Initialize a new empty GS_WSTRING
 * 
 * @param Arena             Arena to use for memory allocations
 * @return PGS_WSTRING      Pointer to newly-created string or NULL on failure.
 */
PGS_WSTRING GsWStringInit(
    _In_ PGS_ARENA Arena
);

/**
 * @brief Initialize a new wide string using the content of a GS_STRING, performing
 * necessary conversion on its content.
 * 
 * @param String            String to be converted and copied
 * @return PGS_WSTRING      Pointer to newly-created string or NULL on failure.
 */
PGS_WSTRING GsWStringInitWithString(
    _In_ PGS_STRING String
);

/**
 * @brief Initialize a new GS_WSTRING that holds the given content.
 * 
 * @param Arena             Arena to use for memory allocations
 * @param Content           Content to be copied into the new GS_WSTRING
 * @return PGS_WSTRING      Pointer to newly-created string or NULL on failure.
 */
PGS_WSTRING GsWStringInitWithContent(
    _In_ PGS_ARENA  Arena,
    _In_z_ LPCWSTR  Content
);

/**
 * @brief Initialize a new GS_WSTRING that holds the given content.
 * 
 * @param Arena             Arena to use for memory allocations
 * @param Content           Content to be copied into the new GS_WSTRING
 * @return PGS_WSTRING      Pointer to newly-created string or NULL on failure.
 */
PGS_WSTRING GsWStringInitWithNonWideContent(
    _In_ PGS_ARENA  Arena,
    _In_z_ LPCSTR   Content
);

/**
 * @brief Initialize a new GS_WSTRING that holds up to N characters of the given content.
 * 
 * @param Arena             Arena to use for memory allocations
 * @param Content           Content to be copied into the new GS_WSTRING
 * @param ContentLength     Maximum number of characters to copy from content
 * @return PGS_WSTRING      Pointer to newly-created string or NULL on failure.
 */
PGS_WSTRING GsWStringInitWithContentN(
    _In_ PGS_ARENA  Arena,
    _In_z_ LPCWSTR  Content,
    _In_ SIZE_T     ContentLength
);

/**
 * @brief Initialize a new GS_WSTRING that holds up to N characters of the given content.
 * 
 * @param Arena             Arena to use for memory allocations
 * @param Content           Content to be copied into the new GS_WSTRING
 * @param ContentLength     Maximum number of characters to copy from content
 * @return PGS_WSTRING      Pointer to newly-created string or NULL on failure.
 */
PGS_WSTRING GsWStringInitWithNonWideContentN(
    _In_ PGS_ARENA  Arena,
    _In_z_ LPCSTR   Content,
    _In_ SIZE_T     ContentLength
);

/**
 * @brief Return a UNICODE_STRING struct that references the content of the given string.
 * 
 * @param String    String whose content is to be referenced by the returned UNICODE_STRING
 * @return UNICODE_STRING 
 */
UNICODE_STRING GsWStringToUnicodeString(
    _In_ PGS_WSTRING String
);

/**
 * @brief Append new content onto the given string.
 * 
 * @param String            String onto which the given content should be concatenated
 * @param Content           Content to be concatenated onto the string
 * @return GsWStringError   GsWStringSuccess on success
 */
_Success_(return == GsWStringSuccess)
GsWStringError GsWStringConcat(
    _In_ PGS_WSTRING    String,
    _In_z_ LPCWSTR      Content
);

/**
 * @brief Append new content onto the given string up to the specified number of characters.
 * 
 * @param String            String onto which the given content should be concatenated
 * @param Content           Content to be concatenated onto the string
 * @param ContentLength     Maximum number of characters to concatenate
 * @return GsWStringError   GsWStringSuccess on success
 */
_Success_(return == GsWStringSuccess)
GsWStringError GsWStringConcatN(
    _In_ PGS_WSTRING    String,
    _In_z_ LPCWSTR      Content,
    _In_ SIZE_T         ContentLength
);

/**
 * @brief Ensure that the given string has at least the specified capacity.
 * 
 * @param String            String whose capacity is to be modified
 * @param Capacity          New minimum capacity in number of characters.
 * @return GsWStringError   GsWStringSuccess on success
 */
_Success_(return == GsWStringSuccess)
GsWStringError GsWStringReserve(
    _Inout_ PGS_WSTRING String,
    _In_ SIZE_T         Capacity
);

/**
 * @brief Clear the contents of the given string by resettings its length to zero.
 * 
 * @param String String to be cleared
 * @return VOID 
 */
VOID GsWStringClear(
    _Inout_ PGS_WSTRING String
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
SIZE_T GsWStringFindFirstOf(
    _In_ PGS_WSTRING    String,
    _In_ SIZE_T         Offset,
    _In_ LPCWSTR        Characters
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
SIZE_T GsWStringFindLastOf(
    _In_ PGS_WSTRING    String,
    _In_ SIZE_T         Offset,
    _In_ LPCWSTR        Characters
);

#ifdef __cplusplus
}
#endif

#endif // GS_UTIL_WSTRING_H