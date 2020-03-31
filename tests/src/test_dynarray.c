#include <string.h>

#if defined(VALGRIND_SUPPORTED)
# include "valgrind/valgrind.h"
# define CHECK_DONT_FORK_DYNARRAY_TEST RUNNING_ON_VALGRIND
#endif
#ifndef CHECK_DONT_FORK_DYNARRAY_TEST
# ifndef NDEBUG
#  define CHECK_DONT_FORK_DYNARRAY_TEST 1
# else
#  define CHECK_DONT_FORK_DYNARRAY_TEST 0
# endif
#endif

#include "check.h"

#include "../../private/dynarray.h"

static size_t dynArrayTestMallocCount = 0;
static bool dynArrayTestMallocFail = false;
static size_t dynArrayTestReallocCount = 0;
static bool dynArrayTestReallocFail = false;
static size_t dynArrayTestFreeCount = 0;

static void* dynArrayTestMalloc(size_t size) {
    if(!dynArrayTestMallocFail) {
        void* tmp = malloc(size);
        if(tmp != nullptr)
            ++dynArrayTestMallocCount;
        return tmp;
    } else {
        return nullptr;
    }
}
static void* dynArrayTestRealloc(void* ptr, size_t size) {
    if(!dynArrayTestReallocFail) {
        void* tmp = realloc(ptr, size);
        if(tmp != nullptr)
            ++dynArrayTestReallocCount;
        return tmp;
    } else {
        return nullptr;
    }
}
static void dynArrayTestFree(void* ptr) {
    ++dynArrayTestFreeCount;
    free(ptr);
}

#define malloc(size) dynArrayTestMalloc(size)
#define realloc(ptr, size) dynArrayTestRealloc(ptr, size)
#define free(ptr) dynArrayTestFree(ptr)

static const size_t dynArrayTestCapacityMin = 8;
static const size_t dynArrayTestCapacityMax = 32;
static const size_t dynArrayTestCapacityMul = 2;

static inline size_t calculateCapacityTest(size_t requiredSize) {
    return calculateCapacityGeneric(requiredSize, dynArrayTestCapacityMin, dynArrayTestCapacityMax, dynArrayTestCapacityMul);
}

DYNAMIC_ARRAY_DEFINITION(DynStringTestArray, char)

START_TEST(test_dynarray_construct_destruct_valid)
{
    DynStringTestArray* array;
    dynArrayTestMallocFail = false;
    dynArrayTestMallocCount = dynArrayTestFreeCount = 0;
    array = dynArrayConstruct(DynStringTestArray);
    ck_assert_uint_eq(array->size, 0);
    ck_assert_uint_eq(array->capacity, dynArrayTestCapacityMin);
    ck_assert_ptr_nonnull(array->buffer);
    ck_assert_ptr_nonnull(array->calculateCapacity);
    dynArrayDestruct(array);
    ck_assert_uint_eq(dynArrayTestMallocCount, dynArrayTestFreeCount);
}
END_TEST
START_TEST(test_dynarray_construct_noMemoryAvailable)
{
    DynStringTestArray* array;
    dynArrayTestMallocFail = true;
    dynArrayTestMallocCount = dynArrayTestFreeCount = 0;
    array = dynArrayConstruct(DynStringTestArray);
    ck_assert_ptr_null(array);
    dynArrayDestruct(array); // should do nothing
    ck_assert_uint_eq(dynArrayTestMallocCount, 0);
    ck_assert_uint_eq(dynArrayTestFreeCount, 0);
}
END_TEST
START_TEST(test_dynarray_destruct_nullptr)
{
    DynStringTestArray* array = nullptr;
    dynArrayTestFreeCount = 0;
    dynArrayDestruct(array); // should do nothing
    ck_assert_uint_eq(dynArrayTestFreeCount, 0);
}
END_TEST


START_TEST(test_dynarray_size_valid)
{
    DynStringTestArray array;
    array.capacity = 0;
    array.buffer = nullptr;
    array.calculateCapacity = nullptr;
    array.size = 42;
    ck_assert_uint_eq(dynArraySize(&array), 42);
    array.size = 13;
    ck_assert_uint_eq(dynArraySize(&array), 13);
    array.size = 0;
    ck_assert_uint_eq(dynArraySize(&array), 0);
}
START_TEST(test_dynarray_size_nullptr)
{
    DynStringTestArray* arrayPtr = nullptr;
    ck_assert_uint_eq(dynArraySize(arrayPtr), 0);
}


START_TEST(test_dynarray_reserve_success_enoughCapacityBufferNotNull)
{
    DynStringTestArray array;
    array.size = 0;
    array.calculateCapacity = nullptr;
    array.capacity = dynArrayTestCapacityMin;
    dynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    dynArrayTestReallocFail = true;
    ck_assert_uint_eq(dynArrayReserve(&array, array.capacity), true);
    ck_assert_uint_eq(array.size, 0);
    ck_assert_uint_eq(array.capacity, dynArrayTestCapacityMin);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_ptr_null(array.calculateCapacity);
    free(array.buffer);
}
START_TEST(test_dynarray_reserve_success_enoughCapacityBufferNull)
{
    DynStringTestArray array;
    array.size = 0;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = dynArrayTestCapacityMin;
    array.buffer = nullptr;
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayReserve(&array, array.capacity), true);
    ck_assert_uint_eq(array.size, 0);
    ck_assert_uint_eq(array.capacity, dynArrayTestCapacityMin);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 1);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_reserve_success_notEnoughCapacity)
{
    DynStringTestArray array;
    array.size = 0;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = dynArrayTestCapacityMin;
    dynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayReserve(&array, dynArrayTestCapacityMax), true);
    ck_assert_uint_eq(array.size, 0);
    ck_assert_uint_eq(array.capacity, dynArrayTestCapacityMax);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 1);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_reserve_failure_biggerThanMaxCapacity)
{
    DynStringTestArray array;
    array.size = 0;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = dynArrayTestCapacityMin;
    array.buffer = nullptr;
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayReserve(&array, dynArrayTestCapacityMax + 1), false);
    ck_assert_uint_eq(array.size, 0);
    ck_assert_uint_eq(array.capacity, dynArrayTestCapacityMin);
    ck_assert_ptr_null(array.buffer);
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
}
END_TEST
START_TEST(test_dynarray_reserve_failure_noMemoryAvailable)
{
    DynStringTestArray array;
    array.size = 0;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = dynArrayTestCapacityMin;
    array.buffer = nullptr;
    dynArrayTestReallocFail = true;
    ck_assert_uint_eq(dynArrayReserve(&array, dynArrayTestCapacityMax), false);
    ck_assert_uint_eq(array.size, 0);
    ck_assert_uint_eq(array.capacity, dynArrayTestCapacityMin);
    ck_assert_ptr_null(array.buffer);
    ck_assert_ptr_nonnull(array.calculateCapacity);
}
END_TEST
START_TEST(test_dynarray_reserve_failure_calculateCapacityNull)
{
    DynStringTestArray array;
    array.size = 0;
    array.calculateCapacity = nullptr;
    array.capacity = dynArrayTestCapacityMin;
    array.buffer = nullptr;
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayReserve(&array, dynArrayTestCapacityMax), false);
    ck_assert_uint_eq(array.size, 0);
    ck_assert_uint_eq(array.capacity, dynArrayTestCapacityMin);
    ck_assert_ptr_null(array.buffer);
    ck_assert_ptr_null(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
}
END_TEST
START_TEST(test_dynarray_reserve_failure_nullptr)
{
    DynStringTestArray* arrayPtr = nullptr;
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayReserve(arrayPtr, dynArrayTestCapacityMax), false);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
}
END_TEST


