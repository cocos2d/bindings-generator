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
#include "cocos2dx.hpp"

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
	JS_CallFunctionValue(cx, p->obj, funcRef, 1, &dataVal, &retval);
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
	if (len)
		fprintf(stderr, "JS: %s\n", _js_log_buf);
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
	JS_DefineFunction(this->cx, global, "require", ScriptingCore::executeScript, 0, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineFunction(this->cx, global, "log", ScriptingCore::log, 0, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineFunction(this->cx, global, "executeScript", ScriptingCore::executeScript, 1, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineFunction(this->cx, global, "forceGC", ScriptingCore::forceGC, 0, JSPROP_READONLY | JSPROP_PERMANENT);
}

bool ScriptingCore::evalString(const char *string, jsval *outVal)
{
	jsval rval;
	JSString *str;
	JSBool ok;
	const char *filename = "noname";
	uint32_t lineno = 0;
	if (outVal == NULL) {
		outVal = &rval;
	}
	ok = JS_EvaluateScript(cx, global, string, strlen(string), filename, lineno, outVal);
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
	unsigned char *content = NULL;
	unsigned long contentSize = 0;
    content = futil->getFileData(realPath, "r", &contentSize);
	if (content && contentSize) {
		JSBool ok;
		jsval rval;
		ok = JS_EvaluateScript(this->cx, this->global, (char *)content, contentSize, path, 1, &rval);
		if (ok == JS_FALSE) {
			js_log("error evaluating script:\n%s", content);
		}
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
	
    
	//    JSFunction *c = (JSFunction *)p->ptr;
	//    
	//    JS_CallFunction(this->cx, p->obj, c, 1, &dataVal, retval);
    //  getJSHandler(self, nHandler);                                                                                                                                                                                                           
    return 1;
}


int ScriptingCore::executeTouchesEvent(int nHandler, int eventType, CCSet *pTouches, CCNode *self) {
	//    js_proxy_t * p;
	//    JS_GET_PROXY(p, self);
	//    
	//    assert(p);
	//    
	//    jsval *retval;
	//    jsval dataVal = INT_TO_JSVAL(eventType);
	//    JS_CallFunction(this->cx, p->obj, (JSFunction *)p->ptr, 1, &dataVal, retval);
	//    //getJSHandler(self, nHandler);
    return 1;
}
