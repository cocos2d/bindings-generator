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

js_proxy_t *_js_global_ht = NULL;
js_type_class_t *_js_global_type_ht = NULL;
char *_js_log_buf = NULL;

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
	const char *realPath = NULL;
    //cocos2d::CCFileUtils::fullPathFromRelativePath(dpath.c_str());
#else
	const char *realPath = NULL;
    //cocos2d::CCFileUtils::fullPathFromRelativePath(path);
#endif
	unsigned char *content = NULL;
	size_t contentSize = 0;
    //cocos2d::CCFileUtils::ccLoadFileIntoMemory(realPath, &content);
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
