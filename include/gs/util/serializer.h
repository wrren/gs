#ifndef GS_UTIL_SERIALIZER_H
#define GS_UTIL_SERIALIZER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gs/core/platform.h>

/**
 * @brief Attempt to serialize source data into the given destination buffer.
 * 
 * @param Destination       Destination buffer
 * @param DestinationSize   Size of the destination buffer in bytes
 * @param Source            Source buffer
 * @param SourceSize        Size of the source buffer in bytes
 * @param Offset            Offset into the destination where serialization should begin
 * @return BOOL             TRUE if the destination buffer has the capacity to receive the data.
 */
_Success_(return == TRUE)
BOOL GsSerialize(
    _In_ PVOID          Destination, 
    _In_ SIZE_T         DestinationSize,
    _In_ PVOID          Source,
    _In_ SIZE_T         SourceSize,
    _Outptr_ PSIZE_T    Offset
);

#ifdef __cplusplus
}
#endif

#endif // GS_UTIL_SERIALIZER_H