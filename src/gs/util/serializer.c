#include <gs/util/serializer.h>

_Success_(return == TRUE)
BOOL GsSerialize(
    _In_ PVOID          Destination, 
    _In_ SIZE_T         DestinationSize,
    _In_ PVOID          Source,
    _In_ SIZE_T         SourceSize,
    _Outptr_ PSIZE_T    Offset
)
{
    if((*Offset + SourceSize) > DestinationSize) {
        return FALSE;
    }

    memcpy(((PUINT8) Destination) + (*Offset), Source, SourceSize);
    *Offset += SourceSize;

    return TRUE;
}