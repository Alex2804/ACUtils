#include "check.h"

#include <string.h>
#include <stdlib.h>

#include "ACUtils/adynarray.h"

static size_t private_ACUtilsTest_ADynArray_reallocCount = 0;
static bool private_ACUtilsTest_ADynArray_reallocFail = false;
static size_t private_ACUtilsTest_ADynArray_freeCount = 0;

static void* private_ACUtilsTest_ADynArray_realloc(void* ptr, size_t size) {
    if(!private_ACUtilsTest_ADynArray_reallocFail) {
        void* tmp = realloc(ptr, size);
        if(tmp != NULL) {
            ++private_ACUtilsTest_ADynArray_reallocCount;
        }
        return tmp;
    } else {
        return NULL;
    }
}
static void private_ACUtilsTest_ADynArray_free(void* ptr) {
    if(ptr != NULL) {
        ++private_ACUtilsTest_ADynArray_freeCount;
    }
    free(ptr);
}

static const size_t private_ACUtilsTest_ADynArray_capacityMin = 8;
static const size_t private_ACUtilsTest_ADynArray_capacityMax = 32;
static const double private_ACUtilsTest_ADynArray_capacityMul = 2;

static size_t private_ACUtilsTest_ADynArray_growStrategy(size_t requiredSize, size_t typeSize) {
    (void) typeSize; /* suppress unused warning */
    return private_ACUtils_ADynArray_growStrategyGeneric(
            requiredSize, private_ACUtilsTest_ADynArray_capacityMin, private_ACUtilsTest_ADynArray_capacityMax, private_ACUtilsTest_ADynArray_capacityMul, 32);
}

struct private_ACUtilsTest_ADynArray_PointStruct
{
    double x, y;
};

A_DYNAMIC_ARRAY_DEFINITION(ADynTestPointArray, struct private_ACUtilsTest_ADynArray_PointStruct);
A_DYNAMIC_ARRAY_DEFINITION(ADynCharArray, char);

START_TEST(test_ADynArray_construct_destruct_valid)
{
    struct ADynCharArray* array;
    array = ADynArray_construct(struct ADynCharArray);
    ck_assert_uint_eq(array->size, 0);
    ck_assert_uint_eq(array->capacity, private_ACUtilsTest_ADynArray_capacityMin);
    ck_assert_ptr_nonnull(array->buffer);
    ck_assert_ptr_nonnull(array->growStrategy);
    ADynArray_destruct(array);
}
END_TEST
START_TEST(test_ADynArray_construct_destruct_withAllocator_valid)
{
    struct ADynCharArray* array;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    private_ACUtilsTest_ADynArray_reallocCount = private_ACUtilsTest_ADynArray_freeCount = 0;
    array = ADynArray_constructWithAllocator(struct ADynCharArray, private_ACUtilsTest_ADynArray_realloc, private_ACUtilsTest_ADynArray_free);
    ck_assert_uint_eq(array->size, 0);
    ck_assert_uint_eq(array->capacity, private_ACUtilsTest_ADynArray_capacityMin);
    ck_assert_ptr_nonnull(array->buffer);
    ck_assert_ptr_nonnull(array->growStrategy);
    ADynArray_destruct(array);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, private_ACUtilsTest_ADynArray_freeCount);
}
END_TEST
START_TEST(test_ADynArray_construct_destruct_withAllocator_invalid)
{
    struct ADynCharArray* array;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    private_ACUtilsTest_ADynArray_reallocCount = private_ACUtilsTest_ADynArray_freeCount = 0;
    array = ADynArray_constructWithAllocator(struct ADynCharArray, NULL, private_ACUtilsTest_ADynArray_free);
    ck_assert_ptr_null(array);
    array = ADynArray_constructWithAllocator(struct ADynCharArray, private_ACUtilsTest_ADynArray_realloc, NULL);
    ck_assert_ptr_null(array);
}
END_TEST
START_TEST(test_ADynArray_construct_destruct_noMemoryAvailable)
{
    struct ADynCharArray* array;
    private_ACUtilsTest_ADynArray_reallocFail = true;
    private_ACUtilsTest_ADynArray_reallocCount = private_ACUtilsTest_ADynArray_freeCount = 0;
    array = ADynArray_constructWithAllocator(struct ADynCharArray, private_ACUtilsTest_ADynArray_realloc,
                                             private_ACUtilsTest_ADynArray_free);
    ck_assert_ptr_null(array);
    ADynArray_destruct(array); /* should do nothing */
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_freeCount, 0);
}
END_TEST
START_TEST(test_ADynArray_construct_destruct_nullptr)
{
    struct ADynCharArray* array = NULL;
    private_ACUtilsTest_ADynArray_freeCount = 0;
    ADynArray_destruct(array); /* should do nothing */
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_freeCount, 0);
}
END_TEST


START_TEST(test_ADynArray_setGrowStrategy)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.capacity = 0;
    array.buffer = NULL;
    array.growStrategy = NULL;
    array.size = 42;
    ADynArray_setGrowStrategy(&array, private_ACUtilsTest_ADynArray_growStrategy);
    ck_assert_ptr_eq(array.growStrategy, private_ACUtilsTest_ADynArray_growStrategy);
    ADynArray_setGrowStrategy(&array, NULL);
    ck_assert_ptr_null(array.growStrategy);
    struct ADynCharArray* arrayPtr = NULL;
    ADynArray_setGrowStrategy(arrayPtr, private_ACUtilsTest_ADynArray_growStrategy);
}


START_TEST(test_ADynArray_size_valid)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.capacity = 0;
    array.buffer = NULL;
    array.growStrategy = NULL;
    array.size = 42;
    ck_assert_uint_eq(ADynArray_size(&array), 42);
    array.size = 13;
    ck_assert_uint_eq(ADynArray_size(&array), 13);
    array.size = 0;
    ck_assert_uint_eq(ADynArray_size(&array), 0);
}
START_TEST(test_ADynArray_size_nullptr)
{
    struct ADynCharArray* arrayPtr = NULL;
    ck_assert_uint_eq(ADynArray_size(arrayPtr), 0);
}


START_TEST(test_ADynArray_capacity_valid)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 0;
    array.buffer = NULL;
    array.growStrategy = NULL;
    array.capacity = 42;
    ck_assert_uint_eq(ADynArray_capacity(&array), 42);
    array.capacity = 13;
    ck_assert_uint_eq(ADynArray_capacity(&array), 13);
    array.capacity = 0;
    ck_assert_uint_eq(ADynArray_capacity(&array), 0);
}
START_TEST(test_ADynArray_capacity_nullptr)
{
    struct ADynCharArray* arrayPtr = NULL;
    ck_assert_uint_eq(ADynArray_capacity(arrayPtr), 0);
}


