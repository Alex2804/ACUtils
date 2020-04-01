#include <string.h>

#include "check.h"

#include "../../dynarray.h"

static size_t aDynArrayTestMallocCount = 0;
static bool aDynArrayTestMallocFail = false;
static size_t aDynArrayTestReallocCount = 0;
static bool aDynArrayTestReallocFail = false;
static size_t aDynArrayTestFreeCount = 0;

static void* aDynArrayTestMalloc(size_t size) {
    if(!aDynArrayTestMallocFail) {
        void* tmp = malloc(size);
        if(tmp != NULL)
            ++aDynArrayTestMallocCount;
        return tmp;
    } else {
        return NULL;
    }
}
static void* aDynArrayTestRealloc(void* ptr, size_t size) {
    if(!aDynArrayTestReallocFail) {
        void* tmp = realloc(ptr, size);
        if(tmp != NULL)
            ++aDynArrayTestReallocCount;
        return tmp;
    } else {
        return NULL;
    }
}
static void aDynArrayTestFree(void* ptr) {
    ++aDynArrayTestFreeCount;
    free(ptr);
}

#define malloc(size) aDynArrayTestMalloc(size)
#define realloc(ptr, size) aDynArrayTestRealloc(ptr, size)
#define free(ptr) aDynArrayTestFree(ptr)

static const size_t aDynArrayTestCapacityMin = 8;
static const size_t aDynArrayTestCapacityMax = 32;
static const size_t aDynArrayTestCapacityMul = 2;

static inline size_t calculateCapacityTest(size_t requiredSize) {
    return calculateCapacityGeneric(requiredSize, aDynArrayTestCapacityMin, aDynArrayTestCapacityMax, aDynArrayTestCapacityMul);
}

A_DYNAMIC_ARRAY_DEFINITION(DynStringTestArray, char)

START_TEST(test_aDynArrayConstruct_aDynArrayDestruct_valid)
{
    DynStringTestArray* array;
    aDynArrayTestMallocFail = false;
    aDynArrayTestMallocCount = aDynArrayTestFreeCount = 0;
    array = aDynArrayConstruct(DynStringTestArray);
    ck_assert_uint_eq(array->size, 0);
    ck_assert_uint_eq(array->capacity, aDynArrayTestCapacityMin);
    ck_assert_ptr_nonnull(array->buffer);
    ck_assert_ptr_nonnull(array->calculateCapacity);
    aDynArrayDestruct(array);
    ck_assert_uint_eq(aDynArrayTestMallocCount, aDynArrayTestFreeCount);
}
END_TEST
START_TEST(test_aDynArrayConstruct_aDynArrayDestruct_noMemoryAvailable)
{
    DynStringTestArray* array;
    aDynArrayTestMallocFail = true;
    aDynArrayTestMallocCount = aDynArrayTestFreeCount = 0;
    array = aDynArrayConstruct(DynStringTestArray);
    ck_assert_ptr_null(array);
    aDynArrayDestruct(array); // should do nothing
    ck_assert_uint_eq(aDynArrayTestMallocCount, 0);
    ck_assert_uint_eq(aDynArrayTestFreeCount, 0);
}
END_TEST
START_TEST(test_aDynArrayConstruct_aDynArrayDestruct_nullptr)
{
    DynStringTestArray* array = NULL;
    aDynArrayTestFreeCount = 0;
    aDynArrayDestruct(array); // should do nothing
    ck_assert_uint_eq(aDynArrayTestFreeCount, 0);
}
END_TEST


START_TEST(test_aDynArraySize_valid)
{
    DynStringTestArray array;
    array.capacity = 0;
    array.buffer = NULL;
    array.calculateCapacity = NULL;
    array.size = 42;
    ck_assert_uint_eq(aDynArraySize(&array), 42);
    array.size = 13;
    ck_assert_uint_eq(aDynArraySize(&array), 13);
    array.size = 0;
    ck_assert_uint_eq(aDynArraySize(&array), 0);
}
START_TEST(test_aDynArraySize_nullptr)
{
    DynStringTestArray* arrayPtr = NULL;
    ck_assert_uint_eq(aDynArraySize(arrayPtr), 0);
}


START_TEST(test_aDynArrayReserve_success_enoughCapacityBufferNotNull)
{
    DynStringTestArray array;
    array.size = 0;
    array.calculateCapacity = NULL;
    array.capacity = aDynArrayTestCapacityMin;
    aDynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    aDynArrayTestReallocFail = true;
    ck_assert_uint_eq(aDynArrayReserve(&array, array.capacity), true);
    ck_assert_uint_eq(array.size, 0);
    ck_assert_uint_eq(array.capacity, aDynArrayTestCapacityMin);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_ptr_null(array.calculateCapacity);
    free(array.buffer);
}
START_TEST(test_aDynArrayReserve_success_enoughCapacityBufferNull)
{
    DynStringTestArray array;
    array.size = 0;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = aDynArrayTestCapacityMin;
    array.buffer = NULL;
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayReserve(&array, array.capacity), true);
    ck_assert_uint_eq(array.size, 0);
    ck_assert_uint_eq(array.capacity, aDynArrayTestCapacityMin);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 1);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayReserve_success_notEnoughCapacity)
{
    DynStringTestArray array;
    array.size = 0;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = aDynArrayTestCapacityMin;
    aDynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayReserve(&array, aDynArrayTestCapacityMax), true);
    ck_assert_uint_eq(array.size, 0);
    ck_assert_uint_eq(array.capacity, aDynArrayTestCapacityMax);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 1);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayReserve_failure_biggerThanMaxCapacity)
{
    DynStringTestArray array;
    array.size = 0;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = aDynArrayTestCapacityMin;
    array.buffer = NULL;
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayReserve(&array, aDynArrayTestCapacityMax + 1), false);
    ck_assert_uint_eq(array.size, 0);
    ck_assert_uint_eq(array.capacity, aDynArrayTestCapacityMin);
    ck_assert_ptr_null(array.buffer);
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
}
END_TEST
START_TEST(test_aDynArrayReserve_failure_noMemoryAvailable)
{
    DynStringTestArray array;
    array.size = 0;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = aDynArrayTestCapacityMin;
    array.buffer = NULL;
    aDynArrayTestReallocFail = true;
    ck_assert_uint_eq(aDynArrayReserve(&array, aDynArrayTestCapacityMax), false);
    ck_assert_uint_eq(array.size, 0);
    ck_assert_uint_eq(array.capacity, aDynArrayTestCapacityMin);
    ck_assert_ptr_null(array.buffer);
    ck_assert_ptr_nonnull(array.calculateCapacity);
}
END_TEST
START_TEST(test_aDynArrayReserve_failure_calculateCapacityNull)
{
    DynStringTestArray array;
    array.size = 0;
    array.calculateCapacity = NULL;
    array.capacity = aDynArrayTestCapacityMin;
    array.buffer = NULL;
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayReserve(&array, aDynArrayTestCapacityMax), false);
    ck_assert_uint_eq(array.size, 0);
    ck_assert_uint_eq(array.capacity, aDynArrayTestCapacityMin);
    ck_assert_ptr_null(array.buffer);
    ck_assert_ptr_null(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
}
END_TEST
START_TEST(test_aDynArrayReserve_failure_nullptr)
{
    DynStringTestArray* arrayPtr = NULL;
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayReserve(arrayPtr, aDynArrayTestCapacityMax), false);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
}
END_TEST


