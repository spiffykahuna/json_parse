#include "json.h"





//#define BUFFER_SIZE 32768
//#define JSON_TOKENS 256


//static char strBuffer[STRING_BUFFER_SIZE];

#ifdef JSON_TEST
void* pvPortMalloc(size_t size){
	return malloc(size);
}

void vPortFree(void* ptr){
	free(ptr);
}
#endif

void * json_malloc(size_t size) {
	return pvPortMalloc(size);
}

void json_free (void* ptr){
	vPortFree(ptr);
}

json_error_t* parsingError(
	json_error_code_t errCode,
	char* errMsg,
	jsmntok_t *token,
	char* jsonString
) {
	json_error_t* err = (json_error_t*) json_malloc(sizeof(json_error_t));
	err->errorMsg = (strbuffer_t*) json_malloc(sizeof(json_error_t));
	strbuffer_init(err->errorMsg);

	err->errorCode = errCode;
	strbuffer_append(err->errorMsg, errMsg);

	if( token != NULL && jsonString != NULL) {
		strbuffer_t *tokenText = json_token_tostr(jsonString, token);
		strbuffer_append(err->errorMsg, " Error was in: ");
		strbuffer_append(err->errorMsg, tokenText->value);
		strbuffer_destroy(tokenText);
	}

	return err;
}

bool json_token_streq(char *js, jsmntok_t *t, char *s)
{
    return (strncmp(js + t->start, s, t->end - t->start) == 0
            && strlen(s) == (size_t) (t->end - t->start));
}


strbuffer_t * json_token_tostr(char *js, jsmntok_t *t)
{
	if(t->start > strlen(js)
			|| t->end > strlen(js)
			|| t->end > strlen(js)
			|| (t->end - t->start) > strlen(js)
	) {
		return NULL;
	}

	strbuffer_t *string = (strbuffer_t *) json_malloc(sizeof(strbuffer_t));
	strbuffer_init(string);

	strbuffer_append_bytes(string, js + t->start, (t->end - t->start) );

	return string;
}

json_t * json_alloc(json_type objectType, size_t childrenCount) {

	json_t *newObject = (json_t*) json_malloc(sizeof(json_t));
	if(!newObject) {
		return NULL;
	}

	newObject->type = objectType;

	if(newObject->type == JSON_OBJECT) {
		if(childrenCount % 2 != 0) {
			return NULL; /* object children count should be even */
		}
		newObject->size = childrenCount/2; /* childrenCount = key value pairs. we need only keys count*/
	} else {
		newObject->size = childrenCount;
	}


	newObject->children = NULL;
	newObject->next = NULL;
	newObject->last = NULL;
	newObject->value = NULL;

	return newObject;
}

json_error_t* json_append_child(json_t *root, json_t *child) {

	json_t* current;
	json_t* last;

	if(!root || !child) {
		return parsingError(
			JSON_NULL_POINTER,
			"Error:  Unable to append child to root object. in json_append_child() function",
			NULL,
			NULL
		);
	}

	if(!root->children) {
		root->children = child;
	} else {

		for(current = root->children; current != NULL; current = current->next ) {
				last = current;
		}
		last->next = child;

	}

	child->next = NULL;

//	root->children->last = child;

	return NULL; /* exit without any error */
}

void json_object_free(json_t **root) {

	json_t* object = *root;
//	json_t* next;

	if(object == NULL) {
		return;
	}

	if(object->children != NULL) {
		json_object_free(&object->children);
		object->children = NULL;
	}

	if(object->next != NULL) {
		json_object_free(&object->next);
		object->next = NULL;
	}

//	if(root->last != NULL) {
//		json_object_free(root->last);
//		root->last = NULL;
//	}

	if(object->value != NULL) {
		json_free(object->value);
		object->value = NULL;
	}
	json_free(object);
	object = NULL;
	root = NULL;
}


json_type decode_jsmn_type(jsmntype_t  jsmn_type) {

	switch(jsmn_type) {
	case JSMN_PRIMITIVE:
		return JSON_PRIMITIVE;
		break;

	case JSMN_OBJECT:
		return JSON_OBJECT;
		break;

	case JSMN_ARRAY:
		return JSON_ARRAY;
		break;

	case JSMN_STRING:
		return JSON_STRING;
		break;

	default:
		return JSON_NULL;
	}
}

