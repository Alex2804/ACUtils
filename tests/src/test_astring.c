#include "check.h"

#include <stdlib.h>

#include "ACUtils/astring.h"

static size_t private_ACUtilsTest_AString_reallocCount = 0;
static size_t private_ACUtilsTest_AString_reallocFailCounter = 0;
static bool private_ACUtilsTest_AString_reallocFail = false;
static size_t private_ACUtilsTest_AString_freeCount = 0;

static void* private_ACUtilsTest_AString_realloc(void* ptr, size_t size) {
    if(!private_ACUtilsTest_AString_reallocFail || private_ACUtilsTest_AString_reallocFailCounter > 0) {
        if(private_ACUtilsTest_AString_reallocFail)
            --private_ACUtilsTest_AString_reallocFailCounter;
        void* tmp = realloc(ptr, size);
        if(tmp != NULL)
            ++private_ACUtilsTest_AString_reallocCount;
        return tmp;
    }
    return NULL;
}
static void private_ACUtilsTest_AString_free(void* ptr) {
    if(ptr != NULL)
        ++private_ACUtilsTest_AString_freeCount;
    free(ptr);
}

static size_t ASTRING_MIN_CAPACITY = 8;
static double ASTRING_CAPACITY_MULTIPLIER = 2;

#ifndef ACUTILS_ONE_SOURCE
struct AString
{
    const ACUtilsReallocator reallocator;
    const ACUtilsDeallocator deallocator;
    size_t capacity;
    size_t size;
    char *buffer;
};
#endif

START_TEST(test_AString_construct_destruct_valid)
{
    struct AString* string;
    string = AString_construct();
    ck_assert_ptr_nonnull(string->reallocator);
    ck_assert_ptr_nonnull(string->deallocator);
    ck_assert_uint_eq(string->size, 0);
    ck_assert_ptr_nonnull(string->buffer);
    AString_destruct(string);
}
END_TEST
START_TEST(test_AString_construct_destruct_withAllocator_valid)
{
    struct AString *string;
    private_ACUtilsTest_AString_reallocFail = false;
    private_ACUtilsTest_AString_reallocCount = private_ACUtilsTest_AString_freeCount = 0;
    string = AString_constructWithAllocator(private_ACUtilsTest_AString_realloc, private_ACUtilsTest_AString_free);
    ck_assert_ptr_eq(string->reallocator, private_ACUtilsTest_AString_realloc);
    ck_assert_ptr_eq(string->deallocator, private_ACUtilsTest_AString_free);
    ck_assert_uint_eq(string->size, 0);
    ck_assert_ptr_nonnull(string->buffer);
    AString_destruct(string);
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, private_ACUtilsTest_AString_freeCount);
}
END_TEST
START_TEST(test_AString_construct_destruct_withAllocator_invalid)
{
    struct AString *string;
    private_ACUtilsTest_AString_reallocFail = false;
    private_ACUtilsTest_AString_reallocCount = private_ACUtilsTest_AString_freeCount = 0;
    string = AString_constructWithAllocator(NULL, private_ACUtilsTest_AString_free);
    ck_assert_ptr_null(string);
    string = AString_constructWithAllocator(private_ACUtilsTest_AString_realloc, NULL);
    ck_assert_ptr_null(string);
}
END_TEST
START_TEST(test_AString_construct_destruct_noMemoryAvailable)
{
    struct AString *string;
    private_ACUtilsTest_AString_reallocFailCounter = 0;
    private_ACUtilsTest_AString_reallocFail = true;
    private_ACUtilsTest_AString_reallocCount = private_ACUtilsTest_AString_freeCount = 0;
    string = AString_constructWithAllocator(private_ACUtilsTest_AString_realloc, private_ACUtilsTest_AString_free);
    ck_assert_ptr_null(string);
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    ck_assert_uint_eq(private_ACUtilsTest_AString_freeCount, 0);
    private_ACUtilsTest_AString_reallocFailCounter = 1;
    string = AString_constructWithAllocator(private_ACUtilsTest_AString_realloc, private_ACUtilsTest_AString_free);
    ck_assert_ptr_null(string);
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 1);
    ck_assert_uint_eq(private_ACUtilsTest_AString_freeCount, 1);
}
END_TEST
START_TEST(test_AString_construct_destruct_nullptr)
{
    struct AString *string = NULL;
    private_ACUtilsTest_AString_freeCount = 0;
    AString_destruct(string);
    ck_assert_uint_eq(private_ACUtilsTest_AString_freeCount, 0);
}
END_TEST


START_TEST(test_AString_size_valid)
{
    struct AString string = {.reallocator = NULL, .deallocator = NULL};
    string.size = 42;
    string.capacity = 0;
    string.buffer = NULL;
    ck_assert_uint_eq(AString_size(&string), 42);
    string.size = 13;
    ck_assert_uint_eq(AString_size(&string), 13);
    string.size = 0;
    ck_assert_uint_eq(AString_size(&string), 0);
}
START_TEST(test_AString_size_nullptr)
{
    struct AString* string = NULL;
    ck_assert_uint_eq(AString_size(string), 0);
}


START_TEST(test_AString_capacity_valid)
{
    struct AString string = {.reallocator = NULL, .deallocator = NULL};
    string.size = 0;
    string.capacity = 42;
    string.buffer = NULL;
    ck_assert_uint_eq(AString_capacity(&string), 42);
    string.capacity = 13;
    ck_assert_uint_eq(AString_capacity(&string), 13);
    string.capacity = 0;
    ck_assert_uint_eq(AString_capacity(&string), 0);
}
START_TEST(test_AString_capacity_nullptr)
{
    struct AString* string = NULL;
    ck_assert_uint_eq(AString_capacity(string), 0);
}


START_TEST(test_AString_buffer_valid)
{
    struct AString string = {.reallocator = NULL, .deallocator = NULL};
    string.size = 0;
    string.capacity = 0;
    string.buffer = (char*) 42;
    ck_assert_ptr_eq(AString_buffer(&string), (char*) 42);
    string.buffer = (char*) 13;
    ck_assert_ptr_eq(AString_buffer(&string), (char*) 13);
    string.buffer = NULL;
    ck_assert_ptr_null(AString_buffer(&string));
}
START_TEST(test_AString_buffer_nullptr)
{
    struct AString* string = NULL;
    ck_assert_ptr_null(AString_buffer(string));
}


