/*
 ============================================================================
 Name        : json_parse.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"
#include "jsmn.h"

#include "my_cunit_bindings.h"
#include "CUnit/Basic.h"

char* readJSONFromFile(const char *fileName);

/* Pointer to the file used by the tests. */
//static FILE* json_test_file = NULL;

static char * json_test_object = NULL;


/* The suite initialization function.
 * Opens the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int setUp(void)
{
	json_test_object = readJSONFromFile("json_test_files/test_json.txt");
	if(json_test_object != NULL) {
		return 0;
	}
	return -1;
}

/* The suite cleanup function.
 * Closes the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int tearDown(void)
{
	if(json_test_object != NULL) {
		free(json_test_object);
		return 0;
	}
	return -1;
}

void testJsmnParsesProvidedJson(void)
{

	char *js = json_test_object;
	assertNotNull(&js);


	int resultCode = 0;
	jsmn_parser parser;
	jsmntok_t tokens[TOKENS_COUNT];

	json_t *root = NULL;
//	json_t *key;
//	json_t *value;

	size_t tokenIndex = 0;

	memset(tokens, 0, TOKENS_COUNT*sizeof(jsmntok_t));

	jsmn_init(&parser);
	assertNotNull(&parser);

	resultCode = jsmn_parse(&parser, js, tokens, TOKENS_COUNT);
	assertEquals(resultCode, JSMN_SUCCESS);

	json_error_t *parseError = createJsonObject(&root, js, tokens, TOKENS_COUNT, &tokenIndex );
	assertNull(parseError);
	assertNotNull(&root);
	assertTrue(root != NULL);


}

void test_json_token_tostr(void)
{
	char *js = json_test_object;
	assertNotNull(&js);
	//json_t* root = json_from_string(js);

	jsmntok_t token;
	token.type = JSMN_STRING;
	token.start = 16;
	token.end = 27;
	token.size = 0;

	strbuffer_t *str = json_token_tostr(js, &token);
	assertTrue(strcmp(str->value, "lorem ipsum") == 0);

	strbuffer_close(str);
}


void test_json_from_string(void) {
	char *js = json_test_object;
	char *invalid;
	char *invalidFileName;
	int i;

	assertNotNull(&js);

	json_t *root = NULL;
	json_error_t *parseError = json_from_string(js, &root);
	assertNull(parseError);
	assertNotNull(&root);
	assertTrue(root != NULL);

	json_object_free(&root);

	root = NULL;

	parseError = json_from_string(NULL, &root);
	assertNull(root);
	assertNotNull(parseError);
	assertEquals(parseError->errorCode, JSON_NULL_POINTER);
	assertNotNull(parseError->errorMsg);

	json_free(parseError->errorMsg);
	json_free(parseError);
	json_object_free(&root);

	char * invalidSet[] = {
			""
			,"[]]"
			, "{}}"
			, "{ \"foo\" : \"bar\", [ 0 ] : \"a\" }"
//			, "{ \"foo\" : \"bar\", \"a\" }"
	};

	for(i = 0; i < LENGTH(invalidSet); i++) {
		invalid = invalidSet[i];
		parseError = json_from_string(invalid, &root);
		assertNull(root);
		assertNotNull(parseError);

		json_free(parseError->errorMsg);
		json_free(parseError);
		json_object_free(&root);
	}

	char * invalidFileSet[] = {
		"json_test_files/invalid_01.json"
	};

	for (i = 0; i < LENGTH(invalidFileSet); ++i) {
		invalidFileName = invalidFileSet[i];
		invalid = readJSONFromFile(invalidFileName);

//		puts(invalid);
//		parseError = json_from_string(invalid, &root);
//		assertNull(root);
//		assertNotNull(parseError);
////		puts(parseError->errorMsg->value);
//		json_free(parseError->errorMsg);
//		json_free(parseError);
//		json_object_free(&root);

	}

}

void test_json_object_get(void) {
	char *js = json_test_object;
	assertNotNull(&js);

	json_t *root = NULL;
	json_t *value = NULL;
	json_t *nestedValue = NULL;
	json_error_t *parseError = json_from_string(js, &root);
	assertNull(parseError);
	assertNotNull(&root);
	assertTrue(root != NULL);


	value = json_object_get(root, "x^2 array");
	assertNotNull(&value);
	assertEquals(value->type, JSON_ARRAY);
	assertNull(value->next);
	assertNotNull(value->children);
	assertEquals(value->size, 11);
	assertStringEquals(value->value, "[0, 1, 4, 9, 16, 25, 36, 49, 64, 81, 100]");


	value = json_object_get(root, "utf string" );
	assertNotNull(&value);
	assertEquals(value->type, JSON_STRING);
	assertNull(value->next);
	assertNull(value->children);
	assertEquals(value->size, 0); /* no children */
	assertTrue(
		strstr(value->value, "corem" ) != NULL
		&& strstr(value->value, "ipsum" ) != NULL
	);

	value = json_object_get(root, "utf-8 string" );
	assertNotNull(&value);
	assertEquals(value->type, JSON_STRING);
	assertNull(value->next);
	assertNull(value->children);
	assertEquals(value->size, 0); /* no children */
	char japan[] = {0xE3, 0x81, 0x82, 0xE3, 0x81, 0x84, 0xE3, 0x81, 0x86, 0xE3, 0x81, 0x88, 0xE3, 0x81, 0x8A}; /* ć�‚ć�„ć�†ć��ć�� */
	assertStringEquals(value->value, japan );


	value = json_object_get(root, "string" );
	assertNotNull(&value);
	assertEquals(value->type, JSON_STRING);
	assertNull(value->next);
	assertNull(value->children);
	assertEquals(value->size, 0); /* no children */
	assertStringEquals(value->value, "lorem ipsum");

	value = json_object_get(root, "positive one" );
	assertNotNull(&value);
	assertEquals(value->type, JSON_NUMBER);
	assertNull(value->next);
	assertNull(value->children);
	assertEquals(value->size, 0); /* no children */
	assertStringEquals(value->value, "1");


	value = json_object_get(root, "negative one" );
	assertNotNull(&value);
	assertEquals(value->type, JSON_NUMBER);
	assertNull(value->next);
	assertNull(value->children);
	assertEquals(value->size, 0); /* no children */
	assertStringEquals(value->value, "-1");

	value = json_object_get(root, "pi" );
	assertNotNull(&value);
	assertEquals(value->type, JSON_NUMBER);
	assertNull(value->next);
	assertNull(value->children);
	assertEquals(value->size, 0); /* no children */
	assertStringEquals(value->value, "3.14");

	value = json_object_get(root, "hard to parse number");
	assertNotNull(&value);
	assertEquals(value->type, JSON_NUMBER);
	assertNull(value->next);
	assertNull(value->children);
	assertEquals(value->size, 0); /* no children */
	assertStringEquals(value->value, "-3.14e-4");

	value = json_object_get(root, "boolean true" );
	assertNotNull(&value);
	assertEquals(value->type, JSON_TRUE);
	assertNull(value->next);
	assertNull(value->children);
	assertEquals(value->size, 0); /* no children */
	assertStringEquals(value->value, "true");

	value = json_object_get(root, "boolean false" );
	assertNotNull(&value);
	assertEquals(value->type, JSON_FALSE);
	assertNull(value->next);
	assertNull(value->children);
	assertEquals(value->size, 0); /* no children */
	assertStringEquals(value->value, "false");

	value = json_object_get(root, "null" );
	assertNotNull(&value);
	assertEquals(value->type, JSON_NULL);
	assertNull(value->next);
	assertNull(value->children);
	assertEquals(value->size, 0); /* no children */
	assertStringEquals(value->value, "null");

	value = json_object_get(root, "string array");
	assertNotNull(&value);
	assertEquals(value->type, JSON_ARRAY);
	assertNull(value->next);
	assertNotNull(value->children);
	assertEquals(value->size, 2);
	assertStringEquals(value->value, "[\"lorem\", \"ipsum\"]");

	value = json_object_get(root, "object_empty");
	assertNotNull(&value);
	assertEquals(value->type, JSON_OBJECT);
	assertNull(value->next);
	assertNull(value->children);
	assertEquals(value->size, 0);
	assertStringEquals(value->value, "{ }");

	value = json_object_get(root, "object");
	assertNotNull(&value);
	assertEquals(value->type, JSON_OBJECT);
	assertNull(value->next);
	assertNotNull(value->children);
	assertEquals(value->size, 6);
	//assertStringEquals(value->value, "{ }");

	nestedValue = json_object_get(value, "nested string");
	assertNotNull(&nestedValue);
	assertEquals(nestedValue->type, JSON_STRING);
	assertNull(nestedValue->next);
	assertNull(nestedValue->children);
	assertEquals(nestedValue->size, 0); /* no children */
	assertStringEquals(nestedValue->value, "str");

	nestedValue = json_object_get(value, "nested true");
	assertNotNull(&nestedValue);
	assertEquals(nestedValue->type, JSON_TRUE);
	assertNull(nestedValue->next);
	assertNull(nestedValue->children);
	assertEquals(nestedValue->size, 0); /* no children */
	assertStringEquals(nestedValue->value, "true");

	nestedValue = json_object_get(value, "nested false");
	assertNotNull(&nestedValue);
	assertEquals(nestedValue->type, JSON_FALSE);
	assertNull(nestedValue->next);
	assertNull(nestedValue->children);
	assertEquals(nestedValue->size, 0); /* no children */
	assertStringEquals(nestedValue->value, "false");

	nestedValue = json_object_get(value, "nested null");
	assertNotNull(&nestedValue);
	assertEquals(nestedValue->type, JSON_NULL);
	assertNull(nestedValue->next);
	assertNull(nestedValue->children);
	assertEquals(nestedValue->size, 0); /* no children */
	assertStringEquals(nestedValue->value, "null");

	nestedValue = json_object_get(value, "nested number");
	assertNotNull(&nestedValue);
	assertEquals(nestedValue->type, JSON_NUMBER);
	assertNull(nestedValue->next);
	assertNull(nestedValue->children);
	assertEquals(nestedValue->size, 0); /* no children */
	assertStringEquals(nestedValue->value, "123");

	nestedValue = json_object_get(value, "nested array");
	assertNotNull(&nestedValue);
	assertEquals(nestedValue->type, JSON_ARRAY);
	assertNull(nestedValue->next);
	assertNotNull(nestedValue->children);
	assertEquals(nestedValue->size, 2); /* no children */
	assertStringEquals(nestedValue->value, "[\"lorem\", \"ipsum\"]");


	json_object_free(&root);
}