json_t* json_object_get(json_t* object, const char* key) {

	if(object->type != JSON_OBJECT) {
		return NULL;
	}
	json_t* node;
	if(object->children != NULL) {
		for(node = object->children; node != NULL; node = node->next) {
			if(node->type == JSON_KEY) {
				if(strcmp(node->value, key) == 0) {
					return node->children;
				}
			}

		}
	}

	return NULL;
}

json_t* json_array_get(json_t* array, int index) {
	int i = 0;
	if(array->type != JSON_ARRAY) {
			return NULL;
	}
	json_t* node;
	if(array->children != NULL) {
		for(node = array->children; node != NULL; node = node->next) {
			if(i++ == index) {
				return node;
			}
		}
	}
	return NULL;
}


strbuffer_t * json_to_string(json_t* object) {

	strbuffer_t* sb = strbuffer_new();
	strbuffer_t* tempString;

	if(object == NULL) {
		strbuffer_append(sb, "");
		return sb;
	}

	switch(object->type) {
	case JSON_OBJECT:
		tempString = json_object_to_string(object);
		strbuffer_append(sb, tempString->value);
		strbuffer_destroy(tempString);
		break;

	case JSON_ARRAY:
		tempString = json_array_to_string(object);
		strbuffer_append(sb, tempString->value);
		strbuffer_destroy(tempString);
		break;

	case JSON_STRING:
		strbuffer_append(sb, "\"" );
		strbuffer_append(sb, object->value );
		strbuffer_append(sb, "\"" );
		break;

	case JSON_NUMBER:
		strbuffer_append(sb, object->value);
		break;
	case JSON_TRUE:
		strbuffer_append(sb, "true");
		break;
	case JSON_FALSE:
		strbuffer_append(sb, "false");
		break;
	case JSON_NULL:
		strbuffer_append(sb, "null");
		break;

	default:
		break;
	}

	return sb;
}

inline
strbuffer_t * json_array_to_string(json_t* array) {

	size_t i = 0;

	json_t* elem;

	strbuffer_t* sb = strbuffer_new();
	strbuffer_t* tempString;

	strbuffer_append(sb, "[ ");
	for(i = 0; i < array->size; i++ ) {
		if(i != 0) {
			strbuffer_append(sb, ", ");
		}

		elem = json_array_get(array, i);

		tempString = json_to_string(elem);
		strbuffer_append(sb, tempString->value);
		strbuffer_destroy(tempString);
	}
	strbuffer_append(sb, " ]");

	return sb;
}

inline
strbuffer_t * json_object_to_string(json_t* object){
	json_t *key;
	size_t i = 0;

	strbuffer_t* sb = strbuffer_new();
	strbuffer_t* tempString;

	strbuffer_append(sb, "{ ");
	if(object->children != NULL) {
		for(key = object->children; key != NULL; key = key->next) {
			if(i++ != 0) {
				strbuffer_append(sb, ", ");
			}

			strbuffer_append(sb, "\"" );
			strbuffer_append(sb, key->value );
			strbuffer_append(sb, "\" : " );

			tempString = json_to_string(key->children);
			strbuffer_append(sb, tempString->value);
			strbuffer_destroy(tempString);
		}
	}
	strbuffer_append(sb,  " }");
	return sb;
}

inline
void freeResources(json_t* root, json_t* key, json_t* value) {
	json_object_free(&root);
	json_object_free(&key);
	json_object_free(&value);
}