START_TEST(test_dynarray_shrinkToFit_success_hasLeastCapacityBufferNotNull)
{
    DynStringTestArray array;
    array.size = dynArrayTestCapacityMin - 1;
    array.capacity = dynArrayTestCapacityMin;
    dynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    array.calculateCapacity = calculateCapacityTest;
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayShrinkToFit(&array), true);
    ck_assert_uint_eq(array.size, dynArrayTestCapacityMin - 1);
    ck_assert_uint_eq(array.capacity, dynArrayTestCapacityMin);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(array.buffer);
}
START_TEST(test_dynarray_shrinkToFit_success_hasLeastCapacityBufferNull)
{
    DynStringTestArray array;
    array.size = dynArrayTestCapacityMin - 1;
    array.capacity = dynArrayTestCapacityMin;
    array.buffer = nullptr;
    array.calculateCapacity = calculateCapacityTest;
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayShrinkToFit(&array), true);
    ck_assert_uint_eq(array.size, dynArrayTestCapacityMin - 1);
    ck_assert_uint_eq(array.capacity, dynArrayTestCapacityMin);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 1);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_shrinkToFit_success_hasNotLeastCapacity)
{
    DynStringTestArray array;
    array.size = dynArrayTestCapacityMin - 1;
    array.capacity = calculateCapacityTest(dynArrayTestCapacityMin + 1);
    dynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    array.calculateCapacity = calculateCapacityTest;
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayShrinkToFit(&array), true);
    ck_assert_uint_eq(array.size, dynArrayTestCapacityMin - 1);
    ck_assert_uint_eq(array.capacity, dynArrayTestCapacityMin);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 1);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_shrinkToFit_failure_noMemoryAvailable)
{
    DynStringTestArray array;
    array.size = dynArrayTestCapacityMin - 1;
    array.capacity = calculateCapacityTest(dynArrayTestCapacityMin + 1);
    dynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    array.calculateCapacity = calculateCapacityTest;
    dynArrayTestReallocFail = true;
    ck_assert_uint_eq(dynArrayShrinkToFit(&array), false);
    ck_assert_uint_eq(array.size, dynArrayTestCapacityMin - 1);
    ck_assert_uint_eq(array.capacity, calculateCapacityTest(dynArrayTestCapacityMin + 1));
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_ptr_nonnull(array.calculateCapacity);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_shrinkToFit_failure_calculateCapacityNull)
{
    DynStringTestArray array;
    array.size = dynArrayTestCapacityMin - 1;
    array.capacity = calculateCapacityTest(dynArrayTestCapacityMin + 1);
    dynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    array.calculateCapacity = nullptr;
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayShrinkToFit(&array), false);
    ck_assert_uint_eq(array.size, dynArrayTestCapacityMin - 1);
    ck_assert_uint_eq(array.capacity, calculateCapacityTest(dynArrayTestCapacityMin + 1));
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_ptr_null(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_shrinkToFit_failure_nullptr)
{
    DynStringTestArray* arrayPtr = nullptr;
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayShrinkToFit(arrayPtr), false);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
}
END_TEST


START_TEST(test_dynarray_clear_success_shrinked)
{
    DynStringTestArray array;
    array.size = dynArrayTestCapacityMin + 1;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = dynArrayTestCapacityMin * dynArrayTestCapacityMul;
    dynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayClear(&array), true);
    ck_assert_uint_eq(array.size, 0);
    ck_assert_uint_eq(array.capacity, dynArrayTestCapacityMin);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 1);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_clear_failure_notShrinked)
{
    DynStringTestArray array;
    array.size = dynArrayTestCapacityMin + 1;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = dynArrayTestCapacityMin * dynArrayTestCapacityMul;
    dynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    dynArrayTestReallocFail = true;
    ck_assert_uint_eq(dynArrayClear(&array), false);
    ck_assert_uint_eq(array.size, 0);
    ck_assert_uint_eq(array.capacity, dynArrayTestCapacityMin * dynArrayTestCapacityMul);
    ck_assert_ptr_nonnull(array.buffer);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_clear_failure_nullptr)
{
    DynStringTestArray* arrayPtr = nullptr;
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayClear(arrayPtr), false);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
}
END_TEST


