#include <gs/loader/api/v2.h>
#include <gs/util/wstring.h>
#include <gs/util/buffer.h>

//
// Support for downlevel API set schema version 2.
//
typedef struct _API_SET_VALUE_ENTRY_V2 {
    ULONG NameOffset;
    ULONG NameLength;
    ULONG ValueOffset;
    ULONG ValueLength;
} API_SET_VALUE_ENTRY_V2, *PAPI_SET_VALUE_ENTRY_V2;

typedef struct _API_SET_VALUE_ARRAY_V2 {
    ULONG Count;
    API_SET_VALUE_ENTRY_V2 Array[ANYSIZE_ARRAY];
} API_SET_VALUE_ARRAY_V2, *PAPI_SET_VALUE_ARRAY_V2;

typedef struct _API_SET_NAMESPACE_ENTRY_V2 {
    ULONG NameOffset;
    ULONG NameLength;
    ULONG DataOffset;   // API_SET_VALUE_ARRAY_V2
} API_SET_NAMESPACE_ENTRY_V2, *PAPI_SET_NAMESPACE_ENTRY_V2;

typedef struct _API_SET_NAMESPACE_ARRAY_V2 {
    ULONG Version;
    ULONG Count;
    API_SET_NAMESPACE_ENTRY_V2 Array[ANYSIZE_ARRAY];
} API_SET_NAMESPACE_ARRAY_V2, *PAPI_SET_NAMESPACE_ARRAY_V2;

typedef const API_SET_VALUE_ENTRY_V2 *PCAPI_SET_VALUE_ENTRY_V2;
typedef const API_SET_VALUE_ARRAY_V2 *PCAPI_SET_VALUE_ARRAY_V2;
typedef const API_SET_NAMESPACE_ENTRY_V2 *PCAPI_SET_NAMESPACE_ENTRY_V2;
typedef const API_SET_NAMESPACE_ARRAY_V2 *PCAPI_SET_NAMESPACE_ARRAY_V2;

typedef struct _GS_API_ENTRY_V2_EVALUATOR_CONTEXT
{
    UNICODE_STRING              Name;
    PAPI_SET_NAMESPACE          SetNamespace;
} GS_API_ENTRY_V2_EVALUATOR_CONTEXT, *PGS_API_ENTRY_V2_EVALUATOR_CONTEXT;

static INT GsLoaderApiEntryV2Evaluator(
    _In_ PVOID Element,
    _In_ PVOID Context
);

GsLoaderApiError GsApiSetResolveToHostV2(
    _In_ PAPI_SET_NAMESPACE     APISetNamespace,
    _In_ LPCSTR                 LibraryName,
    _Out_ PGS_STRING            Output
)
{
    PGS_ARENA Scratch = GsArena();

    PGS_STRING LibraryNameString = GsStringInitWithContent(Scratch, LibraryName);
    if(LibraryNameString == NULL) {
        return GsLoaderApiAllocationError;
    }

    LibraryNameString->Content  = LibraryNameString->Content + strlen("api-");
    LibraryNameString->Length   -= strlen("api-");
    PCHAR SuffixStart = strstr(LibraryNameString->Content, ".dll");
    if(SuffixStart == NULL) {
        GsArenaRelease(Scratch);
        return GsLoaderApiInvalidNameError;
    }

    (*SuffixStart) = L'\0';
    LibraryNameString->Length -= strlen(".dll");

    PGS_WSTRING WideLibraryName = GsWStringInitWithString(LibraryNameString);
    if(WideLibraryName == NULL) {
        GsArenaRelease(Scratch);
        return GsLoaderApiInvalidNameError;
    }

    UNICODE_STRING UnicodeLibraryName = GsWStringToUnicodeString(WideLibraryName);

    GS_API_ENTRY_V2_EVALUATOR_CONTEXT Context = { UnicodeLibraryName, APISetNamespace };

    PAPI_SET_NAMESPACE_ARRAY_V2 V2Array = (PAPI_SET_NAMESPACE_ARRAY_V2) APISetNamespace;
    SIZE_T Index = GsBufferBinarySearch(
        (PVOID) &V2Array->Array[0],
        sizeof(PAPI_SET_NAMESPACE_ENTRY_V2) * V2Array->Count,
        V2Array->Count,
        GsLoaderApiEntryV2Evaluator,
        &Context
    );

    GsArenaRelease(Scratch);

    return GsLoaderApiSuccess;
}

INT 
GsLoaderApiEntryV2Evaluator(
    _In_ PVOID Element,
    _In_ PVOID Context
    )
{
    PGS_API_ENTRY_V2_EVALUATOR_CONTEXT EContext     = (PGS_API_ENTRY_V2_EVALUATOR_CONTEXT) Context;
    PAPI_SET_NAMESPACE_ENTRY_V2 Entry               = (PAPI_SET_NAMESPACE_ENTRY_V2) Element;

    UNICODE_STRING EntryString;
    EntryString.Length          = (USHORT) Entry->NameLength;
    EntryString.MaximumLength   = (USHORT) Entry->NameLength;
    EntryString.Buffer          = (PWCHAR)((ULONG_PTR) EContext->SetNamespace + Entry->NameOffset);

    return RtlCompareUnicodeString(&EContext->Name, &EntryString, TRUE);
}