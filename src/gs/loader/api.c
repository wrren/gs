#include <gs/loader/api.h>
#include <gs/nt/ps.h>
#include <gs/nt/api.h>
#include <gs/util/wstring.h>
#include <gs/util/buffer.h>

#include <gs/loader/api/v2.h>
#include <gs/loader/api/v3.h>
#include <gs/loader/api/v4.h>
#include <gs/loader/api/v6.h>

// API Set calling convention
#define APISETAPI NTAPI

//
// API schema definitions.
//
#define API_SET_SCHEMA_VERSION_V2       0x00000002
#define API_SET_SCHEMA_VERSION_V3       0x00000003 // No offline support.
#define API_SET_SCHEMA_VERSION_V4       0x00000004
#define API_SET_SCHEMA_VERSION_V6       0x00000006

#define API_SET_SCHEMA_FLAGS_SEALED              0x00000001
#define API_SET_SCHEMA_FLAGS_HOST_EXTENSION      0x00000002

#define API_SET_SCHEMA_ENTRY_FLAGS_SEALED        0x00000001
#define API_SET_SCHEMA_ENTRY_FLAGS_EXTENSION     0x00000002

BOOL GsIsApiSetReference(
    _In_z_ LPCSTR LibraryName
)
{
    if(strstr(LibraryName, "api-") == NULL && strstr(LibraryName, "ext-") == NULL) {
        return FALSE;
    }

    return TRUE;
}

_Success_(return == GsLoaderApiSuccess)
GsLoaderApiError GsResolveApiSetToLibrary(
    _In_z_  LPCSTR          ApiSetName,
    _Inout_ PGS_STRING      LibraryName
)
{
    if(GsIsApiSetReference(ApiSetName) == FALSE) {
        return GsLoaderApiInvalidNameError;
    }

    HANDLE                      CurrentProcess = GetCurrentProcess();
    PROCESS_BASIC_INFORMATION   BasicInfo;
    ULONG                       ReturnLength;
    PPEB                        ProcessPEB;
    PAPI_SET_NAMESPACE          APISetNamespace;

    NTSTATUS Status = NtQueryInformationProcess(
        CurrentProcess,
        ProcessBasicInformation,
        &BasicInfo,
        sizeof(BasicInfo),
        &ReturnLength
    );

    if(!NT_SUCCESS(Status)) {
        return GsLoaderApiProcessQueryError;
    }

    ProcessPEB  = BasicInfo.PebBaseAddress;
    APISetNamespace   = (PAPI_SET_NAMESPACE) ProcessPEB->ApiSetMap;
    if(!APISetNamespace) {
        return GsLoaderApiSetMapNotFoundError;
    }

    switch(APISetNamespace->Version) {
        case API_SET_SCHEMA_VERSION_V2:
            return GsApiSetResolveToHostV2(APISetNamespace, ApiSetName, LibraryName);
        case API_SET_SCHEMA_VERSION_V3:
            return GsApiSetResolveToHostV3(APISetNamespace, ApiSetName, LibraryName);
        case API_SET_SCHEMA_VERSION_V4:
            return GsApiSetResolveToHostV4(APISetNamespace, ApiSetName, LibraryName);
        case API_SET_SCHEMA_VERSION_V6:
            return GsApiSetResolveToHostV6(APISetNamespace, ApiSetName, LibraryName);
    }

    return GsLoaderApiUnhandledVersionError;
}