START_TEST(test_ADynArray_reserve_success_enoughCapacityBufferNotNull)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 0;
    array.growStrategy = NULL;
    array.capacity = private_ACUtilsTest_ADynArray_capacityMin;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity);
    private_ACUtilsTest_ADynArray_reallocFail = true;
    ck_assert_uint_eq(ADynArray_reserve(&array, array.capacity), true);
    ck_assert_uint_eq(array.size, 0);
    ck_assert_uint_eq(array.capacity, private_ACUtilsTest_ADynArray_capacityMin);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_ptr_null(array.growStrategy);
    array.deallocator(array.buffer);
}
START_TEST(test_ADynArray_reserve_success_enoughCapacityBufferNull)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 0;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = private_ACUtilsTest_ADynArray_capacityMin;
    array.buffer = NULL;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_reserve(&array, array.capacity), true);
    ck_assert_uint_eq(array.size, 0);
    ck_assert_uint_eq(array.capacity, private_ACUtilsTest_ADynArray_capacityMin);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_ptr_nonnull(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 1);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_reserve_success_notEnoughCapacity)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 0;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = private_ACUtilsTest_ADynArray_capacityMin;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_reserve(&array, private_ACUtilsTest_ADynArray_capacityMax), true);
    ck_assert_uint_eq(array.size, 0);
    ck_assert_uint_eq(array.capacity, private_ACUtilsTest_ADynArray_capacityMax);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_ptr_nonnull(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 1);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_reserve_failure_biggerThanMaxCapacity)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 0;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = private_ACUtilsTest_ADynArray_capacityMin;
    array.buffer = NULL;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_reserve(&array, private_ACUtilsTest_ADynArray_capacityMax + 1), false);
    ck_assert_uint_eq(array.size, 0);
    ck_assert_uint_eq(array.capacity, private_ACUtilsTest_ADynArray_capacityMin);
    ck_assert_ptr_null(array.buffer);
    ck_assert_ptr_nonnull(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
}
END_TEST
START_TEST(test_ADynArray_reserve_failure_noMemoryAvailable)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 0;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = private_ACUtilsTest_ADynArray_capacityMin;
    array.buffer = NULL;
    private_ACUtilsTest_ADynArray_reallocFail = true;
    ck_assert_uint_eq(ADynArray_reserve(&array, private_ACUtilsTest_ADynArray_capacityMax), false);
    ck_assert_uint_eq(array.size, 0);
    ck_assert_uint_eq(array.capacity, private_ACUtilsTest_ADynArray_capacityMin);
    ck_assert_ptr_null(array.buffer);
    ck_assert_ptr_nonnull(array.growStrategy);
}
END_TEST
START_TEST(test_ADynArray_reserve_failure_growStrategyNull)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 0;
    array.growStrategy = NULL;
    array.capacity = private_ACUtilsTest_ADynArray_capacityMin;
    array.buffer = NULL;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_reserve(&array, private_ACUtilsTest_ADynArray_capacityMax + 5), true);
    ck_assert_uint_eq(array.size, 0);
    ck_assert_uint_eq(array.capacity, private_ACUtilsTest_ADynArray_capacityMax + 5);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_ptr_null(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 1);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_reserve_failure_nullptr)
{
    struct ADynCharArray* arrayPtr = NULL;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_reserve(arrayPtr, private_ACUtilsTest_ADynArray_capacityMax), false);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
}
END_TEST


START_TEST(test_ADynArray_shrinkToFit_success_hasLeastCapacityBufferNotNull)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = private_ACUtilsTest_ADynArray_capacityMin - 1;
    array.capacity = private_ACUtilsTest_ADynArray_capacityMin;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity);
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_shrinkToFit(&array), true);
    ck_assert_uint_eq(array.size, private_ACUtilsTest_ADynArray_capacityMin - 1);
    ck_assert_uint_eq(array.capacity, private_ACUtilsTest_ADynArray_capacityMin);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_ptr_nonnull(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    array.deallocator(array.buffer);
}
START_TEST(test_ADynArray_shrinkToFit_success_hasLeastCapacityBufferNull)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = private_ACUtilsTest_ADynArray_capacityMin - 1;
    array.capacity = private_ACUtilsTest_ADynArray_capacityMin;
    array.buffer = NULL;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_shrinkToFit(&array), true);
    ck_assert_uint_eq(array.size, private_ACUtilsTest_ADynArray_capacityMin - 1);
    ck_assert_uint_eq(array.capacity, private_ACUtilsTest_ADynArray_capacityMin);
    ck_assert_ptr_null(array.buffer);
    ck_assert_ptr_nonnull(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_shrinkToFit_success_hasNotLeastCapacity)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = private_ACUtilsTest_ADynArray_capacityMin - 1;
    array.capacity = private_ACUtilsTest_ADynArray_growStrategy(private_ACUtilsTest_ADynArray_capacityMin + 1, sizeof(char));
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity);
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_shrinkToFit(&array), true);
    ck_assert_uint_eq(array.size, private_ACUtilsTest_ADynArray_capacityMin - 1);
    ck_assert_uint_eq(array.capacity, private_ACUtilsTest_ADynArray_capacityMin);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_ptr_nonnull(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 1);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_shrinkToFit_failure_noMemoryAvailable)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = private_ACUtilsTest_ADynArray_capacityMin - 1;
    array.capacity = private_ACUtilsTest_ADynArray_growStrategy(private_ACUtilsTest_ADynArray_capacityMin + 1, sizeof(char));
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity);
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    private_ACUtilsTest_ADynArray_reallocFail = true;
    ck_assert_uint_eq(ADynArray_shrinkToFit(&array), false);
    ck_assert_uint_eq(array.size, private_ACUtilsTest_ADynArray_capacityMin - 1);
    ck_assert_uint_eq(array.capacity, private_ACUtilsTest_ADynArray_growStrategy(private_ACUtilsTest_ADynArray_capacityMin + 1, sizeof(char)));
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_ptr_nonnull(array.growStrategy);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_shrinkToFit_failure_growStrategyNull)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = private_ACUtilsTest_ADynArray_capacityMin - 1;
    array.capacity = private_ACUtilsTest_ADynArray_growStrategy(private_ACUtilsTest_ADynArray_capacityMin + 1, sizeof(char));
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity);
    array.growStrategy = NULL;
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_shrinkToFit(&array), false);
    ck_assert_uint_eq(array.size, private_ACUtilsTest_ADynArray_capacityMin - 1);
    ck_assert_uint_eq(array.capacity, private_ACUtilsTest_ADynArray_growStrategy(private_ACUtilsTest_ADynArray_capacityMin + 1, sizeof(char)));
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_ptr_null(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_shrinkToFit_failure_nullptr)
{
    struct ADynCharArray* arrayPtr = NULL;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_shrinkToFit(arrayPtr), false);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
}
END_TEST


START_TEST(test_ADynArray_clear)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = private_ACUtilsTest_ADynArray_capacityMin + 1;
    array.growStrategy = NULL;
    array.capacity = (size_t) (private_ACUtilsTest_ADynArray_capacityMin * private_ACUtilsTest_ADynArray_capacityMul);
    array.buffer = NULL;
    private_ACUtilsTest_ADynArray_reallocFail = true;
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ADynArray_clear(&array);
    ck_assert_uint_eq(array.size, 0);
    ck_assert_uint_eq(array.capacity, private_ACUtilsTest_ADynArray_capacityMin * private_ACUtilsTest_ADynArray_capacityMul);
    ck_assert_ptr_null(array.buffer);
    ck_assert_ptr_null(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
}
END_TEST
START_TEST(test_ADynArray_clear_nullptr)
{
    struct ADynCharArray* arrayPtr = NULL;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ADynArray_clear(arrayPtr);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
}
END_TEST


START_TEST(test_ADynArray_insert_success_zeroIndex)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 5;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = 8;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity);
    memcpy(array.buffer, "1234", 5);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    char c = '0';
    ck_assert_uint_eq(ADynArray_insert(&array, 0, c), true);
    ck_assert_uint_eq(array.size, 6);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_int_eq(array.buffer[0], '0');
    ck_assert_str_eq(array.buffer, "01234");
    ck_assert_ptr_nonnull(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_insert_success_middleIndex)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 5;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = 8;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity);
    memcpy(array.buffer, "0134", 5);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    char c = '2';
    ck_assert_uint_eq(ADynArray_insert(&array, 2, c), true);
    ck_assert_uint_eq(array.size, 6);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_int_eq(array.buffer[2], '2');
    ck_assert_str_eq(array.buffer, "01234");
    ck_assert_ptr_nonnull(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_insert_success_endIndex)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 5;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = 8;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity);
    memcpy(array.buffer, "01234", 5);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    char c = '\0';
    ck_assert_uint_eq(ADynArray_insert(&array, 5, c), true);
    ck_assert_uint_eq(array.size, 6);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_int_eq(array.buffer[5], '\0');
    ck_assert_str_eq(array.buffer, "01234");
    ck_assert_ptr_nonnull(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_insert_success_beyondEndIndex)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 5;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = 8;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity);
    memcpy(array.buffer, "01234", 5);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    char c = '\0';
    ck_assert_uint_eq(ADynArray_insert(&array, 666, c), true);
    ck_assert_uint_eq(array.size, 6);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_int_eq(array.buffer[5], '\0');
    ck_assert_str_eq(array.buffer, "01234");
    ck_assert_ptr_nonnull(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_insert_success_bufferExpanded)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 5;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = 5;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity);
    memcpy(array.buffer, "0134", 5);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    char c = '2';
    ck_assert_uint_eq(ADynArray_insert(&array, 2, c), true);
    ck_assert_uint_eq(array.size, 6);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_int_eq(array.buffer[2], '2');
    ck_assert_str_eq(array.buffer, "01234");
    ck_assert_ptr_nonnull(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 1);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_insert_success_negativeIndexGetsMaxIndex)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 5;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = 8;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity);
    memcpy(array.buffer, "01234", 5);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    char c = '\0';
    ck_assert_uint_eq(ADynArray_insert(&array, -1, c), true);
    ck_assert_uint_eq(array.size, 6);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "01234");
    ck_assert_ptr_nonnull(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_insert_failure_bufferExpansionFailed)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 5;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = 5;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity);
    memcpy(array.buffer, "0134", 5);
    private_ACUtilsTest_ADynArray_reallocFail = true;
    char c = '2';
    ck_assert_uint_eq(ADynArray_insert(&array, 2, c), false);
    ck_assert_uint_eq(array.size, 5);
    ck_assert_uint_eq(array.capacity, 5);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0134");
    ck_assert_ptr_nonnull(array.growStrategy);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_insert_failure_nullptr)
{
    struct ADynCharArray* arrayPtr = NULL;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    char c = '0';
    ck_assert_uint_eq(ADynArray_insert(arrayPtr, 0, c), false);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
}
END_TEST