json_error_t* createJsonObject(
		json_t **json_root,
		char *js,
		jsmntok_t tokens[TOKENS_COUNT],
		size_t tokensCount,
		size_t *tokenIndex
) {

	typedef enum { START, KEY, PRINT, VALUE, ARRAY, STOP } parse_state;
	parse_state state = START;

	json_error_t* error = NULL;
	//static char msgBuffer[MSG_BUFFER_SIZE];

	//size_t tokenIndex = 0;

//	strbuffer_t *tokenStr = NULL;

	size_t object_tokens = 0;

	json_t *root = NULL;
	json_t *key = NULL;
	json_t *value = NULL;

	for (; *tokenIndex < tokensCount; *tokenIndex += 1 ) {
		jsmntok_t *token = &tokens[*tokenIndex];

//		size_t heapSize = xPortGetFreeHeapSize();
//		sprintf(msgBuffer, "HEAP SIZE: %d\n", heapSize);
//		log_d(msgBuffer);

		if(token->start > strlen(js)
					|| token->end > strlen(js)
					|| token->end > strlen(js)
					|| (token->end - token->start) > strlen(js)
		) {
			continue;
		}


//		tokenStr = json_token_tostr(js, token);

		switch (state) {

		case START:
			if (token->type != JSMN_OBJECT && token->type != JSMN_ARRAY) {
				return parsingError(
					JSON_INVALID_OBJECT_TYPE,
					"Error:  Root object is not object or array.",
					token,
					js
				);
			}

			switch(token->type) {

			case JSMN_OBJECT:

				root = json_alloc(JSON_OBJECT, token->size);
				if(root == NULL) {
					freeResources(root, key, value);
					return parsingError(
						JSON_MEMORY_ALLOC_FAILED,
						"Error:  Unable to allocate root object.",
						token,
						js
					);
				}

				state = KEY;
				break;

			case JSMN_ARRAY:
				state = ARRAY;
				root = json_alloc(JSON_ARRAY, token->size);
				if(root == NULL) {
					freeResources(root, key, value);
					return parsingError(
						JSON_MEMORY_ALLOC_FAILED,
						"Error:  Unable to allocate root array.",
						token,
						js
					);
				}
				break;

			default:
				freeResources(root, key, value);
				return parsingError(
					JSON_INVALID_OBJECT_TYPE,
					"Error:  Root object is not object or array.",
					token,
					js
				);
			}


			object_tokens = token->size;


			if (object_tokens == 0) {
				*json_root = root;
				return NULL; /* exit with no errors */
			}


//			if (object_tokens % 2 != 0)
//				log_die("Invalid response: object must have even number of children. %d\n", tokenIndex);

			break;

		case KEY:

			if (token->type != JSMN_STRING) {
				freeResources(root, key, value);
				return parsingError(
					JSON_INVALID_OBJECT_TYPE,
					"Error:  Object key is not string.",
					token,
					js
				);
			}

			key = json_alloc(JSON_KEY, token->size);
			if(key == NULL) {
				freeResources(root, key, value);
				return parsingError(
					JSON_MEMORY_ALLOC_FAILED,
					"Error: Unable to allocate memory for json key object.",
					token,
					js
				);
			}

			add_value_from_token(key, js, token);

			if(key->value == NULL) {
				freeResources(root, key, value);
				return parsingError(
					JSON_MEMORY_ALLOC_FAILED,
					"Error: Unable to allocate memory for json key string.",
					token,
					js
				);
			}


			error = json_append_child(root, key);

			if( error != NULL) {
				freeResources(root, key, value);
				return error;
			}

			object_tokens--;
			state = VALUE;

			break;

		case VALUE:
//			if (token->type != JSMN_STRING && token->type != JSMN_PRIMITIVE)
//				printf("failed: not string and not primitive skip %d\n", tokenIndex);



			error = allocate_object_by_type(
					&value,
					token, tokens, tokensCount,
					tokenIndex, js);
			if(error != NULL) {
				freeResources(root, key, value);
				return error;
			}

			error = json_append_child(key, value);
			if(error != NULL) {
				freeResources(root, key, value);
				return error;
			}

			object_tokens--;
			state = KEY;

			if (object_tokens == 0) {
				*json_root = root;
				return NULL; /* exit with no errors */
			}

			break;

		case ARRAY:
			if(root->type != JSON_ARRAY) {
				freeResources(root, key, value);
				return parsingError(
					JSON_INVALID_OBJECT_TYPE,
					"Error: In state ARRAY. Parent element is not array",
					token,
					js
				);
			}

			error = allocate_object_by_type(
					&value,
					token, tokens, tokensCount,
					tokenIndex, js);
			if(error != NULL) {
				freeResources(root, key, value);
				return error;
			}

			error = json_append_child(root, value);
			if(error != NULL) {
				freeResources(root, key, value);
				return error;
			}

			object_tokens--;
			state = ARRAY;

			if (object_tokens == 0) {
				*json_root = root;
				return NULL; /* exit with no errors */
			}

			break;

		case STOP:
			// Just consume the tokens
			return NULL; /* exit with no errors */
			break;

		default:
			return parsingError(
				JSON_INLLEGAL_STATE_EXCEPTION,
				"Error: Invalid parsing state. Program should not get here!",
				NULL,
				NULL
			);
		}

	}

	return parsingError(
		JSON_INLLEGAL_STATE_EXCEPTION,
		"Error: Program should not get here!",
		NULL,
		NULL
	);


	//json_object_free(root);

//	for (tokenIndex = 0, tokensLeft = 1; tokensLeft > 0;
//			tokenIndex++, tokensLeft--) {
//
//		jsmntok_t *t = &tokens[tokenIndex];
//
//		// Should never reach uninitialized tokens
//		log_assert(t->start != -1 && t->end != -1);
//
//		if (t->type == JSMN_ARRAY || t->type == JSMN_OBJECT)
//			tokensLeft += t->size;
//
//		switch (state) {
//		case START:
//			if (t->type != JSMN_OBJECT)
//				printf("failed NOt object: %d\n", tokenIndex);
//
//			state = KEY;
//			object_tokens = t->size;
//
//			if (object_tokens == 0)
//				state = STOP;
//
//			if (object_tokens % 2 != 0)
//				printf("failed: %d\n", tokenIndex);
//
//			break;
//
//		case KEY:
//			object_tokens--;
//
//			if (t->type != JSMN_STRING)
//				printf("failed: Key is not string %d\n", tokenIndex);
//
//			state = SKIP;
//
//			for (i = 0; i < sizeof(KEYS) / sizeof(char *); i++) {
//				if (json_token_streq(js, t, KEYS[i])) {
//					printf("%s: ", KEYS[i]);
//					state = PRINT;
//					break;
//				}
//			}
//
//			break;
//
//		case SKIP:
//			if (t->type != JSMN_STRING && t->type != JSMN_PRIMITIVE)
//				printf("failed: not string and not primitive skip %d\n", tokenIndex);
//
//			object_tokens--;
//			state = KEY;
//
//			if (object_tokens == 0)
//				state = STOP;
//
//			break;
//
//		case PRINT:
//			if (t->type != JSMN_STRING && t->type != JSMN_PRIMITIVE)
//				printf("failed not string and not primitive print: %d\n", tokenIndex);
//
//			char *str = json_token_tostr(js, t);
//			puts(str);
//
//			object_tokens--;
//			state = KEY;
//
//			if (object_tokens == 0)
//				state = STOP;
//
//			break;
//
//		case STOP:
//			// Just consume the tokens
//			break;
//
//		default:
//			log_die("Invalid state %u", state);
//		}
//	}
}

