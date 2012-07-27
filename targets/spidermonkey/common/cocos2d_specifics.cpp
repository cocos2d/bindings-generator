#include "cocos2d.h"
#include "cocos2d_specifics.hpp"


void JSTouchDelegate::setJSObject(JSObject *obj) {
    _mObj = obj;
}

void JSTouchDelegate::registerStandardDelegate() {
    CCDirector* pDirector = CCDirector::sharedDirector();
    pDirector->getTouchDispatcher()->addStandardDelegate(this,0);
}

void JSTouchDelegate::registerTargettedDelegate(int priority, bool swallowsTouches) {
    CCDirector* pDirector = CCDirector::sharedDirector();
    pDirector->getTouchDispatcher()->addTargetedDelegate(this,
                                                         priority,
                                                         swallowsTouches);

}


static void addCallBackAndThis(JSObject *obj, jsval callback, jsval &thisObj) {
    if(callback != JSVAL_VOID) {
        ScriptingCore::getInstance()->setReservedSpot(0, obj, callback);
    }
    if(thisObj != JSVAL_VOID) {
        ScriptingCore::getInstance()->setReservedSpot(1, obj, thisObj);
    }
}

template<class T>
JSObject* bind_menu_item(JSContext *cx, T* nativeObj, jsval callback, jsval thisObj) {    
	js_proxy_t *p;
	JS_GET_PROXY(p, nativeObj);
	if (p) {
		addCallBackAndThis(p->obj, callback, thisObj);
		return p->obj;
	} else {
		js_type_class_t *classType = js_get_type_from_native<T>(nativeObj);
		assert(classType);
		JSObject *tmp = JS_NewObject(cx, classType->jsclass, classType->proto, classType->parentProto);

		// bind nativeObj <-> JSObject
		js_proxy_t *proxy;
		JS_NEW_PROXY(proxy, nativeObj, tmp);
        
		addCallBackAndThis(tmp, callback, thisObj);

		return tmp;
	}
}

JSBool js_cocos2dx_CCNode_getChildren(JSContext *cx, uint32_t argc, jsval *vp)
{
	JSObject *thisObj = JSVAL_TO_OBJECT(JS_THIS(cx, vp));
	if (thisObj) {
		js_proxy_t *proxy;
		JS_GET_NATIVE_PROXY(proxy, thisObj);
		if (proxy) {
			cocos2d::CCNode *node = (cocos2d::CCNode *)(proxy->ptr ? proxy->ptr : NULL);
			cocos2d::CCArray *children = node->getChildren();
			JSObject *jsarr = JS_NewArrayObject(cx, children->count(), NULL);
			for (int i=0; i < children->count(); i++) {
				cocos2d::CCNode *child = (cocos2d::CCNode*)children->objectAtIndex(i);
				js_proxy_t *childProxy = js_get_or_create_proxy<cocos2d::CCNode>(cx, child);
				jsval childVal = OBJECT_TO_JSVAL(childProxy->obj);
				JS_SetElement(cx, jsarr, i, &childVal);
			}
			JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(jsarr));
		}
		return JS_TRUE;
	}
	return JS_FALSE;
}

JSBool js_cocos2dx_CCMenu_create(JSContext *cx, uint32_t argc, jsval *vp)
{
	jsval *argv = JS_ARGV(cx, vp);
	if (argc > 0) {
		cocos2d::CCArray* array = cocos2d::CCArray::create();
		int i = 0;
		while (i < argc) {
			js_proxy_t *proxy;
			JSObject *tmpObj = JSVAL_TO_OBJECT(argv[i]);
			JS_GET_NATIVE_PROXY(proxy, tmpObj);
			cocos2d::CCObject *item = (cocos2d::CCObject*)(proxy ? proxy->ptr : NULL);
			TEST_NATIVE_OBJECT(cx, item)
			array->addObject(item);
			i++;
		}
		cocos2d::CCMenu* ret = cocos2d::CCMenu::create(array);
		jsval jsret;
		do {
			if (ret) {
				js_proxy_t *p;
				JS_GET_PROXY(p, ret);
				if (p) {
					jsret = OBJECT_TO_JSVAL(p->obj);
				} else {
					// create a new js obj of that class
					js_proxy_t *proxy = js_get_or_create_proxy<cocos2d::CCMenu>(cx, ret);
					jsret = OBJECT_TO_JSVAL(proxy->obj);
				}
			} else {
				jsret = JSVAL_NULL;
			}
		} while (0);
		JS_SET_RVAL(cx, vp, jsret);
		return JS_TRUE;
	}
	if (argc == 0) {
		cocos2d::CCMenu* ret = cocos2d::CCMenu::create();
		jsval jsret;
		do {
			if (ret) {
				js_proxy_t *p;
				JS_GET_PROXY(p, ret);
				if (p) {
					jsret = OBJECT_TO_JSVAL(p->obj);
				} else {
					// create a new js obj of that class
					js_proxy_t *proxy = js_get_or_create_proxy<cocos2d::CCMenu>(cx, ret);
					jsret = OBJECT_TO_JSVAL(proxy->obj);
				}
			} else {
				jsret = JSVAL_NULL;
			}
		} while (0);
		JS_SET_RVAL(cx, vp, jsret);
		return JS_TRUE;
	}
	return JS_FALSE;
}

