#ifndef GS_LOADER_API_V6_H
#define GS_LOADER_API_V6_H

#include <gs/loader/api.h>

/**
 * @brief Resolve an API Set v6 reference.
 * 
 * @param APISetNamespace       Version 6 API Set Namespace
 * @param LibraryName           Library name to search for
 * @param Output                Output library name
 * @return GsLoaderApiSuccess   On success
 */
GsLoaderApiError GsApiSetResolveToHostV6(
    _In_ PAPI_SET_NAMESPACE     APISetNamespace,
    _In_ LPCSTR                 LibraryName,
    _Out_ PGS_STRING            Output
);

#endif // GS_LOADER_API_V6_H