START_TEST(test_AString_reserve_success_enoughCapacity)
{
    size_t i;
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = ASTRING_MIN_CAPACITY;
    string.capacity = ASTRING_MIN_CAPACITY;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    for(i = 0; i < string.size; ++i)
        string.buffer[i] = '5';
    string.buffer[string.size] = '0';
    private_ACUtilsTest_AString_reallocFailCounter = 0;
    private_ACUtilsTest_AString_reallocFail = true;
    ck_assert_uint_eq(AString_reserve(&string, 1), true);
    ck_assert_uint_eq(string.size, ASTRING_MIN_CAPACITY);
    ck_assert_uint_eq(string.capacity, ASTRING_MIN_CAPACITY);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_uint_eq(AString_reserve(&string, string.capacity), true);
    ck_assert_uint_eq(string.size, ASTRING_MIN_CAPACITY);
    ck_assert_uint_eq(string.capacity, ASTRING_MIN_CAPACITY);
    ck_assert_ptr_nonnull(string.buffer);
    for(i = 0; i < ASTRING_MIN_CAPACITY; ++i)
        ck_assert_int_eq(string.buffer[i], '5');
    ck_assert_int_eq(string.buffer[ASTRING_MIN_CAPACITY], '0');
    string.deallocator(string.buffer);
}
START_TEST(test_AString_reserve_success_notEnoughCapacity)
{
    size_t i;
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = ASTRING_MIN_CAPACITY;
    string.capacity = ASTRING_MIN_CAPACITY;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    for(i = 0; i < string.size; ++i)
        string.buffer[i] = '5';
    string.buffer[string.size] = '0';
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_reserve(&string, ASTRING_MIN_CAPACITY + 1), true);
    ck_assert_uint_eq(string.size, ASTRING_MIN_CAPACITY);
    ck_assert_uint_eq(string.capacity, ASTRING_MIN_CAPACITY * ASTRING_CAPACITY_MULTIPLIER);
    ck_assert_ptr_nonnull(string.buffer);
    for(i = 0; i < ASTRING_MIN_CAPACITY; ++i)
        ck_assert_int_eq(string.buffer[i], '5');
    ck_assert_int_eq(string.buffer[ASTRING_MIN_CAPACITY], '0');
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 1);
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_reserve_failure_noMemoryAvailable)
{
    size_t i;
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = ASTRING_MIN_CAPACITY;
    string.capacity = ASTRING_MIN_CAPACITY;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    for(i = 0; i < string.size; ++i)
        string.buffer[i] = '5';
    string.buffer[string.size] = '0';
    private_ACUtilsTest_AString_reallocFailCounter = 0;
    private_ACUtilsTest_AString_reallocFail = true;
    ck_assert_uint_eq(AString_reserve(&string, ASTRING_MIN_CAPACITY + 1), false);
    ck_assert_uint_eq(string.size, ASTRING_MIN_CAPACITY);
    ck_assert_uint_eq(string.capacity, ASTRING_MIN_CAPACITY);
    ck_assert_ptr_nonnull(string.buffer);
    for(i = 0; i < ASTRING_MIN_CAPACITY; ++i)
        ck_assert_int_eq(string.buffer[i], '5');
    ck_assert_int_eq(string.buffer[ASTRING_MIN_CAPACITY], '0');
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_reserve_failure_nullptr)
{
    struct AString *string = NULL;
    private_ACUtilsTest_AString_reallocFail = false;
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_reserve(string, 42), false);
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
}
END_TEST


START_TEST(test_AString_shrinkToFit_success_hasLeastCapacity)
{
    size_t i;
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = ASTRING_MIN_CAPACITY + 1;
    string.capacity = ASTRING_MIN_CAPACITY + 1;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    for(i = 0; i < string.size; ++i)
        string.buffer[i] = '5';
    string.buffer[string.size] = '0';
    private_ACUtilsTest_AString_reallocFail = false;
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_shrinkToFit(&string), true);
    ck_assert_uint_eq(string.size, ASTRING_MIN_CAPACITY + 1);
    ck_assert_uint_eq(string.capacity, ASTRING_MIN_CAPACITY + 1);
    ck_assert_ptr_nonnull(string.buffer);
    for(i = 0; i < ASTRING_MIN_CAPACITY + 1; ++i)
        ck_assert_int_eq(string.buffer[i], '5');
    ck_assert_int_eq(string.buffer[ASTRING_MIN_CAPACITY + 1], '0');
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    string.deallocator(string.buffer);
}
START_TEST(test_AString_shrinkToFit_success_smallerThanMinSize)
{
    size_t i;
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = ASTRING_MIN_CAPACITY - 1;
    string.capacity = ASTRING_MIN_CAPACITY + 1;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    for(i = 0; i < string.size; ++i)
        string.buffer[i] = '5';
    string.buffer[string.size] = '0';
    private_ACUtilsTest_AString_reallocFail = false;
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_shrinkToFit(&string), true);
    ck_assert_uint_eq(string.size, ASTRING_MIN_CAPACITY - 1);
    ck_assert_uint_eq(string.capacity, ASTRING_MIN_CAPACITY);
    ck_assert_ptr_nonnull(string.buffer);
    for(i = 0; i < ASTRING_MIN_CAPACITY - 1; ++i)
        ck_assert_int_eq(string.buffer[i], '5');
    ck_assert_int_eq(string.buffer[ASTRING_MIN_CAPACITY - 1], '0');
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 1);
    string.deallocator(string.buffer);
}
START_TEST(test_AString_shrinkToFit_success_hasNotLeastCapacity)
{
    size_t i;
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = ASTRING_MIN_CAPACITY + 1;
    string.capacity = ASTRING_MIN_CAPACITY + 2;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    for(i = 0; i < string.size; ++i)
        string.buffer[i] = '5';
    string.buffer[string.size] = '0';
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_shrinkToFit(&string), true);
    ck_assert_uint_eq(string.size, ASTRING_MIN_CAPACITY + 1);
    ck_assert_uint_eq(string.capacity, ASTRING_MIN_CAPACITY + 1);
    ck_assert_ptr_nonnull(string.buffer);
    for(i = 0; i < ASTRING_MIN_CAPACITY + 1; ++i)
        ck_assert_int_eq(string.buffer[i], '5');
    ck_assert_int_eq(string.buffer[ASTRING_MIN_CAPACITY + 1], '0');
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 1);
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_shrinkToFit_failure_noMemoryAvailable)
{
    size_t i;
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = ASTRING_MIN_CAPACITY + 1;
    string.capacity = ASTRING_MIN_CAPACITY + 2;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    for(i = 0; i < string.size; ++i)
        string.buffer[i] = '5';
    string.buffer[string.size] = '0';
    private_ACUtilsTest_AString_reallocFailCounter = 0;
    private_ACUtilsTest_AString_reallocFail = true;
    ck_assert_uint_eq(AString_shrinkToFit(&string), false);
    ck_assert_uint_eq(string.size, ASTRING_MIN_CAPACITY + 1);
    ck_assert_uint_eq(string.capacity, ASTRING_MIN_CAPACITY + 2);
    ck_assert_ptr_nonnull(string.buffer);
    for(i = 0; i < ASTRING_MIN_CAPACITY + 1; ++i)
        ck_assert_int_eq(string.buffer[i], '5');
    ck_assert_int_eq(string.buffer[ASTRING_MIN_CAPACITY + 1], '0');
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_shrinkToFit_failure_nullptr)
{
    struct AString *string = NULL;
    private_ACUtilsTest_AString_reallocFail = false;
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_shrinkToFit(string), false);
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
}
END_TEST