START_TEST(test_aDynArrayShrinkToFit_success_hasLeastCapacityBufferNotNull)
{
    DynStringTestArray array;
    array.size = aDynArrayTestCapacityMin - 1;
    array.capacity = aDynArrayTestCapacityMin;
    aDynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    array.calculateCapacity = calculateCapacityTest;
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayShrinkToFit(&array), true);
    ck_assert_uint_eq(array.size, aDynArrayTestCapacityMin - 1);
    ck_assert_uint_eq(array.capacity, aDynArrayTestCapacityMin);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(array.buffer);
}
START_TEST(test_aDynArrayShrinkToFit_success_hasLeastCapacityBufferNull)
{
    DynStringTestArray array;
    array.size = aDynArrayTestCapacityMin - 1;
    array.capacity = aDynArrayTestCapacityMin;
    array.buffer = NULL;
    array.calculateCapacity = calculateCapacityTest;
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayShrinkToFit(&array), true);
    ck_assert_uint_eq(array.size, aDynArrayTestCapacityMin - 1);
    ck_assert_uint_eq(array.capacity, aDynArrayTestCapacityMin);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 1);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayShrinkToFit_success_hasNotLeastCapacity)
{
    DynStringTestArray array;
    array.size = aDynArrayTestCapacityMin - 1;
    array.capacity = calculateCapacityTest(aDynArrayTestCapacityMin + 1);
    aDynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    array.calculateCapacity = calculateCapacityTest;
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayShrinkToFit(&array), true);
    ck_assert_uint_eq(array.size, aDynArrayTestCapacityMin - 1);
    ck_assert_uint_eq(array.capacity, aDynArrayTestCapacityMin);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 1);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayShrinkToFit_failure_noMemoryAvailable)
{
    DynStringTestArray array;
    array.size = aDynArrayTestCapacityMin - 1;
    array.capacity = calculateCapacityTest(aDynArrayTestCapacityMin + 1);
    aDynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    array.calculateCapacity = calculateCapacityTest;
    aDynArrayTestReallocFail = true;
    ck_assert_uint_eq(aDynArrayShrinkToFit(&array), false);
    ck_assert_uint_eq(array.size, aDynArrayTestCapacityMin - 1);
    ck_assert_uint_eq(array.capacity, calculateCapacityTest(aDynArrayTestCapacityMin + 1));
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_ptr_nonnull(array.calculateCapacity);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayShrinkToFit_failure_calculateCapacityNull)
{
    DynStringTestArray array;
    array.size = aDynArrayTestCapacityMin - 1;
    array.capacity = calculateCapacityTest(aDynArrayTestCapacityMin + 1);
    aDynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    array.calculateCapacity = NULL;
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayShrinkToFit(&array), false);
    ck_assert_uint_eq(array.size, aDynArrayTestCapacityMin - 1);
    ck_assert_uint_eq(array.capacity, calculateCapacityTest(aDynArrayTestCapacityMin + 1));
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_ptr_null(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayShrinkToFit_failure_nullptr)
{
    DynStringTestArray* arrayPtr = NULL;
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayShrinkToFit(arrayPtr), false);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
}
END_TEST


START_TEST(test_aDynArrayClear_success_shrinked)
{
    DynStringTestArray array;
    array.size = aDynArrayTestCapacityMin + 1;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = aDynArrayTestCapacityMin * aDynArrayTestCapacityMul;
    aDynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayClear(&array), true);
    ck_assert_uint_eq(array.size, 0);
    ck_assert_uint_eq(array.capacity, aDynArrayTestCapacityMin);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 1);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayClear_failure_notShrinked)
{
    DynStringTestArray array;
    array.size = aDynArrayTestCapacityMin + 1;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = aDynArrayTestCapacityMin * aDynArrayTestCapacityMul;
    aDynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    aDynArrayTestReallocFail = true;
    ck_assert_uint_eq(aDynArrayClear(&array), false);
    ck_assert_uint_eq(array.size, 0);
    ck_assert_uint_eq(array.capacity, aDynArrayTestCapacityMin * aDynArrayTestCapacityMul);
    ck_assert_ptr_nonnull(array.buffer);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayClear_failure_nullptr)
{
    DynStringTestArray* arrayPtr = NULL;
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayClear(arrayPtr), false);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
}
END_TEST


START_TEST(test_aDynArrayInsert_success_zeroIndex)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 8;
    aDynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "1234", 5);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayInsert(&array, 0, '0'), true);
    ck_assert_uint_eq(array.size, 6);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_int_eq(array.buffer[0], '0');
    ck_assert_str_eq(array.buffer, "01234");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayInsert_success_middleIndex)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 8;
    aDynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "0134", 5);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayInsert(&array, 2, '2'), true);
    ck_assert_uint_eq(array.size, 6);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_int_eq(array.buffer[2], '2');
    ck_assert_str_eq(array.buffer, "01234");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayInsert_success_endIndex)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 8;
    aDynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "01234", 5);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayInsert(&array, 5, '\0'), true);
    ck_assert_uint_eq(array.size, 6);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_int_eq(array.buffer[5], '\0');
    ck_assert_str_eq(array.buffer, "01234");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayInsert_success_beyondEndIndex)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 8;
    aDynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "01234", 5);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayInsert(&array, 666, '\0'), true);
    ck_assert_uint_eq(array.size, 6);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_int_eq(array.buffer[5], '\0');
    ck_assert_str_eq(array.buffer, "01234");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayInsert_success_bufferExpanded)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 5;
    aDynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "0134", 5);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayInsert(&array, 2, '2'), true);
    ck_assert_uint_eq(array.size, 6);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_int_eq(array.buffer[2], '2');
    ck_assert_str_eq(array.buffer, "01234");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 1);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayInsert_failure_negativeIndex)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 8;
    aDynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "0123", 5);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayInsert(&array, -1, '-'), false);
    ck_assert_uint_eq(array.size, 5);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayInsert_failure_bufferExpansionFailed)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 5;
    aDynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "0134", 5);
    aDynArrayTestReallocFail = true;
    ck_assert_uint_eq(aDynArrayInsert(&array, 2, '2'), false);
    ck_assert_uint_eq(array.size, 5);
    ck_assert_uint_eq(array.capacity, 5);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0134");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayInsert_failure_nullptr)
{
    DynStringTestArray* arrayPtr = NULL;
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayInsert(arrayPtr, 0, '0'), false);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
}
END_TEST


