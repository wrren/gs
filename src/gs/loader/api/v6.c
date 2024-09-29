#include <gs/loader/api/v6.h>
#include <gs/util/wstring.h>
#include <gs/util/buffer.h>

//
// API set schema version 6.
//
typedef struct _API_SET_NAMESPACE_V6 {
    ULONG Version;
    ULONG Size;
    ULONG Flags;
    ULONG Count;
    ULONG EntryOffset;  // API_SET_NAMESPACE_ENTRY_V6
    ULONG HashOffset;   // API_SET_NAMESPACE_HASH_ENTRY_V6
    ULONG HashFactor;
} API_SET_NAMESPACE_V6, *PAPI_SET_NAMESPACE_V6;

typedef struct _API_SET_NAMESPACE_ENTRY_V6 {
    ULONG Flags;
    ULONG NameOffset;
    ULONG NameLength;
    ULONG HashedLength;
    ULONG ValueOffset;
    ULONG ValueCount;
} API_SET_NAMESPACE_ENTRY_V6, *PAPI_SET_NAMESPACE_ENTRY_V6;

typedef struct _API_SET_HASH_ENTRY_V6 {
    ULONG Hash;
    ULONG Index;
} API_SET_HASH_ENTRY_V6, *PAPI_SET_HASH_ENTRY_V6;

typedef struct _API_SET_VALUE_ENTRY_V6 {
    ULONG Flags;
    ULONG NameOffset;
    ULONG NameLength;
    ULONG ValueOffset;
    ULONG ValueLength;
} API_SET_VALUE_ENTRY_V6, *PAPI_SET_VALUE_ENTRY_V6; 

typedef const API_SET_VALUE_ENTRY_V6 *PCAPI_SET_VALUE_ENTRY_V6;
typedef const API_SET_HASH_ENTRY_V6 *PCAPI_SET_HASH_ENTRY_V6;
typedef const API_SET_NAMESPACE_ENTRY_V6 *PCAPI_SET_NAMESPACE_ENTRY_V6;
typedef const API_SET_NAMESPACE_V6 *PCAPI_SET_NAMESPACE_V6;

#define GET_API_SET_NAMESPACE_ENTRY_V6(ApiSetNamespace, Index) \
    ((PAPI_SET_NAMESPACE_ENTRY_V6)((ULONG_PTR)(ApiSetNamespace) + \
                                ((PAPI_SET_NAMESPACE_V6)(ApiSetNamespace))->EntryOffset + \
                                    ((Index) * sizeof(API_SET_NAMESPACE_ENTRY_V6))))

#define GET_API_SET_NAMESPACE_VALUE_ENTRY_V6(ApiSetNamespace, Entry, Index) \
    ((PAPI_SET_VALUE_ENTRY_V6)((ULONG_PTR)(ApiSetNamespace) + \
                                ((PAPI_SET_NAMESPACE_ENTRY_V6)(Entry))->ValueOffset + \
                                    ((Index) * sizeof(API_SET_VALUE_ENTRY_V6))))

#define GET_API_SET_NAMESPACE_ENTRY_NAME_V6(ApiSetNamespace, Entry) \
    ((PWCHAR)((ULONG_PTR)(ApiSetNamespace) + ((PAPI_SET_NAMESPACE_ENTRY_V6)(Entry))->NameOffset))

#define GET_API_SET_NAMESPACE_ENTRY_VALUE_V6(ApiSetNamespace, Entry) \
    ((PWCHAR)((ULONG_PTR)(ApiSetNamespace) + ((PAPI_SET_NAMESPACE_ENTRY_V6)(Entry))->ValueOffset))

#define GET_API_SET_VALUE_ENTRY_NAME_V6(ApiSetNamespace, Entry) \
    ((PWCHAR)((ULONG_PTR)(ApiSetNamespace) + ((PAPI_SET_VALUE_ENTRY_V6)(Entry))->NameOffset))

#define GET_API_SET_VALUE_ENTRY_VALUE_V6(ApiSetNamespace, Entry) \
    ((PWCHAR)((ULONG_PTR)(ApiSetNamespace) + ((PAPI_SET_VALUE_ENTRY_V6)(Entry))->ValueOffset))

#define GET_API_SET_HASH_ENTRY_V6(ApiSetNamespace, Middle) \
    ((PAPI_SET_HASH_ENTRY_V6)((ULONG_PTR)(ApiSetNamespace) + \
                            ((PAPI_SET_NAMESPACE_V6)(ApiSetNamespace))->HashOffset + \
                                ((Middle) * sizeof(API_SET_HASH_ENTRY_V6))))

