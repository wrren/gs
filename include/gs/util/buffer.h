#ifndef GS_BUFFER_H
#define GS_BUFFER_H

#include <gs/core/platform.h>

typedef INT (*GsBufferEvaluator)(
    _In_ PVOID Element, 
    _In_ PVOID Context
);

typedef PVOID (*GsBufferElementRetriever)(
    _In_ PVOID  Buffer,
    _In_ SIZE_T Index,
    _In_ PVOID  Context
);

/**
 * @brief Search the given buffer linearly for the first element for which the given comparator
 * returns TRUE. The returned index is expressed in terms of the element size rather than
 * bytes from the beginning of the buffer.
 * 
 * @param Buffer        Buffer to be searched
 * @param BufferSize    Size of the buffer in bytes
 * @param ElementSize   Size of a single element in bytes
 * @param Offset        Offset, in terms of numbers of elements, where searching should begin
 * @param Evaluator     Evaluator used to determine whether an element matches criteria
 * @param Context       Context passed to the comparator
 * @return SIZE_T       Index of the first matching element or SIZE_MAX if not match was found
 */
SIZE_T GsBufferSearch(
    _In_ PVOID              Buffer,
    _In_ SIZE_T             BufferSize,
    _In_ SIZE_T             ElementSize,
    _In_ SIZE_T             Offset,
    _In_ GsBufferEvaluator  Evaluator,
    _In_ PVOID              Context     
);

/**
 * @brief Search the given buffer linearly for the first element for which the given comparator
 * returns TRUE. The returned index is expressed in terms of the element size rather than
 * bytes from the beginning of the buffer. This version of the search function does not treat the
 * buffer as a linear memory buffer, but rather as a more complex structure from which elements
 * can be retrieved using the given retriever function.
 * 
 * @param Buffer            Buffer to be searched
 * @param NumElements       Number of elements in the buffer
 * @param Offset            Offset, in terms of numbers of elements, where searching should begin
 * @param Evaluator         Evaluator used to determine whether an element matches criteria
 * @param EvaluatorContext  Context passed to the evaluator
 * @param Retriever         Function used to retrieve an element from the buffer
 * @param RetrieverContext  Context passed to the retriever
 * @return SIZE_T           Index of the first matching element or SIZE_MAX if not match was found
 */
SIZE_T GsBufferSearchWithRetriever(
    _In_ PVOID                      Buffer,
    _In_ SIZE_T                     NumElements,
    _In_ SIZE_T                     Offset,
    _In_ GsBufferEvaluator          Evaluator,
    _In_ PVOID                      EvaluatorContext,
    _In_ GsBufferElementRetriever   Retriever,
    _In_ PVOID                      RetrieverContext
);

/**
 * @brief Search the given buffer linearly in reverse order for the first element for which the given comparator
 * returns TRUE. The returned index is expressed in terms of the element size rather than
 * bytes from the beginning of the buffer.
 * 
 * @param Buffer        Buffer to be searched
 * @param BufferSize    Size of the buffer in bytes
 * @param ElementSize   Size of a single element in bytes
 * @param Offset        Offset, in terms of numbers of elements, from the end of the buffer where searching should begin
 * @param Evaluator     Evaluator used to determine whether an element matches criteria
 * @param Context       Context passed to the comparator
 * @return SIZE_T       Index of the first matching element or SIZE_MAX if not match was found
 */
SIZE_T GsBufferReverseSearch(
    _In_ PVOID              Buffer,
    _In_ SIZE_T             BufferSize,
    _In_ SIZE_T             ElementSize,
    _In_ SIZE_T             Offset,
    _In_ GsBufferEvaluator  Evaluator,
    _In_ PVOID              Context 
);

/**
 * @brief Search the given buffer in reverse order for the first element for which the given comparator
 * returns TRUE. The returned index is expressed in terms of the element size rather than
 * bytes from the beginning of the buffer. This version of the search function does not treat the
 * buffer as a linear memory buffer, but rather as a more complex structure from which elements
 * can be retrieved using the given retriever function.
 * 
 * @param Buffer            Buffer to be searched
 * @param NumElements       Number of elements in the buffer
 * @param Offset            Offset, in terms of numbers of elements, from the end of the buffer where searching should begin
 * @param Evaluator         Evaluator used to determine whether an element matches criteria
 * @param EvaluatorContext  Context passed to the evaluator
 * @param Retriever         Function used to retrieve an element from the buffer
 * @param RetrieverContext  Context passed to the retriever
 * @return SIZE_T           Index of the first matching element or SIZE_MAX if not match was found
 */
SIZE_T GsBufferReverseSearchWithRetriever(
    _In_ PVOID                      Buffer,
    _In_ SIZE_T                     NumElements,
    _In_ SIZE_T                     Offset,
    _In_ GsBufferEvaluator          Evaluator,
    _In_ PVOID                      EvaluatorContext,
    _In_ GsBufferElementRetriever   Retriever,
    _In_ PVOID                      RetrieverContext
);

/**
 * @brief Search the given sorted buffer using binary search for the element for which the given comparator
 * returns TRUE. The returned index is expressed in terms of the element size rather than
 * bytes from the beginning of the buffer.
 * 
 * @param Buffer        Buffer to be searched
 * @param BufferSize    Size of the buffer in bytes
 * @param ElementSize   Size of a single element in bytes
 * @param NumElements   Number of elements in the buffer
 * @param Evaluator     Evaluator used to determine whether an element matches criteria
 * @param Context       Context passed to the comparator
 * @return SIZE_T       Index of the matching element or SIZE_MAX if not match was found
 */
SIZE_T GsBufferBinarySearch(
    _In_ PVOID              Buffer,
    _In_ SIZE_T             BufferSize,
    _In_ SIZE_T             ElementSize,
    _In_ GsBufferEvaluator  Evaluator,
    _In_ PVOID              Context  
);

/**
 * @brief Search the given sorted buffer using binary search for the element for which the given comparator
 * returns TRUE. The returned index is expressed in terms of the element size rather than
 * bytes from the beginning of the buffer. This version of the search function does not treat the
 * buffer as a linear memory buffer, but rather as a more complex structure from which elements
 * can be retrieved using the given retriever function.
 * 
 * @param Buffer            Buffer to be searched
 * @param NumElements       Number of elements in the buffer
 * @param Evaluator         Evaluator used to determine whether an element matches criteria
 * @param EvaluatorContext  Context passed to the evaluator
 * @param Retriever         Function used to retrieve an element from the buffer
 * @param RetrieverContext  Context passed to the retriever
 * @return SIZE_T           Index of the first matching element or SIZE_MAX if not match was found
 */
SIZE_T GsBufferBinarySearchWithRetriever(
    _In_ PVOID                      Buffer,
    _In_ SIZE_T                     NumElements,
    _In_ GsBufferEvaluator          Evaluator,
    _In_ PVOID                      EvaluatorContext,
    _In_ GsBufferElementRetriever   Retriever,
    _In_ PVOID                      RetrieverContext
);

/**
 * @brief Perform in-place sorting of a buffer.
 * 
 * @param Buffer        Buffer to be sorted
 * @param BufferSize    Size, in bytes, of the buffer
 * @param ElementSize   Size of an element in bytes
 * @param Comparator    Function used to compare two elements
 */
VOID GsBufferSort(
    _In_ PVOID              Buffer,
    _In_ SIZE_T             BufferSize,
    _In_ SIZE_T             ElementSize,
    _In_ GsBufferEvaluator  Comparator
);

#endif // GS_BUFFER_H