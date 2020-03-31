#ifndef APLUGINSDK_DYNARRAY_H
#define APLUGINSDK_DYNARRAY_H

#ifdef __cplusplus
# include <cstdlib>
# include <cstring>
#else
# include <stdlib.h>
# include <string.h>
# ifndef __STDC_VERSION__
#  define __STDC_VERSION__ 0L
# endif
# if __STDC_VERSION__ >= 199901L
#  include <stdbool.h>
# else
    typedef enum { false, true } bool;
# endif
# define nullptr NULL
#endif

#include "macros.h"

static inline size_t calculateCapacityGeneric(size_t requiredSize, size_t minCapacity, size_t maxCapacity, size_t multiplier) {
    size_t multiplierExponent, capacity;
    if(requiredSize <= minCapacity)
        return minCapacity;
    multiplierExponent = ceil(log((double)requiredSize / minCapacity) / log(multiplier));
    capacity = minCapacity * pow(multiplier, multiplierExponent);
    if(capacity > maxCapacity)
        return maxCapacity;
    return capacity;
}
static inline size_t calculateCapacityDefault(size_t requiredSize) {
    return calculateCapacityGeneric(requiredSize, 8, 8388608, 2); // maxCapacity = minCapacity * pow(multiplier, 20)
}

/**
 *
 */
#define DYNAMIC_ARRAY_DEFINITION(name, type) \
    typedef struct name { \
        type* buffer; \
        size_t size; \
        size_t capacity; \
        size_t(*calculateCapacity)(size_t); \
    } name;

/**
 * Constructs a dynamic array of the passed type and initializes it with size 0.
 * The passed dynamic array type must be defined with DYNAMIC_ARRAY_DEFINITION(name, type).
 *
 * @param ArrayType The type of the dynamic array to construct.
 */
#define dynArrayConstruct(ArrayType) ({ \
    ArrayType* dynArray = (ArrayType*) malloc(sizeof(ArrayType));\
    if(dynArray != nullptr) { \
        dynArray->size = 0; \
        dynArray->calculateCapacity = calculateCapacityDefault; \
        dynArray->capacity = dynArray->calculateCapacity(dynArray->size); \
        dynArray->buffer = malloc(dynArray->capacity * sizeof(*dynArray->buffer)); \
        if(dynArray->buffer == nullptr) { \
            free(dynArray); \
            dynArray = nullptr; \
        } \
    } \
    dynArray; \
})

/**
 * Destructs the dynamic array and releases all held resources.
 *
 * @param dynArray The dynamic array to destruct.
 */
#define dynArrayDestruct(dynArray) ({ \
    if((dynArray) != nullptr) { \
        free((dynArray)->buffer); \
        free(dynArray); \
    } \
})
/**
 * @param dynArray The dynamic array to get the size from.
 * @return The size (number of elements) of the passed dynamic array.
 */
#define dynArraySize(dynArray) ({ \
    (dynArray) == nullptr ? 0 : (dynArray)->size; \
})

/**
 * Resize dynArray, that it can hold at least reserveSize count items without resizing.
 * If the passed reserveSize is smaller than the current size or the current capacity (number of items that the array
 * can hold without resizing) of dynArray, nothing happens.
 *
 * If reserveSize <= dynamicArrayCapacityMax, reserveSize is treated if it is dynamicArrayCapacityMax and
 * if reserveSize > dynamicArrayCapacityMax, reserveSize is treated if it is dynamicArrayCapacityMax.
 *
 * If reallocation of memory fails, the dynamic array is not modified.
 *
 * @param dynArray The dynamic array to resize the buffer if necessary.
 * @param reserveSize The number of items that dynArray should be able to hold without resizing.
 *
 * @return True if dynArray can hold at least reserveSize count elements after this operation, false if not.
 */
#define dynArrayReserve(dynArray, reserveSize) ({ \
    bool dynArrayReserveReturnValue = false; \
    if((dynArray) != nullptr) { \
        if((dynArray)->capacity < (reserveSize) || (dynArray)->buffer == nullptr) { \
            if((dynArray)->calculateCapacity != nullptr) { \
                void* dynArrayReserveTmpBuffer; \
                size_t dynArrayReserveAimedCapacity = (dynArray)->calculateCapacity(reserveSize); \
                if(dynArrayReserveAimedCapacity >= reserveSize) { \
                    dynArrayReserveTmpBuffer = realloc((dynArray)->buffer, dynArrayReserveAimedCapacity * sizeof(*((dynArray)->buffer))); \
                    dynArrayReserveReturnValue = (dynArrayReserveTmpBuffer != nullptr); \
                    if(dynArrayReserveReturnValue) { \
                        (dynArray)->capacity = dynArrayReserveAimedCapacity; \
                        (dynArray)->buffer = dynArrayReserveTmpBuffer; \
                    } \
                } \
            } \
        } else { \
            dynArrayReserveReturnValue = true; \
        } \
    } \
    dynArrayReserveReturnValue; \
})
/**
 * Resize dynArray to the minimum size to fit its content (dependent on the resize strategy, which means that the
 * buffer is maybe not exactly as big as the size).
 *
 * @param dynArray The dynamic array to shrink the capacity to fit its content.
 *
 * @return True if dynArray is small as possible or was successfully resized, false if not.
 */