START_TEST(test_AString_clear)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = ASTRING_MIN_CAPACITY + 1;
    string.capacity = (size_t) (ASTRING_MIN_CAPACITY * ASTRING_CAPACITY_MULTIPLIER);
    string.buffer = string.reallocator(NULL, string.capacity);
    private_ACUtilsTest_AString_reallocFailCounter = 0;
    private_ACUtilsTest_AString_reallocFail = true;
    private_ACUtilsTest_AString_reallocCount = 0;
    AString_clear(&string);
    ck_assert_uint_eq(string.size, 0);
    ck_assert_uint_eq(string.capacity, ASTRING_MIN_CAPACITY * ASTRING_CAPACITY_MULTIPLIER);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_clear_nullptr)
{
    struct AString *string = NULL;
    private_ACUtilsTest_AString_reallocFail = false;
    private_ACUtilsTest_AString_reallocCount = 0;
    AString_clear(string);
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
}
END_TEST


START_TEST(test_AString_remove_indexRangeInBounds)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 10;
    string.capacity = 16;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "0123456789", 11);
    private_ACUtilsTest_AString_reallocCount = 0;
    AString_remove(&string, 2, 6);
    ck_assert_uint_eq(string.size, 4);
    ck_assert_uint_eq(string.capacity, 16);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "0189");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_remove_rangeBeyondBounds)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 10;
    string.capacity = 16;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "0123456789", 11);
    private_ACUtilsTest_AString_reallocCount = 0;
    AString_remove(&string, 2, 100);
    ck_assert_uint_eq(string.size, 2);
    ck_assert_uint_eq(string.capacity, 16);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "01");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_remove_zeroRange)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 10;
    string.capacity = 16;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "0123456789", 11);
    private_ACUtilsTest_AString_reallocCount = 0;
    AString_remove(&string, 2, 0);
    ck_assert_uint_eq(string.size, 10);
    ck_assert_uint_eq(string.capacity, 16);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "0123456789");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_remove_indexBeyoundBounds)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 10;
    string.capacity = 16;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "0123456789", 11);
    private_ACUtilsTest_AString_reallocCount = 0;
    AString_remove(&string, 13, 5);
    ck_assert_uint_eq(string.size, 10);
    ck_assert_uint_eq(string.capacity, 16);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "0123456789");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_remove_nullptr)
{
    struct AString *string = NULL;
    private_ACUtilsTest_AString_reallocFail = false;
    private_ACUtilsTest_AString_reallocCount = 0;
    AString_remove(string, 5, 10);
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
}
END_TEST


START_TEST(test_AString_insert_success_zeroIndex)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 5;
    string.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "1234", 5);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_insert(&string, 0, '0'), true);
    ck_assert_uint_eq(string.size, 6);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "01234");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_insert_success_middleIndex)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 5;
    string.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "0134", 5);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_insert(&string, 2, '2'), true);
    ck_assert_uint_eq(string.size, 6);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "01234");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_insert_success_endIndex)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 4;
    string.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "0123", 5);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_insert(&string, 4, '4'), true);
    ck_assert_uint_eq(string.size, 5);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_int_eq(string.buffer[4], '4');
    ck_assert_str_eq(string.buffer, "01234");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_insert_success_beyondEndIndex)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 4;
    string.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "0123", 5);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_insert(&string, 666, '4'), true);
    ck_assert_uint_eq(string.size, 5);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_int_eq(string.buffer[4], '4');
    ck_assert_str_eq(string.buffer, "01234");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_insert_success_bufferExpanded)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 8;
    string.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "01345678", 9);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_insert(&string, 2, '2'), true);
    ck_assert_uint_eq(string.size, 9);
    ck_assert_uint_eq(string.capacity, 16);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_int_eq(string.buffer[2], '2');
    ck_assert_str_eq(string.buffer, "012345678");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 1);
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_insert_failure_bufferExpansionFailed)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 8;
    string.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "01345678", 9);
    private_ACUtilsTest_AString_reallocFailCounter = 0;
    private_ACUtilsTest_AString_reallocFail = true;
    ck_assert_uint_eq(AString_insert(&string, 2, '2'), false);
    ck_assert_uint_eq(string.size, 8);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "01345678");
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_insert_failure_nullptr)
{
    struct AString *string = NULL;
    private_ACUtilsTest_AString_reallocFail = false;
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_insert(string, 0, '0'), false);
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
}
END_TEST


START_TEST(test_AString_insertCString_success_zeroIndex)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 5;
    string.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "3456", 6);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_insertCString(&string, 0, "012", 3), true);
    ck_assert_uint_eq(string.size, 8);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "0123456");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_insertCString_success_middleIndex)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 5;
    string.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "01567", 6);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_insertCString(&string, 2, "234", 3), true);
    ck_assert_uint_eq(string.size, 8);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "01234567");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_insertCString_success_endIndex)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 5;
    string.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "01234", 6);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_insertCString(&string, 5, "567", 3), true);
    ck_assert_uint_eq(string.size, 8);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "01234567");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_insertCString_success_beyondEndIndex)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 5;
    string.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "01234", 6);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_insertCString(&string, 666, "567", 3), true);
    ck_assert_uint_eq(string.size, 8);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "01234567");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_insertArray_success_bufferExpanded)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 8;
    string.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "01567890", 9);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_insertCString(&string, 2, "234", 3), true);
    ck_assert_uint_eq(string.size, 11);
    ck_assert_uint_eq(string.capacity, 16);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "01234567890");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 1);
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_insertArray_success_nullptrArray)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    char* nullptrArray = NULL;
    string.size = 5;
    string.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "01567", 6);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_insertCString(&string, 2, nullptrArray, 3), true);
    ck_assert_uint_eq(string.size, 5);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "01567");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_insertArray_success_zeroArraySize)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 5;
    string.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "01567", 6);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_insertCString(&string, 2, "234", 0), true);
    ck_assert_uint_eq(string.size, 5);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "01567");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_insertArray_success_negativeIndexGetsMaxIndex)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 5;
    string.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "01234", 6);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_insertCString(&string, -1, "567", 3), true);
    ck_assert_uint_eq(string.size, 8);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "01234567");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_insertArray_failure_bufferExpansionFailed)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 8;
    string.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "01567890", 9);
    private_ACUtilsTest_AString_reallocFailCounter = 0;
    private_ACUtilsTest_AString_reallocFail = true;
    ck_assert_uint_eq(AString_insertCString(&string, 2, "234", 3), false);
    ck_assert_uint_eq(string.size, 8);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "01567890");
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_insertArray_failure_nullptrDestArray)
{
    struct AString* string = NULL;
    private_ACUtilsTest_AString_reallocFail = false;
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_insertCString(string, 0, "012", 3), false);
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
}
END_TEST


