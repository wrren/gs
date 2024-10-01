#ifndef GS_PE_H
#define GS_PE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gs/util/arena.h>
#include <gs/util/string.h>
#include <gs/util/list.h>

typedef enum {
    GsPeSuccess,
    GsPeMemoryAllocationError,
    GsPeArenaCleanupError,
    GsPeFileOpenError,
    GsPeFileReadError,
    GsPeInvalidFileFormatError,
    GsPeUnhandledMachineError,
    GsPeSerializationError,
    GsPeListInsertionError,
    GsPeImportResolutionError,
    GsPeEntryPointCallError,
    GsPeImageNotLoadedError
} GsPeError;

/**
 * @brief Represents a single PE section and its data
 * 
 */
typedef struct _GS_PE_SECTION
{
    IMAGE_SECTION_HEADER    Header;
    PVOID                   Data;
} GS_PE_SECTION, *PGS_PE_SECTION;

/**
 * @brief Represents a parsed and decoded PE file.
 * 
 */
typedef struct _GS_PE
{
    PGS_ARENA               Arena;
    IMAGE_DOS_HEADER        DosHeader;
    DWORD                   Signature;
    IMAGE_FILE_HEADER       FileHeader;
    IMAGE_OPTIONAL_HEADER64 OptionalHeader;
    PGS_PE_SECTION          Sections;
    PVOID                   ImageBase;
} GS_PE, *PGS_PE;

/**
 * @brief Represents the name and export address of a library loaded by `GsPeLoad`
 * 
 */
typedef struct _GS_PE_EXPORT
{
    PGS_STRING  Name;
    WORD        Ordinal;
    PVOID       Address;
} GS_PE_EXPORT, *PGS_PE_EXPORT;

/**
 * @brief Attempt to read a PE format file and generate a GS_PE struct from its contents.
 * 
 * @param Path      Path to the PE file
 * @param Error     Output error on failure
 * @return PGS_PE   Pointer to the generated PE struct or NULL on failure.
 */
_Success_(return != NULL)
PGS_PE GsPeReadFromFile(
    _In_z_ LPCWSTR          Path,
    _Outptr_opt_ GsPeError* Error
);

/**
 * @brief Given a pointer to an existing image loaded into this process' memory, generate
 * a PE image struct, a list of exports and a pointer to its entry point.
 * 
 * @param ImageBase     Pointer to the base of the image
 * @param Exports       List of exports found in the image
 * @param Error         Output error set when reading is unsuccessful
 * @return PGS_PE       Pointer to PE image struct on success, NULL on failure.
 */
_Success_(return != NULL)
PGS_PE GsPeReadFromMemory(
    _In_ PVOID              ImageBase,
    _Inout_ PGS_LIST        Exports,
    _Outptr_opt_ GsPeError* Error    
);

/**
 * @brief Load the given PE into the address space of this process.
 * 
 * @param PE            PE to be loaded
 * @param EntryPoint    Output pointer to the entry point of the loaded image
 * @param Exports       Output list of exports for the loaded image
 * @param Error         Output error set when loading is unsuccessful
 * @return PVOID        Pointer to image base or NULL on failure
 */
_Success_(return != NULL)
PVOID GsPeLoad(
    _In_ PGS_PE             PE,
    _Inout_ PGS_LIST        Exports,
    _Outptr_opt_ GsPeError* Error            
);

/**
 * @brief Resolve the imports for the given PE Image
 * 
 * @param PE                PE image struct
 * @return GsPeSuccess     On success
 */
_Success_(return == GsPeSuccess)
GsPeError GsPeResolveImports(
    _In_ PGS_PE             PE
);

/**
 * @brief Call the EntryPoint/DLL Attach function for the given loaded PE image
 * 
 * @param PE            PE Image
 * @return GsPeSuccess  On success 
 */
_Success_(return == GsPeSuccess)
GsPeError GsPeAttach(
    _In_ PGS_PE             PE
);

/**
 * @brief Unload the given PE image from memory.
 * 
 * @param Pe                PE to be unloaded
 * @return VOID 
 */
VOID GsPeUnload(
    _Inout_ PGS_PE Pe
);

#ifdef __cplusplus
}
#endif

#endif // GS_PE_H