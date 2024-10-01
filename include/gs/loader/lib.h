#ifndef GS_LOADER_LIB_H
#define GS_LOADER_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gs/core/platform.h>

typedef struct _GS_LIBRARY GS_LIBRARY, *PGS_LIBRARY;

/**
 * @brief Initialize resources required by the library subsystem.
 * 
 * @return BOOL TRUE on success, FALSE otherwise.
 */
_Success_(return == TRUE)
BOOL GsLibraryInit();

/**
 * @brief Attempt to manually load the library with the given name into this process' address space.
 * 
 * @param LibraryName   Name of the library to be loaded
 * @return PGS_LIBRARY  Pointer to the loaded library or null on failure.
 */
_Success_(return != NULL)
PGS_LIBRARY GsLibraryLoad(
    _In_z_ LPCSTR  LibraryName
);

/**
 * @brief Attempt to manually load a library from a specific path.
 * 
 * @param LibraryPath   Full path to the library to be loaded
 * @return PGS_LIBRARY  Pointer to the loaded library or null on failure.
 */
PGS_LIBRARY GsLibraryLoadFromPath(
    _In_z_ LPCWSTR  LibraryPath
);

/**
 * @brief Get the address of the exported function with the specified name from the given loaded library.
 * 
 * @param Library       Library loaded using `GsLibraryLoad`
 * @param FunctionName  Name of the exported function whose address is to be retrieved
 * @return PVOID        Pointer to the function address or NULL on failure.
 */
_Success_(return != NULL)
PVOID GsLibraryGetFunctionAddressByName(
    _In_ PGS_LIBRARY    Library,
    _In_z_ LPCSTR       FunctionName
);

/**
 * @brief Get the address of the function with the specified ordinal from the given loaded library.
 * 
 * @param Library           Library loaded using `GsLibraryLoad`
 * @param FunctionOrdinal   Ordinal of the exported function whose address is to be retrieved
 * @return PVOID            Pointer to the function address or NULL on failure.
 */
_Success_(return != NULL)
PVOID GsLibraryGetFunctionAddressByOrdinal(
    _In_ PGS_LIBRARY    Library,
    _In_ WORD           FunctionOrdinal
);

/**
 * @brief Release resources allocated by the library subsystem.
 * 
 * @return VOID 
 */
VOID GsLibraryRelease();

#ifdef __cplusplus
}
#endif

#endif // GS_LOADER_LIB_H