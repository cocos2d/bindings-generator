#include "cocos2d.h"
#include "cocos2dx.hpp"
#include "cocos2d_specifics.hpp"

static void addCallBackAndThis(JSObject *obj, jsval callback, jsval thisObj) {
    ScriptingCore::getInstance()->setReservedSpot(0, obj, callback);
    if(thisObj == JSVAL_VOID) {
        ScriptingCore::getInstance()->setReservedSpot(1, obj, thisObj);
    }
}

template<class T>
JSObject* bind_menu_item(JSContext *cx, T* nativeObj, jsval callback, jsval thisObj) {
    
    js_proxy_t *p;
    JS_GET_PROXY(p, nativeObj);
    if (p) {
        jsval vp;
        JS_ConvertValue(cx, callback, JSTYPE_FUNCTION, &vp);
        addCallBackAndThis(p->obj, callback, thisObj);
        return p->obj;
    } else {

      TypeTest<T> t;
      uint32_t typeId = t.s_id();
      HASH_FIND_INT(_js_global_type_ht, &typeId, classType);
      
      assert(classType);
      JSObject *tmp = JS_NewObject(cx, classType->jsclass, classType->proto, classType->parentProto);

      // bind nativeObj <-> JSObject
      js_proxy_t *proxy;
      JS_NEW_PROXY(proxy, nativeObj, tmp);
        
      addCallBackAndThis(tmp, callback, thisObj);
      
      return tmp;
    }
}
JSBool js_cocos2dx_CCMenuItem_create(JSContext *cx, uint32_t argc, jsval *vp)
{
	if (argc == 1) {
		jsval *argv = JS_ARGV(cx, vp);
		cocos2d::CCMenuItem* ret = cocos2d::CCMenuItem::create();
		JSObject *obj = bind_menu_item<cocos2d::CCMenuItem>(cx, ret, argv[0],
                                                            (argc == 2 ? argv[1] : JSVAL_VOID));
		JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj));
		return JS_TRUE;
	}
	return JS_FALSE;
}

JSBool js_cocos2dx_CCMenuItemSprite_create(JSContext *cx, uint32_t argc, jsval *vp)
{
	if (argc >= 2) {
		jsval *argv = JS_ARGV(cx, vp);
		js_proxy_t *proxy;
		JSObject *tmpObj;
		
		tmpObj = JSVAL_TO_OBJECT(argv[0]);
		JS_GET_NATIVE_PROXY(proxy, tmpObj);
		cocos2d::CCNode* arg0 = (cocos2d::CCNode*)(proxy ? proxy->ptr : NULL);
		TEST_NATIVE_OBJECT(cx, arg0);

		tmpObj = JSVAL_TO_OBJECT(argv[1]);
		JS_GET_NATIVE_PROXY(proxy, tmpObj);
		cocos2d::CCNode* arg1 = (cocos2d::CCNode*)(proxy ? proxy->ptr : NULL);
		TEST_NATIVE_OBJECT(cx, arg1);

		cocos2d::CCNode* arg2 = NULL;
		if (argc >= 3) {
			tmpObj = JSVAL_TO_OBJECT(argv[2]);
			JS_GET_NATIVE_PROXY(proxy, tmpObj);
			arg2 = (cocos2d::CCNode*)(proxy ? proxy->ptr : NULL);
			TEST_NATIVE_OBJECT(cx, arg2);
		}
		cocos2d::CCMenuItemSprite* ret = cocos2d::CCMenuItemSprite::create(arg0, arg1, arg2);
		JSObject *obj = bind_menu_item<cocos2d::CCMenuItemSprite>(cx, ret, (argc >= 4 ? argv[3] : JSVAL_VOID), (argc == 5 ? argv[4] : JSVAL_VOID));
		JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj));
		return JS_TRUE;
	}
	return JS_FALSE;
}

JSBool js_cocos2dx_CCMenuItemImage_create(JSContext *cx, uint32_t argc, jsval *vp)
{
	if (argc >= 2) {
		jsval *argv = JS_ARGV(cx, vp);
		const char *arg0; do { JSString *tmp = JS_ValueToString(cx, argv[0]); arg0 = JS_EncodeString(cx, tmp); } while (0);
		const char *arg1; do { JSString *tmp = JS_ValueToString(cx, argv[1]); arg1 = JS_EncodeString(cx, tmp); } while (0);
		const char *arg2; do { JSString *tmp = JS_ValueToString(cx, argv[2]); arg2 = JS_EncodeString(cx, tmp); } while (0);
		cocos2d::CCMenuItemImage* ret = cocos2d::CCMenuItemImage::create(arg0, arg1, arg2);
		JSObject *obj = bind_menu_item<cocos2d::CCMenuItemImage>(cx, ret, (argc >= 4 ? argv[3] : JSVAL_VOID), (argc == 5 ? argv[4] : JSVAL_VOID));
		JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj));
		return JS_TRUE;
	}
	return JS_FALSE;
}