#define dynArrayShrinkToFit(dynArray) ({ \
    bool dynArrayShrinkToFitReturnValue = false; \
    if((dynArray) != nullptr && (dynArray)->calculateCapacity != nullptr) { \
        if((dynArray)->capacity > (dynArray)->calculateCapacity((dynArray)->size) || (dynArray)->buffer == nullptr) { \
            size_t dynArrayShrinkToFitCapacityBackup = (dynArray)->capacity; \
            (dynArray)->capacity = ((dynArray)->size == 0) ? 0 : (dynArray)->size - 1; \
            dynArrayShrinkToFitReturnValue = dynArrayReserve(dynArray, (dynArray)->capacity + 1); \
            if(!dynArrayShrinkToFitReturnValue) \
                (dynArray)->capacity = dynArrayShrinkToFitCapacityBackup; \
        } else { \
            dynArrayShrinkToFitReturnValue = true; \
        } \
    } \
    dynArrayShrinkToFitReturnValue; \
})
/**
 * Clears the content of dynArray and calls dynArrayShrinkToFit(dynArray) after that.
 *
 * @param dynArray The dynamic array to clear.
 *
 * @return True if dynArray was successfully cleared and shrinked to the minimum possible size, false if only cleared
 * but not shrinked.
 */
#define dynArrayClear(dynArray) ({ \
    bool dynArrayClearReturnValue = false; \
    if((dynArray) != nullptr) { \
        (dynArray)->size = 0; \
        dynArrayClearReturnValue = dynArrayShrinkToFit(dynArray); \
    } \
    dynArrayClearReturnValue; \
})

/**
 * Inserts the value into dynArray at index. If index is bigger or equal to the size of
 * dynArray, the value gets appended.
 * Shifts all elements which were previous at or behind index, one index, to the right.
 *
 * @param dynArray The dynamic array in which the value should be inserted.
 * @param index The index at which the value should be inserted.
 * @param value The value to insert.
 *
 * @return True if the value was inserted successfully, false if not.
 */
#define dynArrayInsert(dynArray, index, value) ({ \
    bool dynArrayInsertReturnValue = false; \
    size_t dynArrayInsertIndex = (index); \
    if((index) >= 0 && dynArrayReserve(dynArray, (dynArray)->size + 1)) { \
        if((index) < (dynArray)->size) \
            memmove((dynArray)->buffer + (index) + 1, (dynArray)->buffer + (index), (dynArray)->size - (index)); \
        else \
            dynArrayInsertIndex = (dynArray)->size; \
        (dynArray)->buffer[dynArrayInsertIndex] = (value); \
        (dynArray)->size += 1; \
        dynArrayInsertReturnValue = true; \
    } \
    dynArrayInsertReturnValue; \
})
/**
 * Inserts the elements of array into dynArray at index. If index is bigger or equal to the size of
 * dynArray, the elements get appended.
 * Shifts all elements which were previous at or behind index, arraySize count indices. to the right.
 *
 * The elements gets copied from array to the buffer of dynArray with memcpy.
 *
 * @param dynArray The dynamic array in which the values should be inserted.
 * @param index The index at which the values should be inserted.
 * @param array A pointer to the array from which the elements should be copied.
 * @param arraySize The count of elements which should be copied from array to dynArray.
 *
 * @return True if the values were added successfully, false if not.
 */
#define dynArrayInsertArray(dynArray, index, array, arraySize) ({ \
    bool dynArrayInsertArrayReturnValue = false; \
    size_t dynArrayInsertArrayIndex = (index); \
    if((dynArray) != nullptr && (index) >= 0) { \
        if((array) != nullptr && (arraySize) > 0) { \
            if(dynArrayReserve(dynArray, (dynArray)->size + (arraySize))) { \
                if((index) < (dynArray)->size) \
                    memmove((dynArray)->buffer + (index) + (arraySize), (dynArray)->buffer + (index), (dynArray)->size - (index)); \
                else \
                    dynArrayInsertArrayIndex = (dynArray)->size; \
                memcpy((dynArray)->buffer + dynArrayInsertArrayIndex, (array), (arraySize)); \
                (dynArray)->size += (arraySize); \
                dynArrayInsertArrayReturnValue = true; \
            } \
        } else { \
            dynArrayInsertArrayReturnValue = true; \
        } \
    }\
    dynArrayInsertArrayReturnValue; \
})
/**
 * Inserts the elements of srcDynArray into destDynArray at index. If index is bigger or equal to the size of
 * destDynArray, the elements get appended.
 * Shifts all elements which were previous at or behind index, the size of srcDynArray indices, to the right.
 *
 * The elements gets copied from the buffer of srcDynArray to the buffer of destDynArray with memcpy.
 *
 * @param destDynArray The dynamic array in which the values should be inserted.
 * @param index The index at which the values should be inserted.
 * @param srcDynArray The dynamic array from which the values should be copied.
 *
 * @return True if the values were inserted successfully, false if not.
 */