JSBool js_cocos2dx_CCSequence_create(JSContext *cx, uint32_t argc, jsval *vp)
{
	jsval *argv = JS_ARGV(cx, vp);
	if (argc > 0) {
		cocos2d::CCArray* array = cocos2d::CCArray::create();
		int i = 0;
		while (i < argc) {
			js_proxy_t *proxy;
			JSObject *tmpObj = JSVAL_TO_OBJECT(argv[i]);
			JS_GET_NATIVE_PROXY(proxy, tmpObj);
			cocos2d::CCObject *item = (cocos2d::CCObject*)(proxy ? proxy->ptr : NULL);
			TEST_NATIVE_OBJECT(cx, item)
			array->addObject(item);
			i++;
		}
		cocos2d::CCFiniteTimeAction* ret = cocos2d::CCSequence::create(array);
		jsval jsret;
		do {
			if (ret) {
				js_proxy_t *p;
				JS_GET_PROXY(p, ret);
				if (p) {
					jsret = OBJECT_TO_JSVAL(p->obj);
				} else {
					// create a new js obj of that class
					js_proxy_t *proxy = js_get_or_create_proxy<cocos2d::CCFiniteTimeAction>(cx, ret);
					jsret = OBJECT_TO_JSVAL(proxy->obj);
				}
			} else {
				jsret = JSVAL_NULL;
			}
		} while (0);
		JS_SET_RVAL(cx, vp, jsret);
		return JS_TRUE;
	}
	return JS_FALSE;
}

JSBool js_cocos2dx_CCSpawn_create(JSContext *cx, uint32_t argc, jsval *vp)
{
	jsval *argv = JS_ARGV(cx, vp);
	if (argc > 0) {
		cocos2d::CCArray* array = cocos2d::CCArray::create();
		int i = 0;
		while (i < argc) {
			js_proxy_t *proxy;
			JSObject *tmpObj = JSVAL_TO_OBJECT(argv[i]);
			JS_GET_NATIVE_PROXY(proxy, tmpObj);
			cocos2d::CCObject *item = (cocos2d::CCObject*)(proxy ? proxy->ptr : NULL);
			TEST_NATIVE_OBJECT(cx, item)
			array->addObject(item);
			i++;
		}
		cocos2d::CCFiniteTimeAction* ret = cocos2d::CCSpawn::create(array);
		jsval jsret;
		do {
			if (ret) {
				js_proxy_t *p;
				JS_GET_PROXY(p, ret);
				if (p) {
					jsret = OBJECT_TO_JSVAL(p->obj);
				} else {
					// create a new js obj of that class
					js_proxy_t *proxy = js_get_or_create_proxy<cocos2d::CCFiniteTimeAction>(cx, ret);
					jsret = OBJECT_TO_JSVAL(proxy->obj);
				}
			} else {
				jsret = JSVAL_NULL;
			}
		} while (0);
		JS_SET_RVAL(cx, vp, jsret);
		return JS_TRUE;
	}
	return JS_FALSE;
}

