#include <gs/util/buffer.h>
#include <gs/util/arena.h>

#define GS_BUFFER_ELEMENT(Buffer, Index, ElementSize) ((PVOID)((UINT_PTR) Buffer + (Index * ElementSize)))

static PVOID GspBufferDefaultRetriever(
    _In_ PVOID  Buffer,
    _In_ SIZE_T Index,
    _In_ PVOID  Context
);

/**
 * @brief Performs a merge sort on sections of the given input buffers.
 * 
 * @param A             Buffer being evaluated
 * @param B             Work (output) buffer
 * @param BufferSize    Size of each buffer in bytes
 * @param ElementSize   Size of a single buffer element
 * @param StartIndex    Start index of the left sub-buffer
 * @param MiddleIndex   Middle index between the two buffers
 * @param EndIndex      End index of the right buffer
 * @param Comparator    Comparator used to compare elements
 */
static VOID GspMergeSort(
    _In_ PVOID              A,
    _In_ PVOID              B,
    _In_ SIZE_T             BufferSize,
    _In_ SIZE_T             ElementSize,
    _In_ SIZE_T             StartIndex,
    _In_ SIZE_T             MiddleIndex,
    _In_ SIZE_T             EndIndex,
    _In_ GsBufferEvaluator  Comparator
);

SIZE_T GsBufferSearch(
    _In_ PVOID              Buffer,
    _In_ SIZE_T             BufferSize,
    _In_ SIZE_T             ElementSize,
    _In_ SIZE_T             Offset,
    _In_ GsBufferEvaluator  Evaluator,
    _In_ PVOID              Context     
)
{
    if(ElementSize == 0) {
        return SIZE_MAX;
    }

    SIZE_T Index        = Offset;
    SIZE_T NumElements  = (BufferSize / ElementSize);

    while(Index < NumElements) {
        PVOID Element = GS_BUFFER_ELEMENT(Buffer, Index, ElementSize);

        if(Evaluator(Element, Context) == 0) {
            return Index;
        }
        ++Index;
    }

    return SIZE_MAX;
}

SIZE_T GsBufferSearchWithRetriever(
    _In_ PVOID                      Buffer,
    _In_ SIZE_T                     NumElements,
    _In_ SIZE_T                     Offset,
    _In_ GsBufferEvaluator          Evaluator,
    _In_ PVOID                      EvaluatorContext,
    _In_ GsBufferElementRetriever   Retriever,
    _In_ PVOID                      RetrieverContext
)
{
    SIZE_T Index        = Offset;

    while(Index < NumElements) {
        PVOID Element = Retriever(Buffer, Index, RetrieverContext);
        if(Element == NULL) {
            return SIZE_MAX;
        }

        if(Evaluator(Element, EvaluatorContext) == 0) {
            return Index;
        }
        ++Index;
    }

    return SIZE_MAX;
}

SIZE_T GsBufferReverseSearch(
    _In_ PVOID              Buffer,
    _In_ SIZE_T             BufferSize,
    _In_ SIZE_T             ElementSize,
    _In_ SIZE_T             Offset,
    _In_ GsBufferEvaluator  Evaluator,
    _In_ PVOID              Context     
)
{
    if(BufferSize < (Offset * ElementSize) || ElementSize == 0) {
        return SIZE_MAX;
    }

    SIZE_T NumElements  = (BufferSize / ElementSize);
    INT64 Index         = NumElements - Offset - 1;

    while(Index >= 0) {
        PVOID Element = GS_BUFFER_ELEMENT(Buffer, Index, ElementSize);

        if(Evaluator(Element, Context) == 0) {
            return Index;
        }

        --Index;
    }

    return SIZE_MAX;
}

SIZE_T GsBufferReverseSearchWithRetriever(
    _In_ PVOID                      Buffer,
    _In_ SIZE_T                     NumElements,
    _In_ SIZE_T                     Offset,
    _In_ GsBufferEvaluator          Evaluator,
    _In_ PVOID                      EvaluatorContext,
    _In_ GsBufferElementRetriever   Retriever,
    _In_ PVOID                      RetrieverContext
)
{
    INT64 Index         = NumElements - Offset - 1;

    while(Index >= 0) {
        PVOID Element = Retriever(Buffer, Index, RetrieverContext);

        if(Evaluator(Element, EvaluatorContext) == 0) {
            return Index;
        }

        --Index;
    }

    return SIZE_MAX;
}