START_TEST(test_AString_insertAString_success_zeroIndex)
{
    struct AString destString = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    struct AString srcString = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    destString.size = 5;
    destString.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    destString.buffer = destString.reallocator(NULL, destString.capacity + 1);
    memcpy(destString.buffer, "3456", 5);
    srcString.size = 3;
    srcString.capacity = 8;
    srcString.buffer = srcString.reallocator(NULL, srcString.capacity + 1);
    memcpy(srcString.buffer, "012", 4);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_insertAString(&destString, 0, &srcString), true);
    ck_assert_uint_eq(destString.size, 8);
    ck_assert_uint_eq(destString.capacity, 8);
    ck_assert_ptr_nonnull(destString.buffer);
    ck_assert_str_eq(destString.buffer, "0123456");
    ck_assert_uint_eq(srcString.size, 3);
    ck_assert_uint_eq(srcString.capacity, 8);
    ck_assert_ptr_nonnull(srcString.buffer);
    ck_assert_str_eq(srcString.buffer, "012");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    destString.deallocator(destString.buffer);
    srcString.deallocator(srcString.buffer);
}
END_TEST
START_TEST(test_AString_insertAString_success_middleIndex)
{
    struct AString destString = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    struct AString srcString = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    destString.size = 5;
    destString.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    destString.buffer = destString.reallocator(NULL, destString.capacity + 1);
    memcpy(destString.buffer, "0156", 5);
    srcString.size = 3;
    srcString.capacity = 8;
    srcString.buffer = srcString.reallocator(NULL, srcString.capacity + 1);
    memcpy(srcString.buffer, "234", 4);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_insertAString(&destString, 2, &srcString), true);
    ck_assert_uint_eq(destString.size, 8);
    ck_assert_uint_eq(destString.capacity, 8);
    ck_assert_ptr_nonnull(destString.buffer);
    ck_assert_str_eq(destString.buffer, "0123456");
    ck_assert_uint_eq(srcString.size, 3);
    ck_assert_uint_eq(srcString.capacity, 8);
    ck_assert_ptr_nonnull(srcString.buffer);
    ck_assert_str_eq(srcString.buffer, "234");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    destString.deallocator(destString.buffer);
    srcString.deallocator(srcString.buffer);
}
END_TEST
START_TEST(test_AString_insertAString_success_endIndex)
{
    struct AString destString = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    struct AString srcString = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    destString.size = 5;
    destString.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    destString.buffer = destString.reallocator(NULL, destString.capacity + 1);
    memcpy(destString.buffer, "01234", 5);
    srcString.size = 3;
    srcString.capacity = 8;
    srcString.buffer = srcString.reallocator(NULL, srcString.capacity + 1);
    memcpy(srcString.buffer, "567", 4);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_insertAString(&destString, 5, &srcString), true);
    ck_assert_uint_eq(destString.size, 8);
    ck_assert_uint_eq(destString.capacity, 8);
    ck_assert_ptr_nonnull(destString.buffer);
    ck_assert_str_eq(destString.buffer, "01234567");
    ck_assert_uint_eq(srcString.size, 3);
    ck_assert_uint_eq(srcString.capacity, 8);
    ck_assert_ptr_nonnull(srcString.buffer);
    ck_assert_str_eq(srcString.buffer, "567");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    destString.deallocator(destString.buffer);
    srcString.deallocator(srcString.buffer);
}
END_TEST
START_TEST(test_AString_insertAString_success_beyondEndIndex)
{
    struct AString destString = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    struct AString srcString = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    destString.size = 5;
    destString.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    destString.buffer = destString.reallocator(NULL, destString.capacity + 1);
    memcpy(destString.buffer, "01234", 5);
    srcString.size = 3;
    srcString.capacity = 8;
    srcString.buffer = srcString.reallocator(NULL, srcString.capacity + 1);
    memcpy(srcString.buffer, "567", 4);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_insertAString(&destString, 666, &srcString), true);
    ck_assert_uint_eq(destString.size, 8);
    ck_assert_uint_eq(destString.capacity, 8);
    ck_assert_ptr_nonnull(destString.buffer);
    ck_assert_str_eq(destString.buffer, "01234567");
    ck_assert_uint_eq(srcString.size, 3);
    ck_assert_uint_eq(srcString.capacity, 8);
    ck_assert_ptr_nonnull(srcString.buffer);
    ck_assert_str_eq(srcString.buffer, "567");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    destString.deallocator(destString.buffer);
    srcString.deallocator(srcString.buffer);
}
END_TEST
START_TEST(test_AString_insertAString_success_bufferExpanded)
{
    struct AString destString = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    struct AString srcString = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    destString.size = 5;
    destString.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    destString.buffer = destString.reallocator(NULL, destString.capacity + 1);
    memcpy(destString.buffer, "01678", 6);
    srcString.size = 4;
    srcString.capacity = 8;
    srcString.buffer = srcString.reallocator(NULL, srcString.capacity + 1);
    memcpy(srcString.buffer, "2345", 5);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_insertAString(&destString, 2, &srcString), true);
    ck_assert_uint_eq(destString.size, 9);
    ck_assert_uint_eq(destString.capacity, 16);
    ck_assert_ptr_nonnull(destString.buffer);
    ck_assert_str_eq(destString.buffer, "012345678");
    ck_assert_uint_eq(srcString.size, 4);
    ck_assert_uint_eq(srcString.capacity, 8);
    ck_assert_ptr_nonnull(srcString.buffer);
    ck_assert_str_eq(srcString.buffer, "2345");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 1);
    destString.deallocator(destString.buffer);
    srcString.deallocator(srcString.buffer);
}
END_TEST
START_TEST(test_AString_insertAString_success_nullptrSrcArray)
{
    struct AString destString = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    struct AString *srcString = NULL;
    destString.size = 5;
    destString.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    destString.buffer = destString.reallocator(NULL, destString.capacity + 1);
    memcpy(destString.buffer, "01234", 6);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_insertAString(&destString, 2, srcString), true);
    ck_assert_uint_eq(destString.size, 5);
    ck_assert_uint_eq(destString.capacity, 8);
    ck_assert_ptr_nonnull(destString.buffer);
    ck_assert_str_eq(destString.buffer, "01234");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    destString.deallocator(destString.buffer);
}
END_TEST
START_TEST(test_AString_insertAString_success_zeroSizeSrcArray)
{
    struct AString destString = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    struct AString srcString = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    destString.size = 5;
    destString.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    destString.buffer = destString.reallocator(NULL, destString.capacity + 1);
    memcpy(destString.buffer, "01567", 6);
    srcString.size = 0;
    srcString.capacity = 8;
    srcString.buffer = srcString.reallocator(NULL, srcString.capacity + 1);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_insertAString(&destString, 2, &srcString), true);
    ck_assert_uint_eq(destString.size, 5);
    ck_assert_uint_eq(destString.capacity, 8);
    ck_assert_ptr_nonnull(destString.buffer);
    ck_assert_str_eq(destString.buffer, "01567");
    ck_assert_uint_eq(srcString.size, 0);
    ck_assert_uint_eq(srcString.capacity, 8);
    ck_assert_ptr_nonnull(srcString.buffer);
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    destString.deallocator(destString.buffer);
    srcString.deallocator(srcString.buffer);
}
END_TEST
START_TEST(test_AString_insertAString_failure_bufferExpansionFailed)
{
    struct AString destString = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    struct AString srcString = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    destString.size = 5;
    destString.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    destString.buffer = destString.reallocator(NULL, destString.capacity + 1);
    memcpy(destString.buffer, "01678", 6);
    srcString.size = 4;
    srcString.capacity = 8;
    srcString.buffer = srcString.reallocator(NULL, srcString.capacity + 1);
    memcpy(srcString.buffer, "2345", 5);
    private_ACUtilsTest_AString_reallocFailCounter = 0;
    private_ACUtilsTest_AString_reallocFail = true;
    ck_assert_uint_eq(AString_insertAString(&destString, 2, &srcString), false);
    ck_assert_uint_eq(destString.size, 5);
    ck_assert_uint_eq(destString.capacity, 8);
    ck_assert_ptr_nonnull(destString.buffer);
    ck_assert_str_eq(destString.buffer, "01678");
    ck_assert_uint_eq(srcString.size, 4);
    ck_assert_uint_eq(srcString.capacity, 8);
    ck_assert_ptr_nonnull(srcString.buffer);
    ck_assert_str_eq(srcString.buffer, "2345");
    destString.deallocator(destString.buffer);
    srcString.deallocator(srcString.buffer);
}
END_TEST
START_TEST(test_AString_insertAString_failure_nullptrDestArray)
{
    struct AString srcString = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    struct AString *destString = NULL;
    srcString.size = 3;
    srcString.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    srcString.buffer = srcString.reallocator(NULL, srcString.capacity + 1);
    memcpy(srcString.buffer, "012", 4);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_insertAString(destString, 0, &srcString), false);
    ck_assert_uint_eq(srcString.size, 3);
    ck_assert_uint_eq(srcString.capacity, 8);
    ck_assert_ptr_nonnull(srcString.buffer);
    ck_assert_str_eq(srcString.buffer, "012");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    srcString.deallocator(srcString.buffer);
}
END_TEST