void testObfuscatedJson(void) {
	char *js = "{\"string\":\"lorem ipsum composes a string with the same text another some more even more more and more and more  and more and more and more and more and more\",\"utf string\":\"coremipsum\",\"utf-8 string\":\"?????\",\"positive one\":1,\"negative one\":-1,\"pi\":3.14,\"hard to parse number\":-3.14e-4,\"boolean true\":true,\"boolean false\":false,\"null\":null,\"string array\":[\"lorem\",\"ipsum\"],\"x^2 array\":[0,1,4,9,16,25,36,49,64,81,100],\"object_empty\":{},\"object\":{\"nested string\":\"str\",\"nested true\":true,\"nested false\":false,\"nested null\":null,\"nested number\":123,\"nested array\":[\"lorem\",\"ipsum\"]}}";
	int resultCode = 0;
	jsmn_parser parser;
	jsmntok_t tokens[TOKENS_COUNT];

	json_t *root = NULL;
//	json_t *key;
	json_t *value;

	size_t tokenIndex = 0;

	memset(tokens, 0, TOKENS_COUNT*sizeof(jsmntok_t));

	jsmn_init(&parser);
	assertNotNull(&parser);

	resultCode = jsmn_parse(&parser, js, tokens, TOKENS_COUNT);
	assertEquals(resultCode, JSMN_SUCCESS);

	json_error_t *parseError = createJsonObject(&root, js, tokens, TOKENS_COUNT, &tokenIndex );
	assertNull(parseError);
	assertNotNull(&root);
	assertTrue(root != NULL);

	value = json_object_get(root, "string" );
	assertNotNull(&value);
	assertEquals(value->type, JSON_STRING);
	assertNull(value->next);
	assertNull(value->children);
	assertEquals(value->size, 0); /* no children */
	assertStringEquals(value->value, "lorem ipsum composes a string with the same text another some more even more more and more and more  and more and more and more and more and more");
}

