#include <gs/loader/lib.h>
#include <gs/util/arena.h>
#include <gs/util/list.h>
#include <gs/util/string.h>
#include <gs/util/wstring.h>
#include <gs/pe/pe.h>
#include <gs/loader/api.h>
#include <stdio.h>

struct _GS_LIBRARY
{
    PGS_WSTRING Path;
    PGS_LIST    Exports;
    PGS_PE      Image;
    PVOID       ImageBase;
};

struct
{
    PGS_LIST    LoadedLibraries;
    PGS_ARENA   Arena;
} GsLibraryContext = { NULL, NULL };

/**
 * @brief List evaluation function used to find the library whose path matches the one given
 * 
 * @param Element   List element (PGS_LIBRARY) to be evaluated
 * @param Context   Context (library path)
 * @return BOOL     TRUE if the PGS_LIBRARY element matches the one being searched for
 */
_Success_(return == TRUE)
static BOOL GspLibraryFind(
    _In_ PVOID Element,
    _In_ PVOID Context
);

/**
 * @brief List evaluation function used to find a matching export in an exported function list.
 * 
 * @param Element   List element (PGS_PE_EXPORT)
 * @param Context   Context (function name)
 * @return BOOL     TRUE on match, FALSE otherwise
 */
_Success_(return == TRUE)
static BOOL GspLibraryExportFindByName(
    _In_ PVOID Element,
    _In_ PVOID Context
);

/**
 * @brief List evaluation function used to find a matching export in an exported function list.
 * 
 * @param Element   List element (PGS_PE_EXPORT)
 * @param Context   Context (function ordinal)
 * @return BOOL     TRUE on match, FALSE otherwise
 */
_Success_(return == TRUE)
static BOOL GspLibraryExportFindByOrdinal(
    _In_ PVOID Element,
    _In_ PVOID Context
);

/**
 * @brief Attempt to find the full path to the library with the given name.
 * 
 * @param LibraryName   Name of the library.
 * @param FullPath      Output full path to the library
 * @return BOOL 
 */
_Success_(return == TRUE)
static BOOL GspLibraryFindPath(
    _In_z_ LPCWSTR      LibraryName,
    _Inout_ PGS_WSTRING FullPath
);

/**
 * @brief Attempt to find the full path to the library with the given name in the specified directory.
 * 
 * @param LibraryName   Name of the library.
 * @param Directory     Directory in which to search for the library
 * @param FullPath      Output full path to the library
 * @return BOOL 
 */
_Success_(return == TRUE)
static BOOL GspLibraryFindInPath(
    _In_z_ LPCWSTR      LibraryName,
    _In_z_ LPCWSTR      Directory,    
    _Inout_ PGS_WSTRING FullPath
);

_Success_(return == TRUE)
BOOL GsLibraryInit()
{
    if(GsLibraryContext.Arena != NULL && GsLibraryContext.LoadedLibraries != NULL) {
        return TRUE;
    }

    GsLibraryContext.Arena  = GsArenaWithReservationAndPageProtection(
        GS_ARENA_DEFAULT_RESERVATION,
        PAGE_EXECUTE_READWRITE
    );

    if(GsLibraryContext.Arena == NULL) {
        return FALSE;
    }

    GsLibraryContext.LoadedLibraries = GsListInit(
        GsLibraryContext.Arena,
        sizeof(GS_LIBRARY)
    );

    if(GsLibraryContext.LoadedLibraries == NULL) {
        return FALSE;
    }

    return TRUE;
}

_Success_(return != NULL)
PGS_LIBRARY GsLibraryLoad(
    _In_z_ LPCSTR LibraryName
)
{
    PGS_ARENA Scratch   = GsArena();
    PGS_LIBRARY Library = NULL;

    if(Scratch == NULL) {
        return NULL;
    }

    if(GsIsApiSetReference(LibraryName)) {
        PGS_STRING ResolvedName = GsStringInit(Scratch);
        if(GsResolveApiSetToLibrary(LibraryName, ResolvedName) == GsLoaderApiSuccess) {
            Library = GsLibraryLoad(ResolvedName->Content);
        }
        GsArenaRelease(Scratch);
        return Library;
    }

    WCHAR LibraryNameWide[MAX_PATH];

    MultiByteToWideChar(
        CP_ACP,
        MB_ERR_INVALID_CHARS,
        LibraryName,
        -1,
        LibraryNameWide,
        MAX_PATH
    );

    if(StrStrIW(LibraryNameWide, L".DLL") == NULL) {
        StrCatW(LibraryNameWide, L".DLL");
    }

    PGS_WSTRING LibraryPath = GsWStringInit(Scratch);
    if(LibraryPath == NULL) {
        GsArenaRelease(Scratch);
        return NULL;
    }

    if(GspLibraryFindPath(LibraryNameWide, LibraryPath)) {
        Library = GsLibraryLoadFromPath(LibraryPath->Content);;
    }

    GsArenaRelease(Scratch);

    return Library;
}