START_TEST(test_dynarray_insert_success_zeroIndex)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 8;
    dynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "1234", 5);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayInsert(&array, 0, '0'), true);
    ck_assert_uint_eq(array.size, 6);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_int_eq(array.buffer[0], '0');
    ck_assert_str_eq(array.buffer, "01234");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_insert_success_middleIndex)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 8;
    dynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "0134", 5);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayInsert(&array, 2, '2'), true);
    ck_assert_uint_eq(array.size, 6);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_int_eq(array.buffer[2], '2');
    ck_assert_str_eq(array.buffer, "01234");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_insert_success_endIndex)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 8;
    dynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "01234", 5);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayInsert(&array, 5, '\0'), true);
    ck_assert_uint_eq(array.size, 6);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_int_eq(array.buffer[5], '\0');
    ck_assert_str_eq(array.buffer, "01234");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_insert_success_beyondEndIndex)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 8;
    dynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "01234", 5);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayInsert(&array, 666, '\0'), true);
    ck_assert_uint_eq(array.size, 6);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_int_eq(array.buffer[5], '\0');
    ck_assert_str_eq(array.buffer, "01234");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_insert_success_bufferExpanded)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 5;
    dynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "0134", 5);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayInsert(&array, 2, '2'), true);
    ck_assert_uint_eq(array.size, 6);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_int_eq(array.buffer[2], '2');
    ck_assert_str_eq(array.buffer, "01234");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 1);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_insert_failure_negativeIndex)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 8;
    dynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "0123", 5);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayInsert(&array, -1, '-'), false);
    ck_assert_uint_eq(array.size, 5);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_insert_failure_bufferExpansionFailed)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 5;
    dynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "0134", 5);
    dynArrayTestReallocFail = true;
    ck_assert_uint_eq(dynArrayInsert(&array, 2, '2'), false);
    ck_assert_uint_eq(array.size, 5);
    ck_assert_uint_eq(array.capacity, 5);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0134");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_insert_failure_nullptr)
{
    DynStringTestArray* arrayPtr = nullptr;
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayInsert(arrayPtr, 0, '0'), false);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
}
END_TEST


START_TEST(test_dynarray_insertArray_success_zeroIndex)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 8;
    dynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "3456", 5);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayInsertArray(&array, 0, "012", 3), true);
    ck_assert_uint_eq(array.size, 8);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123456");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_insertArray_success_middleIndex)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 8;
    dynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "0156", 5);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayInsertArray(&array, 2, "234", 3), true);
    ck_assert_uint_eq(array.size, 8);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123456");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_insertArray_success_endIndex)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 8;
    dynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "01234", 5);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayInsertArray(&array, 5, "56\0", 3), true);
    ck_assert_uint_eq(array.size, 8);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123456");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_insertArray_success_beyondEndIndex)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 8;
    dynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "01234", 5);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayInsertArray(&array, 666, "56\0", 3), true);
    ck_assert_uint_eq(array.size, 8);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123456");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_insertArray_success_bufferExpanded)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 5;
    dynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "0156", 5);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayInsertArray(&array, 2, "234", 3), true);
    ck_assert_uint_eq(array.size, 8);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123456");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 1);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_insertArray_success_nullptrArray)
{
    DynStringTestArray array;
    char* nullptrArray = nullptr;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 5;
    dynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "0156", 5);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayInsertArray(&array, 2, nullptrArray, 3), true);
    ck_assert_uint_eq(array.size, 5);
    ck_assert_uint_eq(array.capacity, 5);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0156");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_insertArray_success_zeroArraySize)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 5;
    dynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "0156", 5);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayInsertArray(&array, 2, "234", 0), true);
    ck_assert_uint_eq(array.size, 5);
    ck_assert_uint_eq(array.capacity, 5);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0156");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_insertArray_failure_negativeIndex)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 8;
    dynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "0123", 5);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayInsertArray(&array, -1, "---", 3), false);
    ck_assert_uint_eq(array.size, 5);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_insertArray_failure_bufferExpansionFailed)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 5;
    dynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "0156", 5);
    dynArrayTestReallocFail = true;
    ck_assert_uint_eq(dynArrayInsertArray(&array, 2, "234", 3), false);
    ck_assert_uint_eq(array.size, 5);
    ck_assert_uint_eq(array.capacity, 5);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0156");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_insertArray_failure_nullptrDestArray)
{
    DynStringTestArray* arrayPtr = nullptr;
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayInsertArray(arrayPtr, 0, "012", 3), false);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
}
END_TEST


