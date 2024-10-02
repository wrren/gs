#include <gs/util/string.h>
#include <gs/util/buffer.h>
#include <string.h>

/// Growth factor for committed capacity
#define GS_STRING_CAPACITY_GROWTH_FACTOR 2

INT GspStringSearchFunction(
    _In_ PVOID Element,
    _In_ PVOID Context
)
{
    PCHAR Character = (PCHAR) Element;
    PCHAR Needles   = (PCHAR) Context;

    while((*Needles) != '\0') {
        if((*Needles) == (*Character)) {
            return 0;
        }
        ++Needles;
    }

    return -1;
}

PGS_STRING GsStringInit(
    _In_ PGS_ARENA Arena
)
{
    return GsStringInitWithContentN(Arena, NULL, 0);
}

VOID GsStringCopy(
    _In_ PGS_STRING ThisString,
    _In_ PGS_STRING ToCopy
)
{
    if(ThisString->Capacity < ToCopy->Capacity) {
        
    }
}

PGS_STRING GsStringInitWithContent(
    _In_ PGS_ARENA  Arena,
    _In_z_ LPCSTR   Content
)
{
    return GsStringInitWithContentN(Arena, Content, strlen(Content));
}

PGS_STRING GsStringInitWithWideContent(
    _In_ PGS_ARENA  Arena,
    _In_z_ LPCWSTR  Content
)
{
    return GsStringInitWithWideContentN(Arena, Content, wcslen(Content));
}

PGS_STRING GsStringInitWithContentN(
    _In_ PGS_ARENA  Arena,
    _In_z_ LPCSTR   Content,
    _In_ SIZE_T     ContentLength
)
{
    PGS_STRING String = (PGS_STRING) GsArenaAlloc(Arena, sizeof(GS_STRING));
    if(String == NULL) {
        return NULL;
    }

    String->Arena       = Arena;
    String->Content     = NULL;
    String->Capacity    = 0;
    String->Length      = 0;

    if(Content != NULL && ContentLength > 0) {
        SIZE_T RequiredCapacity = ContentLength + 1;

        String->Content = (LPSTR) GsArenaAlloc(Arena, RequiredCapacity);
        if(String->Content == NULL) {
            return NULL;
        }

        String->Capacity    = RequiredCapacity;

        if(strncpy_s(String->Content, String->Capacity, Content, ContentLength)) {
            return NULL;
        }

        String->Length      = ContentLength;
    }

    return String;    
}

PGS_STRING GsStringInitWithWideContentN(
    _In_ PGS_ARENA  Arena,
    _In_z_ LPCWSTR  Content,
    _In_ SIZE_T     ContentLength
)
{
    INT Length = WideCharToMultiByte(CP_ACP, 0, Content, (INT) ContentLength, NULL, 0, NULL, FALSE);

    if(Length == 0) {
        return NULL;
    }

    PGS_STRING String = (PGS_STRING) GsArenaAlloc(Arena, sizeof(GS_STRING));
    if(String == NULL) {
        return NULL;
    }

    String->Arena       = Arena;
    String->Content     = GsArenaAlloc(Arena, (Length + 1));
    String->Capacity    = Length + 1;
    String->Length      = Length;

    if(String->Content == NULL) {
        return NULL;
    }

    if(WideCharToMultiByte(CP_ACP, 0, Content, (INT) ContentLength, String->Content, (INT) String->Capacity, NULL, FALSE) != Length) {
        return NULL;
    }

    return String;
}

GsStringError GsStringConcat(
    _In_ PGS_STRING String,
    _In_z_ LPCSTR   Content
)
{
    return GsStringConcatN(String, Content, strlen(Content));
}

GsStringError GsStringConcatN(
    _In_ PGS_STRING String,
    _In_z_ LPCSTR   Content,
    _In_ SIZE_T     ContentLength
)
{
    if(ContentLength == 0 || Content == NULL) {
        return GsStringSuccess;
    }

    SIZE_T NewLength    = String->Length + ContentLength;
    SIZE_T NewCapacity  = max(NewLength + 1, String->Capacity * GS_STRING_CAPACITY_GROWTH_FACTOR);

    if(NewCapacity > String->Capacity) {
        LPSTR NewBuffer = GsArenaRealloc(String->Arena, String->Content, String->Capacity, NewCapacity);
        if(NewBuffer == NULL) {
            return GsStringAllocationError;
        }

        String->Capacity = NewCapacity;
        String->Content = NewBuffer;
    }  
    
    if(String->Length > 0) {
        if(strncat_s(String->Content, String->Capacity, Content, ContentLength)) {
            return GsStringConcatenationError;
        }
    } else {
        if(strncpy_s(String->Content, String->Capacity, Content, ContentLength)) {
            return GsStringCopyError;
        }
    }
    
    String->Length = strnlen_s(String->Content, String->Capacity);

    return GsStringSuccess;
}

_Success_(return == GsStringSuccess)
GsStringError GsStringReserve(
    _Inout_ PGS_STRING  String,
    _In_ SIZE_T         Capacity
)
{
    if(String->Capacity < Capacity) {
        SIZE_T NewCapacity  = max(Capacity, String->Capacity * GS_STRING_CAPACITY_GROWTH_FACTOR);
        LPSTR NewBuffer = GsArenaRealloc(String->Arena, String->Content, String->Capacity, NewCapacity);
        if(NewBuffer == NULL) {
            return GsStringAllocationError;
        }

        String->Capacity    = NewCapacity;
        String->Content     = NewBuffer;
    }

    return GsStringSuccess;
}

VOID GsStringClear(
    _Inout_ PGS_STRING String
)
{
    String->Length = 0;
}

SIZE_T GsStringFindFirstOf(
    _In_ PGS_STRING String,
    _In_ SIZE_T     Offset,
    _In_ LPCSTR     Characters
)
{
    return GsBufferSearch(
        String->Content,
        String->Length,
        sizeof(CHAR),
        Offset,
        GspStringSearchFunction,
        (PVOID) Characters
    );
}

SIZE_T GsStringFindLastOf(
    _In_ PGS_STRING String,
    _In_ SIZE_T     Offset,
    _In_ LPCSTR     Characters
)
{
    return GsBufferReverseSearch(
        String->Content,
        String->Length,
        sizeof(CHAR),
        Offset,
        GspStringSearchFunction,
        (PVOID) Characters
    );
}