START_TEST(test_ADynArray_insertArray_success_zeroIndex)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 5;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = 8;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity);
    memcpy(array.buffer, "3456", 5);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_insertArray(&array, 0, "012", 3), true);
    ck_assert_uint_eq(array.size, 8);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123456");
    ck_assert_ptr_nonnull(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_insertArray_success_middleIndex)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 5;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = 8;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity);
    memcpy(array.buffer, "0156", 5);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_insertArray(&array, 2, "234", 3), true);
    ck_assert_uint_eq(array.size, 8);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123456");
    ck_assert_ptr_nonnull(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_insertArray_success_endIndex)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 5;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = 8;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity);
    memcpy(array.buffer, "01234", 5);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_insertArray(&array, 5, "56\0", 3), true);
    ck_assert_uint_eq(array.size, 8);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123456");
    ck_assert_ptr_nonnull(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_insertArray_success_beyondEndIndex)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 5;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = 8;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity);
    memcpy(array.buffer, "01234", 5);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_insertArray(&array, 666, "56\0", 3), true);
    ck_assert_uint_eq(array.size, 8);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123456");
    ck_assert_ptr_nonnull(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_insertArray_success_bufferExpanded)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 5;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = 5;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity);
    memcpy(array.buffer, "0156", 5);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_insertArray(&array, 2, "234", 3), true);
    ck_assert_uint_eq(array.size, 8);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123456");
    ck_assert_ptr_nonnull(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 1);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_insertArray_success_nullptrArray)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    char* nullptrArray = NULL;
    array.size = 5;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = 5;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity);
    memcpy(array.buffer, "0156", 5);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_insertArray(&array, 2, nullptrArray, 3), true);
    ck_assert_uint_eq(array.size, 5);
    ck_assert_uint_eq(array.capacity, 5);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0156");
    ck_assert_ptr_nonnull(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_insertArray_success_zeroArraySize)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 5;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = 5;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity);
    memcpy(array.buffer, "0156", 5);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_insertArray(&array, 2, "234", 0), true);
    ck_assert_uint_eq(array.size, 5);
    ck_assert_uint_eq(array.capacity, 5);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0156");
    ck_assert_ptr_nonnull(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_insertArray_success_negativeIndexGetsMaxIndex)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 5;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = 8;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity);
    memcpy(array.buffer, "01234", 5);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_insertArray(&array, -1, "56\0", 3), true);
    ck_assert_uint_eq(array.size, 8);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123456");
    ck_assert_ptr_nonnull(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_insertArray_failure_bufferExpansionFailed)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 5;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = 5;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity);
    memcpy(array.buffer, "0156", 5);
    private_ACUtilsTest_ADynArray_reallocFail = true;
    ck_assert_uint_eq(ADynArray_insertArray(&array, 2, "234", 3), false);
    ck_assert_uint_eq(array.size, 5);
    ck_assert_uint_eq(array.capacity, 5);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0156");
    ck_assert_ptr_nonnull(array.growStrategy);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_insertArray_failure_nullptrDestArray)
{
    struct ADynCharArray* arrayPtr = NULL;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_insertArray(arrayPtr, 0, "012", 3), false);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
}
END_TEST


