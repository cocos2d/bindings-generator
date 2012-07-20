//
//  ScriptingCore.cpp
//  testmonkey
//
//  Created by Rolando Abarca on 3/14/12.
//  Copyright (c) 2012 Zynga Inc. All rights reserved.
//

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "ScriptingCore.h"
#include "cocos2d.h"

#ifdef ANDROID
#include <android/log.h>
#endif

js_proxy_t *_native_js_global_ht = NULL;
js_proxy_t *_js_native_global_ht = NULL;
js_type_class_t *_js_global_type_ht = NULL;
char *_js_log_buf = NULL;

static void executeJSFunctionFromReservedSpot(JSContext *cx, js_proxy_t *p, 
                                              jsval &dataVal, jsval &retval) {
    JSBool hasAction;
    jsval temp_retval;
    
    
    //  if(p->jsclass->JSCLASS_HAS_RESERVED_SLOTS(1)) {
    jsval func = JS_GetReservedSlot(p->obj, 0);
    jsval funcRef;
    JS_ConvertValue(cx, func, JSTYPE_FUNCTION, &funcRef);
    
    jsval thisObj = JS_GetReservedSlot(p->obj, 1);
    if(thisObj == JSVAL_VOID) {
        JS_CallFunctionValue(cx, p->obj, funcRef, 1, &dataVal, &retval);
    } else {
        assert(!JSVAL_IS_PRIMITIVE(thisObj));
        JS_CallFunctionValue(cx, JSVAL_TO_OBJECT(thisObj), funcRef, 1, &dataVal, &retval);
    }        
    //  }
}


static void executeJSFunctionWithName(JSContext *cx, js_proxy_t *p, 
                                      const char *funcName, jsval &dataVal,
                                      jsval &retval) {
    JSBool hasAction;
    jsval temp_retval;
	
    if (JS_HasProperty(cx, p->obj, funcName, &hasAction) && hasAction) {
        if(!JS_GetProperty(cx, p->obj, funcName, &temp_retval)) {
            return;
        }
        if(temp_retval == JSVAL_VOID) {
            return;
        }
        JS_CallFunctionName(cx, p->obj, funcName, 
                            1, &dataVal, &retval);
    }
	
}

void js_log(const char *format, ...) {
	if (_js_log_buf == NULL) {
		_js_log_buf = (char *)calloc(sizeof(char), 257);
	}
	va_list vl;
	va_start(vl, format);
	int len = vsnprintf(_js_log_buf, 256, format, vl);
	va_end(vl);
	if (len) {
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_DEBUG, "js_log", _js_log_buf);
#else
		fprintf(stderr, "JS: %s\n", _js_log_buf);
#endif
    }
}

static JSClass global_class = {
	"global", JSCLASS_GLOBAL_FLAGS,
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
	JSCLASS_NO_OPTIONAL_MEMBERS
};