JSBool js_cocos2dx_CCMenuItemLabel_create(JSContext *cx, uint32_t argc, jsval *vp)
{
	if (argc >= 1) {
		jsval *argv = JS_ARGV(cx, vp);
		js_proxy_t *proxy;
		JSObject *tmpObj = JSVAL_TO_OBJECT(argv[0]);
		JS_GET_NATIVE_PROXY(proxy, tmpObj);
		cocos2d::CCNode* arg0 = (cocos2d::CCNode*)(proxy ? proxy->ptr : NULL);
		TEST_NATIVE_OBJECT(cx, arg0)
		cocos2d::CCMenuItemLabel* ret = cocos2d::CCMenuItemLabel::create(arg0);
		JSObject *obj = bind_menu_item<cocos2d::CCMenuItemLabel>(cx, ret, (argc >= 2 ? argv[1] : JSVAL_VOID),  (argc == 3 ? argv[2] : JSVAL_VOID));
		JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj));
		return JS_TRUE;
	}
	return JS_FALSE;
}

JSBool js_cocos2dx_CCMenuItemAtlasFont_create(JSContext *cx, uint32_t argc, jsval *vp)
{
	if (argc >= 5) {
		jsval *argv = JS_ARGV(cx, vp);
		const char *arg0; do { JSString *tmp = JS_ValueToString(cx, argv[0]); arg0 = JS_EncodeString(cx, tmp); } while (0);
		const char *arg1; do { JSString *tmp = JS_ValueToString(cx, argv[1]); arg1 = JS_EncodeString(cx, tmp); } while (0);
		int arg2; if (!JS_ValueToInt32(cx, argv[2], &arg2)) return JS_FALSE;
		int arg3; if (!JS_ValueToInt32(cx, argv[3], &arg3)) return JS_FALSE;
		int arg4; if (!JS_ValueToInt32(cx, argv[4], &arg4)) return JS_FALSE;
		cocos2d::CCMenuItemAtlasFont* ret = cocos2d::CCMenuItemAtlasFont::create(arg0, arg1, arg2, arg3, arg4);
		JSObject *obj = bind_menu_item<cocos2d::CCMenuItemAtlasFont>(cx, ret, (argc >= 6 ? argv[5] : JSVAL_VOID), (argc == 7 ? argv[6] : JSVAL_VOID));
		JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj));
		return JS_TRUE;
	}
	return JS_FALSE;
}

JSBool js_cocos2dx_CCMenuItemFont_create(JSContext *cx, uint32_t argc, jsval *vp)
{
	if (argc >= 1) {
		jsval *argv = JS_ARGV(cx, vp);
		const char *arg0; do { JSString *tmp = JS_ValueToString(cx, argv[0]); arg0 = JS_EncodeString(cx, tmp); } while (0);
		cocos2d::CCMenuItemFont* ret = cocos2d::CCMenuItemFont::create(arg0);
		JSObject *obj = bind_menu_item<cocos2d::CCMenuItemFont>(cx, ret, (argc >= 2 ? argv[1] : JSVAL_VOID), (argc == 3 ? argv[2] : JSVAL_VOID));
		JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj));
		return JS_TRUE;
	}
	return JS_FALSE;
}

JSBool js_cocos2dx_CCMenuItemToggle_create(JSContext *cx, uint32_t argc, jsval *vp)
{
	if (argc >= 1) {
		jsval *argv = JS_ARGV(cx, vp);
		cocos2d::CCMenuItemToggle* ret = cocos2d::CCMenuItemToggle::create(NULL, NULL, NULL);
		JSObject *obj = bind_menu_item<cocos2d::CCMenuItemToggle>(cx, ret, argv[0], (argc == 2 ? argv[1] : JSVAL_VOID));
		for (int i=1; i < argc; i++) {
			js_proxy_t *proxy;
			JSObject *tmpObj = JSVAL_TO_OBJECT(argv[i]);
			JS_GET_NATIVE_PROXY(proxy, tmpObj);
			cocos2d::CCMenuItem* item = (cocos2d::CCMenuItem*)(proxy ? proxy->ptr : NULL);
			TEST_NATIVE_OBJECT(cx, item)
			ret->addSubItem(item);
		}
		JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj));
		return JS_TRUE;
	}
	return JS_FALSE;
}