START_TEST(test_aDynArrayInsertArray_success_zeroIndex)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 8;
    aDynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "3456", 5);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayInsertArray(&array, 0, "012", 3), true);
    ck_assert_uint_eq(array.size, 8);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123456");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayInsertArray_success_middleIndex)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 8;
    aDynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "0156", 5);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayInsertArray(&array, 2, "234", 3), true);
    ck_assert_uint_eq(array.size, 8);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123456");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayInsertArray_success_endIndex)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 8;
    aDynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "01234", 5);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayInsertArray(&array, 5, "56\0", 3), true);
    ck_assert_uint_eq(array.size, 8);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123456");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayInsertArray_success_beyondEndIndex)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 8;
    aDynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "01234", 5);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayInsertArray(&array, 666, "56\0", 3), true);
    ck_assert_uint_eq(array.size, 8);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123456");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayInsertArray_success_bufferExpanded)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 5;
    aDynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "0156", 5);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayInsertArray(&array, 2, "234", 3), true);
    ck_assert_uint_eq(array.size, 8);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123456");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 1);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayInsertArray_success_nullptrArray)
{
    DynStringTestArray array;
    char* nullptrArray = NULL;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 5;
    aDynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "0156", 5);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayInsertArray(&array, 2, nullptrArray, 3), true);
    ck_assert_uint_eq(array.size, 5);
    ck_assert_uint_eq(array.capacity, 5);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0156");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayInsertArray_success_zeroArraySize)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 5;
    aDynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "0156", 5);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayInsertArray(&array, 2, "234", 0), true);
    ck_assert_uint_eq(array.size, 5);
    ck_assert_uint_eq(array.capacity, 5);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0156");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayInsertArray_failure_negativeIndex)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 8;
    aDynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "0123", 5);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayInsertArray(&array, -1, "---", 3), false);
    ck_assert_uint_eq(array.size, 5);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayInsertArray_failure_bufferExpansionFailed)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 5;
    aDynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "0156", 5);
    aDynArrayTestReallocFail = true;
    ck_assert_uint_eq(aDynArrayInsertArray(&array, 2, "234", 3), false);
    ck_assert_uint_eq(array.size, 5);
    ck_assert_uint_eq(array.capacity, 5);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0156");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayInsertArray_failure_nullptrDestArray)
{
    DynStringTestArray* arrayPtr = NULL;
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayInsertArray(arrayPtr, 0, "012", 3), false);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
}
END_TEST


