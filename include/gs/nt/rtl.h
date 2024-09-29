/*
Copyright (c) 2017-2019 Aidan Khoury

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <gs/nt/api.h>

// Hacky case manipulation macros.

#define RTL_UPCASE(wch) (                                                       \
    ((wch) < 'a' ?                                                              \
        (wch)                                                                   \
    :                                                                           \
        ((wch) <= 'z' ?                                                         \
            (wch) - ('a'-'A')                                                   \
        :                                                                       \
            ((WCHAR)(wch))                                                      \
        )                                                                       \
    )                                                                           \
)

#define RTL_DOWNCASE(wch) (                                                     \
    ((wch) < 'A' ?                                                              \
        (wch)                                                                   \
    :                                                                           \
        ((wch) <= 'Z' ?                                                         \
            (wch) + ('a'-'A')                                                   \
        :                                                                       \
            ((WCHAR)(wch))                                                      \
        )                                                                       \
    )                                                                           \
)

//NTSYSAPI
LONG
NTAPI
RtlCompareUnicodeStrings(
    IN CONST WCHAR* String1,
    IN SIZE_T Length1,
    IN CONST WCHAR* String2,
    IN SIZE_T Length2,
    IN BOOLEAN CaseInSensitive
    );

#if !defined(_NTDDK_)

FORCEINLINE
LONG
NTAPI
RtlCompareUnicodeString(
    IN PCUNICODE_STRING String1,
    IN PCUNICODE_STRING String2,
    IN BOOLEAN CaseInSensitive
)
{
    return RtlCompareUnicodeStrings(String1->Buffer, String1->Length,
                                    String2->Buffer, String2->Length,
                                    CaseInSensitive);
}

_At_(DestinationString->Buffer, _Post_equal_to_(SourceString))
_At_(DestinationString->Length, _Post_equal_to_(_String_length_(SourceString) * sizeof(WCHAR)))
_At_(DestinationString->MaximumLength, _Post_equal_to_((_String_length_(SourceString)+1) * sizeof(WCHAR)))
//NTSYSAPI
VOID
NTAPI
RtlInitUnicodeString(
    _Out_ PUNICODE_STRING DestinationString,
    _In_opt_z_ PCWSTR SourceString
    );

_At_(UnicodeString->Buffer, _Post_equal_to_(Buffer))
_At_(UnicodeString->Length, _Post_equal_to_(0))
_At_(UnicodeString->MaximumLength, _Post_equal_to_(BufferSize))
FORCEINLINE
VOID
RtlInitEmptyUnicodeString(
    OUT PUNICODE_STRING UnicodeString,
    IN _Writable_bytes_(BufferSize) _When_(BufferSize != 0, _Notnull_)
    PWCHAR Buffer,
    IN UINT16 BufferSize
)
{
    memset(UnicodeString, 0, sizeof(*UnicodeString));
    UnicodeString->MaximumLength = BufferSize;
    UnicodeString->Buffer = Buffer;
}

_At_(AnsiString->Buffer, _Post_equal_to_(Buffer))
_At_(AnsiString->Length, _Post_equal_to_(0))
_At_(AnsiString->MaximumLength, _Post_equal_to_(BufferSize))
FORCEINLINE
VOID
RtlInitEmptyAnsiString(
    OUT PANSI_STRING AnsiString,
    IN _Pre_readable_size_(BufferSize) CHAR* Buffer,
    IN UINT16 BufferSize
)
{
    AnsiString->Length = 0;
    AnsiString->MaximumLength = BufferSize;
    AnsiString->Buffer = Buffer;
}

#endif // !_NTDDK_