void test_json_to_string(void) {
	json_t *root = NULL;
	json_t *key = NULL;
	strbuffer_t *string = NULL;

	char *js = "{ \"key\" : \"value\" }";
	json_error_t *parseError = json_from_string(js, &root);
	assertNull(parseError);
	assertNotNull(&root);
	assertEquals(root->type, JSON_OBJECT);
	assertTrue(root != NULL);
	string = json_to_string(root);
	assertStringEquals(string->value, js);
	strbuffer_destroy(string);
	json_object_free(&root);
	root = NULL;

	js = "{ \"key\" : 1 }";
	parseError = json_from_string(js, &root);
	assertNull(parseError);
	assertNotNull(&root);
	assertTrue(root != NULL);
	assertEquals(root->type, JSON_OBJECT);
	string = json_to_string(root);
	assertStringEquals(string->value, js);
	strbuffer_destroy(string);

	key = json_object_get(root, "key");
	assertNotNull(key);
	assertNull(key->children);
	assertEquals(key->type, JSON_NUMBER );
	string = json_to_string(key);
	assertStringEquals(string->value, "1");
	strbuffer_destroy(string);
	json_object_free(&root);
	root = NULL;

	js = "{ \"hard to parse number\" : -3.14e-4, \"pi\" : 3.14, \"some\" : null }";
	parseError = json_from_string(js, &root);
	assertNull(parseError);
	assertNotNull(&root);
	assertTrue(root != NULL);
	assertEquals(root->type, JSON_OBJECT);
	string = json_to_string(root);
	assertStringEquals(string->value, js);
	strbuffer_destroy(string);

	key = json_object_get(root, "hard to parse number");
	assertNotNull(key);
	assertNull(key->children);
	assertEquals(key->type, JSON_NUMBER );
	string = json_to_string(key);
	assertStringEquals(string->value, "-3.14e-4");
	strbuffer_destroy(string);

	key = json_object_get(root, "pi");
	assertNotNull(key);
	assertNull(key->children);
	assertEquals(key->type, JSON_NUMBER );
	string = json_to_string(key);
	assertStringEquals(string->value, "3.14");
	strbuffer_destroy(string);

	key = json_object_get(root, "some");
	assertNotNull(key);
	assertNull(key->children);
	assertEquals(key->type, JSON_NULL);
	string = json_to_string(key);
	assertStringEquals(string->value, "null");
	strbuffer_destroy(string);

	json_object_free(&root);
	root = NULL;

	js = "[ 0, 1, 4, 9, 16, 25, 36, 49, 64, 81, 100 ]";
	parseError = json_from_string(js, &root);
	assertNull(parseError);
	assertNotNull(&root);
	assertTrue(root != NULL);
	assertEquals(root->type, JSON_ARRAY);
	string = json_to_string(root);
	assertStringEquals(string->value, js);
	strbuffer_destroy(string);
	json_object_free(&root);
	root = NULL;

	js = "[ \"lorem\", \"ipsum\" ]";
	parseError = json_from_string(js, &root);
	assertNull(parseError);
	assertNotNull(&root);
	assertTrue(root != NULL);
	assertEquals(root->type, JSON_ARRAY);
	string = json_to_string(root);
	assertStringEquals(string->value, js);
	strbuffer_destroy(string);
	json_object_free(&root);
	root = NULL;

	js = "{ \"object\": { \"nested string\" : \"str\", \"nested true\" : true, \"nested false\" : false, \"nested null\" : null, \"nested number\" : 123, \"nested array\" : [ \"lorem\", \"ipsum\" ] }}";
	parseError = json_from_string(js, &root);
	assertNull(parseError);
	assertNotNull(&root);
	assertTrue(root != NULL);
	assertEquals(root->type, JSON_OBJECT);
	string = json_to_string(root);

	strbuffer_destroy(string);
	json_object_free(&root);
	root = NULL;


}