START_TEST(test_dynarray_insertDynArray_success_zeroIndex)
{
    DynStringTestArray destArray, srcArray;
    destArray.size = 5;
    destArray.calculateCapacity = calculateCapacityTest;
    destArray.capacity = 8;
    dynArrayTestMallocFail = false;
    destArray.buffer = malloc(destArray.capacity);
    memcpy(destArray.buffer, "3456", 5);
    srcArray.size = 3;
    srcArray.calculateCapacity = calculateCapacityTest;
    srcArray.capacity = 8;
    srcArray.buffer = malloc(srcArray.capacity);
    memcpy(srcArray.buffer, "012\0", 4);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayInsertDynArray(&destArray, 0, &srcArray), true);
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
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(destArray.buffer);
    free(srcArray.buffer);
}
END_TEST
START_TEST(test_dynarray_insertDynArray_success_middleIndex)
{
    DynStringTestArray destArray, srcArray;
    destArray.size = 5;
    destArray.calculateCapacity = calculateCapacityTest;
    destArray.capacity = 8;
    dynArrayTestMallocFail = false;
    destArray.buffer = malloc(destArray.capacity);
    memcpy(destArray.buffer, "0156", 5);
    srcArray.size = 3;
    srcArray.calculateCapacity = calculateCapacityTest;
    srcArray.capacity = 8;
    srcArray.buffer = malloc(srcArray.capacity);
    memcpy(srcArray.buffer, "234\0", 4);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayInsertDynArray(&destArray, 2, &srcArray), true);
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
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(destArray.buffer);
    free(srcArray.buffer);
}
END_TEST
START_TEST(test_dynarray_insertDynArray_success_endIndex)
{
    DynStringTestArray destArray, srcArray;
    destArray.size = 5;
    destArray.calculateCapacity = calculateCapacityTest;
    destArray.capacity = 8;
    dynArrayTestMallocFail = false;
    destArray.buffer = malloc(destArray.capacity);
    memcpy(destArray.buffer, "01234", 5);
    srcArray.size = 3;
    srcArray.calculateCapacity = calculateCapacityTest;
    srcArray.capacity = 8;
    srcArray.buffer = malloc(srcArray.capacity);
    memcpy(srcArray.buffer, "56\0", 4);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayInsertDynArray(&destArray, 5, &srcArray), true);
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
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(destArray.buffer);
    free(srcArray.buffer);
}
END_TEST
START_TEST(test_dynarray_insertDynArray_success_beyondEndIndex)
{
    DynStringTestArray destArray, srcArray;
    destArray.size = 5;
    destArray.calculateCapacity = calculateCapacityTest;
    destArray.capacity = 8;
    dynArrayTestMallocFail = false;
    destArray.buffer = malloc(destArray.capacity);
    memcpy(destArray.buffer, "01234", 5);
    srcArray.size = 3;
    srcArray.calculateCapacity = calculateCapacityTest;
    srcArray.capacity = 8;
    srcArray.buffer = malloc(srcArray.capacity);
    memcpy(srcArray.buffer, "56", 3);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayInsertDynArray(&destArray, 666, &srcArray), true);
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
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(destArray.buffer);
    free(srcArray.buffer);
}
END_TEST
START_TEST(test_dynarray_insertDynArray_success_bufferExpanded)
{
    DynStringTestArray destArray, srcArray;
    destArray.size = 5;
    destArray.calculateCapacity = calculateCapacityTest;
    destArray.capacity = 5;
    dynArrayTestMallocFail = false;
    destArray.buffer = malloc(destArray.capacity);
    memcpy(destArray.buffer, "0156", 5);
    srcArray.size = 3;
    srcArray.calculateCapacity = calculateCapacityTest;
    srcArray.capacity = 8;
    srcArray.buffer = malloc(srcArray.capacity);
    memcpy(srcArray.buffer, "234", 4);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayInsertDynArray(&destArray, 2, &srcArray), true);
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
    ck_assert_uint_eq(dynArrayTestReallocCount, 1);
    free(destArray.buffer);
    free(srcArray.buffer);
}
END_TEST
START_TEST(test_dynarray_insertDynArray_success_nullptrSrcArray)
{
    DynStringTestArray destArray, *srcArray = nullptr;
    destArray.size = 5;
    destArray.calculateCapacity = calculateCapacityTest;
    destArray.capacity = 5;
    dynArrayTestMallocFail = false;
    destArray.buffer = malloc(destArray.capacity);
    memcpy(destArray.buffer, "0156", 5);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayInsertDynArray(&destArray, 2, srcArray), true);
    ck_assert_uint_eq(destArray.size, 5);
    ck_assert_uint_eq(destArray.capacity, 5);
    ck_assert_ptr_nonnull(destArray.buffer);
    ck_assert_str_eq(destArray.buffer, "0156");
    ck_assert_ptr_nonnull(destArray.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(destArray.buffer);
}
END_TEST
START_TEST(test_dynarray_insertDynArray_success_zeroSizeSrcArray)
{
    DynStringTestArray destArray, srcArray;
    destArray.size = 5;
    destArray.calculateCapacity = calculateCapacityTest;
    destArray.capacity = 8;
    dynArrayTestMallocFail = false;
    destArray.buffer = malloc(destArray.capacity);
    memcpy(destArray.buffer, "0156", 5);
    srcArray.size = 0;
    srcArray.calculateCapacity = calculateCapacityTest;
    srcArray.capacity = 8;
    srcArray.buffer = malloc(srcArray.capacity);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayInsertDynArray(&destArray, 2, &srcArray), true);
    ck_assert_uint_eq(destArray.size, 5);
    ck_assert_uint_eq(destArray.capacity, 8);
    ck_assert_ptr_nonnull(destArray.buffer);
    ck_assert_str_eq(destArray.buffer, "0156");
    ck_assert_ptr_nonnull(destArray.calculateCapacity);
    ck_assert_uint_eq(srcArray.size, 0);
    ck_assert_uint_eq(srcArray.capacity, 8);
    ck_assert_ptr_nonnull(srcArray.buffer);
    ck_assert_ptr_nonnull(srcArray.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(destArray.buffer);
    free(srcArray.buffer);
}
END_TEST
START_TEST(test_dynarray_insertDynArray_failure_negativeIndex)
{
    DynStringTestArray destArray, srcArray;
    destArray.size = 5;
    destArray.calculateCapacity = calculateCapacityTest;
    destArray.capacity = 8;
    dynArrayTestMallocFail = false;
    destArray.buffer = malloc(destArray.capacity);
    memcpy(destArray.buffer, "0123", 5);
    srcArray.size = 3;
    srcArray.calculateCapacity = calculateCapacityTest;
    srcArray.capacity = 8;
    srcArray.buffer = malloc(srcArray.capacity);
    memcpy(srcArray.buffer, "---", 4);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayInsertDynArray(&destArray, -1, &srcArray), false);
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
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(destArray.buffer);
    free(srcArray.buffer);
}
END_TEST
START_TEST(test_dynarray_insertDynArray_failure_bufferExpansionFailed)
{
    DynStringTestArray destArray, srcArray;
    destArray.size = 5;
    destArray.calculateCapacity = calculateCapacityTest;
    destArray.capacity = 5;
    dynArrayTestMallocFail = false;
    destArray.buffer = malloc(destArray.capacity);
    memcpy(destArray.buffer, "0156", 5);
    srcArray.size = 3;
    srcArray.calculateCapacity = calculateCapacityTest;
    srcArray.capacity = 8;
    srcArray.buffer = malloc(srcArray.capacity);
    memcpy(srcArray.buffer, "234", 4);
    dynArrayTestReallocFail = true;
    ck_assert_uint_eq(dynArrayInsertDynArray(&destArray, 2, &srcArray), false);
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
START_TEST(test_dynarray_insertDynArray_failure_nullptrDestArray)
{
    DynStringTestArray srcArray, *destArrayPtr = nullptr;
    srcArray.size = 3;
    srcArray.calculateCapacity = calculateCapacityTest;
    srcArray.capacity = 8;
    srcArray.buffer = malloc(srcArray.capacity);
    memcpy(srcArray.buffer, "012", 4);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayInsertDynArray(destArrayPtr, 0, &srcArray), false);
    ck_assert_uint_eq(srcArray.size, 3);
    ck_assert_uint_eq(srcArray.capacity, 8);
    ck_assert_ptr_nonnull(srcArray.buffer);
    ck_assert_str_eq(srcArray.buffer, "012");
    ck_assert_ptr_nonnull(srcArray.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(srcArray.buffer);
}
END_TEST


START_TEST(test_dynarray_add_success_enoughCapacity)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 8;
    dynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "01234", 5);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayAdd(&array, '\0'), true);
    ck_assert_uint_eq(array.size, 6);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_int_eq(array.buffer[array.size - 1], '\0');
    ck_assert_str_eq(array.buffer, "01234");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_add_success_notEnoughCapacity)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 5;
    dynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "01234", 5);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayAdd(&array, '\0'), true);
    ck_assert_uint_eq(array.size, 6);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_int_eq(array.buffer[array.size - 1], '\0');
    ck_assert_str_eq(array.buffer, "01234");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 1);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_add_failure_bufferExpansionFailed)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 5;
    dynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity + 1);
    memcpy(array.buffer, "01234", 6);
    dynArrayTestReallocFail = true;
    ck_assert_uint_eq(dynArrayAdd(&array, '\0'), false);
    ck_assert_uint_eq(array.size, 5);
    ck_assert_uint_eq(array.capacity, 5);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "01234");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_add_failure_nullptr)
{
    DynStringTestArray* arrayPtr = nullptr;
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayAdd(arrayPtr, '\0'), false);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
}
END_TEST


