#ifndef __SPIDERMONKEY_SPECIFICS_H__
#define __SPIDERMONKEY_SPECIFICS_H__

#include "jsapi.h"
#include "uthash.h"

typedef struct js_proxy {
	void *ptr;
	JSObject *obj;
	UT_hash_handle hh;
} js_proxy_t;

extern js_proxy_t *_js_global_ht;

typedef struct js_type_class {
	const char* type;
	JSClass *jsclass;
	JSObject *proto;
	JSObject *parentProto;
	UT_hash_handle hh;
} js_type_class_t;

extern js_type_class_t *_js_global_type_ht;

#define JS_NEW_PROXY(p, native_obj, js_obj) \
do { \
	p = (js_proxy_t *)malloc(sizeof(js_proxy_t)); \
	assert(p); \
	p->ptr = native_obj; \
	p->obj = js_obj; \
	HASH_ADD_PTR(_js_global_ht, ptr, p); \
} while(0) \

#define JS_GET_PROXY(p, native_obj) \
do { \
	HASH_FIND_PTR(_js_global_ht, &native_obj, p); \
} while (0)

#define JS_REMOVE_PROXY(proxy) \
HASH_DEL(_js_global_ht, proxy)

#define TEST_NATIVE_OBJECT(cx, native_obj) \
if (!native_obj) { \
	JS_ReportError(cx, "Invalid Native Object"); \
	return JS_FALSE; \
}

#define ADD_OBJECT_TYPE(klass) \
static const char* OBJECT_TYPE; \
const char* getObjectType() { return klass::OBJECT_TYPE; }

#define ADD_OBJECT_TYPE_DECL(klass) \
const char* klass::OBJECT_TYPE = #klass;

#endif