START_TEST(test_ADynArray_insertADynArray_success_zeroIndex)
{
    struct ADynCharArray destArray = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    struct ADynCharArray srcArray = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    destArray.size = 5;
    destArray.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    destArray.capacity = 8;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    destArray.buffer = destArray.reallocator(NULL, destArray.capacity);
    memcpy(destArray.buffer, "3456", 5);
    srcArray.size = 3;
    srcArray.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    srcArray.capacity = 8;
    srcArray.buffer = srcArray.reallocator(NULL, srcArray.capacity);
    memcpy(srcArray.buffer, "012\0", 4);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_insertADynArray(&destArray, 0, &srcArray), true);
    ck_assert_uint_eq(destArray.size, 8);
    ck_assert_uint_eq(destArray.capacity, 8);
    ck_assert_ptr_nonnull(destArray.buffer);
    ck_assert_str_eq(destArray.buffer, "0123456");
    ck_assert_ptr_nonnull(destArray.growStrategy);
    ck_assert_uint_eq(srcArray.size, 3);
    ck_assert_uint_eq(srcArray.capacity, 8);
    ck_assert_ptr_nonnull(srcArray.buffer);
    ck_assert_str_eq(srcArray.buffer, "012");
    ck_assert_ptr_nonnull(srcArray.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    destArray.deallocator(destArray.buffer);
    srcArray.deallocator(srcArray.buffer);
}
END_TEST
START_TEST(test_ADynArray_insertADynArray_success_middleIndex)
{
    struct ADynCharArray destArray = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    struct ADynCharArray srcArray = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    destArray.size = 5;
    destArray.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    destArray.capacity = 8;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    destArray.buffer = destArray.reallocator(NULL, destArray.capacity);
    memcpy(destArray.buffer, "0156", 5);
    srcArray.size = 3;
    srcArray.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    srcArray.capacity = 8;
    srcArray.buffer = srcArray.reallocator(NULL, srcArray.capacity);
    memcpy(srcArray.buffer, "234\0", 4);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_insertADynArray(&destArray, 2, &srcArray), true);
    ck_assert_uint_eq(destArray.size, 8);
    ck_assert_uint_eq(destArray.capacity, 8);
    ck_assert_ptr_nonnull(destArray.buffer);
    ck_assert_str_eq(destArray.buffer, "0123456");
    ck_assert_ptr_nonnull(destArray.growStrategy);
    ck_assert_uint_eq(srcArray.size, 3);
    ck_assert_uint_eq(srcArray.capacity, 8);
    ck_assert_ptr_nonnull(srcArray.buffer);
    ck_assert_str_eq(srcArray.buffer, "234");
    ck_assert_ptr_nonnull(srcArray.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    destArray.deallocator(destArray.buffer);
    srcArray.deallocator(srcArray.buffer);
}
END_TEST
START_TEST(test_ADynArray_insertADynArray_success_endIndex)
{
    struct ADynCharArray destArray = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    struct ADynCharArray srcArray = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    destArray.size = 5;
    destArray.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    destArray.capacity = 8;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    destArray.buffer = destArray.reallocator(NULL, destArray.capacity);
    memcpy(destArray.buffer, "01234", 5);
    srcArray.size = 3;
    srcArray.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    srcArray.capacity = 8;
    srcArray.buffer = srcArray.reallocator(NULL, srcArray.capacity);
    memcpy(srcArray.buffer, "56\0", 4);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_insertADynArray(&destArray, 5, &srcArray), true);
    ck_assert_uint_eq(destArray.size, 8);
    ck_assert_uint_eq(destArray.capacity, 8);
    ck_assert_ptr_nonnull(destArray.buffer);
    ck_assert_str_eq(destArray.buffer, "0123456");
    ck_assert_ptr_nonnull(destArray.growStrategy);
    ck_assert_uint_eq(srcArray.size, 3);
    ck_assert_uint_eq(srcArray.capacity, 8);
    ck_assert_ptr_nonnull(srcArray.buffer);
    ck_assert_str_eq(srcArray.buffer, "56");
    ck_assert_ptr_nonnull(srcArray.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    destArray.deallocator(destArray.buffer);
    srcArray.deallocator(srcArray.buffer);
}
END_TEST
START_TEST(test_ADynArray_insertADynArray_success_beyondEndIndex)
{
    struct ADynCharArray destArray = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    struct ADynCharArray srcArray = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    destArray.size = 5;
    destArray.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    destArray.capacity = 8;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    destArray.buffer = destArray.reallocator(NULL, destArray.capacity);
    memcpy(destArray.buffer, "01234", 5);
    srcArray.size = 3;
    srcArray.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    srcArray.capacity = 8;
    srcArray.buffer = srcArray.reallocator(NULL, srcArray.capacity);
    memcpy(srcArray.buffer, "56", 3);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_insertADynArray(&destArray, 666, &srcArray), true);
    ck_assert_uint_eq(destArray.size, 8);
    ck_assert_uint_eq(destArray.capacity, 8);
    ck_assert_ptr_nonnull(destArray.buffer);
    ck_assert_str_eq(destArray.buffer, "0123456");
    ck_assert_ptr_nonnull(destArray.growStrategy);
    ck_assert_uint_eq(srcArray.size, 3);
    ck_assert_uint_eq(srcArray.capacity, 8);
    ck_assert_ptr_nonnull(srcArray.buffer);
    ck_assert_str_eq(srcArray.buffer, "56");
    ck_assert_ptr_nonnull(srcArray.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    destArray.deallocator(destArray.buffer);
    srcArray.deallocator(srcArray.buffer);
}
END_TEST
START_TEST(test_ADynArray_insertADynArray_success_bufferExpanded)
{
    struct ADynCharArray destArray = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    struct ADynCharArray srcArray = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    destArray.size = 5;
    destArray.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    destArray.capacity = 5;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    destArray.buffer = destArray.reallocator(NULL, destArray.capacity);
    memcpy(destArray.buffer, "0156", 5);
    srcArray.size = 3;
    srcArray.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    srcArray.capacity = 8;
    srcArray.buffer = srcArray.reallocator(NULL, srcArray.capacity);
    memcpy(srcArray.buffer, "234", 4);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_insertADynArray(&destArray, 2, &srcArray), true);
    ck_assert_uint_eq(destArray.size, 8);
    ck_assert_uint_eq(destArray.capacity, 8);
    ck_assert_ptr_nonnull(destArray.buffer);
    ck_assert_str_eq(destArray.buffer, "0123456");
    ck_assert_ptr_nonnull(destArray.growStrategy);
    ck_assert_uint_eq(srcArray.size, 3);
    ck_assert_uint_eq(srcArray.capacity, 8);
    ck_assert_ptr_nonnull(srcArray.buffer);
    ck_assert_str_eq(srcArray.buffer, "234");
    ck_assert_ptr_nonnull(srcArray.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 1);
    destArray.deallocator(destArray.buffer);
    srcArray.deallocator(srcArray.buffer);
}
END_TEST
START_TEST(test_ADynArray_insertADynArray_success_nullptrSrcArray)
{
    struct ADynCharArray destArray = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    struct ADynCharArray *srcArray = NULL;
    destArray.size = 5;
    destArray.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    destArray.capacity = 5;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    destArray.buffer = destArray.reallocator(NULL, destArray.capacity);
    memcpy(destArray.buffer, "0156", 5);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_insertADynArray(&destArray, 2, srcArray), true);
    ck_assert_uint_eq(destArray.size, 5);
    ck_assert_uint_eq(destArray.capacity, 5);
    ck_assert_ptr_nonnull(destArray.buffer);
    ck_assert_str_eq(destArray.buffer, "0156");
    ck_assert_ptr_nonnull(destArray.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    destArray.deallocator(destArray.buffer);
}
END_TEST
START_TEST(test_ADynArray_insertADynArray_success_zeroSizeSrcArray)
{
    struct ADynCharArray destArray = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    struct ADynCharArray srcArray = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    destArray.size = 5;
    destArray.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    destArray.capacity = 8;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    destArray.buffer = destArray.reallocator(NULL, destArray.capacity);
    memcpy(destArray.buffer, "0156", 5);
    srcArray.size = 0;
    srcArray.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    srcArray.capacity = 8;
    srcArray.buffer = srcArray.reallocator(NULL, srcArray.capacity);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_insertADynArray(&destArray, 2, &srcArray), true);
    ck_assert_uint_eq(destArray.size, 5);
    ck_assert_uint_eq(destArray.capacity, 8);
    ck_assert_ptr_nonnull(destArray.buffer);
    ck_assert_str_eq(destArray.buffer, "0156");
    ck_assert_ptr_nonnull(destArray.growStrategy);
    ck_assert_uint_eq(srcArray.size, 0);
    ck_assert_uint_eq(srcArray.capacity, 8);
    ck_assert_ptr_nonnull(srcArray.buffer);
    ck_assert_ptr_nonnull(srcArray.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    destArray.deallocator(destArray.buffer);
    srcArray.deallocator(srcArray.buffer);
}
END_TEST
START_TEST(test_ADynArray_insertADynArray_success_negativeIndexGetsMaxIndex)
{
    struct ADynCharArray destArray = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    struct ADynCharArray srcArray = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    destArray.size = 5;
    destArray.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    destArray.capacity = 8;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    destArray.buffer = destArray.reallocator(NULL, destArray.capacity);
    memcpy(destArray.buffer, "01234", 5);
    srcArray.size = 3;
    srcArray.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    srcArray.capacity = 8;
    srcArray.buffer = srcArray.reallocator(NULL, srcArray.capacity);
    memcpy(srcArray.buffer, "56", 3);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_insertADynArray(&destArray, -1, &srcArray), true);
    ck_assert_uint_eq(destArray.size, 8);
    ck_assert_uint_eq(destArray.capacity, 8);
    ck_assert_ptr_nonnull(destArray.buffer);
    ck_assert_str_eq(destArray.buffer, "0123456");
    ck_assert_ptr_nonnull(destArray.growStrategy);
    ck_assert_uint_eq(srcArray.size, 3);
    ck_assert_uint_eq(srcArray.capacity, 8);
    ck_assert_ptr_nonnull(srcArray.buffer);
    ck_assert_str_eq(srcArray.buffer, "56");
    ck_assert_ptr_nonnull(srcArray.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    destArray.deallocator(destArray.buffer);
    srcArray.deallocator(srcArray.buffer);
}
END_TEST
START_TEST(test_ADynArray_insertADynArray_failure_bufferExpansionFailed)
{
    struct ADynCharArray destArray = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    struct ADynCharArray srcArray = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    destArray.size = 5;
    destArray.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    destArray.capacity = 5;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    destArray.buffer = destArray.reallocator(NULL, destArray.capacity);
    memcpy(destArray.buffer, "0156", 5);
    srcArray.size = 3;
    srcArray.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    srcArray.capacity = 8;
    srcArray.buffer = srcArray.reallocator(NULL, srcArray.capacity);
    memcpy(srcArray.buffer, "234", 4);
    private_ACUtilsTest_ADynArray_reallocFail = true;
    ck_assert_uint_eq(ADynArray_insertADynArray(&destArray, 2, &srcArray), false);
    ck_assert_uint_eq(destArray.size, 5);
    ck_assert_uint_eq(destArray.capacity, 5);
    ck_assert_ptr_nonnull(destArray.buffer);
    ck_assert_str_eq(destArray.buffer, "0156");
    ck_assert_ptr_nonnull(destArray.growStrategy);
    ck_assert_uint_eq(srcArray.size, 3);
    ck_assert_uint_eq(srcArray.capacity, 8);
    ck_assert_ptr_nonnull(srcArray.buffer);
    ck_assert_str_eq(srcArray.buffer, "234");
    ck_assert_ptr_nonnull(srcArray.growStrategy);
    destArray.deallocator(destArray.buffer);
    srcArray.deallocator(srcArray.buffer);
}
END_TEST
START_TEST(test_ADynArray_insertADynArray_failure_nullptrDestArray)
{
    struct ADynCharArray srcArray = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    struct ADynCharArray *destArrayPtr = NULL;
    srcArray.size = 3;
    srcArray.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    srcArray.capacity = 8;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    srcArray.buffer = srcArray.reallocator(NULL, srcArray.capacity);
    memcpy(srcArray.buffer, "012", 4);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_insertADynArray(destArrayPtr, 0, &srcArray), false);
    ck_assert_uint_eq(srcArray.size, 3);
    ck_assert_uint_eq(srcArray.capacity, 8);
    ck_assert_ptr_nonnull(srcArray.buffer);
    ck_assert_str_eq(srcArray.buffer, "012");
    ck_assert_ptr_nonnull(srcArray.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    srcArray.deallocator(srcArray.buffer);
}
END_TEST