START_TEST(test_dynarray_addArray_success_enoughCapacity)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 8;
    dynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "01234", 5);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayAddArray(&array, "56\0", 3), true);
    ck_assert_uint_eq(array.size, 8);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123456");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_addArray_success_notEnoughCapacity)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 5;
    dynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "01234", 5);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayAddArray(&array, "56\0", 3), true);
    ck_assert_uint_eq(array.size, 8);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123456");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 1);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_addArray_success_nullptrArray)
{
    DynStringTestArray array;
    char* nullptrArray = nullptr;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 5;
    dynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "0156", 5);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayAddArray(&array, nullptrArray, 3), true);
    ck_assert_uint_eq(array.size, 5);
    ck_assert_uint_eq(array.capacity, 5);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0156");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_addArray_success_zeroArraySize)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 5;
    dynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "0156", 5);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayAddArray(&array, "234", 0), true);
    ck_assert_uint_eq(array.size, 5);
    ck_assert_uint_eq(array.capacity, 5);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0156");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_addArray_failure_bufferExpansionFailed)
{
    DynStringTestArray array;
    array.size = 5;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 5;
    dynArrayTestMallocFail = false;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "0123", 5);
    dynArrayTestReallocFail = true;
    ck_assert_uint_eq(dynArrayInsertArray(&array, 2, "45\0", 3), false);
    ck_assert_uint_eq(array.size, 5);
    ck_assert_uint_eq(array.capacity, 5);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_addArray_failure_nullptrDestArray)
{
    DynStringTestArray* arrayPtr = nullptr;
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayInsertArray(arrayPtr, 0, "012", 3), false);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
}
END_TEST


