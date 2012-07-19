//
//  ScriptingCore.h
//  testmonkey
//
//  Created by Rolando Abarca on 3/14/12.
//  Copyright (c) 2012 Zynga Inc. All rights reserved.
//

#ifndef __SCRIPTING_CORE_H__
#define __SCRIPTING_CORE_H__

#include <assert.h>
#include "uthash.h"
#include "jsapi.h"
#include "spidermonkey_specifics.h"

void js_log(const char *format, ...);

class ScriptingCore
{
	JSRuntime *rt;
	JSContext *cx;
	JSObject  *global;
	
	ScriptingCore();
public:
	~ScriptingCore();
	
	static ScriptingCore *getInstance() {
		static ScriptingCore instance;
		return &instance;
	};
	
	/**
	 * will eval the specified string
	 * @param string The string with the javascript code to be evaluated
	 * @param outVal The jsval that will hold the return value of the evaluation.
	 * Can be NULL.
	 */
	bool evalString(const char *string, jsval *outVal);
	
	/**
	 * will run the specified string
	 * @param string The path of the script to be run
	 */
	void runScript(const char *path);
	
	/**
	 * @return the global context
	 */
	JSContext* getGlobalContext() {
		return cx;
	};
	
	/**
	 * @param cx
	 * @param message
	 * @param report
	 */
	static void reportError(JSContext *cx, const char *message, JSErrorReport *report)
	{
		js_log("%s:%u:%s\n",
			   report->filename ? report->filename : "<no filename=\"filename\">",
			   (unsigned int) report->lineno,
			   message);
	};
	
	/**
	 * Log something using CCLog
	 * @param cx
	 * @param argc
	 * @param vp
	 */
	static JSBool log(JSContext *cx, uint32_t argc, jsval *vp)
	{
		if (argc > 0) {
			JSString *string = NULL;
			JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &string);
			if (string) {
				char *cstr = JS_EncodeString(cx, string);
				js_log(cstr);
			}
		}
		return JS_TRUE;
	};

	JSBool setReservedSpot(uint32_t i, JSObject *obj, jsval value) {
        jsval vp;
        if(JSVAL_IS_PRIMITIVE(value)) {
            js_log("JS Value not a function/object");
        }
        JS_ConvertValue(this->cx, value, JSTYPE_FUNCTION, &vp);
        JS_SetReservedSlot(obj, i, vp);
        return JS_TRUE;
    };

	/**
	 * run a script from script :)
	 */
	static JSBool executeScript(JSContext *cx, uint32_t argc, jsval *vp)
	{
		if (argc == 1) {
			JSString *string;
			if (JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &string) == JS_TRUE) {
				ScriptingCore::getInstance()->runScript(JS_EncodeString(cx, string));
			}
		}
		return JS_TRUE;
	};
	
	/**
	 * Force a cycle of GC
	 * @param cx
	 * @param argc
	 * @param vp
	 */
	static JSBool forceGC(JSContext *cx, uint32_t argc, jsval *vp)
	{
		JS_GC(cx);
		return JS_TRUE;
	};
};

#endif