START_TEST(test_ADynArray_append_success_enoughCapacity)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 5;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = 8;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity);
    memcpy(array.buffer, "01234", 5);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    char c = '\0';
    ck_assert_uint_eq(ADynArray_append(&array, c), true);
    ck_assert_uint_eq(array.size, 6);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_int_eq(array.buffer[array.size - 1], '\0');
    ck_assert_str_eq(array.buffer, "01234");
    ck_assert_ptr_nonnull(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_append_success_notEnoughCapacity)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 5;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = 5;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity);
    memcpy(array.buffer, "01234", 5);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    char c = '\0';
    ck_assert_uint_eq(ADynArray_append(&array, c), true);
    ck_assert_uint_eq(array.size, 6);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_int_eq(array.buffer[array.size - 1], '\0');
    ck_assert_str_eq(array.buffer, "01234");
    ck_assert_ptr_nonnull(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 1);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_append_failure_bufferExpansionFailed)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 5;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = 5;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity + 1);
    memcpy(array.buffer, "01234", 6);
    private_ACUtilsTest_ADynArray_reallocFail = true;
    char c = '\0';
    ck_assert_uint_eq(ADynArray_append(&array, c), false);
    ck_assert_uint_eq(array.size, 5);
    ck_assert_uint_eq(array.capacity, 5);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "01234");
    ck_assert_ptr_nonnull(array.growStrategy);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_append_failure_nullptr)
{
    struct ADynCharArray* arrayPtr = NULL;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    char c = '\0';
    ck_assert_uint_eq(ADynArray_append(arrayPtr, c), false);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
}
END_TEST


START_TEST(test_ADynArray_appendArray_success_enoughCapacity)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 5;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = 8;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity);
    memcpy(array.buffer, "01234", 5);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_appendArray(&array, "56\0", 3), true);
    ck_assert_uint_eq(array.size, 8);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123456");
    ck_assert_ptr_nonnull(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_appendArray_success_notEnoughCapacity)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 5;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = 5;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity);
    memcpy(array.buffer, "01234", 5);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_appendArray(&array, "56\0", 3), true);
    ck_assert_uint_eq(array.size, 8);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123456");
    ck_assert_ptr_nonnull(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 1);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_appendArray_success_nullptrArray)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    char* nullptrArray = NULL;
    array.size = 5;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = 5;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity);
    memcpy(array.buffer, "0156", 5);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_appendArray(&array, nullptrArray, 3), true);
    ck_assert_uint_eq(array.size, 5);
    ck_assert_uint_eq(array.capacity, 5);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0156");
    ck_assert_ptr_nonnull(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_appendArray_success_zeroArraySize)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 5;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = 5;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity);
    memcpy(array.buffer, "0156", 5);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_appendArray(&array, "234", 0), true);
    ck_assert_uint_eq(array.size, 5);
    ck_assert_uint_eq(array.capacity, 5);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0156");
    ck_assert_ptr_nonnull(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_appendArray_failure_bufferExpansionFailed)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 5;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = 5;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity);
    memcpy(array.buffer, "0123", 5);
    private_ACUtilsTest_ADynArray_reallocFail = true;
    ck_assert_uint_eq(ADynArray_insertArray(&array, 2, "45\0", 3), false);
    ck_assert_uint_eq(array.size, 5);
    ck_assert_uint_eq(array.capacity, 5);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123");
    ck_assert_ptr_nonnull(array.growStrategy);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_appendArray_failure_nullptrDestArray)
{
    struct ADynCharArray* arrayPtr = NULL;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_insertArray(arrayPtr, 0, "012", 3), false);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
}
END_TEST