#define dynArrayInsertDynArray(destDynArray, index, srcDynArray) ({ \
    (srcDynArray == nullptr) ? destDynArray != nullptr : dynArrayInsertArray(destDynArray, index, (srcDynArray)->buffer, (srcDynArray)->size); \
})

/**
 * Adds the value to the end of dynArray.
 *
 * @param dynArray The dynamic array to which the value should be added.
 * @param value The value to add.
 *
 * @return True if the value was added successfully, false if not.
 */
#define dynArrayAdd(dynArray, value) ({ \
    dynArrayInsert(dynArray, ((size_t)-1), value); \
})
/**
 * Adds the elements of array to the end of dynArray.
 *
 * The elements gets copied from array to the buffer of dynArray with memcpy.
 *
 * @param dynArray The dynamic array to which the values should be added.
 * @param array A pointer to the array from which the elements should be copied.
 * @param arraySize The count of elements which should be copied from array to dynArray.
 *
 * @return True if the values were added successfully, false if not.
 */
#define dynArrayAddArray(dynArray, array, arraySize) ({ \
    dynArrayInsertArray(dynArray, ((size_t)-1), array, arraySize); \
})
/**
 * Adds the elements of srcDynArray to the end of destDynArray.
 *
 * The elements gets copied from the buffer of srcDynArray to the buffer of destDynArray with memcpy.
 *
 * @param destDynArray The dynamic array to which the values should be added.
 * @param srcDynArray The dynamic array from which the values should be copied.
 *
 * @return True if the values were added successfully, false if not.
 */
#define dynArrayAddDynArray(destDynArray, srcDynArray) ({ \
    dynArrayInsertDynArray(destDynArray, ((size_t)-1), srcDynArray); \
})

/**
 * Sets the element in dynArray at index to value. If index is bigger than the size of dynArray
 * The value is appended to dynArray as new element.
 *
 * @param dynArray The dynamic array to set the index of.
 * @param index The index at which the element should be set to value.
 * @param value The value that should be set at index.
 *
 * @return True if the value was set, false if not.
 */
#define dynArraySet(dynArray, index, value) ({ \
    bool dynArraySetReturnValue = false; \
    if((dynArray) != nullptr) { \
        if((index) < (dynArray)->size) { \
            (dynArray)->buffer[(size_t)index] = value; \
            dynArraySetReturnValue = true; \
        } else if((index) >= (dynArray)->size) { \
            dynArraySetReturnValue = dynArrayAdd(dynArray, value); \
        } \
    } \
    dynArraySetReturnValue; \
})

/**
 * Removes count elements in dynArray starting at index. This operation doesn't affect the capacity.
 * The elements behind index + count are shifted to the right by count.
 *
 * If index + count is bigger or equal to the size of dynArray, all elements behind index gets removed.
 *
 * @param dynArray The dynamic array to remove elements from.
 * @param index The start index from which count elements should be removed.
 * @param count The number of elements to remove starting at index.
 */
#define dynArrayRemove(dynArray, index, count) ({ \
    if(dynArray != nullptr && count > 0 && index >= 0 && index < (dynArray)->size) { \
        if(((size_t)(index)) + (count) <= (dynArray)->size) { \
            memmove((dynArray)->buffer + (index), (dynArray)->buffer + (index) + (count), (dynArray)->size - (index) - (count)); \
            (dynArray)->size -= (count); \
        } else { \
            (dynArray)->size = (index); \
        } \
    } \
})

/**
 * Retrieves the element at index in dynArray.
 *
 * No Boundchecking is performed. If the index is out of bounds, the ends in undefined behavior!
 *
 * @param dynArray The dynamic array to get the element from.
 * @param index The index in dynArray to get the element of.
 *
 * @return The element in dynArray at index.
 */
#define dynArrayGet(dynArray, index) ({ \
    (dynArray)->buffer[index]; \
})

#endif //APLUGINSDK_DYNARRAY_H