inline
int add_value_from_token(json_t	*obj, char* jsonString, jsmntok_t* token) {

	if ((obj != NULL) && (jsonString != NULL) && (token != NULL)) {

		strbuffer_t *tokenStr = json_token_tostr(jsonString, token);

		obj->value = (char*) json_malloc((tokenStr->length+1) * sizeof(char));
		memset(obj->value, 0, (tokenStr->length+1));
		strncpy(obj->value, tokenStr->value, tokenStr->length);
		strbuffer_destroy(tokenStr);

		return 0;
	}
	return -1;
}

inline
json_error_t* allocate_object_by_type(
		json_t **objectPtr,
		jsmntok_t *token,
		jsmntok_t tokens[TOKENS_COUNT], size_t tokensCount, size_t *tokenIndex,
		char* js) {

	json_error_t* parseError = NULL;
	json_t* jsonObject = NULL;

	json_type type = decode_jsmn_type(token->type);



	switch (type) {
	case JSON_STRING:
		jsonObject = json_alloc(JSON_STRING, token->size);
		if(jsonObject == NULL) {
			parseError = parsingError(
				JSON_MEMORY_ALLOC_FAILED,
				"Error: Unable to allocate string object.",
				token,
				js
			);
		}
		break;



	case JSON_PRIMITIVE:
	{
		char *firstCharacter = js + token->start;
		switch(*firstCharacter) {
		case '-': {};
		case '0': {};
		case '1': {};
		case '2': {};
		case '3': {};
		case '4': {};
		case '5': {};
		case '6': {};
		case '7': {};
		case '8': {};
		case '9': {};
			jsonObject = json_alloc(JSON_NUMBER, token->size);
			break;
		case 't':
			jsonObject = json_alloc(JSON_TRUE, token->size);
			break;
		case 'f':
			jsonObject = json_alloc(JSON_FALSE, token->size);
			break;
		case 'n' :
			jsonObject = json_alloc(JSON_NULL, token->size);
			break;
		default:
			parseError = parsingError(
				JSON_INVALID_OBJECT_TYPE,
				"Error: Unable to allocate primitive object by first character. Wrong character at -->",
				token,
				js
			);
			break;
		}
	} break;


	case JSON_OBJECT:
	case JSON_ARRAY:
		parseError = createJsonObject(&jsonObject, js, tokens, tokensCount, tokenIndex);
		break;
	default:
		parseError = parsingError(
			JSON_INVALID_OBJECT_TYPE,
			"Error: Unable to allocate object by type. Wrong type.",
			token,
			js
		);
		break;
	}

	if(parseError == NULL) {

		add_value_from_token(jsonObject, js, token);
//		/* assign returned object */
//		if(objectPtr != NULL) {
//			json_free(objectPtr);
//		}
//		objectPtr = (json_t*) json_malloc(sizeof(json_t));
//		memcpy(objectPtr, jsonObject, sizeof(json_t));
//
//		json_free(jsonObject);

		*objectPtr = jsonObject;


	}


	return parseError;
}

