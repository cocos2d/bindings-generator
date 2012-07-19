\#include "jsapi.h"
\#include "jstypedarray.h"
\#include "${prefix}.hpp"
#for header in $headers
\#include "${os.path.basename(header)}"
#end for

template<class T>
static JSBool dummy_constructor(JSContext *cx, uint32_t argc, jsval *vp) {
	T* cobj = new T();
	js_type_class_t *p;
	const char* type = cobj->getObjectType();
	HASH_FIND_STR(_js_global_type_ht, type, p);
	assert(p);
	JSObject *_tmp = JS_NewObject(cx, p->jsclass, p->proto, p->parentProto);
	JS_SetPrivate(_tmp, cobj);
#ifdef COCOS2D_VERSION
	JS_AddObjectRoot(cx, &_tmp);      
#endif
	js_proxy_t *pp;
	JS_NEW_PROXY(pp, cobj, _tmp);
	JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(_tmp));

	return JS_TRUE;
}

static JSBool empty_constructor(JSContext *cx, uint32_t argc, jsval *vp) {
	return JS_FALSE;
}
