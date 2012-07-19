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
#include "cocos2d.h"
#include "uthash.h"
#include "jsapi.h"
#include "spidermonkey_specifics.h"

void js_log(const char *format, ...);

using namespace cocos2d;

class ScriptingCore : public CCScriptEngineProtocol
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

	lua_State* getLuaState(void) {}
    
    /**
     @brief Remove CCObject from lua state
     @param object to remove
     */
	virtual void removeCCObjectByID(int nLuaID) {}
    
    /**
     @brief Remove Lua function handler
     */
	virtual void removeLuaHandler(int nHandler) {}
    
    /**
     @brief Add a path to find lua files in
     @param path to be added to the Lua path
     */
	virtual void addSearchPath(const char* path) {}
    
    /**
     @brief Execute script code contained in the given string.
     @param codes holding the valid script code that should be executed.
     @return 0 if the string is excuted correctly.
     @return other if the string is excuted wrongly.
     */
	virtual int executeString(const char* codes) {}
    
    /**
     @brief Execute a script file.
     @param filename String object holding the filename of the script file that is to be executed
     */
    virtual  int executeScriptFile(const char* filename) {}
    
    /**
     @brief Execute a scripted global function.
     @brief The function should not take any parameters and should return an integer.
     @param functionName String object holding the name of the function, in the global script environment, that is to be executed.
     @return The integer value returned from the script function.
     */
	virtual int executeGlobalFunction(const char* functionName) {}
    
    /**
     @brief Execute a function by handler
     @param The function handler
     @param Number of parameters
     @return The integer value returned from the script function.
     */
	virtual int executeFunctionByHandler(int nHandler, int numArgs = 0) {}
    virtual int executeFunctionWithIntegerData(int nHandler, int data, CCNode *self);
	virtual int executeFunctionWithFloatData(int nHandler, float data) {}
	virtual int executeFunctionWithBooleanData(int nHandler, bool data) {}
	virtual int executeFunctionWithCCObject(int nHandler, CCObject* pObject, const char* typeName) {}    
	virtual int pushIntegerToLuaStack(int data) {}
	virtual int pushFloatToLuaStack(int data) {}
	virtual int pushBooleanToLuaStack(int data) {}
	virtual int pushCCObjectToLuaStack(CCObject* pObject, const char* typeName) {}
    
    // functions for excute touch event
	virtual int executeTouchEvent(int nHandler, int eventType, CCTouch *pTouch) {}
    virtual int executeTouchesEvent(int nHandler, int eventType, CCSet *pTouches, CCNode *self);
    
    // execute a schedule function
	virtual int executeSchedule(int nHandler, float dt) {}

	/**
	 * will eval the specified string
	 * @param string The string with the javascript code to be evaluated
	 * @param outVal The jsval that will hold the return value of the evaluation.
	 * Can be NULL.
	 */
	bool evalString(const char *string, jsval *outVal, const char *filename = NULL);
	
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