START_TEST(test_aDynArrayInsertDynArray_success_zeroIndex)
{
    DynStringTestArray destArray, srcArray;
    destArray.size = 5;
    destArray.calculateCapacity = calculateCapacityTest;
    destArray.capacity = 8;
    aDynArrayTestMallocFail = false;
    destArray.buffer = malloc(destArray.capacity);
    memcpy(destArray.buffer, "3456", 5);
    srcArray.size = 3;
    srcArray.calculateCapacity = calculateCapacityTest;
    srcArray.capacity = 8;
    srcArray.buffer = malloc(srcArray.capacity);
    memcpy(srcArray.buffer, "012\0", 4);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayInsertDynArray(&destArray, 0, &srcArray), true);
    ck_assert_uint_eq(destArray.size, 8);
    ck_assert_uint_eq(destArray.capacity, 8);
    ck_assert_ptr_nonnull(destArray.buffer);
    ck_assert_str_eq(destArray.buffer, "0123456");
    ck_assert_ptr_nonnull(destArray.calculateCapacity);
    ck_assert_uint_eq(srcArray.size, 3);
    ck_assert_uint_eq(srcArray.capacity, 8);
    ck_assert_ptr_nonnull(srcArray.buffer);
    ck_assert_str_eq(srcArray.buffer, "012");
    ck_assert_ptr_nonnull(srcArray.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(destArray.buffer);
    free(srcArray.buffer);
}
END_TEST
START_TEST(test_aDynArrayInsertDynArray_success_middleIndex)
{
    DynStringTestArray destArray, srcArray;
    destArray.size = 5;
    destArray.calculateCapacity = calculateCapacityTest;
    destArray.capacity = 8;
    aDynArrayTestMallocFail = false;
    destArray.buffer = malloc(destArray.capacity);
    memcpy(destArray.buffer, "0156", 5);
    srcArray.size = 3;
    srcArray.calculateCapacity = calculateCapacityTest;
    srcArray.capacity = 8;
    srcArray.buffer = malloc(srcArray.capacity);
    memcpy(srcArray.buffer, "234\0", 4);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayInsertDynArray(&destArray, 2, &srcArray), true);
    ck_assert_uint_eq(destArray.size, 8);
    ck_assert_uint_eq(destArray.capacity, 8);
    ck_assert_ptr_nonnull(destArray.buffer);
    ck_assert_str_eq(destArray.buffer, "0123456");
    ck_assert_ptr_nonnull(destArray.calculateCapacity);
    ck_assert_uint_eq(srcArray.size, 3);
    ck_assert_uint_eq(srcArray.capacity, 8);
    ck_assert_ptr_nonnull(srcArray.buffer);
    ck_assert_str_eq(srcArray.buffer, "234");
    ck_assert_ptr_nonnull(srcArray.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(destArray.buffer);
    free(srcArray.buffer);
}
END_TEST
START_TEST(test_aDynArrayInsertDynArray_success_endIndex)
{
    DynStringTestArray destArray, srcArray;
    destArray.size = 5;
    destArray.calculateCapacity = calculateCapacityTest;
    destArray.capacity = 8;
    aDynArrayTestMallocFail = false;
    destArray.buffer = malloc(destArray.capacity);
    memcpy(destArray.buffer, "01234", 5);
    srcArray.size = 3;
    srcArray.calculateCapacity = calculateCapacityTest;
    srcArray.capacity = 8;
    srcArray.buffer = malloc(srcArray.capacity);
    memcpy(srcArray.buffer, "56\0", 4);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayInsertDynArray(&destArray, 5, &srcArray), true);
    ck_assert_uint_eq(destArray.size, 8);
    ck_assert_uint_eq(destArray.capacity, 8);
    ck_assert_ptr_nonnull(destArray.buffer);
    ck_assert_str_eq(destArray.buffer, "0123456");
    ck_assert_ptr_nonnull(destArray.calculateCapacity);
    ck_assert_uint_eq(srcArray.size, 3);
    ck_assert_uint_eq(srcArray.capacity, 8);
    ck_assert_ptr_nonnull(srcArray.buffer);
    ck_assert_str_eq(srcArray.buffer, "56");
    ck_assert_ptr_nonnull(srcArray.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(destArray.buffer);
    free(srcArray.buffer);
}
END_TEST
START_TEST(test_aDynArrayInsertDynArray_success_beyondEndIndex)
{
    DynStringTestArray destArray, srcArray;
    destArray.size = 5;
    destArray.calculateCapacity = calculateCapacityTest;
    destArray.capacity = 8;
    aDynArrayTestMallocFail = false;
    destArray.buffer = malloc(destArray.capacity);
    memcpy(destArray.buffer, "01234", 5);
    srcArray.size = 3;
    srcArray.calculateCapacity = calculateCapacityTest;
    srcArray.capacity = 8;
    srcArray.buffer = malloc(srcArray.capacity);
    memcpy(srcArray.buffer, "56", 3);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayInsertDynArray(&destArray, 666, &srcArray), true);
    ck_assert_uint_eq(destArray.size, 8);
    ck_assert_uint_eq(destArray.capacity, 8);
    ck_assert_ptr_nonnull(destArray.buffer);
    ck_assert_str_eq(destArray.buffer, "0123456");
    ck_assert_ptr_nonnull(destArray.calculateCapacity);
    ck_assert_uint_eq(srcArray.size, 3);
    ck_assert_uint_eq(srcArray.capacity, 8);
    ck_assert_ptr_nonnull(srcArray.buffer);
    ck_assert_str_eq(srcArray.buffer, "56");
    ck_assert_ptr_nonnull(srcArray.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(destArray.buffer);
    free(srcArray.buffer);
}
END_TEST
START_TEST(test_aDynArrayInsertDynArray_success_bufferExpanded)
{
    DynStringTestArray destArray, srcArray;
    destArray.size = 5;
    destArray.calculateCapacity = calculateCapacityTest;
    destArray.capacity = 5;
    aDynArrayTestMallocFail = false;
    destArray.buffer = malloc(destArray.capacity);
    memcpy(destArray.buffer, "0156", 5);
    srcArray.size = 3;
    srcArray.calculateCapacity = calculateCapacityTest;
    srcArray.capacity = 8;
    srcArray.buffer = malloc(srcArray.capacity);
    memcpy(srcArray.buffer, "234", 4);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayInsertDynArray(&destArray, 2, &srcArray), true);
    ck_assert_uint_eq(destArray.size, 8);
    ck_assert_uint_eq(destArray.capacity, 8);
    ck_assert_ptr_nonnull(destArray.buffer);
    ck_assert_str_eq(destArray.buffer, "0123456");
    ck_assert_ptr_nonnull(destArray.calculateCapacity);
    ck_assert_uint_eq(srcArray.size, 3);
    ck_assert_uint_eq(srcArray.capacity, 8);
    ck_assert_ptr_nonnull(srcArray.buffer);
    ck_assert_str_eq(srcArray.buffer, "234");
    ck_assert_ptr_nonnull(srcArray.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 1);
    free(destArray.buffer);
    free(srcArray.buffer);
}
END_TEST
START_TEST(test_aDynArrayInsertDynArray_success_nullptrSrcArray)
{
    DynStringTestArray destArray, *srcArray = NULL;
    destArray.size = 5;
    destArray.calculateCapacity = calculateCapacityTest;
    destArray.capacity = 5;
    aDynArrayTestMallocFail = false;
    destArray.buffer = malloc(destArray.capacity);
    memcpy(destArray.buffer, "0156", 5);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayInsertDynArray(&destArray, 2, srcArray), true);
    ck_assert_uint_eq(destArray.size, 5);
    ck_assert_uint_eq(destArray.capacity, 5);
    ck_assert_ptr_nonnull(destArray.buffer);
    ck_assert_str_eq(destArray.buffer, "0156");
    ck_assert_ptr_nonnull(destArray.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(destArray.buffer);
}
END_TEST
START_TEST(test_aDynArrayInsertDynArray_success_zeroSizeSrcArray)
{
    DynStringTestArray destArray, srcArray;
    destArray.size = 5;
    destArray.calculateCapacity = calculateCapacityTest;
    destArray.capacity = 8;
    aDynArrayTestMallocFail = false;
    destArray.buffer = malloc(destArray.capacity);
    memcpy(destArray.buffer, "0156", 5);
    srcArray.size = 0;
    srcArray.calculateCapacity = calculateCapacityTest;
    srcArray.capacity = 8;
    srcArray.buffer = malloc(srcArray.capacity);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayInsertDynArray(&destArray, 2, &srcArray), true);
    ck_assert_uint_eq(destArray.size, 5);
    ck_assert_uint_eq(destArray.capacity, 8);
    ck_assert_ptr_nonnull(destArray.buffer);
    ck_assert_str_eq(destArray.buffer, "0156");
    ck_assert_ptr_nonnull(destArray.calculateCapacity);
    ck_assert_uint_eq(srcArray.size, 0);
    ck_assert_uint_eq(srcArray.capacity, 8);
    ck_assert_ptr_nonnull(srcArray.buffer);
    ck_assert_ptr_nonnull(srcArray.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(destArray.buffer);
    free(srcArray.buffer);
}
END_TEST
START_TEST(test_aDynArrayInsertDynArray_failure_negativeIndex)
{
    DynStringTestArray destArray, srcArray;
    destArray.size = 5;
    destArray.calculateCapacity = calculateCapacityTest;
    destArray.capacity = 8;
    aDynArrayTestMallocFail = false;
    destArray.buffer = malloc(destArray.capacity);
    memcpy(destArray.buffer, "0123", 5);
    srcArray.size = 3;
    srcArray.calculateCapacity = calculateCapacityTest;
    srcArray.capacity = 8;
    srcArray.buffer = malloc(srcArray.capacity);
    memcpy(srcArray.buffer, "---", 4);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayInsertDynArray(&destArray, -1, &srcArray), false);
    ck_assert_uint_eq(destArray.size, 5);
    ck_assert_uint_eq(destArray.capacity, 8);
    ck_assert_ptr_nonnull(destArray.buffer);
    ck_assert_str_eq(destArray.buffer, "0123");
    ck_assert_ptr_nonnull(destArray.calculateCapacity);
    ck_assert_uint_eq(srcArray.size, 3);
    ck_assert_uint_eq(srcArray.capacity, 8);
    ck_assert_ptr_nonnull(srcArray.buffer);
    ck_assert_str_eq(srcArray.buffer, "---");
    ck_assert_ptr_nonnull(srcArray.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(destArray.buffer);
    free(srcArray.buffer);
}
END_TEST
START_TEST(test_aDynArrayInsertDynArray_failure_bufferExpansionFailed)
{
    DynStringTestArray destArray, srcArray;
    destArray.size = 5;
    destArray.calculateCapacity = calculateCapacityTest;
    destArray.capacity = 5;
    aDynArrayTestMallocFail = false;
    destArray.buffer = malloc(destArray.capacity);
    memcpy(destArray.buffer, "0156", 5);
    srcArray.size = 3;
    srcArray.calculateCapacity = calculateCapacityTest;
    srcArray.capacity = 8;
    srcArray.buffer = malloc(srcArray.capacity);
    memcpy(srcArray.buffer, "234", 4);
    aDynArrayTestReallocFail = true;
    ck_assert_uint_eq(aDynArrayInsertDynArray(&destArray, 2, &srcArray), false);
    ck_assert_uint_eq(destArray.size, 5);
    ck_assert_uint_eq(destArray.capacity, 5);
    ck_assert_ptr_nonnull(destArray.buffer);
    ck_assert_str_eq(destArray.buffer, "0156");
    ck_assert_ptr_nonnull(destArray.calculateCapacity);
    ck_assert_uint_eq(srcArray.size, 3);
    ck_assert_uint_eq(srcArray.capacity, 8);
    ck_assert_ptr_nonnull(srcArray.buffer);
    ck_assert_str_eq(srcArray.buffer, "234");
    ck_assert_ptr_nonnull(srcArray.calculateCapacity);
    free(destArray.buffer);
    free(srcArray.buffer);
}
END_TEST
START_TEST(test_aDynArrayInsertDynArray_failure_nullptrDestArray)
{
    DynStringTestArray srcArray, *destArrayPtr = NULL;
    srcArray.size = 3;
    srcArray.calculateCapacity = calculateCapacityTest;
    srcArray.capacity = 8;
    srcArray.buffer = malloc(srcArray.capacity);
    memcpy(srcArray.buffer, "012", 4);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayInsertDynArray(destArrayPtr, 0, &srcArray), false);
    ck_assert_uint_eq(srcArray.size, 3);
    ck_assert_uint_eq(srcArray.capacity, 8);
    ck_assert_ptr_nonnull(srcArray.buffer);
    ck_assert_str_eq(srcArray.buffer, "012");
    ck_assert_ptr_nonnull(srcArray.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(srcArray.buffer);
}
END_TEST