START_TEST(test_AString_append_success_enoughCapacity)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 5;
    string.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "01234", 6);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_append(&string, '5'), true);
    ck_assert_uint_eq(string.size, 6);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "012345");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_append_success_notEnoughCapacity)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 8;
    string.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "01234567", 9);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_append(&string, '8'), true);
    ck_assert_uint_eq(string.size, 9);
    ck_assert_uint_eq(string.capacity, 16);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "012345678");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 1);
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_append_failure_bufferExpansionFailed)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 8;
    string.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "01234567", 9);
    private_ACUtilsTest_AString_reallocFailCounter = 0;
    private_ACUtilsTest_AString_reallocFail = true;
    ck_assert_uint_eq(AString_append(&string, '8'), false);
    ck_assert_uint_eq(string.size, 8);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "01234567");
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_append_failure_nullptr)
{
    struct AString *string = NULL;
    private_ACUtilsTest_AString_reallocFail = false;
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_append(string, '0'), false);
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
}
END_TEST


START_TEST(test_AString_appendCString_success_enoughCapacity)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 5;
    string.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "01234", 6);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_appendCString(&string, "567", 3), true);
    ck_assert_uint_eq(string.size, 8);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "01234567");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_appendCString_success_notEnoughCapacity)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 5;
    string.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "01234", 6);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_appendCString(&string, "5678", 4), true);
    ck_assert_uint_eq(string.size, 9);
    ck_assert_uint_eq(string.capacity, 16);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "012345678");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 1);
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_appendCString_success_nullptrArray)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    const char* nullptrArray = NULL;
    string.size = 5;
    string.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "01234", 6);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_appendCString(&string, nullptrArray, 4), true);
    ck_assert_uint_eq(string.size, 5);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "01234");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_appendCString_success_zeroArraySize)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 5;
    string.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "01234", 6);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_appendCString(&string, "567", 0), true);
    ck_assert_uint_eq(string.size, 5);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "01234");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_appendCString_failure_bufferExpansionFailed)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 5;
    string.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "01234", 6);
    private_ACUtilsTest_AString_reallocFailCounter = 0;
    private_ACUtilsTest_AString_reallocFail = true;
    ck_assert_uint_eq(AString_appendCString(&string, "5678", 4), false);
    ck_assert_uint_eq(string.size, 5);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "01234");
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_appendCString_failure_nullptrDestArray)
{
    struct AString *string = NULL;
    private_ACUtilsTest_AString_reallocFail = false;
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_insertCString(string, 0, "012", 3), false);
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
}
END_TEST