json_error_t * json_from_string(char *jsonString, json_t **destObject) {

		int resultCode = 0;
		jsmn_parser parser;
		jsmntok_t tokens[TOKENS_COUNT];
		size_t tokenIndex = 0;

		jsmntok_t *failedToken = NULL;

		if(jsonString == NULL) {
			return parsingError(
				JSON_NULL_POINTER,
				"Error: Json string is NULL.",
				NULL,
				NULL
			);
		}

		memset(tokens, 0, TOKENS_COUNT*sizeof(jsmntok_t));

		json_t *root = NULL;

		jsmn_init(&parser);
		resultCode = jsmn_parse(&parser, jsonString, tokens, TOKENS_COUNT);
		if(resultCode != JSMN_SUCCESS) {
			failedToken = ((tokenIndex <= TOKENS_COUNT) ? &tokens[tokenIndex] : NULL);

			switch(resultCode) {
			case JSMN_ERROR_INVAL:
				return parsingError(
					JSON_INVALID_INPUT,
					"Error: Invalid character inside JSON string.",
					failedToken,
					jsonString
				);

			case JSMN_ERROR_NOMEM:
				return parsingError(
					JSON_MEMORY_ALLOC_FAILED,
					"Error: Not enough tokens were provided. Please add more tokens in application configuration.",
					failedToken,
					jsonString
				);

			case JSMN_ERROR_PART:
				return parsingError(
					JSON_INVALID_INPUT,
					"Error: The string is not a full JSON packet, more bytes expected.",
					failedToken,
					jsonString
				);
			case JSMN_SUCCESS:
				break;
			default:
				return parsingError(
					JSON_INLLEGAL_STATE_EXCEPTION,
					"Error: Invalid jsmp result code. Normally program should not reach here.",
					NULL,
					NULL
				);
			}

		}

		json_error_t *parseError = NULL;
		parseError = createJsonObject(&root, jsonString, tokens, TOKENS_COUNT, &tokenIndex );

		if(parseError != NULL) {
			json_object_free(&root);
			return parseError;
		}

		*destObject = root;

		return NULL; /* none errors */
}