START_TEST(test_dynarray_addDynArray_success_enoughCapacity)
{
    DynStringTestArray destArray, srcArray;
    destArray.size = 5;
    destArray.calculateCapacity = calculateCapacityTest;
    destArray.capacity = 8;
    dynArrayTestMallocFail = false;
    destArray.buffer = malloc(destArray.capacity);
    memcpy(destArray.buffer, "01234", 5);
    srcArray.size = 3;
    srcArray.calculateCapacity = calculateCapacityTest;
    srcArray.capacity = 8;
    srcArray.buffer = malloc(srcArray.capacity);
    memcpy(srcArray.buffer, "56", 3);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayAddDynArray(&destArray, &srcArray), true);
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
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(destArray.buffer);
    free(srcArray.buffer);
}
END_TEST
START_TEST(test_dynarray_addDynArray_success_notEnoughCapacity)
{
    DynStringTestArray destArray, srcArray;
    destArray.size = 5;
    destArray.calculateCapacity = calculateCapacityTest;
    destArray.capacity = 5;
    dynArrayTestMallocFail = false;
    destArray.buffer = malloc(destArray.capacity);
    memcpy(destArray.buffer, "01234", 5);
    srcArray.size = 3;
    srcArray.calculateCapacity = calculateCapacityTest;
    srcArray.capacity = 8;
    srcArray.buffer = malloc(srcArray.capacity);
    memcpy(srcArray.buffer, "56", 3);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayAddDynArray(&destArray, &srcArray), true);
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
    ck_assert_uint_eq(dynArrayTestReallocCount, 1);
    free(destArray.buffer);
    free(srcArray.buffer);
}
END_TEST
START_TEST(test_dynarray_addDynArray_success_nullptrSrcArray)
{
    DynStringTestArray destArray, *srcArray = nullptr;
    destArray.size = 5;
    destArray.calculateCapacity = calculateCapacityTest;
    destArray.capacity = 5;
    dynArrayTestMallocFail = false;
    destArray.buffer = malloc(destArray.capacity);
    memcpy(destArray.buffer, "0123", 5);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayAddDynArray(&destArray, srcArray), true);
    ck_assert_uint_eq(destArray.size, 5);
    ck_assert_uint_eq(destArray.capacity, 5);
    ck_assert_ptr_nonnull(destArray.buffer);
    ck_assert_str_eq(destArray.buffer, "0123");
    ck_assert_ptr_nonnull(destArray.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(destArray.buffer);
}
END_TEST
START_TEST(test_dynarray_addDynArray_success_zeroSizeSrcArray)
{
    DynStringTestArray destArray, srcArray;
    destArray.size = 5;
    destArray.calculateCapacity = calculateCapacityTest;
    destArray.capacity = 8;
    dynArrayTestMallocFail = false;
    destArray.buffer = malloc(destArray.capacity);
    memcpy(destArray.buffer, "0123", 5);
    srcArray.size = 0;
    srcArray.calculateCapacity = calculateCapacityTest;
    srcArray.capacity = 8;
    srcArray.buffer = malloc(srcArray.capacity);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayAddDynArray(&destArray, &srcArray), true);
    ck_assert_uint_eq(destArray.size, 5);
    ck_assert_uint_eq(destArray.capacity, 8);
    ck_assert_ptr_nonnull(destArray.buffer);
    ck_assert_str_eq(destArray.buffer, "0123");
    ck_assert_ptr_nonnull(destArray.calculateCapacity);
    ck_assert_uint_eq(srcArray.size, 0);
    ck_assert_uint_eq(srcArray.capacity, 8);
    ck_assert_ptr_nonnull(srcArray.buffer);
    ck_assert_ptr_nonnull(srcArray.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(destArray.buffer);
    free(srcArray.buffer);
}
END_TEST
START_TEST(test_dynarray_addDynArray_failure_bufferExpansionFailed)
{
    DynStringTestArray destArray, srcArray;
    destArray.size = 5;
    destArray.calculateCapacity = calculateCapacityTest;
    destArray.capacity = 5;
    dynArrayTestMallocFail = false;
    destArray.buffer = malloc(destArray.capacity);
    memcpy(destArray.buffer, "0156", 5);
    srcArray.size = 3;
    srcArray.calculateCapacity = calculateCapacityTest;
    srcArray.capacity = 8;
    srcArray.buffer = malloc(srcArray.capacity);
    memcpy(srcArray.buffer, "234", 4);
    dynArrayTestReallocFail = true;
    ck_assert_uint_eq(dynArrayAddDynArray(&destArray, &srcArray), false);
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
START_TEST(test_dynarray_addDynArray_failure_nullptrDestArray)
{
    DynStringTestArray srcArray, *destArrayPtr = nullptr;
    srcArray.size = 3;
    srcArray.calculateCapacity = calculateCapacityTest;
    srcArray.capacity = 8;
    srcArray.buffer = malloc(srcArray.capacity);
    memcpy(srcArray.buffer, "012", 4);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArrayAddDynArray(destArrayPtr, &srcArray), false);
    ck_assert_uint_eq(srcArray.size, 3);
    ck_assert_uint_eq(srcArray.capacity, 8);
    ck_assert_ptr_nonnull(srcArray.buffer);
    ck_assert_str_eq(srcArray.buffer, "012");
    ck_assert_ptr_nonnull(srcArray.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(srcArray.buffer);
}
END_TEST


START_TEST(test_dynarray_set_success_indexInBounds)
{
    DynStringTestArray array;
    array.size = 4;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 4;
    array.buffer = malloc(array.capacity);
    memcpy(array.buffer, "012", 4);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArraySet(&array, 0, '2'), true);
    ck_assert_uint_eq(dynArraySet(&array, 1, '1'), true);
    ck_assert_uint_eq(dynArraySet(&array, 2, '0'), true);
    ck_assert_uint_eq(array.size, 4);
    ck_assert_uint_eq(array.capacity, 4);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "210");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_set_success_indexBeyondSize)
{
    DynStringTestArray array;
    array.size = 3;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 4;
    array.buffer = malloc(array.capacity + 1);
    memcpy(array.buffer, "012\0\0", 5);
    dynArrayTestReallocFail = true;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArraySet(&array, 3, '3'), true);
    ck_assert_uint_eq(array.size, 4);
    ck_assert_uint_eq(array.capacity, 4);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_set_success_indexBeyondSize_bufferExpanded)
{
    DynStringTestArray array;
    array.size = 3;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 3;
    array.buffer = malloc(array.capacity + 2);
    memcpy(array.buffer, "012\0\0", 5);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArraySet(&array, 3, '3'), true);
    ck_assert_uint_eq(array.size, 4);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 1);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_set_failure_indexBeyondSize_bufferExpansionFailed)
{
    DynStringTestArray array;
    array.size = 3;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 3;
    array.buffer = malloc(array.capacity + 1);
    memcpy(array.buffer, "012", 4);
    dynArrayTestReallocFail = true;
    ck_assert_uint_eq(dynArraySet(&array, 3, '3'), false);
    ck_assert_uint_eq(array.size, 3);
    ck_assert_uint_eq(array.capacity, 3);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "012");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_set_failure_nullptr)
{
    DynStringTestArray *arrayPtr = nullptr;
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    ck_assert_uint_eq(dynArraySet(arrayPtr, 0, '0'), false);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
}
END_TEST