START_TEST(test_ADynArray_appendADynArray_success_enoughCapacity)
{
    struct ADynCharArray destArray = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    struct ADynCharArray srcArray = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    destArray.size = 5;
    destArray.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    destArray.capacity = 8;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    destArray.buffer = destArray.reallocator(NULL, destArray.capacity);
    memcpy(destArray.buffer, "01234", 5);
    srcArray.size = 3;
    srcArray.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    srcArray.capacity = 8;
    srcArray.buffer = srcArray.reallocator(NULL, srcArray.capacity);
    memcpy(srcArray.buffer, "56", 3);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_appendADynArray(&destArray, &srcArray), true);
    ck_assert_uint_eq(destArray.size, 8);
    ck_assert_uint_eq(destArray.capacity, 8);
    ck_assert_ptr_nonnull(destArray.buffer);
    ck_assert_str_eq(destArray.buffer, "0123456");
    ck_assert_ptr_nonnull(destArray.growStrategy);
    ck_assert_uint_eq(srcArray.size, 3);
    ck_assert_uint_eq(srcArray.capacity, 8);
    ck_assert_ptr_nonnull(srcArray.buffer);
    ck_assert_str_eq(srcArray.buffer, "56");
    ck_assert_ptr_nonnull(srcArray.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    destArray.deallocator(destArray.buffer);
    srcArray.deallocator(srcArray.buffer);
}
END_TEST
START_TEST(test_ADynArray_appendADynArray_success_notEnoughCapacity)
{
    struct ADynCharArray destArray = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    struct ADynCharArray srcArray = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    destArray.size = 5;
    destArray.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    destArray.capacity = 5;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    destArray.buffer = destArray.reallocator(NULL, destArray.capacity);
    memcpy(destArray.buffer, "01234", 5);
    srcArray.size = 3;
    srcArray.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    srcArray.capacity = 8;
    srcArray.buffer = srcArray.reallocator(NULL, srcArray.capacity);
    memcpy(srcArray.buffer, "56", 3);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_appendADynArray(&destArray, &srcArray), true);
    ck_assert_uint_eq(destArray.size, 8);
    ck_assert_uint_eq(destArray.capacity, 8);
    ck_assert_ptr_nonnull(destArray.buffer);
    ck_assert_str_eq(destArray.buffer, "0123456");
    ck_assert_ptr_nonnull(destArray.growStrategy);
    ck_assert_uint_eq(srcArray.size, 3);
    ck_assert_uint_eq(srcArray.capacity, 8);
    ck_assert_ptr_nonnull(srcArray.buffer);
    ck_assert_str_eq(srcArray.buffer, "56");
    ck_assert_ptr_nonnull(srcArray.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 1);
    destArray.deallocator(destArray.buffer);
    srcArray.deallocator(srcArray.buffer);
}
END_TEST
START_TEST(test_ADynArray_appendADynArray_success_nullptrSrcArray)
{
    struct ADynCharArray destArray = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    struct ADynCharArray *srcArray = NULL;
    destArray.size = 5;
    destArray.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    destArray.capacity = 5;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    destArray.buffer = destArray.reallocator(NULL, destArray.capacity);
    memcpy(destArray.buffer, "0123", 5);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_appendADynArray(&destArray, srcArray), true);
    ck_assert_uint_eq(destArray.size, 5);
    ck_assert_uint_eq(destArray.capacity, 5);
    ck_assert_ptr_nonnull(destArray.buffer);
    ck_assert_str_eq(destArray.buffer, "0123");
    ck_assert_ptr_nonnull(destArray.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    destArray.deallocator(destArray.buffer);
}
END_TEST
START_TEST(test_ADynArray_appendADynArray_success_zeroSizeSrcArray)
{
    struct ADynCharArray destArray = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    struct ADynCharArray srcArray = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    destArray.size = 5;
    destArray.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    destArray.capacity = 8;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    destArray.buffer = destArray.reallocator(NULL, destArray.capacity);
    memcpy(destArray.buffer, "0123", 5);
    srcArray.size = 0;
    srcArray.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    srcArray.capacity = 8;
    srcArray.buffer = srcArray.reallocator(NULL, srcArray.capacity);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_appendADynArray(&destArray, &srcArray), true);
    ck_assert_uint_eq(destArray.size, 5);
    ck_assert_uint_eq(destArray.capacity, 8);
    ck_assert_ptr_nonnull(destArray.buffer);
    ck_assert_str_eq(destArray.buffer, "0123");
    ck_assert_ptr_nonnull(destArray.growStrategy);
    ck_assert_uint_eq(srcArray.size, 0);
    ck_assert_uint_eq(srcArray.capacity, 8);
    ck_assert_ptr_nonnull(srcArray.buffer);
    ck_assert_ptr_nonnull(srcArray.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    destArray.deallocator(destArray.buffer);
    srcArray.deallocator(srcArray.buffer);
}
END_TEST
START_TEST(test_ADynArray_appendADynArray_failure_bufferExpansionFailed)
{
    struct ADynCharArray destArray = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    struct ADynCharArray srcArray = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    destArray.size = 5;
    destArray.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    destArray.capacity = 5;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    destArray.buffer = destArray.reallocator(NULL, destArray.capacity);
    memcpy(destArray.buffer, "0156", 5);
    srcArray.size = 3;
    srcArray.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    srcArray.capacity = 8;
    srcArray.buffer = srcArray.reallocator(NULL, srcArray.capacity);
    memcpy(srcArray.buffer, "234", 4);
    private_ACUtilsTest_ADynArray_reallocFail = true;
    ck_assert_uint_eq(ADynArray_appendADynArray(&destArray, &srcArray), false);
    ck_assert_uint_eq(destArray.size, 5);
    ck_assert_uint_eq(destArray.capacity, 5);
    ck_assert_ptr_nonnull(destArray.buffer);
    ck_assert_str_eq(destArray.buffer, "0156");
    ck_assert_ptr_nonnull(destArray.growStrategy);
    ck_assert_uint_eq(srcArray.size, 3);
    ck_assert_uint_eq(srcArray.capacity, 8);
    ck_assert_ptr_nonnull(srcArray.buffer);
    ck_assert_str_eq(srcArray.buffer, "234");
    ck_assert_ptr_nonnull(srcArray.growStrategy);
    destArray.deallocator(destArray.buffer);
    srcArray.deallocator(srcArray.buffer);
}
END_TEST
START_TEST(test_ADynArray_appendADynArray_failure_nullptrDestArray)
{
    struct ADynCharArray srcArray = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    struct ADynCharArray *destArrayPtr = NULL;
    srcArray.size = 3;
    srcArray.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    srcArray.capacity = 8;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    srcArray.buffer = srcArray.reallocator(NULL, srcArray.capacity);
    memcpy(srcArray.buffer, "012", 4);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ck_assert_uint_eq(ADynArray_appendADynArray(destArrayPtr, &srcArray), false);
    ck_assert_uint_eq(srcArray.size, 3);
    ck_assert_uint_eq(srcArray.capacity, 8);
    ck_assert_ptr_nonnull(srcArray.buffer);
    ck_assert_str_eq(srcArray.buffer, "012");
    ck_assert_ptr_nonnull(srcArray.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    srcArray.deallocator(srcArray.buffer);
}
END_TEST


START_TEST(test_ADynArray_set_success_indexInBounds)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 4;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = 4;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity);
    memcpy(array.buffer, "012", 4);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    char c = '2';
    ck_assert_uint_eq(ADynArray_set(&array, 0, c), true);
    c = '1';
    ck_assert_uint_eq(ADynArray_set(&array, 1, c), true);
    c = '0';
    ck_assert_uint_eq(ADynArray_set(&array, 2, c), true);
    ck_assert_uint_eq(array.size, 4);
    ck_assert_uint_eq(array.capacity, 4);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "210");
    ck_assert_ptr_nonnull(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_set_success_indexBeyondSize)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 3;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = 4;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity + 1);
    memcpy(array.buffer, "012\0\0", 5);
    private_ACUtilsTest_ADynArray_reallocFail = true;
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    char c = '3';
    ck_assert_uint_eq(ADynArray_set(&array, 3, c), true);
    ck_assert_uint_eq(array.size, 4);
    ck_assert_uint_eq(array.capacity, 4);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123");
    ck_assert_ptr_nonnull(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_set_success_indexBeyondSize_bufferExpanded)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 3;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = 3;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity + 2);
    memcpy(array.buffer, "012\0\0", 5);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    char c = '3';
    ck_assert_uint_eq(ADynArray_set(&array, 3, c), true);
    ck_assert_uint_eq(array.size, 4);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123");
    ck_assert_ptr_nonnull(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 1);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_set_failure_indexBeyondSize_bufferExpansionFailed)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 3;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = 3;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity + 1);
    memcpy(array.buffer, "012", 4);
    private_ACUtilsTest_ADynArray_reallocFail = true;
    char c = '3';
    ck_assert_uint_eq(ADynArray_set(&array, 3, c), false);
    ck_assert_uint_eq(array.size, 3);
    ck_assert_uint_eq(array.capacity, 3);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "012");
    ck_assert_ptr_nonnull(array.growStrategy);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_set_failure_nullptr)
{
    struct ADynCharArray *arrayPtr = NULL;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    char c = '0';
    ck_assert_uint_eq(ADynArray_set(arrayPtr, 0, c), false);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
}
END_TEST


START_TEST(test_ADynArray_remove_indexRangeInBounds)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 11;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = 16;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity + 1);
    memcpy(array.buffer, "0123456789", 11);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ADynArray_remove(&array, 2, 6);
    ck_assert_uint_eq(array.size, 5);
    ck_assert_uint_eq(array.capacity, 16);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0189");
    ck_assert_ptr_nonnull(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_remove_rangeBeyondBounds)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 11;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = 16;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity + 1);
    memcpy(array.buffer, "0123456789", 11);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ADynArray_remove(&array, 2, 100);
    ck_assert_uint_eq(array.size, 2);
    ck_assert_uint_eq(array.capacity, 16);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_ptr_nonnull(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_remove_zeroRange)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 11;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = 16;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity + 1);
    memcpy(array.buffer, "0123456789", 11);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ADynArray_remove(&array, 2, 0);
    ck_assert_uint_eq(array.size, 11);
    ck_assert_uint_eq(array.capacity, 16);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123456789");
    ck_assert_ptr_nonnull(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_remove_indexBeyoundBounds)
{
    struct ADynCharArray array = {.reallocator = private_ACUtilsTest_ADynArray_realloc, .deallocator = private_ACUtilsTest_ADynArray_free};
    array.size = 11;
    array.growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    array.capacity = 16;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity + 1);
    memcpy(array.buffer, "0123456789", 11);
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ADynArray_remove(&array, 13, 5);
    ck_assert_uint_eq(array.size, 11);
    ck_assert_uint_eq(array.capacity, 16);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "0123456789");
    ck_assert_ptr_nonnull(array.growStrategy);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_ADynArray_remove_nullptr)
{
    struct ADynCharArray *arrayPtr = NULL;
    private_ACUtilsTest_ADynArray_reallocFail = false;
    private_ACUtilsTest_ADynArray_reallocCount = 0;
    ADynArray_remove(arrayPtr, 5, 10);
    ck_assert_uint_eq(private_ACUtilsTest_ADynArray_reallocCount, 0);
}
END_TEST