ScriptingCore::ScriptingCore()
{
	this->rt = JS_NewRuntime(8 * 1024 * 1024);
	this->cx = JS_NewContext(rt, 8192);
	JS_SetOptions(this->cx, JSOPTION_VAROBJFIX);
	JS_SetVersion(this->cx, JSVERSION_LATEST);
	JS_SetErrorReporter(this->cx, ScriptingCore::reportError);
	global = JS_NewCompartmentAndGlobalObject(cx, &global_class, NULL);
	if (!JS_InitStandardClasses(cx, global)) {
		js_log("error initializing the VM");
	}

	// register some global functions
	JS_DefineFunction(this->cx, global, "require", ScriptingCore::executeScript, 1, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineFunction(this->cx, global, "log", ScriptingCore::log, 0, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineFunction(this->cx, global, "executeScript", ScriptingCore::executeScript, 1, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineFunction(this->cx, global, "forceGC", ScriptingCore::forceGC, 0, JSPROP_READONLY | JSPROP_PERMANENT);
}

bool ScriptingCore::evalString(const char *string, jsval *outVal, const char *filename)
{
	jsval rval;
	JSString *str;
	JSBool ok;
	const char *fname = (filename ? filename : "noname");
	uint32_t lineno = 0;
	if (outVal == NULL) {
		outVal = &rval;
	}
	ok = JS_EvaluateScript(cx, global, string, strlen(string), fname, lineno, outVal);
	if (ok == JS_FALSE) {
		js_log("error evaluating script:\n%s", string);
	}
	str = JS_ValueToString(cx, rval);
	return ok;
}

void ScriptingCore::runScript(const char *path)
{
	cocos2d::CCFileUtils *futil = cocos2d::CCFileUtils::sharedFileUtils();
#ifdef DEBUG
	/**
	 * dpath should point to the parent directory of the "JS" folder. If this is
	 * set to "" (as it is now) then it will take the scripts from the app bundle.
	 * By setting the absolute path you can iterate the development only by
	 * modifying those scripts and reloading from the simulator (no recompiling/
	 * relaunching)
	 */
//	std::string dpath("/Users/rabarca/Desktop/testjs/testjs/");
	std::string dpath("");
	dpath += path;
	const char *realPath = futil->fullPathFromRelativePath(dpath.c_str());
#else
	const char *realPath = NULL;
    futil->fullPathFromRelativePath(path);
#endif

    if (!realPath) {
        return;
    }

	unsigned char *content = NULL;
	unsigned long contentSize = 0;

    content = futil->getFileData(realPath, "r", &contentSize);
	if (content && contentSize) {
		ScriptingCore::getInstance()->evalString((const char*) content, NULL);
		free(content);
	}
}

ScriptingCore::~ScriptingCore()
{
	JS_DestroyContext(cx);
	JS_DestroyRuntime(rt);
	JS_ShutDown();
	if (_js_log_buf) {
		free(_js_log_buf);
		_js_log_buf = NULL;
	}
}

int ScriptingCore::executeFunctionWithIntegerData(int nHandler, int data, CCNode *self) {
    js_proxy_t * p;
    JS_GET_PROXY(p, self);
    
    assert(p);    
    
    jsval retval;
    jsval dataVal = INT_TO_JSVAL(1);
	js_proxy_t *proxy;
	JS_GET_PROXY(proxy, self);
    
    std::string funcName = "";
    if(data == kCCNodeOnEnter) {
        executeJSFunctionWithName(this->cx, p, "onEnter", dataVal, retval);
    } else if(data == kCCNodeOnExit) {
        executeJSFunctionWithName(this->cx, p, "onExit", dataVal, retval);
    } else if(data == kCCMenuItemActivated) {
		dataVal = (proxy ? OBJECT_TO_JSVAL(proxy->obj) : JSVAL_NULL);
        executeJSFunctionFromReservedSpot(this->cx, p, dataVal, retval);
    }
	
    
    
    return 1;
}


int ScriptingCore::executeFunctionWithFloatData(int nHandler, float data, CCNode *self) {
    
    
    js_proxy_t * p;
    JS_GET_PROXY(p, self);
    
    assert(p);
    
    jsval retval;
    jsval dataVal = DOUBLE_TO_JSVAL(data);
    
    std::string funcName = "";
    
    executeJSFunctionWithName(this->cx, p, "update", dataVal, retval);
    
    //    if(data == kCCNodeOnEnter) {
    //        executeJSFunctionWithName(this->cx, p, "onEnter", dataVal, retval);
    //    } else if(data == kCCNodeOnExit) {
    //        executeJSFunctionWithName(this->cx, p, "onExit", dataVal, retval);
    //    } else if(data == kCCMenuItemActivated) {
    //        executeJSFunctionFromReservedSpot(this->cx, p, dataVal, retval);
    //    }
    
    return 1;
}

static void getTouchFuncName(int eventType, std::string &funcName) {
    switch(eventType) {
        case CCTOUCHBEGAN:
            funcName = "ccTouchBegan";
            break;
        case CCTOUCHENDED:
            funcName = "ccTouchEnded";
            break;
        case CCTOUCHMOVED:
            funcName = "ccTouchMoved";
            break;
        case CCTOUCHCANCELLED:
            funcName = "ccTouchCancelled";
            break;
    }
    
}


static void getJSTouchObject(JSContext *cx, CCTouch *x, jsval &jsret) {
    
    js_type_class_t *p;
    const char* type = x->getObjectType();
    HASH_FIND_STR(_js_global_type_ht, type, p);
    assert(p);
    JSObject *_tmp = JS_NewObject(cx, p->jsclass, p->proto, p->parentProto);

    js_proxy_t *proxy;
    JS_NEW_PROXY(proxy, x, _tmp);
    
    jsret = OBJECT_TO_JSVAL(_tmp);
    
}


int ScriptingCore::executeTouchesEvent(int nHandler, int eventType, 
                                       CCSet *pTouches, CCNode *self) {
    
    jsval retval;
    
    std::string funcName;
    getTouchFuncName(eventType, funcName);
    
    JSObject *jsretArr = JS_NewArrayObject(this->cx, 0, NULL);
    int count = 0;
    for(CCSetIterator it = pTouches->begin(); it != pTouches->end(); ++it, ++count) {
        jsval jsret;
        getJSTouchObject(this->cx, (CCTouch *) *it, jsret);
        if(!JS_SetElement(this->cx, jsretArr, count, &jsret)) {
            break;
        }
    }
    
    js_proxy_t *lP;
    JS_GET_PROXY(lP, self);
    assert(lP);
    
    jsval jsretArrVal = OBJECT_TO_JSVAL(jsretArr);
    executeJSFunctionWithName(this->cx, lP, funcName.c_str(), jsretArrVal, retval);
    
    return 1;
}


int ScriptingCore::executeSchedule(int nHandler, float dt, CCNode *self) {
    
    executeFunctionWithFloatData(nHandler, dt, self);
    return 1;
}