START_TEST(test_AString_appendAString_success_enoughCapacity)
{
    struct AString destString = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    struct AString srcString = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    destString.size = 5;
    destString.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    destString.buffer = destString.reallocator(NULL, destString.capacity + 1);
    memcpy(destString.buffer, "01234", 6);
    srcString.size = 3;
    srcString.capacity = 8;
    srcString.buffer = srcString.reallocator(NULL, srcString.capacity + 1);
    memcpy(srcString.buffer, "567", 4);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_appendAString(&destString, &srcString), true);
    ck_assert_uint_eq(destString.size, 8);
    ck_assert_uint_eq(destString.capacity, 8);
    ck_assert_ptr_nonnull(destString.buffer);
    ck_assert_str_eq(destString.buffer, "01234567");
    ck_assert_uint_eq(srcString.size, 3);
    ck_assert_uint_eq(srcString.capacity, 8);
    ck_assert_ptr_nonnull(srcString.buffer);
    ck_assert_str_eq(srcString.buffer, "567");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    destString.deallocator(destString.buffer);
    srcString.deallocator(srcString.buffer);
}
END_TEST
START_TEST(test_AString_appendAString_success_notEnoughCapacity)
{
    struct AString destString = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    struct AString srcString = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    destString.size = 5;
    destString.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    destString.buffer = destString.reallocator(NULL, destString.capacity + 1);
    memcpy(destString.buffer, "01234", 6);
    srcString.size = 4;
    srcString.capacity = 8;
    srcString.buffer = srcString.reallocator(NULL, srcString.capacity + 1);
    memcpy(srcString.buffer, "5678", 5);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_appendAString(&destString, &srcString), true);
    ck_assert_uint_eq(destString.size, 9);
    ck_assert_uint_eq(destString.capacity, 16);
    ck_assert_ptr_nonnull(destString.buffer);
    ck_assert_str_eq(destString.buffer, "012345678");
    ck_assert_uint_eq(srcString.size, 4);
    ck_assert_uint_eq(srcString.capacity, 8);
    ck_assert_ptr_nonnull(srcString.buffer);
    ck_assert_str_eq(srcString.buffer, "5678");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 1);
    destString.deallocator(destString.buffer);
    srcString.deallocator(srcString.buffer);
}
END_TEST
START_TEST(test_AString_appendAString_success_nullptrSrcArray)
{
    struct AString destString = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    struct AString *srcString = NULL;
    destString.size = 5;
    destString.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    destString.buffer = destString.reallocator(NULL, destString.capacity + 1);
    memcpy(destString.buffer, "01234", 6);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_appendAString(&destString, srcString), true);
    ck_assert_uint_eq(destString.size, 5);
    ck_assert_uint_eq(destString.capacity, 8);
    ck_assert_ptr_nonnull(destString.buffer);
    ck_assert_str_eq(destString.buffer, "01234");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    destString.deallocator(destString.buffer);
}
END_TEST
START_TEST(test_AString_appendAString_success_zeroSizeSrcArray)
{
    struct AString destString = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    struct AString srcString = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    destString.size = 5;
    destString.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    destString.buffer = destString.reallocator(NULL, destString.capacity + 1);
    memcpy(destString.buffer, "01234", 6);
    srcString.size = 0;
    srcString.capacity = 8;
    srcString.buffer = srcString.reallocator(NULL, srcString.capacity + 1);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_appendAString(&destString, &srcString), true);
    ck_assert_uint_eq(destString.size, 5);
    ck_assert_uint_eq(destString.capacity, 8);
    ck_assert_ptr_nonnull(destString.buffer);
    ck_assert_str_eq(destString.buffer, "01234");
    ck_assert_uint_eq(srcString.size, 0);
    ck_assert_uint_eq(srcString.capacity, 8);
    ck_assert_ptr_nonnull(srcString.buffer);
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    destString.deallocator(destString.buffer);
    srcString.deallocator(srcString.buffer);
}
END_TEST
START_TEST(test_AString_appendAString_failure_bufferExpansionFailed)
{
    struct AString destString = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    struct AString srcString = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    destString.size = 5;
    destString.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    destString.buffer = destString.reallocator(NULL, destString.capacity + 1);
    memcpy(destString.buffer, "01234", 6);
    srcString.size = 4;
    srcString.capacity = 8;
    srcString.buffer = srcString.reallocator(NULL, srcString.capacity + 1);
    memcpy(srcString.buffer, "5678", 5);
    private_ACUtilsTest_AString_reallocFailCounter = 0;
    private_ACUtilsTest_AString_reallocFail = true;
    ck_assert_uint_eq(AString_appendAString(&destString, &srcString), false);
    ck_assert_uint_eq(destString.size, 5);
    ck_assert_uint_eq(destString.capacity, 8);
    ck_assert_ptr_nonnull(destString.buffer);
    ck_assert_str_eq(destString.buffer, "01234");
    ck_assert_uint_eq(srcString.size, 4);
    ck_assert_uint_eq(srcString.capacity, 8);
    ck_assert_ptr_nonnull(srcString.buffer);
    ck_assert_str_eq(srcString.buffer, "5678");
    destString.deallocator(destString.buffer);
    srcString.deallocator(srcString.buffer);
}
END_TEST
START_TEST(test_AString_appendAString_failure_nullptrDestArray)
{
    struct AString srcString = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    struct AString *destString = NULL;
    srcString.size = 3;
    srcString.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    srcString.buffer = srcString.reallocator(NULL, srcString.capacity + 1);
    memcpy(srcString.buffer, "012", 4);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_appendAString(destString, &srcString), false);
    ck_assert_uint_eq(srcString.size, 3);
    ck_assert_uint_eq(srcString.capacity, 8);
    ck_assert_ptr_nonnull(srcString.buffer);
    ck_assert_str_eq(srcString.buffer, "012");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    srcString.deallocator(srcString.buffer);
}
END_TEST


START_TEST(test_AString_set_success_indexInBounds)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 8;
    string.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "01234567", 9);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_set(&string, 0, 'x'), true);
    ck_assert_uint_eq(string.size, 8);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "x1234567");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    ck_assert_uint_eq(AString_set(&string, 1, 'y'), true);
    ck_assert_uint_eq(string.size, 8);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "xy234567");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    ck_assert_uint_eq(AString_set(&string, 2, 'z'), true);
    ck_assert_uint_eq(string.size, 8);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "xyz34567");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_set_success_indexBeyondSize)
{
    struct AString array = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    array.size = 7;
    array.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    array.buffer = array.reallocator(NULL, array.capacity + 1);
    memcpy(array.buffer, "0123456", 8);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_set(&array, 666, '7'), true);
    ck_assert_uint_eq(array.size, 8);
    ck_assert_uint_eq(array.capacity, 8);
    ck_assert_ptr_nonnull(array.buffer);
    ck_assert_str_eq(array.buffer, "01234567");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    array.deallocator(array.buffer);
}
END_TEST
START_TEST(test_AString_set_success_indexBeyondSize_bufferExpanded)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 8;
    string.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "01234567", 9);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_set(&string, 666, '8'), true);
    ck_assert_uint_eq(string.size, 9);
    ck_assert_uint_eq(string.capacity, 16);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "012345678");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 1);
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_set_failure_indexBeyondSize_bufferExpansionFailed)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 8;
    string.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "01234567", 9);
    private_ACUtilsTest_AString_reallocFailCounter = 0;
    private_ACUtilsTest_AString_reallocFail = true;
    ck_assert_uint_eq(AString_set(&string, 666, '8'), false);
    ck_assert_uint_eq(string.size, 8);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "01234567");
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_set_failure_nullptr)
{
    struct AString *string = NULL;
    private_ACUtilsTest_AString_reallocFail = false;
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_set(string, 0, '0'), false);
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
}
END_TEST