JSBool js_cocos2dx_CCMenuItem_create(JSContext *cx, uint32_t argc, jsval *vp)
{
	if (argc >= 1) {
		jsval *argv = JS_ARGV(cx, vp);
		cocos2d::CCMenuItem* ret = cocos2d::CCMenuItem::create();
		JSObject *obj = bind_menu_item<cocos2d::CCMenuItem>(cx, ret, argc == 2? argv[1] : JSVAL_VOID, argv[0]);
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

        int last = 2;
		cocos2d::CCNode* arg2 = NULL;
		if (argc == 5 || argc == 3) {
			tmpObj = JSVAL_TO_OBJECT(argv[2]);
			JS_GET_NATIVE_PROXY(proxy, tmpObj);
			arg2 = (cocos2d::CCNode*)(proxy ? proxy->ptr : NULL);
			TEST_NATIVE_OBJECT(cx, arg2);
            last = 3;
		}
		cocos2d::CCMenuItemSprite* ret = cocos2d::CCMenuItemSprite::create(arg0, arg1, arg2);

        jsval thisObj = argv[last++];
        jsval callback = argv[last];
		JSObject *obj = bind_menu_item<cocos2d::CCMenuItemSprite>(cx, ret, callback, thisObj);
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
		const char *arg2 = NULL;
		int last = 2;
		if (JSVAL_IS_STRING(argv[2])) {
			do { JSString *tmp = JS_ValueToString(cx, argv[2]); arg2 = JS_EncodeString(cx, tmp); } while (0);
			last = 3;
		}
		cocos2d::CCMenuItemImage* ret = cocos2d::CCMenuItemImage::create(arg0, arg1, arg2);
		jsval thisObj = argv[last++];
		jsval callback = argv[last];
		JSObject *obj = bind_menu_item<cocos2d::CCMenuItemImage>(cx, ret, callback, thisObj);
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
		JSObject *obj = bind_menu_item<cocos2d::CCMenuItemLabel>(cx, ret, (argc == 3 ? argv[2] : JSVAL_VOID),  (argc >= 2 ? argv[1] : JSVAL_VOID));
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
		JSObject *obj = bind_menu_item<cocos2d::CCMenuItemAtlasFont>(cx, ret, (argc == 7 ? argv[6] : JSVAL_VOID), (argc >= 6 ? argv[5] : JSVAL_VOID));
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
		JSObject *obj = bind_menu_item<cocos2d::CCMenuItemFont>(cx, ret, (argc == 3 ? argv[2] : JSVAL_VOID), (argc >= 2 ? argv[1] : JSVAL_VOID));
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
		JSObject *obj = bind_menu_item(cx, ret, (argc == 2 ? argv[1] : JSVAL_VOID), argv[0]);
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


JSBool js_cocos2dx_setCallback(JSContext *cx, uint32_t argc, jsval *vp) {

    if(argc == 2) {
        jsval *argv = JS_ARGV(cx, vp);
        JSObject *obj = JS_THIS_OBJECT(cx, vp);
        js_proxy_t *proxy;
        JS_GET_NATIVE_PROXY(proxy, obj);
        cocos2d::CCMenuItem* item = (cocos2d::CCMenuItem*)(proxy ? proxy->ptr : NULL);
        TEST_NATIVE_OBJECT(cx, item)
        bind_menu_item(cx, item, argv[1], argv[0]);
        return JS_TRUE;
    }
    return JS_FALSE;
}


JSBool js_cocos2dx_CCAnimation_create(JSContext *cx, uint32_t argc, jsval *vp)
{
	jsval *argv = JS_ARGV(cx, vp);
	if (argc <= 3) {
		cocos2d::CCArray* arg0;
		if (argc > 0 && JSVAL_IS_OBJECT(argv[0])) {
			arg0 = cocos2d::CCArray::create();
			JSObject *jsarr = JSVAL_TO_OBJECT(argv[0]);
			uint32_t len;
			if (JS_IsArrayObject(cx, jsarr) && JS_GetArrayLength(cx, jsarr, &len)) {
				for (int i=0; i < len; i++) {
					jsval elt;
					if (JS_GetElement(cx, jsarr, i, &elt)) {
						js_proxy_t *proxy;
						JSObject *tmpObj = JSVAL_TO_OBJECT(elt);
						JS_GET_NATIVE_PROXY(proxy, tmpObj);
						cocos2d::CCObject *tmpCObj = (cocos2d::CCObject *)(proxy ? proxy->ptr : NULL);
						TEST_NATIVE_OBJECT(cx, tmpCObj);
						arg0->addObject(tmpCObj);
					}
				}
			}
		}
		cocos2d::CCAnimation* ret;
		double arg1 = 0.0f;
		if (argc > 0 && argc == 2) {
			if (argc == 2) {
				JS_ValueToNumber(cx, argv[1], &arg1);
			}
			ret = cocos2d::CCAnimation::create(arg0, arg1);
		} else if (argc > 0) {
			unsigned int loops;
			JS_ValueToNumber(cx, argv[1], &arg1);
			JS_ValueToECMAUint32(cx, argv[1], &loops);
			ret = cocos2d::CCAnimation::create(arg0, arg1, loops);
		} else if (argc == 0) {
			ret = cocos2d::CCAnimation::create();
		}
		jsval jsret;
		if (ret) {
			js_proxy_t *proxy;
			JS_GET_PROXY(proxy, ret);
			if (proxy) {
				jsret = OBJECT_TO_JSVAL(proxy->obj);
			} else {
				// create a new js obj of that class
				proxy = js_get_or_create_proxy<cocos2d::CCAnimation>(cx, ret);
				jsret = OBJECT_TO_JSVAL(proxy->obj);
			}
		} else {
			jsret = JSVAL_NULL;
		}
		JS_SET_RVAL(cx, vp, jsret);
		return JS_TRUE;
	}
	return JS_FALSE;
}


JSBool js_cocos2dx_JSTouchDelegate_registerStandardDelegate(JSContext *cx, uint32_t argc, jsval *vp)
{
	if (argc >= 1) {
		jsval *argv = JS_ARGV(cx, vp);
        
        JSTouchDelegate *touch = new JSTouchDelegate();
        touch->registerStandardDelegate();
        touch->setJSObject((argc == 1 ? JSVAL_TO_OBJECT(argv[0]) : JSVAL_TO_OBJECT(JSVAL_VOID)));
        
		return JS_TRUE;
	}
	return JS_FALSE;
}

JSBool js_cocos2dx_JSTouchDelegate_registerTargettedDelegate(JSContext *cx, uint32_t argc, jsval *vp)
{
	if (argc >= 1) {
		jsval *argv = JS_ARGV(cx, vp);
        
        JSTouchDelegate *touch = new JSTouchDelegate();
        touch->registerTargettedDelegate((argc >= 1 ? JSVAL_TO_INT(argv[0]) : 0), (argc >= 2 ? JSVAL_TO_BOOLEAN(argv[1]) : true));
        touch->setJSObject((argc == 3 ? JSVAL_TO_OBJECT(argv[2]) : JSVAL_TO_OBJECT(JSVAL_VOID)));
        
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
				JS_REMOVE_PROXY(jsproxy, nproxy);
				JS_NEW_PROXY(nproxy, ptrTwo, one);
			}
		}
	}
	return JS_TRUE;
}

