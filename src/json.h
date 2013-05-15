/*
 * json.h
 *
 *  Created on: 15.04.2013
 *      Author: Denisk
 */

#ifndef JSON_H_
#define JSON_H_

#pragma once

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>


#include "jsmn.h"
#include "strbuffer.h"


#ifndef TOKENS_COUNT
#define TOKENS_COUNT 1000
#endif


typedef enum {
	JSON_NULL = 1,
	JSON_PRIMITIVE = 2,
    JSON_OBJECT = 3,
    JSON_ARRAY = 4,
    JSON_STRING = 5,
    JSON_NUMBER = 6,
    JSON_TRUE = 7,
    JSON_FALSE = 8,
    JSON_KEY = 9
} json_type;

typedef struct json_tt {
    json_type type;
    size_t size;
    struct json_tt* next;
    struct json_tt* last;
    struct json_tt* children;
    char * value;
} json_t;


typedef enum {
	JSON_SUCCESS = 0,
	JSON_INVALID_OBJECT_TYPE,
	JSON_MEMORY_ALLOC_FAILED,
	JSON_INLLEGAL_STATE_EXCEPTION,
	JSON_INVALID_INPUT,
	JSON_NULL_POINTER
} json_error_code_t;

typedef struct json_error_tt {
	json_error_code_t errorCode;
	strbuffer_t *errorMsg;
} json_error_t;

bool json_token_streq(char *js, jsmntok_t *t, char *s);

strbuffer_t* json_token_tostr(char *js, jsmntok_t *t);

json_t* json_alloc(json_type objectType, size_t childrenCount);

void* json_malloc(size_t size);
void json_free (void* ptr);

void json_object_free(json_t*  /* root */);

json_error_t* json_append_child(json_t* /* root */, json_t* /* child*/);

json_type decode_jsmn_type(jsmntype_t /* jsmn_type */);

json_t* json_object_get(json_t* /* object */, const char* /* key */);
json_t* json_array_get(json_t* /* array */, int /* index */);

inline strbuffer_t * json_object_to_string(json_t* /* object */);
inline strbuffer_t * json_array_to_string(json_t* /* array */);

strbuffer_t * json_to_string(json_t* /* object */);
inline int add_value_from_token(json_t	* /* obj */, char* /* jsonStr */, jsmntok_t* /*token*/);

inline json_error_t* allocate_object_by_type(
		json_t ** /* objectPtr */,
		jsmntok_t* /* token */,
		jsmntok_t* /* tokens[TOKENS_COUNT] */, size_t /* tokensCount */, size_t* /* tokenIndex */,
		char* /*js */);

json_error_t* createJsonObject(json_t **json_root, char* /*js*/, jsmntok_t* /* tokens[TOKENS_COUNT] */, size_t /* tokensCount */, size_t* /* startTokenIndex */);

json_error_t * json_from_string(char * /* jsonString */, json_t ** /* destObject */);


#endif /* JSON_H_ */