START_TEST(test_AString_setRange_success_indexAndRangeInBounds)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 8;
    string.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "01234567", 9);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_setRange(&string, 0, 2, 'x'), true);
    ck_assert_uint_eq(string.size, 8);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "xx234567");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    ck_assert_uint_eq(AString_setRange(&string, 1, 2, 'y'), true);
    ck_assert_uint_eq(string.size, 8);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "xyy34567");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    ck_assert_uint_eq(AString_setRange(&string, 2, 3, 'z'), true);
    ck_assert_uint_eq(string.size, 8);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "xyzzz567");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_setRange_success_indexInBoundsRangeBeyondSize)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 3;
    string.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "012", 4);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_setRange(&string, 2, 2, 'x'), true);
    ck_assert_uint_eq(string.size, 4);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "01xx");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    ck_assert_uint_eq(AString_setRange(&string, 2, 4, 'y'), true);
    ck_assert_uint_eq(string.size, 6);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "01yyyy");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_setRange_success_indexInBoundsRangeBeyondSize_bufferExpanded)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 3;
    string.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "012", 4);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_setRange(&string, 2, 7, 'x'), true);
    ck_assert_uint_eq(string.size, 9);
    ck_assert_uint_eq(string.capacity, 16);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "01xxxxxxx");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 1);
    ck_assert_uint_eq(AString_setRange(&string, 6, 13, 'y'), true);
    ck_assert_uint_eq(string.size, 19);
    ck_assert_uint_eq(string.capacity, 32);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "01xxxxyyyyyyyyyyyyy");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 2);
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_setRange_success_indexAndRangeBeyondSize)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 3;
    string.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "012", 4);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_setRange(&string, 3, 2, 'x'), true);
    ck_assert_uint_eq(string.size, 5);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "012xx");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    ck_assert_uint_eq(AString_setRange(&string, 2342, 3, 'y'), true);
    ck_assert_uint_eq(string.size, 8);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "012xxyyy");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_setRange_success_indexAndRangeBeyondSize_bufferExpanded)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 3;
    string.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "012", 4);
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_setRange(&string, 3, 6, 'x'), true);
    ck_assert_uint_eq(string.size, 9);
    ck_assert_uint_eq(string.capacity, 16);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "012xxxxxx");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 1);
    ck_assert_uint_eq(AString_setRange(&string, 666, 8, 'y'), true);
    ck_assert_uint_eq(string.size, 17);
    ck_assert_uint_eq(string.capacity, 32);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "012xxxxxxyyyyyyyy");
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 2);
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_setRange_failure_indexInBoundsRangeBeyondSize_bufferExpansionFailed)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 3;
    string.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "012", 4);
    private_ACUtilsTest_AString_reallocFailCounter = 0;
    private_ACUtilsTest_AString_reallocFail = true;
    ck_assert_uint_eq(AString_setRange(&string, 2, 7, 'x'), false);
    ck_assert_uint_eq(string.size, 3);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "012");
    ck_assert_uint_eq(AString_setRange(&string, 6, 12, 'y'), false);
    ck_assert_uint_eq(string.size, 3);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "012");
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_setRange_failure_indexAndRangeBeyondSize_bufferExpansionFailed)
{
    struct AString string = {.reallocator = private_ACUtilsTest_AString_realloc, .deallocator = private_ACUtilsTest_AString_free};
    string.size = 3;
    string.capacity = 8;
    private_ACUtilsTest_AString_reallocFail = false;
    string.buffer = string.reallocator(NULL, string.capacity + 1);
    memcpy(string.buffer, "012", 4);
    private_ACUtilsTest_AString_reallocFailCounter = 0;
    private_ACUtilsTest_AString_reallocFail = true;
    ck_assert_uint_eq(AString_setRange(&string, 3, 6, 'x'), false);
    ck_assert_uint_eq(string.size, 3);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "012");
    ck_assert_uint_eq(AString_setRange(&string, 666, 8, 'y'), false);
    ck_assert_uint_eq(string.size, 3);
    ck_assert_uint_eq(string.capacity, 8);
    ck_assert_ptr_nonnull(string.buffer);
    ck_assert_str_eq(string.buffer, "012");
    string.deallocator(string.buffer);
}
END_TEST
START_TEST(test_AString_setRange_failure_nullptr)
{
    struct AString *string = NULL;
    private_ACUtilsTest_AString_reallocFail = false;
    private_ACUtilsTest_AString_reallocCount = 0;
    ck_assert_uint_eq(AString_setRange(string, 0, 0, '0'), false);
    ck_assert_uint_eq(private_ACUtilsTest_AString_reallocCount, 0);
}
END_TEST