JSBool js_cocos2dx_CCNode_copy(JSContext *cx, uint32_t argc, jsval *vp)
{
	if (argc == 0) {
		JSObject *obj = JSVAL_TO_OBJECT(JS_THIS(cx, vp));
		js_proxy_t *proxy;
		JS_GET_NATIVE_PROXY(proxy, obj);
		cocos2d::CCNode *node = (cocos2d::CCNode *)(proxy ? proxy->ptr : NULL);
		TEST_NATIVE_OBJECT(cx, node)
		JSClass *jsclass = JS_GetClass(obj);
		JSObject *proto = JS_GetPrototype(obj);
		JSObject *parent = JS_GetParent(obj);
		JSObject *jsret = JS_NewObject(cx, jsclass, proto, parent);
		cocos2d::CCObject *ret = node->copy();
		if (ret && jsret) {
			JS_NEW_PROXY(proxy, ret, jsret);
			JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(jsret));
			return JS_TRUE;
		}
	}
	return JS_FALSE;
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


void JSCallFunc::setJSCallbackFunc(jsval func) {
    jsCallback = func;
}

void JSCallFunc::setJSCallbackThis(jsval thisObj) {
    jsThisObj = thisObj;
}

void JSCallFunc::setExtraDataField(jsval data) {
     extraData = new jsval();
     *extraData = data;
}

JSBool js_callFunc(JSContext *cx, uint32_t argc, jsval *vp)
{
    
    if (argc >= 1) {        
		jsval *argv = JS_ARGV(cx, vp);

        JSCallFunc *tmpCobj = new JSCallFunc();

        tmpCobj->setJSCallbackThis(argv[0]);
        if(argc >= 2) {
            tmpCobj->setJSCallbackFunc(argv[1]);
        } if(argc == 3) {
            tmpCobj->setExtraDataField(argv[2]);
        }
        
        CCCallFunc *ret = (CCCallFunc *)CCCallFuncN::create((CCObject *)tmpCobj, 
                                             callfuncN_selector(JSCallFunc::callbackFunc));
        
		js_proxy_t *proxy = js_get_or_create_proxy<cocos2d::CCCallFunc>(cx, ret);
		JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(proxy->obj));
      //  test->execute();
    }
    return JS_TRUE;
}

