#ifndef GS_LOADER_API_V4_H
#define GS_LOADER_API_V4_H

#include <gs/loader/api.h>

/**
 * @brief Resolve an API Set v4 reference.
 * 
 * @param APISetNamespace       Version 4 API Set Namespace
 * @param LibraryName           Library name to search for
 * @param Output                Output library name
 * @return GsLoaderApiSuccess   On success
 */
GsLoaderApiError GsApiSetResolveToHostV4(
    _In_ PAPI_SET_NAMESPACE     APISetNamespace,
    _In_ LPCSTR                 LibraryName,
    _Out_ PGS_STRING            Output
);

#endif // GS_LOADER_API_V4_H