PVOID GsLoaderRetrieveApiSetV6Entry(
    _In_ PVOID  Namespace,
    _In_ SIZE_T Index,
    _In_ PVOID  Context
)
{
    return (
                (PAPI_SET_HASH_ENTRY_V6)((ULONG_PTR)(Namespace) +
                ((PAPI_SET_NAMESPACE_V6)(Namespace))->HashOffset +
                ((Index) * sizeof(API_SET_HASH_ENTRY_V6)))
    );
}

INT GsLoaderEvaluateApiSetV6Entry(
    _In_ PVOID Entry,
    _In_ PVOID Context
)
{
    PAPI_SET_HASH_ENTRY_V6 HashEntry    = (PAPI_SET_HASH_ENTRY_V6) Entry;
    PULONG HashKey                      = (PULONG) Context;

    return (INT)(HashEntry->Hash - (*HashKey));
}

GsLoaderApiError GsApiSetResolveToHostV6(
    _In_ PAPI_SET_NAMESPACE     APISetNamespace,
    _In_ LPCSTR                 LibraryName,
    _Out_ PGS_STRING            Output
)
{
    PGS_ARENA Scratch = GsArena();

    PGS_WSTRING LibraryNameString = GsWStringInitWithNonWideContent(Scratch, LibraryName);
    if(LibraryNameString == NULL) {
        GsArenaRelease(Scratch);
        return GsLoaderApiInvalidNameError;
    }

    SIZE_T LastHyphenIndex = GsWStringFindLastOf(LibraryNameString, 0, L"-");
    if(LastHyphenIndex == SIZE_MAX) {
        GsArenaRelease(Scratch);
        return GsLoaderApiInvalidNameError;
    }

    ULONG HashKey       = 0;
    SIZE_T Count        = LastHyphenIndex;
    PWCHAR CurrentChar  = &(LibraryNameString->Content[0]);

    do {
        HashKey = HashKey * ((PCAPI_SET_NAMESPACE_V6) APISetNamespace)->HashFactor + tolower(*CurrentChar);
        ++CurrentChar;
        --Count;
    } while(Count);

    SIZE_T Index = GsBufferBinarySearchWithRetriever(
        APISetNamespace,
        ((PCAPI_SET_NAMESPACE_V6) APISetNamespace)->Count,
        GsLoaderEvaluateApiSetV6Entry,
        &HashKey,
        GsLoaderRetrieveApiSetV6Entry,
        NULL
    );

    if(Index == SIZE_MAX) {
        GsArenaRelease(Scratch);
        return GsLoaderApiInvalidNameError;
    }

    PAPI_SET_HASH_ENTRY_V6 HashEntry = (PAPI_SET_HASH_ENTRY_V6) GsLoaderRetrieveApiSetV6Entry(APISetNamespace, Index, NULL);

    PAPI_SET_NAMESPACE_ENTRY_V6 Entry = ((PAPI_SET_NAMESPACE_ENTRY_V6)((ULONG_PTR)(APISetNamespace) +
                                        ((PAPI_SET_NAMESPACE_V6)(APISetNamespace))->EntryOffset +
                                        ((HashEntry->Index) * sizeof(API_SET_NAMESPACE_ENTRY_V6))));

    if(Entry == NULL) {
        GsArenaRelease(Scratch);
        return GsLoaderApiInvalidNameError;
    }

    PWCHAR EntryName = ((PWCHAR)((ULONG_PTR)(APISetNamespace) + ((PAPI_SET_NAMESPACE_ENTRY_V6)(Entry))->NameOffset));

    if (RtlCompareUnicodeStrings(LibraryNameString->Content,
                                 LastHyphenIndex,
                                 EntryName,
                                 Entry->HashedLength / sizeof(WCHAR),
                                 TRUE) != 0) {
        GsArenaRelease(Scratch);
        return GsLoaderApiInvalidNameError;
    }

    if(Entry->ValueCount == 0) {
        GsArenaRelease(Scratch);
        return GsLoaderApiInvalidNameError;
    }

    PAPI_SET_VALUE_ENTRY_V6 HostLibraryEntry = GET_API_SET_NAMESPACE_VALUE_ENTRY_V6(
        APISetNamespace,
        Entry,
        0);

    PWCHAR ResolvedEntryName = GET_API_SET_VALUE_ENTRY_VALUE_V6(APISetNamespace, HostLibraryEntry);
 
    PGS_STRING ResolvedString = GsStringInitWithWideContentN(
        Scratch,
        ResolvedEntryName,
        (HostLibraryEntry->ValueLength / sizeof(WCHAR))
    );

    GsStringClear(Output);
    GsStringConcatN(Output, ResolvedString->Content, ResolvedString->Length);

    GsArenaRelease(Scratch);

    return GsLoaderApiSuccess;
}