START_TEST(test_aDynArrayMixedWithStruct)
{
    size_t i;
    struct ADynTestPointArray *tmpDynArray, *dynArray;
    struct private_ACUtilsTest_ADynArray_PointStruct *tmpArray = (struct private_ACUtilsTest_ADynArray_PointStruct*) malloc(9 * sizeof(struct private_ACUtilsTest_ADynArray_PointStruct));
    tmpArray[0] = (struct private_ACUtilsTest_ADynArray_PointStruct) {.x = 0.1, .y = 0.2};
    tmpArray[1] = (struct private_ACUtilsTest_ADynArray_PointStruct) {.x = 1.1, .y = 1.2};
    tmpArray[2] = (struct private_ACUtilsTest_ADynArray_PointStruct) {.x = 2.1, .y = 2.2};
    tmpArray[3] = (struct private_ACUtilsTest_ADynArray_PointStruct) {.x = 3.1, .y = 3.2};
    tmpArray[4] = (struct private_ACUtilsTest_ADynArray_PointStruct) {.x = 4.1, .y = 4.2};
    tmpArray[5] = (struct private_ACUtilsTest_ADynArray_PointStruct) {.x = 5.1, .y = 5.2};
    tmpArray[6] = (struct private_ACUtilsTest_ADynArray_PointStruct) {.x = 6.1, .y = 6.2};
    tmpArray[7] = (struct private_ACUtilsTest_ADynArray_PointStruct) {.x = 7.1, .y = 7.2};
    tmpArray[8] = (struct private_ACUtilsTest_ADynArray_PointStruct) {.x = 8.1, .y = 8.2};
    dynArray = ADynArray_constructWithAllocator(struct ADynTestPointArray, private_ACUtilsTest_ADynArray_realloc, private_ACUtilsTest_ADynArray_free);
    dynArray->growStrategy = private_ACUtilsTest_ADynArray_growStrategy;
    ck_assert_uint_eq(ADynArray_size(dynArray), 0);
    ck_assert_uint_eq(ADynArray_shrinkToFit(dynArray), true);
    ck_assert_uint_eq(ADynArray_size(dynArray), 0);
    ck_assert_uint_eq(ADynArray_capacity(dynArray), 8);
    tmpDynArray = ADynArray_constructWithAllocator(struct ADynTestPointArray, private_ACUtilsTest_ADynArray_realloc, private_ACUtilsTest_ADynArray_free);
    ADynArray_appendArray(tmpDynArray, tmpArray + 6, 3);
    ck_assert_uint_eq(ADynArray_size(tmpDynArray), 3);
    ck_assert_uint_eq(ADynArray_capacity(tmpDynArray), 8);
    ck_assert_uint_eq(ADynArray_insertADynArray(dynArray, 0, tmpDynArray), true);
    ck_assert_uint_eq(ADynArray_size(dynArray), 3);
    ck_assert_uint_eq(ADynArray_capacity(dynArray), 8);
    ADynArray_destruct(tmpDynArray);
    ck_assert_uint_eq(ADynArray_insert(dynArray, 0, tmpArray[5]), true);
    ck_assert_uint_eq(ADynArray_size(dynArray), 4);
    ck_assert_uint_eq(ADynArray_capacity(dynArray), 8);
    ck_assert_uint_eq(ADynArray_insert(dynArray, 0, tmpArray[4]), true);
    ck_assert_uint_eq(ADynArray_size(dynArray), 5);
    ck_assert_uint_eq(ADynArray_capacity(dynArray), 8);
    ck_assert_uint_eq(ADynArray_insert(dynArray, 0, tmpArray[3]), true);
    ck_assert_uint_eq(ADynArray_size(dynArray), 6);
    ck_assert_uint_eq(ADynArray_capacity(dynArray), 8);
    ck_assert_uint_eq(ADynArray_insertArray(dynArray, 0, tmpArray, 3), true);
    ck_assert_uint_eq(ADynArray_size(dynArray), 9);
    ck_assert_uint_eq(ADynArray_capacity(dynArray), 16);
    for(i = 0; i < ADynArray_size(dynArray); ++i) {
        ck_assert_double_eq(ADynArray_get(dynArray, i).x, tmpArray[i].x);
        ck_assert_double_eq(ADynArray_get(dynArray, i).y, tmpArray[i].y);
    }
    ADynArray_remove(dynArray, 0, 3);
    ck_assert_uint_eq(ADynArray_size(dynArray), 6);
    ck_assert_uint_eq(ADynArray_capacity(dynArray), 16);
    for(i = 0; i < ADynArray_size(dynArray); ++i) {
        ck_assert_double_eq(ADynArray_get(dynArray, i).x, tmpArray[i + 3].x);
        ck_assert_double_eq(ADynArray_get(dynArray, i).y, tmpArray[i + 3].y);
    }
    ADynArray_clear(dynArray);
    ck_assert_uint_eq(ADynArray_size(dynArray), 0);
    ck_assert_uint_eq(ADynArray_capacity(dynArray), 16);
    ADynArray_destruct(dynArray);
    free(tmpArray);
}
END_TEST