Suite* private_ACUtilsTest_AString_getTestSuite(void)
{
    Suite *s;
    TCase *test_case_AString_construct_destruct, *test_case_AString_size, *test_case_AString_capacity,
          *test_case_AString_buffer, *test_case_AString_reserve, *test_case_AString_shrinkToFit,
          *test_case_AString_clear, *test_case_AString_remove, *test_case_AString_insert,
          *test_case_AString_insertCString, *test_case_AString_insertAString, *test_case_AString_append,
          *test_case_AString_appendCString, *test_case_AString_appendAString, *test_case_AString_set,
          *test_case_AString_setRange, *test_case_AString_mixed;

    s = suite_create("AString Test Suite");

    test_case_AString_construct_destruct = tcase_create("AString Test Case: AString_construct / AString_destruct");
    tcase_add_test(test_case_AString_construct_destruct, test_AString_construct_destruct_valid);
    tcase_add_test(test_case_AString_construct_destruct, test_AString_construct_destruct_withAllocator_valid);
    tcase_add_test(test_case_AString_construct_destruct, test_AString_construct_destruct_withAllocator_invalid);
    tcase_add_test(test_case_AString_construct_destruct, test_AString_construct_destruct_noMemoryAvailable);
    tcase_add_test(test_case_AString_construct_destruct, test_AString_construct_destruct_nullptr);
    suite_add_tcase(s, test_case_AString_construct_destruct);

    test_case_AString_size = tcase_create("AString Test Case: AString_size");
    tcase_add_test(test_case_AString_size, test_AString_size_valid);
    tcase_add_test(test_case_AString_size, test_AString_size_nullptr);
    suite_add_tcase(s, test_case_AString_size);

    test_case_AString_capacity = tcase_create("AString Test Case: AString_capacity");
    tcase_add_test(test_case_AString_capacity, test_AString_capacity_valid);
    tcase_add_test(test_case_AString_capacity, test_AString_capacity_nullptr);
    suite_add_tcase(s, test_case_AString_capacity);

    test_case_AString_buffer = tcase_create("AString Test Case: AString_buffer");
    tcase_add_test(test_case_AString_buffer, test_AString_buffer_valid);
    tcase_add_test(test_case_AString_buffer, test_AString_buffer_nullptr);
    suite_add_tcase(s, test_case_AString_buffer);

    test_case_AString_reserve = tcase_create("AString Test Case: AString_reserve");
    tcase_add_test(test_case_AString_reserve, test_AString_reserve_success_enoughCapacity);
    tcase_add_test(test_case_AString_reserve, test_AString_reserve_success_notEnoughCapacity);
    tcase_add_test(test_case_AString_reserve, test_AString_reserve_failure_noMemoryAvailable);
    tcase_add_test(test_case_AString_reserve, test_AString_reserve_failure_nullptr);
    suite_add_tcase(s, test_case_AString_reserve);

    test_case_AString_shrinkToFit = tcase_create("AString Test Case: AString_shrinkToFit");
    tcase_add_test(test_case_AString_shrinkToFit, test_AString_shrinkToFit_success_hasLeastCapacity);
    tcase_add_test(test_case_AString_shrinkToFit, test_AString_shrinkToFit_success_smallerThanMinSize);
    tcase_add_test(test_case_AString_shrinkToFit, test_AString_shrinkToFit_success_hasNotLeastCapacity);
    tcase_add_test(test_case_AString_shrinkToFit, test_AString_shrinkToFit_failure_noMemoryAvailable);
    tcase_add_test(test_case_AString_shrinkToFit, test_AString_shrinkToFit_failure_nullptr);
    suite_add_tcase(s, test_case_AString_shrinkToFit);

    test_case_AString_clear = tcase_create("AString Test Case: AString_clear");
    tcase_add_test(test_case_AString_clear, test_AString_clear);
    tcase_add_test(test_case_AString_clear, test_AString_clear_nullptr);
    suite_add_tcase(s, test_case_AString_clear);

    test_case_AString_remove = tcase_create("AString Test Case: AString_remove");
    tcase_add_test(test_case_AString_remove, test_AString_remove_indexRangeInBounds);
    tcase_add_test(test_case_AString_remove, test_AString_remove_rangeBeyondBounds);
    tcase_add_test(test_case_AString_remove, test_AString_remove_zeroRange);
    tcase_add_test(test_case_AString_remove, test_AString_remove_indexBeyoundBounds);
    tcase_add_test(test_case_AString_remove, test_AString_remove_nullptr);
    suite_add_tcase(s, test_case_AString_remove);

    test_case_AString_insert = tcase_create("AString Test Case: AString_insert");
    tcase_add_test(test_case_AString_insert, test_AString_insert_success_zeroIndex);
    tcase_add_test(test_case_AString_insert, test_AString_insert_success_middleIndex);
    tcase_add_test(test_case_AString_insert, test_AString_insert_success_endIndex);
    tcase_add_test(test_case_AString_insert, test_AString_insert_success_beyondEndIndex);
    tcase_add_test(test_case_AString_insert, test_AString_insert_success_bufferExpanded);
    tcase_add_test(test_case_AString_insert, test_AString_insert_failure_bufferExpansionFailed);
    tcase_add_test(test_case_AString_insert, test_AString_insert_failure_nullptr);
    suite_add_tcase(s, test_case_AString_insert);

    test_case_AString_insertCString = tcase_create("AString Test Case: AString_insertCString");
    tcase_add_test(test_case_AString_insertCString, test_AString_insertCString_success_zeroIndex);
    tcase_add_test(test_case_AString_insertCString, test_AString_insertCString_success_middleIndex);
    tcase_add_test(test_case_AString_insertCString, test_AString_insertCString_success_endIndex);
    tcase_add_test(test_case_AString_insertCString, test_AString_insertCString_success_beyondEndIndex);
    tcase_add_test(test_case_AString_insertCString, test_AString_insertArray_success_bufferExpanded);
    tcase_add_test(test_case_AString_insertCString, test_AString_insertArray_success_nullptrArray);
    tcase_add_test(test_case_AString_insertCString, test_AString_insertArray_success_zeroArraySize);
    tcase_add_test(test_case_AString_insertCString, test_AString_insertArray_success_negativeIndexGetsMaxIndex);
    tcase_add_test(test_case_AString_insertCString, test_AString_insertArray_failure_bufferExpansionFailed);
    tcase_add_test(test_case_AString_insertCString, test_AString_insertArray_failure_nullptrDestArray);
    suite_add_tcase(s, test_case_AString_insertCString);

    test_case_AString_insertAString = tcase_create("AString Test Case: AString_insertAString");
    tcase_add_test(test_case_AString_insertAString, test_AString_insertAString_success_zeroIndex);
    tcase_add_test(test_case_AString_insertAString, test_AString_insertAString_success_middleIndex);
    tcase_add_test(test_case_AString_insertAString, test_AString_insertAString_success_endIndex);
    tcase_add_test(test_case_AString_insertAString, test_AString_insertAString_success_beyondEndIndex);
    tcase_add_test(test_case_AString_insertAString, test_AString_insertAString_success_bufferExpanded);
    tcase_add_test(test_case_AString_insertAString, test_AString_insertAString_success_nullptrSrcArray);
    tcase_add_test(test_case_AString_insertAString, test_AString_insertAString_success_zeroSizeSrcArray);
    tcase_add_test(test_case_AString_insertAString, test_AString_insertAString_failure_bufferExpansionFailed);
    tcase_add_test(test_case_AString_insertAString, test_AString_insertAString_failure_nullptrDestArray);
    suite_add_tcase(s, test_case_AString_insertAString);

    test_case_AString_append = tcase_create("AString Test Case: AString_append");
    tcase_add_test(test_case_AString_append, test_AString_append_success_enoughCapacity);
    tcase_add_test(test_case_AString_append, test_AString_append_success_notEnoughCapacity);
    tcase_add_test(test_case_AString_append, test_AString_append_failure_bufferExpansionFailed);
    tcase_add_test(test_case_AString_append, test_AString_append_failure_nullptr);
    suite_add_tcase(s, test_case_AString_append);

    test_case_AString_appendCString = tcase_create("AString Test Case: AString_appendCString");
    tcase_add_test(test_case_AString_appendCString, test_AString_appendCString_success_enoughCapacity);
    tcase_add_test(test_case_AString_appendCString, test_AString_appendCString_success_notEnoughCapacity);
    tcase_add_test(test_case_AString_appendCString, test_AString_appendCString_success_nullptrArray);
    tcase_add_test(test_case_AString_appendCString, test_AString_appendCString_success_zeroArraySize);
    tcase_add_test(test_case_AString_appendCString, test_AString_appendCString_failure_bufferExpansionFailed);
    tcase_add_test(test_case_AString_appendCString, test_AString_appendCString_failure_nullptrDestArray);
    suite_add_tcase(s, test_case_AString_appendCString);

    test_case_AString_appendAString = tcase_create("AString Test Case: AString_appendAString");
    tcase_add_test(test_case_AString_appendAString, test_AString_appendAString_success_enoughCapacity);
    tcase_add_test(test_case_AString_appendAString, test_AString_appendAString_success_notEnoughCapacity);
    tcase_add_test(test_case_AString_appendAString, test_AString_appendAString_success_nullptrSrcArray);
    tcase_add_test(test_case_AString_appendAString, test_AString_appendAString_success_zeroSizeSrcArray);
    tcase_add_test(test_case_AString_appendAString, test_AString_appendAString_failure_bufferExpansionFailed);
    tcase_add_test(test_case_AString_appendAString, test_AString_appendAString_failure_nullptrDestArray);
    suite_add_tcase(s, test_case_AString_appendAString);

    test_case_AString_set = tcase_create("AString Test Case: AString_set");
    tcase_add_test(test_case_AString_set, test_AString_set_success_indexInBounds);
    tcase_add_test(test_case_AString_set, test_AString_set_success_indexBeyondSize);
    tcase_add_test(test_case_AString_set, test_AString_set_success_indexBeyondSize_bufferExpanded);
    tcase_add_test(test_case_AString_set, test_AString_set_failure_indexBeyondSize_bufferExpansionFailed);
    tcase_add_test(test_case_AString_set, test_AString_set_failure_nullptr);
    suite_add_tcase(s, test_case_AString_set);

    test_case_AString_setRange = tcase_create("ADynArray Test Case: AString_setRange");
    tcase_add_test(test_case_AString_setRange, test_AString_setRange_success_indexAndRangeInBounds);
    tcase_add_test(test_case_AString_setRange, test_AString_setRange_success_indexInBoundsRangeBeyondSize);
    tcase_add_test(test_case_AString_setRange, test_AString_setRange_success_indexInBoundsRangeBeyondSize_bufferExpanded);
    tcase_add_test(test_case_AString_setRange, test_AString_setRange_success_indexAndRangeBeyondSize);
    tcase_add_test(test_case_AString_setRange, test_AString_setRange_success_indexAndRangeBeyondSize_bufferExpanded);
    tcase_add_test(test_case_AString_setRange, test_AString_setRange_failure_indexInBoundsRangeBeyondSize_bufferExpansionFailed);
    tcase_add_test(test_case_AString_setRange, test_AString_setRange_failure_indexAndRangeBeyondSize_bufferExpansionFailed);
    tcase_add_test(test_case_AString_setRange, test_AString_setRange_failure_nullptr);
    suite_add_tcase(s, test_case_AString_setRange);
    
    return s;
}
