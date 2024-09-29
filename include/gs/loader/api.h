#ifndef GS_LOADER_API_H
#define GS_LOADER_API_H

#include <gs/nt/api.h>
#include <gs/util/string.h>

typedef enum {
    GsLoaderApiSuccess,
    GsLoaderApiProcessQueryError,
    GsLoaderApiSetMapNotFoundError,
    GsLoaderApiUnhandledVersionError,
    GsLoaderApiAllocationError,
    GsLoaderApiInvalidNameError,
    GsLoaderApiNotImplementedError
} GsLoaderApiError;

/**
 * @brief API Set Namespace
 * 
 */
typedef struct _API_SET_NAMESPACE {
    ULONG Version;
} API_SET_NAMESPACE, *PAPI_SET_NAMESPACE;

/**
 * @brief Given a DLL name specified either by a user or in the import table
 * of another DLL, determine whether it refers to an API set or is a direct
 * reference to an API name.
 * 
 * @param LibraryName   Library name to be checked
 * @return TRUE         If this library name refers to an API set
 * @return FALSE        If this library name is a direct reference 
 */
BOOL GsIsApiSetReference(
    _In_z_ LPCSTR LibraryName
);

/**
 * @brief Given an API set name, resolve it to an actual library name.
 * 
 * @param ApiSetName            API set name
 * @param LibraryName           Resolved library name
 * @return GsLoaderApiSuccess   On Success 
 */
_Success_(return == GsLoaderApiSuccess)
GsLoaderApiError GsResolveApiSetToLibrary(
    _In_z_  LPCSTR      ApiSetName,
    _Inout_ PGS_STRING  LibraryName
);

#endif // GS_LOADER_API_H
