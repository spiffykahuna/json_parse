#include "CUnit/Basic.h"

#define assertTrue(value) \
	{ CU_ASSERT(value);}

#define assertEquals(actual, expected) \
	{ CU_ASSERT_EQUAL(actual, expected); };

#define assertFalse(value) \
	{ CU_ASSERT_FALSE(value);}

#define assertNull(value) \
	{ CU_ASSERT_PTR_NULL(value);}

#define assertNotNull(value) \
	{ CU_ASSERT_PTR_NOT_NULL(value);}

#define assertSame(actual, expected) \
	{ CU_ASSERT_PTR_EQUAL(actual, expected); };

#define assertNotSame(actual, expected) \
	{ CU_ASSERT_PTR_NOT_EQUAL(actual, expected); };


#define assertStringEquals(actual, expected) \
	{ CU_ASSERT_STRING_EQUAL(actual, expected); assertEquals(strlen((const char*)actual), strlen((const char*)expected)); }
/*

    assertEquals
    assertFalse
    assertNotNull
    assertNull
    assertNotSame
    assertSame
    assertTrue

*/