_Success_(return != NULL)
PGS_LIBRARY GsLibraryLoadFromPath(
    _In_z_ LPCWSTR LibraryPath
)
{
    GsPeError Error = GsPeSuccess;;

    PGS_LIBRARY Match = (PGS_LIBRARY) GsListFindIf(GsLibraryContext.LoadedLibraries, GspLibraryFind, (PVOID) LibraryPath);
    if(Match != NULL) {
        return Match;
    }

    PGS_LIBRARY Library = (PGS_LIBRARY) GsArenaAlloc(GsLibraryContext.Arena, sizeof(GS_LIBRARY));
    if(Library == NULL) {
        return NULL;
    }

    Library->Exports = GsListInit(GsLibraryContext.Arena, sizeof(GS_PE_EXPORT));
    if(Library->Exports == NULL) {
        return NULL;
    }

    Library->Path = GsWStringInit(GsLibraryContext.Arena);
    if(Library->Path == NULL) {
        return NULL;
    }

    if(GsWStringConcat(Library->Path, LibraryPath) != GsWStringSuccess) {
        return NULL;
    }

    PGS_PE PE = GsPeReadFromFile(LibraryPath, NULL);
    if(PE == NULL) {
        wprintf(L"Failed to Load Library %ws\n", Library->Path->Content);
        return NULL;
    }

    
    Library->Image      = PE;
    Library->ImageBase  = GsPeLoad(PE, Library->Exports, &Error);
    if(Library->ImageBase == NULL) {
        wprintf(L"Failed to Load Library %ws: %d\n", Library->Path->Content, Error);
        return NULL;
    }    

    GsListInsert(GsLibraryContext.LoadedLibraries, Library);

    Error = GsPeResolveImports(PE);
    if(Error != GsPeSuccess) {
        wprintf(L"Failed to resolve library imports for %ws: %d\n", Library->Path->Content, Error);
        return NULL;
    }

    Error = GsPeAttach(PE);
    if(Error != GsPeSuccess) {
        wprintf(L"Failed to call entry point for %ws: %d\n", Library->Path->Content, Error);
        return NULL;
    }

    wprintf(L"Loaded Library %ws\n", Library->Path->Content);

    return Library;
}

_Success_(return != NULL)
PVOID GsLibraryGetFunctionAddressByName(
    _In_ PGS_LIBRARY    Library,
    _In_z_ LPCSTR       FunctionName
)
{
    PGS_PE_EXPORT Match = (PGS_PE_EXPORT) GsListFindIf(Library->Exports, GspLibraryExportFindByName, (PVOID) FunctionName);
    if(Match != NULL) {
        return Match->Address;
    }

    return NULL;
}

_Success_(return != NULL)
PVOID GsLibraryGetFunctionAddressByOrdinal(
    _In_ PGS_LIBRARY    Library,
    _In_ WORD           FunctionOrdinal
)
{
    PGS_PE_EXPORT Match = (PGS_PE_EXPORT) GsListFindIf(Library->Exports, GspLibraryExportFindByOrdinal, (PVOID) &FunctionOrdinal);
    if(Match != NULL) {
        return Match->Address;
    }

    return NULL;
}

_Success_(return == TRUE)
BOOL GspLibraryFind(
    _In_ PVOID Element,
    _In_ PVOID Context
)
{
    PGS_LIBRARY Library = (PGS_LIBRARY) Element;
    LPCWSTR Path = (LPCWSTR) Context;

    if(Library == NULL || Path == NULL) {
        return FALSE;
    }

    return _wcsnicmp(Library->Path->Content, Path, Library->Path->Length) == 0;
}

_Success_(return == TRUE)
BOOL GspLibraryExportFindByName(
    _In_ PVOID Element,
    _In_ PVOID Context
)
{
    PGS_PE_EXPORT Export = (PGS_PE_EXPORT) Element;
    LPCSTR FunctionName = (LPCSTR) Context;

    if(Export == NULL || FunctionName == NULL) {
        return FALSE;
    }

    return strncmp(Export->Name->Content, FunctionName, Export->Name->Length) == 0;
}

_Success_(return == TRUE)
BOOL GspLibraryExportFindByOrdinal(
    _In_ PVOID Element,
    _In_ PVOID Context
)
{
    PGS_PE_EXPORT Export    = (PGS_PE_EXPORT) Element;
    PWORD Ordinal           = (PWORD) Context;

    if(Export == NULL || Ordinal == NULL) {
        return FALSE;
    }

    return Export->Ordinal == (*Ordinal);
}

_Success_(return == TRUE)
BOOL GspLibraryFindPath(
    _In_z_ LPCWSTR      LibraryName,
    _Inout_ PGS_WSTRING FullPath
)
{
    WCHAR SystemDirectoryPath[MAX_PATH];
    GetSystemDirectory(SystemDirectoryPath, MAX_PATH);

    if(GspLibraryFindInPath(LibraryName, SystemDirectoryPath, FullPath))
    {
        return TRUE;
    }

    return FALSE;
}

_Success_(return == TRUE)
BOOL GspLibraryFindInPath(
    _In_z_ LPCWSTR      LibraryName,
    _In_z_ LPCWSTR      Directory,    
    _Inout_ PGS_WSTRING FullPath
)
{
    if(GsWStringReserve(FullPath, MAX_PATH) != GsWStringSuccess) {
        return FALSE;
    }

    GsWStringClear(FullPath);

    if((GsWStringConcat(FullPath, Directory) != GsWStringSuccess) ||
       (GsWStringConcat(FullPath, L"\\") != GsWStringSuccess) ||
       (GsWStringConcat(FullPath, LibraryName) != GsWStringSuccess)) {
        return FALSE;
    }

    return PathFileExists(FullPath->Content);
}

VOID GsLibraryRelease()
{
    PGS_LIBRARY Library = (PGS_LIBRARY) GsListPopBack(GsLibraryContext.LoadedLibraries);

    while(Library != NULL) {
        GsPeUnload(Library->Image);
        Library = (PGS_LIBRARY) GsListPopBack(GsLibraryContext.LoadedLibraries);
    }

    GsArenaRelease(GsLibraryContext.Arena);

    GsLibraryContext.Arena              = NULL;
    GsLibraryContext.LoadedLibraries    = NULL;
}