START_TEST(test_aDynArrayAdd_success_enoughCapacity)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 8;
    aDynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "01234", 5);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayAdd(&array, '\0'), true);
    ck_assert_uint_eq(array.size, 6);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_int_eq(array.buffer[array.size - 1], '\0');
    ck_assert_str_eq(array.buffer, "01234");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayAdd_success_notEnoughCapacity)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 5;
    aDynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "01234", 5);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayAdd(&array, '\0'), true);
    ck_assert_uint_eq(array.size, 6);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_int_eq(array.buffer[array.size - 1], '\0');
    ck_assert_str_eq(array.buffer, "01234");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 1);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayAdd_failure_bufferExpansionFailed)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 5;
    aDynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity + 1);
    memcpy(array.buffer, "01234", 6);
    aDynArrayTestReallocFail = true;
    ck_assert_uint_eq(aDynArrayAdd(&array, '\0'), false);
    ck_assert_uint_eq(array.size, 5);
    ck_assert_uint_eq(array.capacity, 5);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "01234");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayAdd_failure_nullptr)
{
    DynStringTestArray* arrayPtr = NULL;
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayAdd(arrayPtr, '\0'), false);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
}
END_TEST


START_TEST(test_aDynArrayAddArray_success_enoughCapacity)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 8;
    aDynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "01234", 5);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayAddArray(&array, "56\0", 3), true);
    ck_assert_uint_eq(array.size, 8);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123456");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayAddArray_success_notEnoughCapacity)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 5;
    aDynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "01234", 5);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayAddArray(&array, "56\0", 3), true);
    ck_assert_uint_eq(array.size, 8);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123456");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 1);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayAddArray_success_nullptrArray)
{
    DynStringTestArray array;
    char* nullptrArray = NULL;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 5;
    aDynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "0156", 5);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayAddArray(&array, nullptrArray, 3), true);
    ck_assert_uint_eq(array.size, 5);
    ck_assert_uint_eq(array.capacity, 5);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0156");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayAddArray_success_zeroArraySize)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 5;
    aDynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "0156", 5);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayAddArray(&array, "234", 0), true);
    ck_assert_uint_eq(array.size, 5);
    ck_assert_uint_eq(array.capacity, 5);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0156");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayAddArray_failure_bufferExpansionFailed)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 5;
    aDynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "0123", 5);
    aDynArrayTestReallocFail = true;
    ck_assert_uint_eq(aDynArrayInsertArray(&array, 2, "45\0", 3), false);
    ck_assert_uint_eq(array.size, 5);
    ck_assert_uint_eq(array.capacity, 5);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayAddArray_failure_nullptrDestArray)
{
    DynStringTestArray* arrayPtr = NULL;
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayInsertArray(arrayPtr, 0, "012", 3), false);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
}
END_TEST