char* readJSONFromFile(const char *fileName) {
	FILE* pFile;
	long lSize;
	char* buffer;
	size_t result;
	pFile = fopen(fileName, "r");
	if (pFile == NULL ) {
		fputs("File error", stderr);
		exit(1);
	}
	// obtain file size:
	fseek(pFile, 0, SEEK_END);
	lSize = ftell(pFile);
	rewind(pFile);
	// allocate memory to contain the whole file:
	buffer = (char*) json_malloc(sizeof(char) * lSize);
	if (buffer == NULL ) {
		fputs("Memory error", stderr);
		exit(2);
	}
	// copy the file into the buffer:
	result = fread(buffer, 1, lSize, pFile);
	if (result != lSize) {
		fputs("Reading error", stderr);
		exit(3);
	}
	/* the whole file is now loaded in the memory buffer. */
	// terminate
	fclose(pFile);
	return buffer;
}

int main(void) {
	CU_pSuite pSuite = NULL;

	/* initialize the CUnit test registry */
	if (CUE_SUCCESS != CU_initialize_registry())
	  return CU_get_error();

	/* add a suite to the registry */
	pSuite = CU_add_suite("json_parse_test", setUp, tearDown);
	if (NULL == pSuite) {
	  CU_cleanup_registry();
	  return CU_get_error();
	}

	/* add the tests to the suite */
	if (
	   (NULL == CU_add_test(pSuite, "testJsmnParsesProvidedJson", testJsmnParsesProvidedJson))
	   || (NULL == CU_add_test(pSuite, "test_json_token_tostr", test_json_token_tostr))
	   || (NULL == CU_add_test(pSuite, "test_json_from_string", test_json_from_string))
	   || (NULL == CU_add_test(pSuite, "test_json_object_get", test_json_object_get))
	   || (NULL == CU_add_test(pSuite, "testObfuscatedJson", testObfuscatedJson))
	   || (NULL == CU_add_test(pSuite, "test_json_to_string", test_json_to_string))
	) {
	  CU_cleanup_registry();
	  return CU_get_error();
	}

	/* Run all tests using the CUnit Basic interface */
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();
	return CU_get_error();
}