START_TEST(test_dynarray_remove_indexRangeInBounds)
{
    DynStringTestArray array;
    array.size = 11;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 16;
    array.buffer = malloc(array.capacity + 1);
    memcpy(array.buffer, "0123456789", 11);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    dynArrayRemove(&array, 2, 6);
    ck_assert_uint_eq(array.size, 5);
    ck_assert_uint_eq(array.capacity, 16);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0189");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_remove_rangeBeyondBounds)
{
    DynStringTestArray array;
    array.size = 11;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 16;
    array.buffer = malloc(array.capacity + 1);
    memcpy(array.buffer, "0123456789", 11);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    dynArrayRemove(&array, 2, 100);
    ck_assert_uint_eq(array.size, 2);
    ck_assert_uint_eq(array.capacity, 16);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_remove_zeroRange)
{
    DynStringTestArray array;
    array.size = 11;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 16;
    array.buffer = malloc(array.capacity + 1);
    memcpy(array.buffer, "0123456789", 11);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    dynArrayRemove(&array, 2, 0);
    ck_assert_uint_eq(array.size, 11);
    ck_assert_uint_eq(array.capacity, 16);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123456789");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_remove_indexBeyoundBounds)
{
    DynStringTestArray array;
    array.size = 11;
    array.calculateCapacity = calculateCapacityTest;
    array.capacity = 16;
    array.buffer = malloc(array.capacity + 1);
    memcpy(array.buffer, "0123456789", 11);
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    dynArrayRemove(&array, 13, 5);
    ck_assert_uint_eq(array.size, 11);
    ck_assert_uint_eq(array.capacity, 16);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123456789");
    ck_assert_ptr_nonnull(array.calculateCapacity);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
    free(array.buffer);
}
END_TEST
START_TEST(test_dynarray_remove_nullptr)
{
    DynStringTestArray *arrayPtr = nullptr;
    dynArrayTestReallocFail = false;
    dynArrayTestReallocCount = 0;
    dynArrayRemove(arrayPtr, 5, 10);
    ck_assert_uint_eq(dynArrayTestReallocCount, 0);
}
END_TEST



Suite* test_suite_dynarray()
{
    Suite *s;
    TCase *test_case_dynarray_construct_destruct, *test_case_dynarray_size, *test_case_dynarray_reserve,
          *test_case_dynarray_shrinkToFit, *test_case_dynarray_clear, *test_case_dynarray_insert,
          *test_case_dynarray_insertArray, *test_case_dynarray_insertDynArray, *test_case_dynarray_add,
          *test_case_dynarray_addArray, *test_case_dynarray_addDynArray, *test_case_dynarray_set,
          *test_case_dynarray_remove;

    s = suite_create("Dynamic Array Test Suite");

    test_case_dynarray_construct_destruct = tcase_create("Dynamic Array Test Case: construct/destruct");
    tcase_add_test(test_case_dynarray_construct_destruct, test_dynarray_construct_destruct_valid);
    tcase_add_test(test_case_dynarray_construct_destruct, test_dynarray_construct_noMemoryAvailable);
    tcase_add_test(test_case_dynarray_construct_destruct, test_dynarray_destruct_nullptr);
    suite_add_tcase(s, test_case_dynarray_construct_destruct);

    test_case_dynarray_size = tcase_create("Dynamic Array Test Case: size");
    tcase_add_test(test_case_dynarray_size, test_dynarray_size_valid);
    tcase_add_test(test_case_dynarray_size, test_dynarray_size_nullptr);
    suite_add_tcase(s, test_case_dynarray_size);

    test_case_dynarray_reserve = tcase_create("Dynamic Array Test Case: reserve");
    tcase_add_test(test_case_dynarray_reserve, test_dynarray_reserve_success_enoughCapacityBufferNotNull);
    tcase_add_test(test_case_dynarray_reserve, test_dynarray_reserve_success_enoughCapacityBufferNull);
    tcase_add_test(test_case_dynarray_reserve, test_dynarray_reserve_success_notEnoughCapacity);
    tcase_add_test(test_case_dynarray_reserve, test_dynarray_reserve_failure_biggerThanMaxCapacity);
    tcase_add_test(test_case_dynarray_reserve, test_dynarray_reserve_failure_noMemoryAvailable);
    tcase_add_test(test_case_dynarray_reserve, test_dynarray_reserve_failure_calculateCapacityNull);
    tcase_add_test(test_case_dynarray_reserve, test_dynarray_reserve_failure_nullptr);
    suite_add_tcase(s, test_case_dynarray_reserve);

    test_case_dynarray_shrinkToFit = tcase_create("Dynamic Array Test Case: shrinkToFit");
    tcase_add_test(test_case_dynarray_shrinkToFit, test_dynarray_shrinkToFit_success_hasLeastCapacityBufferNotNull);
    tcase_add_test(test_case_dynarray_shrinkToFit, test_dynarray_shrinkToFit_success_hasLeastCapacityBufferNull);
    tcase_add_test(test_case_dynarray_shrinkToFit, test_dynarray_shrinkToFit_success_hasNotLeastCapacity);
    tcase_add_test(test_case_dynarray_shrinkToFit, test_dynarray_shrinkToFit_failure_noMemoryAvailable);
    tcase_add_test(test_case_dynarray_shrinkToFit, test_dynarray_shrinkToFit_failure_calculateCapacityNull);
    tcase_add_test(test_case_dynarray_shrinkToFit, test_dynarray_shrinkToFit_failure_nullptr);
    suite_add_tcase(s, test_case_dynarray_shrinkToFit);

    test_case_dynarray_clear = tcase_create("Dynamic Array Test Case: clear");
    tcase_add_test(test_case_dynarray_clear, test_dynarray_clear_success_shrinked);
    tcase_add_test(test_case_dynarray_clear, test_dynarray_clear_failure_notShrinked);
    tcase_add_test(test_case_dynarray_clear, test_dynarray_clear_failure_nullptr);
    suite_add_tcase(s, test_case_dynarray_clear);

    test_case_dynarray_insert = tcase_create("Dynamic Array Test Case: insert");
    tcase_add_test(test_case_dynarray_insert, test_dynarray_insert_success_zeroIndex);
    tcase_add_test(test_case_dynarray_insert, test_dynarray_insert_success_middleIndex);
    tcase_add_test(test_case_dynarray_insert, test_dynarray_insert_success_endIndex);
    tcase_add_test(test_case_dynarray_insert, test_dynarray_insert_success_beyondEndIndex);
    tcase_add_test(test_case_dynarray_insert, test_dynarray_insert_success_bufferExpanded);
    tcase_add_test(test_case_dynarray_insert, test_dynarray_insert_failure_negativeIndex);
    tcase_add_test(test_case_dynarray_insert, test_dynarray_insert_failure_bufferExpansionFailed);
    tcase_add_test(test_case_dynarray_insert, test_dynarray_insert_failure_nullptr);
    suite_add_tcase(s, test_case_dynarray_insert);

    test_case_dynarray_insertArray = tcase_create("Dynamic Array Test Case: insertArray");
    tcase_add_test(test_case_dynarray_insertArray, test_dynarray_insertArray_success_zeroIndex);
    tcase_add_test(test_case_dynarray_insertArray, test_dynarray_insertArray_success_middleIndex);
    tcase_add_test(test_case_dynarray_insertArray, test_dynarray_insertArray_success_endIndex);
    tcase_add_test(test_case_dynarray_insertArray, test_dynarray_insertArray_success_beyondEndIndex);
    tcase_add_test(test_case_dynarray_insertArray, test_dynarray_insertArray_success_bufferExpanded);
    tcase_add_test(test_case_dynarray_insertArray, test_dynarray_insertArray_success_nullptrArray);
    tcase_add_test(test_case_dynarray_insertArray, test_dynarray_insertArray_success_zeroArraySize);
    tcase_add_test(test_case_dynarray_insertArray, test_dynarray_insertArray_failure_negativeIndex);
    tcase_add_test(test_case_dynarray_insertArray, test_dynarray_insertArray_failure_bufferExpansionFailed);
    tcase_add_test(test_case_dynarray_insertArray, test_dynarray_insertArray_failure_nullptrDestArray);
    suite_add_tcase(s, test_case_dynarray_insertArray);

    test_case_dynarray_insertDynArray = tcase_create("Dynamic Array Test Case: insertDynArray");
    tcase_add_test(test_case_dynarray_insertDynArray, test_dynarray_insertDynArray_success_zeroIndex);
    tcase_add_test(test_case_dynarray_insertDynArray, test_dynarray_insertDynArray_success_middleIndex);
    tcase_add_test(test_case_dynarray_insertDynArray, test_dynarray_insertDynArray_success_endIndex);
    tcase_add_test(test_case_dynarray_insertDynArray, test_dynarray_insertDynArray_success_beyondEndIndex);
    tcase_add_test(test_case_dynarray_insertDynArray, test_dynarray_insertDynArray_success_bufferExpanded);
    tcase_add_test(test_case_dynarray_insertDynArray, test_dynarray_insertDynArray_success_nullptrSrcArray);
    tcase_add_test(test_case_dynarray_insertDynArray, test_dynarray_insertDynArray_success_zeroSizeSrcArray);
    tcase_add_test(test_case_dynarray_insertDynArray, test_dynarray_insertDynArray_failure_negativeIndex);
    tcase_add_test(test_case_dynarray_insertDynArray, test_dynarray_insertDynArray_failure_bufferExpansionFailed);
    tcase_add_test(test_case_dynarray_insertDynArray, test_dynarray_insertDynArray_failure_nullptrDestArray);
    suite_add_tcase(s, test_case_dynarray_insertDynArray);

    test_case_dynarray_add = tcase_create("Dynamic Array Test Case: add");
    tcase_add_test(test_case_dynarray_add, test_dynarray_add_success_enoughCapacity);
    tcase_add_test(test_case_dynarray_add, test_dynarray_add_success_notEnoughCapacity);
    tcase_add_test(test_case_dynarray_add, test_dynarray_add_failure_bufferExpansionFailed);
    tcase_add_test(test_case_dynarray_add, test_dynarray_add_failure_nullptr);
    suite_add_tcase(s, test_case_dynarray_add);

    test_case_dynarray_addArray = tcase_create("Dynamic Array Test Case: addArray");
    tcase_add_test(test_case_dynarray_addArray, test_dynarray_addArray_success_enoughCapacity);
    tcase_add_test(test_case_dynarray_addArray, test_dynarray_addArray_success_notEnoughCapacity);
    tcase_add_test(test_case_dynarray_addArray, test_dynarray_addArray_success_nullptrArray);
    tcase_add_test(test_case_dynarray_addArray, test_dynarray_addArray_success_zeroArraySize);
    tcase_add_test(test_case_dynarray_addArray, test_dynarray_addArray_failure_bufferExpansionFailed);
    tcase_add_test(test_case_dynarray_addArray, test_dynarray_addArray_failure_nullptrDestArray);
    suite_add_tcase(s, test_case_dynarray_addArray);

    test_case_dynarray_addDynArray = tcase_create("Dynamic Array Test Case: addDynArray");
    tcase_add_test(test_case_dynarray_addDynArray, test_dynarray_addDynArray_success_enoughCapacity);
    tcase_add_test(test_case_dynarray_addDynArray, test_dynarray_addDynArray_success_notEnoughCapacity);
    tcase_add_test(test_case_dynarray_addDynArray, test_dynarray_addDynArray_success_nullptrSrcArray);
    tcase_add_test(test_case_dynarray_addDynArray, test_dynarray_addDynArray_success_zeroSizeSrcArray);
    tcase_add_test(test_case_dynarray_addDynArray, test_dynarray_addDynArray_failure_bufferExpansionFailed);
    tcase_add_test(test_case_dynarray_addDynArray, test_dynarray_addDynArray_failure_nullptrDestArray);
    suite_add_tcase(s, test_case_dynarray_addDynArray);

    test_case_dynarray_set = tcase_create("Dynamic Array Test Case: set");
    tcase_add_test(test_case_dynarray_set, test_dynarray_set_success_indexInBounds);
    tcase_add_test(test_case_dynarray_set, test_dynarray_set_success_indexBeyondSize);
    tcase_add_test(test_case_dynarray_set, test_dynarray_set_success_indexBeyondSize_bufferExpanded);
    tcase_add_test(test_case_dynarray_set, test_dynarray_set_failure_indexBeyondSize_bufferExpansionFailed);
    tcase_add_test(test_case_dynarray_set, test_dynarray_set_failure_nullptr);
    suite_add_tcase(s, test_case_dynarray_set);

    test_case_dynarray_remove = tcase_create("Dynamic Array Test Case: remove");
    tcase_add_test(test_case_dynarray_remove, test_dynarray_remove_indexRangeInBounds);
    tcase_add_test(test_case_dynarray_remove, test_dynarray_remove_rangeBeyondBounds);
    tcase_add_test(test_case_dynarray_remove, test_dynarray_remove_zeroRange);
    tcase_add_test(test_case_dynarray_remove, test_dynarray_remove_indexBeyoundBounds);
    tcase_add_test(test_case_dynarray_remove, test_dynarray_remove_nullptr);
    suite_add_tcase(s, test_case_dynarray_remove);

    return s;
}


#include <stdio.h>

static size_t test() {
    printf("test function!");
    return 0;
}

size_t testSize = test();

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = test_suite_dynarray();
    sr = srunner_create(s);

    if(CHECK_DONT_FORK_DYNARRAY_TEST)
        srunner_set_fork_status(sr, CK_NOFORK);
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    test();
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}