JSBool js_forceGC(JSContext *cx, uint32_t argc, jsval *vp) {
    JS_GC(cx);
    return JS_TRUE;
}

#ifndef ANDROID
JSBool js_break(JSContext *cx, uint32_t argc, jsval *vp)
{
	JSObject *obj = NULL;
	if (argc == 1) {
		jsval *argv = JS_ARGV(cx, vp);
		if (JSVAL_IS_OBJECT(argv[0])) {
			JS_ValueToObject(cx, argv[0], &obj);
		}
	}
	__builtin_trap();
	return JS_TRUE;
}
#endif

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
		} else {
			typeId = reinterpret_cast<int>(typeid(T).name());
		}
		HASH_FIND_INT(_js_global_type_ht, &typeId, typeProxy);
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

extern JSObject* js_cocos2dx_CCNode_prototype;
extern JSObject* js_cocos2dx_CCAction_prototype;
extern JSObject* js_cocos2dx_CCMenuItem_prototype;

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
#ifndef ANDROID
	JS_DefineFunction(cx, global, "__break", js_break, 0, JSPROP_READONLY | JSPROP_PERMANENT);
#endif

	JSObject *tmpObj;
	JS_DefineFunction(cx, js_cocos2dx_CCNode_prototype, "getChildren", js_cocos2dx_CCNode_getChildren, 1, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineFunction(cx, js_cocos2dx_CCNode_prototype, "copy", js_cocos2dx_CCNode_copy, 1, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineFunction(cx, js_cocos2dx_CCAction_prototype, "copy", js_cocos2dx_CCNode_copy, 1, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineFunction(cx, js_cocos2dx_CCMenuItem_prototype, "setCallback", js_cocos2dx_setCallback, 2, JSPROP_READONLY | JSPROP_PERMANENT);
	tmpObj = JSVAL_TO_OBJECT(anonEvaluate(cx, global, "(function () { return cc.Node.prototype; })()"));
	JS_DefineFunction(cx, tmpObj, "copy", js_cocos2dx_CCNode_copy, 1, JSPROP_READONLY | JSPROP_PERMANENT);
	tmpObj = JSVAL_TO_OBJECT(anonEvaluate(cx, global, "(function () { return cc.Menu; })()"));
	JS_DefineFunction(cx, tmpObj, "create", js_cocos2dx_CCMenu_create, 0, JSPROP_READONLY | JSPROP_PERMANENT);
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
	tmpObj = JSVAL_TO_OBJECT(anonEvaluate(cx, global, "(function () { return cc.Sequence; })()"));
	JS_DefineFunction(cx, tmpObj, "create", js_cocos2dx_CCSequence_create, 0, JSPROP_READONLY | JSPROP_PERMANENT);
	tmpObj = JSVAL_TO_OBJECT(anonEvaluate(cx, global, "(function () { return cc.Spawn; })()"));
	JS_DefineFunction(cx, tmpObj, "create", js_cocos2dx_CCSpawn_create, 0, JSPROP_READONLY | JSPROP_PERMANENT);
	tmpObj = JSVAL_TO_OBJECT(anonEvaluate(cx, global, "(function () { return cc.Animation; })()"));
	JS_DefineFunction(cx, tmpObj, "create", js_cocos2dx_CCAnimation_create, 0, JSPROP_READONLY | JSPROP_PERMANENT);
    
	JS_DefineFunction(cx, ns, "registerTargettedDelegate", js_cocos2dx_JSTouchDelegate_registerTargettedDelegate, 1, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineFunction(cx, ns, "registerStandardDelegate", js_cocos2dx_JSTouchDelegate_registerStandardDelegate, 1, JSPROP_READONLY | JSPROP_PERMANENT);

	tmpObj = JSVAL_TO_OBJECT(anonEvaluate(cx, global, "(function () { return cc.CallFunc; })()"));
	JS_DefineFunction(cx, tmpObj, "create", js_callFunc, 1, JSPROP_READONLY | JSPROP_PERMANENT);

     tmpObj = JSVAL_TO_OBJECT(anonEvaluate(cx, global, "(function () { return this; })()"));
    JS_DefineFunction(cx, tmpObj, "garbageCollect", js_forceGC, 1, JSPROP_READONLY | JSPROP_PERMANENT);

}