START_TEST(test_aDynArrayAddDynArray_success_enoughCapacity)
{
    DynStringTestArray destArray, srcArray;
    destArray.size = 5;
    destArray.calculateCapacity = calculateCapacityTest;
    destArray.capacity = 8;
    aDynArrayTestMallocFail = false;
    destArray.buffer = malloc(destArray.capacity);
    memcpy(destArray.buffer, "01234", 5);
    srcArray.size = 3;
    srcArray.calculateCapacity = calculateCapacityTest;
    srcArray.capacity = 8;
    srcArray.buffer = malloc(srcArray.capacity);
    memcpy(srcArray.buffer, "56", 3);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayAddDynArray(&destArray, &srcArray), true);
    ck_assert_uint_eq(destArray.size, 8);
    ck_assert_uint_eq(destArray.capacity, 8);
    ck_assert_ptr_nonnull(destArray.buffer);
    ck_assert_str_eq(destArray.buffer, "0123456");
    ck_assert_ptr_nonnull(destArray.calculateCapacity);
    ck_assert_uint_eq(srcArray.size, 3);
    ck_assert_uint_eq(srcArray.capacity, 8);
    ck_assert_ptr_nonnull(srcArray.buffer);
    ck_assert_str_eq(srcArray.buffer, "56");
    ck_assert_ptr_nonnull(srcArray.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(destArray.buffer);
    free(srcArray.buffer);
}
END_TEST
START_TEST(test_aDynArrayAddDynArray_success_notEnoughCapacity)
{
    DynStringTestArray destArray, srcArray;
    destArray.size = 5;
    destArray.calculateCapacity = calculateCapacityTest;
    destArray.capacity = 5;
    aDynArrayTestMallocFail = false;
    destArray.buffer = malloc(destArray.capacity);
    memcpy(destArray.buffer, "01234", 5);
    srcArray.size = 3;
    srcArray.calculateCapacity = calculateCapacityTest;
    srcArray.capacity = 8;
    srcArray.buffer = malloc(srcArray.capacity);
    memcpy(srcArray.buffer, "56", 3);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayAddDynArray(&destArray, &srcArray), true);
    ck_assert_uint_eq(destArray.size, 8);
    ck_assert_uint_eq(destArray.capacity, 8);
    ck_assert_ptr_nonnull(destArray.buffer);
    ck_assert_str_eq(destArray.buffer, "0123456");
    ck_assert_ptr_nonnull(destArray.calculateCapacity);
    ck_assert_uint_eq(srcArray.size, 3);
    ck_assert_uint_eq(srcArray.capacity, 8);
    ck_assert_ptr_nonnull(srcArray.buffer);
    ck_assert_str_eq(srcArray.buffer, "56");
    ck_assert_ptr_nonnull(srcArray.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 1);
    free(destArray.buffer);
    free(srcArray.buffer);
}
END_TEST
START_TEST(test_aDynArrayAddDynArray_success_nullptrSrcArray)
{
    DynStringTestArray destArray, *srcArray = NULL;
    destArray.size = 5;
    destArray.calculateCapacity = calculateCapacityTest;
    destArray.capacity = 5;
    aDynArrayTestMallocFail = false;
    destArray.buffer = malloc(destArray.capacity);
    memcpy(destArray.buffer, "0123", 5);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayAddDynArray(&destArray, srcArray), true);
    ck_assert_uint_eq(destArray.size, 5);
    ck_assert_uint_eq(destArray.capacity, 5);
    ck_assert_ptr_nonnull(destArray.buffer);
    ck_assert_str_eq(destArray.buffer, "0123");
    ck_assert_ptr_nonnull(destArray.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(destArray.buffer);
}
END_TEST
START_TEST(test_aDynArrayAddDynArray_success_zeroSizeSrcArray)
{
    DynStringTestArray destArray, srcArray;
    destArray.size = 5;
    destArray.calculateCapacity = calculateCapacityTest;
    destArray.capacity = 8;
    aDynArrayTestMallocFail = false;
    destArray.buffer = malloc(destArray.capacity);
    memcpy(destArray.buffer, "0123", 5);
    srcArray.size = 0;
    srcArray.calculateCapacity = calculateCapacityTest;
    srcArray.capacity = 8;
    srcArray.buffer = malloc(srcArray.capacity);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayAddDynArray(&destArray, &srcArray), true);
    ck_assert_uint_eq(destArray.size, 5);
    ck_assert_uint_eq(destArray.capacity, 8);
    ck_assert_ptr_nonnull(destArray.buffer);
    ck_assert_str_eq(destArray.buffer, "0123");
    ck_assert_ptr_nonnull(destArray.calculateCapacity);
    ck_assert_uint_eq(srcArray.size, 0);
    ck_assert_uint_eq(srcArray.capacity, 8);
    ck_assert_ptr_nonnull(srcArray.buffer);
    ck_assert_ptr_nonnull(srcArray.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(destArray.buffer);
    free(srcArray.buffer);
}
END_TEST
START_TEST(test_aDynArrayAddDynArray_failure_bufferExpansionFailed)
{
    DynStringTestArray destArray, srcArray;
    destArray.size = 5;
    destArray.calculateCapacity = calculateCapacityTest;
    destArray.capacity = 5;
    aDynArrayTestMallocFail = false;
    destArray.buffer = malloc(destArray.capacity);
    memcpy(destArray.buffer, "0156", 5);
    srcArray.size = 3;
    srcArray.calculateCapacity = calculateCapacityTest;
    srcArray.capacity = 8;
    srcArray.buffer = malloc(srcArray.capacity);
    memcpy(srcArray.buffer, "234", 4);
    aDynArrayTestReallocFail = true;
    ck_assert_uint_eq(aDynArrayAddDynArray(&destArray, &srcArray), false);
    ck_assert_uint_eq(destArray.size, 5);
    ck_assert_uint_eq(destArray.capacity, 5);
    ck_assert_ptr_nonnull(destArray.buffer);
    ck_assert_str_eq(destArray.buffer, "0156");
    ck_assert_ptr_nonnull(destArray.calculateCapacity);
    ck_assert_uint_eq(srcArray.size, 3);
    ck_assert_uint_eq(srcArray.capacity, 8);
    ck_assert_ptr_nonnull(srcArray.buffer);
    ck_assert_str_eq(srcArray.buffer, "234");
    ck_assert_ptr_nonnull(srcArray.calculateCapacity);
    free(destArray.buffer);
    free(srcArray.buffer);
}
END_TEST
START_TEST(test_aDynArrayAddDynArray_failure_nullptrDestArray)
{
    DynStringTestArray srcArray, *destArrayPtr = NULL;
    srcArray.size = 3;
    srcArray.calculateCapacity = calculateCapacityTest;
    srcArray.capacity = 8;
    srcArray.buffer = malloc(srcArray.capacity);
    memcpy(srcArray.buffer, "012", 4);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArrayAddDynArray(destArrayPtr, &srcArray), false);
    ck_assert_uint_eq(srcArray.size, 3);
    ck_assert_uint_eq(srcArray.capacity, 8);
    ck_assert_ptr_nonnull(srcArray.buffer);
    ck_assert_str_eq(srcArray.buffer, "012");
    ck_assert_ptr_nonnull(srcArray.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(srcArray.buffer);
}
END_TEST


START_TEST(test_aDynArraySet_success_indexInBounds)
{
    DynStringTestArray array;
    array.size = 4;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 4;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "012", 4);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArraySet(&array, 0, '2'), true);
    ck_assert_uint_eq(aDynArraySet(&array, 1, '1'), true);
    ck_assert_uint_eq(aDynArraySet(&array, 2, '0'), true);
    ck_assert_uint_eq(array.size, 4);
    ck_assert_uint_eq(array.capacity, 4);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "210");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArraySet_success_indexBeyondSize)
{
    DynStringTestArray array;
    array.size = 3;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 4;
    array.buffer = malloc(array.capacity + 1);
    memcpy(array.buffer, "012\0\0", 5);
    aDynArrayTestReallocFail = true;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArraySet(&array, 3, '3'), true);
    ck_assert_uint_eq(array.size, 4);
    ck_assert_uint_eq(array.capacity, 4);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArraySet_success_indexBeyondSize_bufferExpanded)
{
    DynStringTestArray array;
    array.size = 3;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 3;
    array.buffer = malloc(array.capacity + 2);
    memcpy(array.buffer, "012\0\0", 5);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArraySet(&array, 3, '3'), true);
    ck_assert_uint_eq(array.size, 4);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 1);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArraySet_failure_indexBeyondSize_bufferExpansionFailed)
{
    DynStringTestArray array;
    array.size = 3;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 3;
    array.buffer = malloc(array.capacity + 1);
    memcpy(array.buffer, "012", 4);
    aDynArrayTestReallocFail = true;
    ck_assert_uint_eq(aDynArraySet(&array, 3, '3'), false);
    ck_assert_uint_eq(array.size, 3);
    ck_assert_uint_eq(array.capacity, 3);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "012");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArraySet_failure_nullptr)
{
    DynStringTestArray *arrayPtr = NULL;
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    ck_assert_uint_eq(aDynArraySet(arrayPtr, 0, '0'), false);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
}
END_TEST