Suite* test_suite_dynarray()
{
    Suite *s;
    TCase *test_case_ADynArray_construct_destruct, *test_case_ADynArray_setGrowStrategy, *test_case_ADynArray_size,
        *test_case_ADynArray_capacity, *test_case_ADynArray_reserve, *test_case_ADynArray_shrinkToFit,
        *test_case_ADynArray_clear, *test_case_ADynArray_insert, *test_case_ADynArray_insertArray,
        *test_case_ADynArray_insertADynArray, *test_case_ADynArray_add, *test_case_ADynArray_addArray,
        *test_case_ADynArray_addADynArray, *test_case_ADynArray_set, *test_case_ADynArray_remove,
        *test_case_ADynArray_mixed;

    s = suite_create("Dynamic Array Test Suite");

    test_case_ADynArray_construct_destruct = tcase_create("Dynamic Array Test Case: ADynArray_construct / ADynArray_destruct");
    tcase_add_test(test_case_ADynArray_construct_destruct, test_ADynArray_construct_destruct_valid);
    tcase_add_test(test_case_ADynArray_construct_destruct, test_ADynArray_construct_destruct_withAllocator_valid);
    tcase_add_test(test_case_ADynArray_construct_destruct, test_ADynArray_construct_destruct_withAllocator_invalid);
    tcase_add_test(test_case_ADynArray_construct_destruct, test_ADynArray_construct_destruct_noMemoryAvailable);
    tcase_add_test(test_case_ADynArray_construct_destruct, test_ADynArray_construct_destruct_nullptr);
    suite_add_tcase(s, test_case_ADynArray_construct_destruct);

    test_case_ADynArray_setGrowStrategy = tcase_create("Dynamic Array Test Case: ADynArray_setGrowStrategy");
    tcase_add_test(test_case_ADynArray_setGrowStrategy, test_ADynArray_setGrowStrategy);
    suite_add_tcase(s, test_case_ADynArray_setGrowStrategy);

    test_case_ADynArray_size = tcase_create("Dynamic Array Test Case: ADynArray_size");
    tcase_add_test(test_case_ADynArray_size, test_ADynArray_size_valid);
    tcase_add_test(test_case_ADynArray_size, test_ADynArray_size_nullptr);
    suite_add_tcase(s, test_case_ADynArray_size);

    test_case_ADynArray_capacity = tcase_create("Dynamic Array Test Case: ADynArray_capacity");
    tcase_add_test(test_case_ADynArray_capacity, test_ADynArray_capacity_valid);
    tcase_add_test(test_case_ADynArray_capacity, test_ADynArray_capacity_nullptr);
    suite_add_tcase(s, test_case_ADynArray_capacity);

    test_case_ADynArray_reserve = tcase_create("Dynamic Array Test Case: ADynArray_reserve");
    tcase_add_test(test_case_ADynArray_reserve, test_ADynArray_reserve_success_enoughCapacityBufferNotNull);
    tcase_add_test(test_case_ADynArray_reserve, test_ADynArray_reserve_success_enoughCapacityBufferNull);
    tcase_add_test(test_case_ADynArray_reserve, test_ADynArray_reserve_success_notEnoughCapacity);
    tcase_add_test(test_case_ADynArray_reserve, test_ADynArray_reserve_failure_biggerThanMaxCapacity);
    tcase_add_test(test_case_ADynArray_reserve, test_ADynArray_reserve_failure_noMemoryAvailable);
    tcase_add_test(test_case_ADynArray_reserve, test_ADynArray_reserve_failure_growStrategyNull);
    tcase_add_test(test_case_ADynArray_reserve, test_ADynArray_reserve_failure_nullptr);
    suite_add_tcase(s, test_case_ADynArray_reserve);

    test_case_ADynArray_shrinkToFit = tcase_create("Dynamic Array Test Case: ADynArray_shrinkToFit");
    tcase_add_test(test_case_ADynArray_shrinkToFit, test_ADynArray_shrinkToFit_success_hasLeastCapacityBufferNotNull);
    tcase_add_test(test_case_ADynArray_shrinkToFit, test_ADynArray_shrinkToFit_success_hasLeastCapacityBufferNull);
    tcase_add_test(test_case_ADynArray_shrinkToFit, test_ADynArray_shrinkToFit_success_hasNotLeastCapacity);
    tcase_add_test(test_case_ADynArray_shrinkToFit, test_ADynArray_shrinkToFit_failure_noMemoryAvailable);
    tcase_add_test(test_case_ADynArray_shrinkToFit, test_ADynArray_shrinkToFit_failure_growStrategyNull);
    tcase_add_test(test_case_ADynArray_shrinkToFit, test_ADynArray_shrinkToFit_failure_nullptr);
    suite_add_tcase(s, test_case_ADynArray_shrinkToFit);

    test_case_ADynArray_clear = tcase_create("Dynamic Array Test Case: ADynArray_clear");
    tcase_add_test(test_case_ADynArray_clear, test_ADynArray_clear);
    tcase_add_test(test_case_ADynArray_clear, test_ADynArray_clear_nullptr);
    suite_add_tcase(s, test_case_ADynArray_clear);

    test_case_ADynArray_insert = tcase_create("Dynamic Array Test Case: ADynArray_insert");
    tcase_add_test(test_case_ADynArray_insert, test_ADynArray_insert_success_zeroIndex);
    tcase_add_test(test_case_ADynArray_insert, test_ADynArray_insert_success_middleIndex);
    tcase_add_test(test_case_ADynArray_insert, test_ADynArray_insert_success_endIndex);
    tcase_add_test(test_case_ADynArray_insert, test_ADynArray_insert_success_beyondEndIndex);
    tcase_add_test(test_case_ADynArray_insert, test_ADynArray_insert_success_bufferExpanded);
    tcase_add_test(test_case_ADynArray_insert, test_ADynArray_insert_success_negativeIndexGetsMaxIndex);
    tcase_add_test(test_case_ADynArray_insert, test_ADynArray_insert_failure_bufferExpansionFailed);
    tcase_add_test(test_case_ADynArray_insert, test_ADynArray_insert_failure_nullptr);
    suite_add_tcase(s, test_case_ADynArray_insert);

    test_case_ADynArray_insertArray = tcase_create("Dynamic Array Test Case: ADynArray_insertArray");
    tcase_add_test(test_case_ADynArray_insertArray, test_ADynArray_insertArray_success_zeroIndex);
    tcase_add_test(test_case_ADynArray_insertArray, test_ADynArray_insertArray_success_middleIndex);
    tcase_add_test(test_case_ADynArray_insertArray, test_ADynArray_insertArray_success_endIndex);
    tcase_add_test(test_case_ADynArray_insertArray, test_ADynArray_insertArray_success_beyondEndIndex);
    tcase_add_test(test_case_ADynArray_insertArray, test_ADynArray_insertArray_success_bufferExpanded);
    tcase_add_test(test_case_ADynArray_insertArray, test_ADynArray_insertArray_success_nullptrArray);
    tcase_add_test(test_case_ADynArray_insertArray, test_ADynArray_insertArray_success_zeroArraySize);
    tcase_add_test(test_case_ADynArray_insertArray, test_ADynArray_insertArray_success_negativeIndexGetsMaxIndex);
    tcase_add_test(test_case_ADynArray_insertArray, test_ADynArray_insertArray_failure_bufferExpansionFailed);
    tcase_add_test(test_case_ADynArray_insertArray, test_ADynArray_insertArray_failure_nullptrDestArray);
    suite_add_tcase(s, test_case_ADynArray_insertArray);

    test_case_ADynArray_insertADynArray = tcase_create("Dynamic Array Test Case: ADynArray_insertADynArray");
    tcase_add_test(test_case_ADynArray_insertADynArray, test_ADynArray_insertADynArray_success_zeroIndex);
    tcase_add_test(test_case_ADynArray_insertADynArray, test_ADynArray_insertADynArray_success_middleIndex);
    tcase_add_test(test_case_ADynArray_insertADynArray, test_ADynArray_insertADynArray_success_endIndex);
    tcase_add_test(test_case_ADynArray_insertADynArray, test_ADynArray_insertADynArray_success_beyondEndIndex);
    tcase_add_test(test_case_ADynArray_insertADynArray, test_ADynArray_insertADynArray_success_bufferExpanded);
    tcase_add_test(test_case_ADynArray_insertADynArray, test_ADynArray_insertADynArray_success_nullptrSrcArray);
    tcase_add_test(test_case_ADynArray_insertADynArray, test_ADynArray_insertADynArray_success_zeroSizeSrcArray);
    tcase_add_test(test_case_ADynArray_insertADynArray, test_ADynArray_insertADynArray_success_negativeIndexGetsMaxIndex);
    tcase_add_test(test_case_ADynArray_insertADynArray, test_ADynArray_insertADynArray_failure_bufferExpansionFailed);
    tcase_add_test(test_case_ADynArray_insertADynArray, test_ADynArray_insertADynArray_failure_nullptrDestArray);
    suite_add_tcase(s, test_case_ADynArray_insertADynArray);

    test_case_ADynArray_add = tcase_create("Dynamic Array Test Case: ADynArray_append");
    tcase_add_test(test_case_ADynArray_add, test_ADynArray_append_success_enoughCapacity);
    tcase_add_test(test_case_ADynArray_add, test_ADynArray_append_success_notEnoughCapacity);
    tcase_add_test(test_case_ADynArray_add, test_ADynArray_append_failure_bufferExpansionFailed);
    tcase_add_test(test_case_ADynArray_add, test_ADynArray_append_failure_nullptr);
    suite_add_tcase(s, test_case_ADynArray_add);

    test_case_ADynArray_addArray = tcase_create("Dynamic Array Test Case: ADynArray_appendArray");
    tcase_add_test(test_case_ADynArray_addArray, test_ADynArray_appendArray_success_enoughCapacity);
    tcase_add_test(test_case_ADynArray_addArray, test_ADynArray_appendArray_success_notEnoughCapacity);
    tcase_add_test(test_case_ADynArray_addArray, test_ADynArray_appendArray_success_nullptrArray);
    tcase_add_test(test_case_ADynArray_addArray, test_ADynArray_appendArray_success_zeroArraySize);
    tcase_add_test(test_case_ADynArray_addArray, test_ADynArray_appendArray_failure_bufferExpansionFailed);
    tcase_add_test(test_case_ADynArray_addArray, test_ADynArray_appendArray_failure_nullptrDestArray);
    suite_add_tcase(s, test_case_ADynArray_addArray);

    test_case_ADynArray_addADynArray = tcase_create("Dynamic Array Test Case: ADynArray_appendADynArray");
    tcase_add_test(test_case_ADynArray_addADynArray, test_ADynArray_appendADynArray_success_enoughCapacity);
    tcase_add_test(test_case_ADynArray_addADynArray, test_ADynArray_appendADynArray_success_notEnoughCapacity);
    tcase_add_test(test_case_ADynArray_addADynArray, test_ADynArray_appendADynArray_success_nullptrSrcArray);
    tcase_add_test(test_case_ADynArray_addADynArray, test_ADynArray_appendADynArray_success_zeroSizeSrcArray);
    tcase_add_test(test_case_ADynArray_addADynArray, test_ADynArray_appendADynArray_failure_bufferExpansionFailed);
    tcase_add_test(test_case_ADynArray_addADynArray, test_ADynArray_appendADynArray_failure_nullptrDestArray);
    suite_add_tcase(s, test_case_ADynArray_addADynArray);

    test_case_ADynArray_set = tcase_create("Dynamic Array Test Case: ADynArray_set");
    tcase_add_test(test_case_ADynArray_set, test_ADynArray_set_success_indexInBounds);
    tcase_add_test(test_case_ADynArray_set, test_ADynArray_set_success_indexBeyondSize);
    tcase_add_test(test_case_ADynArray_set, test_ADynArray_set_success_indexBeyondSize_bufferExpanded);
    tcase_add_test(test_case_ADynArray_set, test_ADynArray_set_failure_indexBeyondSize_bufferExpansionFailed);
    tcase_add_test(test_case_ADynArray_set, test_ADynArray_set_failure_nullptr);
    suite_add_tcase(s, test_case_ADynArray_set);

    test_case_ADynArray_remove = tcase_create("Dynamic Array Test Case: ADynArray_remove");
    tcase_add_test(test_case_ADynArray_remove, test_ADynArray_remove_indexRangeInBounds);
    tcase_add_test(test_case_ADynArray_remove, test_ADynArray_remove_rangeBeyondBounds);
    tcase_add_test(test_case_ADynArray_remove, test_ADynArray_remove_zeroRange);
    tcase_add_test(test_case_ADynArray_remove, test_ADynArray_remove_indexBeyoundBounds);
    tcase_add_test(test_case_ADynArray_remove, test_ADynArray_remove_nullptr);
    suite_add_tcase(s, test_case_ADynArray_remove);

    test_case_ADynArray_mixed = tcase_create("Dynamic Array Test Case: ADynArray mixed tests");
    tcase_add_test(test_case_ADynArray_mixed, test_aDynArrayMixedWithStruct);
    suite_add_tcase(s, test_case_ADynArray_mixed);

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