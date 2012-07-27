\#include "jsapi.h"
\#include "jstypedarray.h"
\#include "${out_file}.hpp"
#for header in $headers
\#include "${os.path.basename(header)}"
#end for

/**
 * You don't need to manage the returned pointer. They live for the whole life of
 * the app.
 */
template <class T>
js_type_class_t *js_get_type_from_native(T* native_obj) {
	js_type_class_t *typeProxy;
	uint32_t typeId = reinterpret_cast<int>(typeid(*native_obj).name());
	HASH_FIND_INT(_js_global_type_ht, &typeId, typeProxy);
	if (!typeProxy) {
		TypeInfo *typeInfo = dynamic_cast<TypeInfo *>(native_obj);
		if (typeInfo) {
			typeId = typeInfo->getClassTypeInfo();
			HASH_FIND_INT(_js_global_type_ht, &typeId, typeProxy);
		}
	}
	return typeProxy;
}

/**
 * you don't need to manage the returned pointer. The returned pointer should be deleted
 * using JS_REMOVE_PROXY. Most of the time you do that in the C++ destructor.
 */
template<class T>
js_proxy_t *js_get_or_create_proxy(JSContext *cx, T *native_obj) {
	js_proxy_t *proxy;
	HASH_FIND_PTR(_native_js_global_ht, &native_obj, proxy);
	if (!proxy) {
		js_type_class_t *typeProxy = js_get_type_from_native<T>(native_obj);
		assert(typeProxy);
		JSObject* js_obj = JS_NewObject(cx, typeProxy->jsclass, typeProxy->proto, typeProxy->parentProto);
		JS_AddObjectRoot(cx, &js_obj);
		JS_NEW_PROXY(proxy, native_obj, js_obj);
		return proxy;
	} else {
		return proxy;
	}
	return NULL;
}

template<class T>
static JSBool dummy_constructor(JSContext *cx, uint32_t argc, jsval *vp) {
	TypeTest<T> t;
	T* cobj = new T();
	js_type_class_t *p;
	uint32_t typeId = t.s_id();
	HASH_FIND_INT(_js_global_type_ht, &typeId, p);
	assert(p);
	JSObject *_tmp = JS_NewObject(cx, p->jsclass, p->proto, p->parentProto);
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