JSBool js_cocos2dx_swap_native_object(JSContext *cx, uint32_t argc, jsval *vp)
{
	if (argc == 2) {
		// get the native object from the second object to the first object
		jsval *argv = JS_ARGV(cx, vp);
		JSObject *one = JSVAL_TO_OBJECT(argv[0]);
		JSObject *two = JSVAL_TO_OBJECT(argv[1]);
		js_proxy_t *nproxy;
		JS_GET_NATIVE_PROXY(nproxy, two);
		void *ptrTwo = (nproxy ? nproxy->ptr : NULL);
		if (nproxy) {
			js_proxy_t *jsproxy;
			JS_GET_PROXY(jsproxy, ptrTwo);
			if (jsproxy) {
				JS_REMOVE_PROXY(nproxy, jsproxy);
				JS_NEW_PROXY(nproxy, ptrTwo, one);
			}
		}
	}
	return JS_TRUE;
}

JSObject* getObjectFromNamespace(JSContext* cx, JSObject *ns, const char *name) {
	jsval out;
	if (JS_GetProperty(cx, ns, name, &out) == JS_TRUE) {
		JSObject *obj;
		if (JS_ValueToObject(cx, out, &obj) == JS_TRUE) {
			
		}
	}
	return NULL;
}

jsval anonEvaluate(JSContext *cx, JSObject *thisObj, const char* string) {
	jsval out;
	if (JS_EvaluateScript(cx, thisObj, string, strlen(string), "(string)", 1, &out) == JS_TRUE) {
		return out;
	}
	return JSVAL_VOID;
}

JSBool js_platform(JSContext *cx, uint32_t argc, jsval *vp)
{
	JSString *str = JS_NewStringCopyZ(cx, "mobile");
	jsval out = STRING_TO_JSVAL(str);
	JS_SET_RVAL(cx, vp, out);
	return JS_TRUE;
}

void register_cocos2dx_js_extensions()
{
	JSContext *cx = ScriptingCore::getInstance()->getGlobalContext();
	JSObject *global = JS_GetGlobalObject(cx);
	// first, try to get the ns
	jsval nsval;
	JSObject *ns;
	JS_GetProperty(cx, global, "cc", &nsval);
	if (nsval == JSVAL_VOID) {
		ns = JS_NewObject(cx, NULL, NULL, NULL);
		nsval = OBJECT_TO_JSVAL(ns);
		JS_SetProperty(cx, global, "cc", &nsval);
	} else {
		JS_ValueToObject(cx, nsval, &ns);
	}

	JS_DefineFunction(cx, global, "__associateObjWithNative", js_cocos2dx_swap_native_object, 2, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineFunction(cx, global, "__getPlatform", js_platform, 0, JSPROP_READONLY | JSPROP_PERMANENT);

	// add the properties
	JSObject *tmpObj;
	tmpObj = JSVAL_TO_OBJECT(anonEvaluate(cx, global, "(function () { return cc.MenuItem; })()"));
	JS_DefineFunction(cx, tmpObj, "create", js_cocos2dx_CCMenuItem_create, 1, JSPROP_READONLY | JSPROP_PERMANENT);
	tmpObj = JSVAL_TO_OBJECT(anonEvaluate(cx, global, "(function () { return cc.MenuItemSprite; })()"));
	JS_DefineFunction(cx, tmpObj, "create", js_cocos2dx_CCMenuItemSprite_create, 1, JSPROP_READONLY | JSPROP_PERMANENT);
	tmpObj = JSVAL_TO_OBJECT(anonEvaluate(cx, global, "(function () { return cc.MenuItemImage; })()"));
	JS_DefineFunction(cx, tmpObj, "create", js_cocos2dx_CCMenuItemImage_create, 1, JSPROP_READONLY | JSPROP_PERMANENT);
	tmpObj = JSVAL_TO_OBJECT(anonEvaluate(cx, global, "(function () { return cc.MenuItemLabel; })()"));
	JS_DefineFunction(cx, tmpObj, "create", js_cocos2dx_CCMenuItemLabel_create, 1, JSPROP_READONLY | JSPROP_PERMANENT);
	tmpObj = JSVAL_TO_OBJECT(anonEvaluate(cx, global, "(function () { return cc.MenuItemAtlasFont; })()"));
	JS_DefineFunction(cx, tmpObj, "create", js_cocos2dx_CCMenuItemAtlasFont_create, 1, JSPROP_READONLY | JSPROP_PERMANENT);
	tmpObj = JSVAL_TO_OBJECT(anonEvaluate(cx, global, "(function () { return cc.MenuItemFont; })()"));
	JS_DefineFunction(cx, tmpObj, "create", js_cocos2dx_CCMenuItemFont_create, 1, JSPROP_READONLY | JSPROP_PERMANENT);
	tmpObj = JSVAL_TO_OBJECT(anonEvaluate(cx, global, "(function () { return cc.MenuItemToggle; })()"));
	JS_DefineFunction(cx, tmpObj, "create", js_cocos2dx_CCMenuItemToggle_create, 1, JSPROP_READONLY | JSPROP_PERMANENT);
}