START_TEST(test_aDynArrayRemove_indexRangeInBounds)
{
    DynStringTestArray array;
    array.size = 11;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 16;
    array.buffer = malloc(array.capacity + 1);
    memcpy(array.buffer, "0123456789", 11);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    aDynArrayRemove(&array, 2, 6);
    ck_assert_uint_eq(array.size, 5);
    ck_assert_uint_eq(array.capacity, 16);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0189");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayRemove_rangeBeyondBounds)
{
    DynStringTestArray array;
    array.size = 11;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 16;
    array.buffer = malloc(array.capacity + 1);
    memcpy(array.buffer, "0123456789", 11);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    aDynArrayRemove(&array, 2, 100);
    ck_assert_uint_eq(array.size, 2);
    ck_assert_uint_eq(array.capacity, 16);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayRemove_zeroRange)
{
    DynStringTestArray array;
    array.size = 11;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 16;
    array.buffer = malloc(array.capacity + 1);
    memcpy(array.buffer, "0123456789", 11);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    aDynArrayRemove(&array, 2, 0);
    ck_assert_uint_eq(array.size, 11);
    ck_assert_uint_eq(array.capacity, 16);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123456789");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayRemove_indexBeyoundBounds)
{
    DynStringTestArray array;
    array.size = 11;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 16;
    array.buffer = malloc(array.capacity + 1);
    memcpy(array.buffer, "0123456789", 11);
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    aDynArrayRemove(&array, 13, 5);
    ck_assert_uint_eq(array.size, 11);
    ck_assert_uint_eq(array.capacity, 16);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123456789");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_aDynArrayRemove_nullptr)
{
    DynStringTestArray *arrayPtr = NULL;
    aDynArrayTestReallocFail = false;
    aDynArrayTestReallocCount = 0;
    aDynArrayRemove(arrayPtr, 5, 10);
    ck_assert_uint_eq(aDynArrayTestReallocCount, 0);
}
END_TEST



Suite* test_suite_dynarray()
{
    Suite *s;
    TCase *test_case_aDynArrayConstruct_aDynArrayDestruct, *test_case_aDynArraySize, *test_case_aDynArrayReserve,
          *test_case_aDynArrayShrinkToFit, *test_case_aDynArrayClear, *test_case_aDynArrayInsert,
          *test_case_aDynArrayInsertArray, *test_case_aDynArrayInsertDynArray, *test_case_aDynArrayAdd,
          *test_case_aDynArrayAddArray, *test_case_aDynArrayAddDynArray, *test_case_aDynArraySet,
          *test_case_aDynArrayRemove;

    s = suite_create("Dynamic Array Test Suite");

    test_case_aDynArrayConstruct_aDynArrayDestruct = tcase_create("Dynamic Array Test Case: aDynArrayConstruct / aDynArrayDestruct");
    tcase_add_test(test_case_aDynArrayConstruct_aDynArrayDestruct, test_aDynArrayConstruct_aDynArrayDestruct_valid);
    tcase_add_test(test_case_aDynArrayConstruct_aDynArrayDestruct, test_aDynArrayConstruct_aDynArrayDestruct_noMemoryAvailable);
    tcase_add_test(test_case_aDynArrayConstruct_aDynArrayDestruct, test_aDynArrayConstruct_aDynArrayDestruct_nullptr);
    suite_add_tcase(s, test_case_aDynArrayConstruct_aDynArrayDestruct);

    test_case_aDynArraySize = tcase_create("Dynamic Array Test Case: aDynArraySize");
    tcase_add_test(test_case_aDynArraySize, test_aDynArraySize_valid);
    tcase_add_test(test_case_aDynArraySize, test_aDynArraySize_nullptr);
    suite_add_tcase(s, test_case_aDynArraySize);

    test_case_aDynArrayReserve = tcase_create("Dynamic Array Test Case: aDynArrayReserve");
    tcase_add_test(test_case_aDynArrayReserve, test_aDynArrayReserve_success_enoughCapacityBufferNotNull);
    tcase_add_test(test_case_aDynArrayReserve, test_aDynArrayReserve_success_enoughCapacityBufferNull);
    tcase_add_test(test_case_aDynArrayReserve, test_aDynArrayReserve_success_notEnoughCapacity);
    tcase_add_test(test_case_aDynArrayReserve, test_aDynArrayReserve_failure_biggerThanMaxCapacity);
    tcase_add_test(test_case_aDynArrayReserve, test_aDynArrayReserve_failure_noMemoryAvailable);
    tcase_add_test(test_case_aDynArrayReserve, test_aDynArrayReserve_failure_calculateCapacityNull);
    tcase_add_test(test_case_aDynArrayReserve, test_aDynArrayReserve_failure_nullptr);
    suite_add_tcase(s, test_case_aDynArrayReserve);

    test_case_aDynArrayShrinkToFit = tcase_create("Dynamic Array Test Case: aDynArrayShrinkToFit");
    tcase_add_test(test_case_aDynArrayShrinkToFit, test_aDynArrayShrinkToFit_success_hasLeastCapacityBufferNotNull);
    tcase_add_test(test_case_aDynArrayShrinkToFit, test_aDynArrayShrinkToFit_success_hasLeastCapacityBufferNull);
    tcase_add_test(test_case_aDynArrayShrinkToFit, test_aDynArrayShrinkToFit_success_hasNotLeastCapacity);
    tcase_add_test(test_case_aDynArrayShrinkToFit, test_aDynArrayShrinkToFit_failure_noMemoryAvailable);
    tcase_add_test(test_case_aDynArrayShrinkToFit, test_aDynArrayShrinkToFit_failure_calculateCapacityNull);
    tcase_add_test(test_case_aDynArrayShrinkToFit, test_aDynArrayShrinkToFit_failure_nullptr);
    suite_add_tcase(s, test_case_aDynArrayShrinkToFit);

    test_case_aDynArrayClear = tcase_create("Dynamic Array Test Case: aDynArrayClear");
    tcase_add_test(test_case_aDynArrayClear, test_aDynArrayClear_success_shrinked);
    tcase_add_test(test_case_aDynArrayClear, test_aDynArrayClear_failure_notShrinked);
    tcase_add_test(test_case_aDynArrayClear, test_aDynArrayClear_failure_nullptr);
    suite_add_tcase(s, test_case_aDynArrayClear);

    test_case_aDynArrayInsert = tcase_create("Dynamic Array Test Case: aDynArrayInsert");
    tcase_add_test(test_case_aDynArrayInsert, test_aDynArrayInsert_success_zeroIndex);
    tcase_add_test(test_case_aDynArrayInsert, test_aDynArrayInsert_success_middleIndex);
    tcase_add_test(test_case_aDynArrayInsert, test_aDynArrayInsert_success_endIndex);
    tcase_add_test(test_case_aDynArrayInsert, test_aDynArrayInsert_success_beyondEndIndex);
    tcase_add_test(test_case_aDynArrayInsert, test_aDynArrayInsert_success_bufferExpanded);
    tcase_add_test(test_case_aDynArrayInsert, test_aDynArrayInsert_failure_negativeIndex);
    tcase_add_test(test_case_aDynArrayInsert, test_aDynArrayInsert_failure_bufferExpansionFailed);
    tcase_add_test(test_case_aDynArrayInsert, test_aDynArrayInsert_failure_nullptr);
    suite_add_tcase(s, test_case_aDynArrayInsert);

    test_case_aDynArrayInsertArray = tcase_create("Dynamic Array Test Case: aDynArrayInsertArray");
    tcase_add_test(test_case_aDynArrayInsertArray, test_aDynArrayInsertArray_success_zeroIndex);
    tcase_add_test(test_case_aDynArrayInsertArray, test_aDynArrayInsertArray_success_middleIndex);
    tcase_add_test(test_case_aDynArrayInsertArray, test_aDynArrayInsertArray_success_endIndex);
    tcase_add_test(test_case_aDynArrayInsertArray, test_aDynArrayInsertArray_success_beyondEndIndex);
    tcase_add_test(test_case_aDynArrayInsertArray, test_aDynArrayInsertArray_success_bufferExpanded);
    tcase_add_test(test_case_aDynArrayInsertArray, test_aDynArrayInsertArray_success_nullptrArray);
    tcase_add_test(test_case_aDynArrayInsertArray, test_aDynArrayInsertArray_success_zeroArraySize);
    tcase_add_test(test_case_aDynArrayInsertArray, test_aDynArrayInsertArray_failure_negativeIndex);
    tcase_add_test(test_case_aDynArrayInsertArray, test_aDynArrayInsertArray_failure_bufferExpansionFailed);
    tcase_add_test(test_case_aDynArrayInsertArray, test_aDynArrayInsertArray_failure_nullptrDestArray);
    suite_add_tcase(s, test_case_aDynArrayInsertArray);

    test_case_aDynArrayInsertDynArray = tcase_create("Dynamic Array Test Case: aDynArrayInsertDynArray");
    tcase_add_test(test_case_aDynArrayInsertDynArray, test_aDynArrayInsertDynArray_success_zeroIndex);
    tcase_add_test(test_case_aDynArrayInsertDynArray, test_aDynArrayInsertDynArray_success_middleIndex);
    tcase_add_test(test_case_aDynArrayInsertDynArray, test_aDynArrayInsertDynArray_success_endIndex);
    tcase_add_test(test_case_aDynArrayInsertDynArray, test_aDynArrayInsertDynArray_success_beyondEndIndex);
    tcase_add_test(test_case_aDynArrayInsertDynArray, test_aDynArrayInsertDynArray_success_bufferExpanded);
    tcase_add_test(test_case_aDynArrayInsertDynArray, test_aDynArrayInsertDynArray_success_nullptrSrcArray);
    tcase_add_test(test_case_aDynArrayInsertDynArray, test_aDynArrayInsertDynArray_success_zeroSizeSrcArray);
    tcase_add_test(test_case_aDynArrayInsertDynArray, test_aDynArrayInsertDynArray_failure_negativeIndex);
    tcase_add_test(test_case_aDynArrayInsertDynArray, test_aDynArrayInsertDynArray_failure_bufferExpansionFailed);
    tcase_add_test(test_case_aDynArrayInsertDynArray, test_aDynArrayInsertDynArray_failure_nullptrDestArray);
    suite_add_tcase(s, test_case_aDynArrayInsertDynArray);

    test_case_aDynArrayAdd = tcase_create("Dynamic Array Test Case: aDynArrayAdd");
    tcase_add_test(test_case_aDynArrayAdd, test_aDynArrayAdd_success_enoughCapacity);
    tcase_add_test(test_case_aDynArrayAdd, test_aDynArrayAdd_success_notEnoughCapacity);
    tcase_add_test(test_case_aDynArrayAdd, test_aDynArrayAdd_failure_bufferExpansionFailed);
    tcase_add_test(test_case_aDynArrayAdd, test_aDynArrayAdd_failure_nullptr);
    suite_add_tcase(s, test_case_aDynArrayAdd);

    test_case_aDynArrayAddArray = tcase_create("Dynamic Array Test Case: aDynArrayAddArray");
    tcase_add_test(test_case_aDynArrayAddArray, test_aDynArrayAddArray_success_enoughCapacity);
    tcase_add_test(test_case_aDynArrayAddArray, test_aDynArrayAddArray_success_notEnoughCapacity);
    tcase_add_test(test_case_aDynArrayAddArray, test_aDynArrayAddArray_success_nullptrArray);
    tcase_add_test(test_case_aDynArrayAddArray, test_aDynArrayAddArray_success_zeroArraySize);
    tcase_add_test(test_case_aDynArrayAddArray, test_aDynArrayAddArray_failure_bufferExpansionFailed);
    tcase_add_test(test_case_aDynArrayAddArray, test_aDynArrayAddArray_failure_nullptrDestArray);
    suite_add_tcase(s, test_case_aDynArrayAddArray);

    test_case_aDynArrayAddDynArray = tcase_create("Dynamic Array Test Case: aDynArrayAddDynArray");
    tcase_add_test(test_case_aDynArrayAddDynArray, test_aDynArrayAddDynArray_success_enoughCapacity);
    tcase_add_test(test_case_aDynArrayAddDynArray, test_aDynArrayAddDynArray_success_notEnoughCapacity);
    tcase_add_test(test_case_aDynArrayAddDynArray, test_aDynArrayAddDynArray_success_nullptrSrcArray);
    tcase_add_test(test_case_aDynArrayAddDynArray, test_aDynArrayAddDynArray_success_zeroSizeSrcArray);
    tcase_add_test(test_case_aDynArrayAddDynArray, test_aDynArrayAddDynArray_failure_bufferExpansionFailed);
    tcase_add_test(test_case_aDynArrayAddDynArray, test_aDynArrayAddDynArray_failure_nullptrDestArray);
    suite_add_tcase(s, test_case_aDynArrayAddDynArray);

    test_case_aDynArraySet = tcase_create("Dynamic Array Test Case: aDynArraySet");
    tcase_add_test(test_case_aDynArraySet, test_aDynArraySet_success_indexInBounds);
    tcase_add_test(test_case_aDynArraySet, test_aDynArraySet_success_indexBeyondSize);
    tcase_add_test(test_case_aDynArraySet, test_aDynArraySet_success_indexBeyondSize_bufferExpanded);
    tcase_add_test(test_case_aDynArraySet, test_aDynArraySet_failure_indexBeyondSize_bufferExpansionFailed);
    tcase_add_test(test_case_aDynArraySet, test_aDynArraySet_failure_nullptr);
    suite_add_tcase(s, test_case_aDynArraySet);

    test_case_aDynArrayRemove = tcase_create("Dynamic Array Test Case: aDynArrayRemove");
    tcase_add_test(test_case_aDynArrayRemove, test_aDynArrayRemove_indexRangeInBounds);
    tcase_add_test(test_case_aDynArrayRemove, test_aDynArrayRemove_rangeBeyondBounds);
    tcase_add_test(test_case_aDynArrayRemove, test_aDynArrayRemove_zeroRange);
    tcase_add_test(test_case_aDynArrayRemove, test_aDynArrayRemove_indexBeyoundBounds);
    tcase_add_test(test_case_aDynArrayRemove, test_aDynArrayRemove_nullptr);
    suite_add_tcase(s, test_case_aDynArrayRemove);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = test_suite_dynarray();
    sr = srunner_create(s);

    srunner_set_fork_status(sr, CK_NOFORK);
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}