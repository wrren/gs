#include <gs/util/wstring.h>
#include <gs/util/string.h>
#include <gs/util/buffer.h>
#include <string.h>

/// Growth factor for committed capacity
#define GS_WSTRING_CAPACITY_GROWTH_FACTOR 2

INT GspWStringSearchFunction(
    _In_ PVOID Element,
    _In_ PVOID Context
)
{
    PWCHAR Character = (PWCHAR) Element;
    PWCHAR Needles   = (PWCHAR) Context;

    while((*Needles) != L'\0') {
        if((*Needles) == (*Character)) {
            return 0;
        }
        ++Needles;
    }

    return -1;
}

PGS_WSTRING GsWStringInit(
    _In_ PGS_ARENA Arena
)
{
    return GsWStringInitWithContentN(Arena, NULL, 0);
}

PGS_WSTRING GsWStringInitWithString(
    _In_ PGS_STRING String
)
{
    return GsWStringInitWithNonWideContentN(
        String->Arena,
        String->Content,
        String->Length
    );
}

PGS_WSTRING GsWStringInitWithContent(
    _In_ PGS_ARENA  Arena,
    _In_z_ LPCWSTR  Content
)
{
    return GsWStringInitWithContentN(Arena, Content, wcslen(Content));
}

PGS_WSTRING GsWStringInitWithNonWideContent(
    _In_ PGS_ARENA  Arena,
    _In_z_ LPCSTR   Content
)
{
    return GsWStringInitWithNonWideContentN(Arena, Content, strlen(Content));
}

PGS_WSTRING GsWStringInitWithContentN(
    _In_ PGS_ARENA  Arena,
    _In_z_ LPCWSTR  Content,
    _In_ SIZE_T     ContentLength
)
{
    PGS_WSTRING String = (PGS_WSTRING) GsArenaAlloc(Arena, sizeof(GS_WSTRING));
    if(String == NULL) {
        return NULL;
    }

    String->Arena       = Arena;
    String->Content     = NULL;
    String->Capacity    = 0;
    String->Length      = 0;

    if(Content != NULL && ContentLength > 0) {
        SIZE_T RequiredCapacity     = (ContentLength + 1);
        SIZE_T RequiredAllocation   = RequiredCapacity * sizeof(WCHAR);

        String->Content = (LPWSTR) GsArenaAlloc(Arena, RequiredAllocation);
        if(String->Content == NULL) {
            return NULL;
        }

        String->Capacity = RequiredCapacity;

        if(wcscpy_s(String->Content, String->Capacity, Content)) {
            return NULL;
        }

        String->Length      = ContentLength;
    }

    return String;    
}

PGS_WSTRING GsWStringInitWithNonWideContentN(
    _In_ PGS_ARENA  Arena,
    _In_z_ LPCSTR   Content,
    _In_ SIZE_T     ContentLength
)
{
    if(ContentLength == 0) {
        return GsWStringInitWithContentN(Arena, NULL, 0);
    }

    INT Length = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, Content, (INT) ContentLength, NULL, 0);

    if(Length == 0) {
        return NULL;
    }

    PGS_WSTRING String = (PGS_WSTRING) GsArenaAlloc(Arena, sizeof(GS_WSTRING));
    if(String == NULL) {
        return NULL;
    }

    String->Arena       = Arena;
    String->Content     = GsArenaAlloc(Arena, (Length + 1) * sizeof(WCHAR));
    String->Capacity    = Length + 1;
    String->Length      = Length;

    if(String->Content == NULL) {
        return NULL;
    }

    if(MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, Content, (INT) ContentLength, String->Content, (INT) String->Capacity) != Length) {
        return NULL;
    }

    return String;
}

UNICODE_STRING GsWStringToUnicodeString(
    _In_ PGS_WSTRING String
)
{
    UNICODE_STRING UnicodeString;

    RtlInitUnicodeString(&UnicodeString, String->Content);

    return UnicodeString;
}

GsWStringError GsWStringConcat(
    _In_ PGS_WSTRING    String,
    _In_z_ LPCWSTR      Content
)
{
    return GsWStringConcatN(String, Content, wcslen(Content));
}

GsWStringError GsWStringConcatN(
    _In_ PGS_WSTRING    String,
    _In_z_ LPCWSTR      Content,
    _In_ SIZE_T         ContentLength
)
{
    if(ContentLength == 0 || Content == NULL) {
        return GsWStringSuccess;
    }

    SIZE_T NewLength    = String->Length + ContentLength;
    SIZE_T NewCapacity  = NewLength + 1;

    if(NewCapacity > String->Capacity) {
        SIZE_T RequiredAllocation   = NewCapacity * sizeof(WCHAR);
        SIZE_T CurrentAllocation    = String->Capacity * sizeof(WCHAR);
        LPWSTR NewBuffer            = GsArenaRealloc(String->Arena, String->Content, CurrentAllocation, RequiredAllocation);
        if(NewBuffer == NULL) {
            return GsWStringAllocationError;
        }

        String->Capacity    = NewCapacity;
        String->Content     = NewBuffer;
    }  
    
    if(String->Length > 0) {
        if(wcscat_s(String->Content, String->Capacity, Content)) {
            return GsWStringConcatenationError;
        }
    } else {
        if(wcscpy_s(String->Content, String->Capacity, Content)) {
            return GsWStringCopyError;
        }
    }
    
    String->Length = wcsnlen_s(String->Content, String->Capacity);

    return GsWStringSuccess;
}

_Success_(return == GsWStringSuccess)
GsWStringError GsWStringReserve(
    _Inout_ PGS_WSTRING String,
    _In_ SIZE_T         Capacity
)
{
    if(String->Capacity < Capacity) {
        SIZE_T RequiredAllocation   = Capacity * sizeof(WCHAR);
        SIZE_T CurrentAllocation    = String->Capacity * sizeof(WCHAR);
        LPWSTR NewBuffer            = GsArenaRealloc(String->Arena, String->Content, CurrentAllocation, RequiredAllocation);
        if(NewBuffer == NULL) {
            return GsWStringAllocationError;
        }

        String->Capacity    = Capacity;
        String->Content     = NewBuffer;
    }

    return GsWStringSuccess;
}

VOID GsWStringClear(
    _Inout_ PGS_WSTRING String
)
{
    String->Length = 0;
}

SIZE_T GsWStringFindFirstOf(
    _In_ PGS_WSTRING    String,
    _In_ SIZE_T         Offset,
    _In_ LPCWSTR        Characters
)
{
    return GsBufferSearch(
        String->Content,
        String->Length * sizeof(WCHAR),
        sizeof(WCHAR),
        Offset,
        GspWStringSearchFunction,
        (PVOID) Characters
    );
}

SIZE_T GsWStringFindLastOf(
    _In_ PGS_WSTRING    String,
    _In_ SIZE_T         Offset,
    _In_ LPCWSTR        Characters
)
{
    return GsBufferReverseSearch(
        String->Content,
        String->Length * sizeof(WCHAR),
        sizeof(WCHAR),
        Offset,
        GspWStringSearchFunction,
        (PVOID) Characters
    );
}