SIZE_T GsBufferBinarySearch(
    _In_ PVOID              Buffer,
    _In_ SIZE_T             BufferSize,
    _In_ SIZE_T             ElementSize,
    _In_ GsBufferEvaluator  Evaluator,
    _In_ PVOID              Context  
)
{
    SIZE_T Low      = 0;
    SIZE_T High     = (BufferSize / ElementSize) - 1;
    SIZE_T Middle   = 0;

    while(High >= Low) {
        Middle          = (Low + High) >> 1;
        PVOID Element   = GS_BUFFER_ELEMENT(Buffer, Middle, ElementSize);
        INT Result      = Evaluator(Element, Context);

        if(Result > 0) {
            High = Middle - 1;
        } else if(Result < 0) {
            Low = Middle + 1;
        } else {
            return Middle;
        }
    }

    return SIZE_MAX;
}

SIZE_T GsBufferBinarySearchWithRetriever(
    _In_ PVOID                      Buffer,
    _In_ SIZE_T                     NumElements,
    _In_ GsBufferEvaluator          Evaluator,
    _In_ PVOID                      EvaluatorContext,
    _In_ GsBufferElementRetriever   Retriever,
    _In_ PVOID                      RetrieverContext
)
{
    SIZE_T Low      = 0;
    SIZE_T High     = NumElements - 1;
    SIZE_T Middle   = 0;

    while(High >= Low) {
        Middle          = (Low + High) >> 1;
        PVOID Element   = Retriever(Buffer, Middle, RetrieverContext);
        INT Result      = Evaluator(Element, EvaluatorContext);

        if(Result > 0) {
            High = Middle - 1;
        } else if(Result < 0) {
            Low = Middle + 1;
        } else {
            return Middle;
        }
    }

    return SIZE_MAX;
}

VOID GsBufferSort(
    _In_ PVOID              Buffer,
    _In_ SIZE_T             BufferSize,
    _In_ SIZE_T             ElementSize,
    _In_ GsBufferEvaluator  Comparator
)
{
    SIZE_T NumElements = (BufferSize / ElementSize);
    if(NumElements == 0) {
        return;
    }

    PGS_ARENA Arena = GsArena();
    if(Arena == NULL) {
        return;
    }
    
    PVOID Copy = GsArenaAlloc(Arena, BufferSize);
    if(Copy == NULL) {
        return;
    }

    PVOID A = Buffer;
    PVOID B = Copy;

    for(SIZE_T Width = 1; Width < NumElements; Width = 2 * Width) {
        for(SIZE_T i = 0; i < NumElements; i = i + (2 * Width)) {
            GspMergeSort(A, B, BufferSize, ElementSize, i, min(i + Width, NumElements), min(i + (2 * Width), NumElements), Comparator);
        }
        memcpy(A, B, BufferSize);
    }

    GsArenaRelease(Arena);
}

static VOID GspMergeSort(
    _In_ PVOID              A,
    _In_ PVOID              B,
    _In_ SIZE_T             BufferSize,
    _In_ SIZE_T             ElementSize,
    _In_ SIZE_T             StartIndex,
    _In_ SIZE_T             MiddleIndex,
    _In_ SIZE_T             EndIndex,
    _In_ GsBufferEvaluator  Comparator
)
{
    SIZE_T i = StartIndex;
    SIZE_T j = MiddleIndex;

    for(SIZE_T k = StartIndex; k < EndIndex; k++) {
        PVOID iElement = GS_BUFFER_ELEMENT(A, i, ElementSize);
        PVOID jElement = GS_BUFFER_ELEMENT(A, j, ElementSize);
        INT Comparison = Comparator(iElement, jElement);

        if(i < MiddleIndex && (j >= EndIndex || Comparison < 0)) {
            memcpy(GS_BUFFER_ELEMENT(B, k, ElementSize), GS_BUFFER_ELEMENT(A, i, ElementSize), ElementSize);
            i = i + 1;
        } else {
            memcpy(GS_BUFFER_ELEMENT(B, k, ElementSize), GS_BUFFER_ELEMENT(A, j, ElementSize), ElementSize);
            j = j + 1;
        }
    }
}