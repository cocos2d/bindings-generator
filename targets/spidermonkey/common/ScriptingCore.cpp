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
#include <sys/stat.h>
#include <fcntl.h>
#include "ScriptingCore.h"
#include "cocos2d.h"

#ifdef ANDROID
#include <android/log.h>
#include <android/asset_manager.h>
#include <jni/JniHelper.h>
#endif

#ifdef ANDROID
#define  LOG_TAG    "ScriptingCore.cpp"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#else
#define  LOGD(...) js_log(...)
#endif

js_proxy_t *_native_js_global_ht = NULL;
js_proxy_t *_js_native_global_ht = NULL;
js_type_class_t *_js_global_type_ht = NULL;
char *_js_log_buf = NULL;

static void executeJSFunctionFromReservedSpot(JSContext *cx, JSObject *obj, 
                                              jsval &dataVal, jsval &retval) {

    //  if(p->jsclass->JSCLASS_HAS_RESERVED_SLOTS(1)) {
    jsval func = JS_GetReservedSlot(obj, 0);
    
    if(func == JSVAL_VOID) { return; }
    jsval thisObj = JS_GetReservedSlot(obj, 1);
    if(thisObj == JSVAL_VOID) {
        JS_CallFunctionValue(cx, obj, func, 1, &dataVal, &retval);
    } else {
        assert(!JSVAL_IS_PRIMITIVE(thisObj));
        JS_CallFunctionValue(cx, JSVAL_TO_OBJECT(thisObj), func, 1, &dataVal, &retval);
    }        
    //  }
}


static void executeJSFunctionWithName(JSContext *cx, JSObject *obj, 
                                      const char *funcName, jsval &dataVal,
                                      jsval &retval) {
    JSBool hasAction;
    jsval temp_retval;
	
    if (JS_HasProperty(cx, obj, funcName, &hasAction) && hasAction) {
        if(!JS_GetProperty(cx, obj, funcName, &temp_retval)) {
            return;
        }
        if(temp_retval == JSVAL_VOID) {
            return;
        }
        JS_CallFunctionName(cx, obj, funcName, 
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

void ScriptingCore::string_report(jsval val) {
    if (JSVAL_IS_NULL(val)) {
        LOGD("val : (JSVAL_IS_NULL(val)");
        // return 1;
    } else if ((JSVAL_IS_BOOLEAN(val)) &&
               (JS_FALSE == (JSVAL_TO_BOOLEAN(val)))) {
        LOGD("val : (return value is JS_FALSE");
        // return 1;
    } else if (JSVAL_IS_STRING(val)) {
        JSString *str = JS_ValueToString(this->getGlobalContext(), val);
        if (NULL == str) {
            LOGD("val : return string is NULL");
        } else {
            LOGD("val : return string =\n%s\n",
                 JS_EncodeString(this->getGlobalContext(), str));
        }
    } else if (JSVAL_IS_NUMBER(val)) {
        double number;
        if (JS_FALSE ==
            JS_ValueToNumber(this->getGlobalContext(), val, &number)) {
            LOGD("val : return number could not be converted");
        } else {
            LOGD("val : return number =\n%f", number);
        }
    }
}

JSBool ScriptingCore::evalString(const char *string, jsval *outVal, const char *filename)
{
    jsval rval;
    const char *fname = (filename ? filename : "noname");
    uint32_t lineno = 0;
    if (outVal == NULL) {
        outVal = &rval;
    }

    JSBool evaluatedOK = JS_EvaluateScript(cx, global,
                                           string, strlen(string),
                                           fname, lineno, outVal);

    if (JS_FALSE == evaluatedOK) {
        LOGD("evaluatedOK == JS_FALSE)");
    } else {
        this->string_report(*outVal);
    }

    return evaluatedOK;
}

#ifdef PLATFORM_IOS
static size_t readFileInMemory(const char *path, unsigned char **buff) {
    struct stat buf;
    int file = open(path, O_RDONLY);
    long readBytes = -1;
    if (file) {
        if (fstat(file, &buf) == 0) {
            *buff = (unsigned char *)calloc(buf.st_size + 1, 1);
            if (*buff) {
                readBytes = read(file, *buff, buf.st_size);
            }
        }
    }
    close(file);
    return readBytes;
}

JSBool ScriptingCore::runScript(const char *path)
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
//  std::string dpath("/Users/rabarca/Desktop/testjs/testjs/");
    std::string dpath("");
    dpath += path;
    const char *realPath = futil->fullPathFromRelativePath(dpath.c_str());
#else
    const char *realPath = NULL;
    futil->fullPathFromRelativePath(path);
#endif

    if (!realPath) {
        return JS_FALSE;
    }

    unsigned char *content = NULL;
    unsigned long contentSize = 0;

    contentSize = readFileInMemory(realPath, &content);
    JSBool ret = JS_FALSE;
    if (content && contentSize) {
        jsval rval;
        ret = this->evalString((const char *)content, &rval, path);
        free(content);
    }
    return ret;
}
#endif //PLATFORM_IOS

#ifdef ANDROID

static unsigned long
fileutils_read_into_new_memory(const char* relativepath,
                               unsigned char** content) {
    *content = NULL;

    AAssetManager* assetmanager =
        JniHelper::getAssetManager();
    if (NULL == assetmanager) {
        LOGD("assetmanager : is NULL");
        return 0;
    }

    // read asset data
    AAsset* asset =
        AAssetManager_open(assetmanager,
                           relativepath,
                           AASSET_MODE_UNKNOWN);
    if (NULL == asset) {
        LOGD("asset : is NULL");
        return 0; 
    }

    off_t size = AAsset_getLength(asset);
    LOGD("size = %d ", size);

    unsigned char* buf =
        (unsigned char*) malloc((sizeof(unsigned char)) * (size+1));
    if (NULL == buf) {
        LOGD("memory allocation failed");
        AAsset_close(asset);
        return 0;
    }

    int bytesread = AAsset_read(asset, buf, size);
    LOGD("bytesread = %d ", bytesread);
    buf[bytesread] = '\0';

    AAsset_close(asset);

    *content = (unsigned char*) buf;
    return bytesread;
}

JSBool ScriptingCore::runScript(const char *path)
{
    LOGD("ScriptingCore::runScript(%s)", path);

    if (NULL == path) {
        return JS_FALSE;
    }

    unsigned char* content = NULL;
    unsigned long contentsize = 0;

    contentsize = fileutils_read_into_new_memory(path, &content);

    if (NULL == content) {
        LOGD("(NULL == content)");
        return JS_FALSE;
    }

    if (contentsize <= 0) {
        LOGD("(contentsize <= 0)");
        free(content);
        return JS_FALSE;
    }

    jsval rval;
    JSBool ret = this->evalString((const char *)content, &rval, path);
    free(content);

    LOGD("... ScriptingCore::runScript(%s) done successfully.", path);

    return ret;
}

#endif //ANDROID

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
    
    if (!p) return 0;
    
    jsval retval;
    jsval dataVal = INT_TO_JSVAL(1);
    js_proxy_t *proxy;
    JS_GET_PROXY(proxy, self);
    
    std::string funcName = "";
    if(data == kCCNodeOnEnter) {
        executeJSFunctionWithName(this->cx, p->obj, "onEnter", dataVal, retval);
    } else if(data == kCCNodeOnExit) {
        executeJSFunctionWithName(this->cx, p->obj, "onExit", dataVal, retval);
    } else if(data == kCCMenuItemActivated) {
		dataVal = (proxy ? OBJECT_TO_JSVAL(proxy->obj) : JSVAL_NULL);
        executeJSFunctionFromReservedSpot(this->cx, p->obj, dataVal, retval);
    }
    
    
    
    return 1;
}


int ScriptingCore::executeFunctionWithFloatData(int nHandler, float data, CCNode *self) {
    
    
    js_proxy_t * p;
    JS_GET_PROXY(p, self);
    
    if (!p) return 0;
    
    jsval retval;
    jsval dataVal = DOUBLE_TO_JSVAL(data);
    
    std::string funcName = "";
    
    executeJSFunctionWithName(this->cx, p->obj, "update", dataVal, retval);
    
    //    if(data == kCCNodeOnEnter) {
    //        executeJSFunctionWithName(this->cx, p, "onEnter", dataVal, retval);
    //    } else if(data == kCCNodeOnExit) {
    //        executeJSFunctionWithName(this->cx, p, "onExit", dataVal, retval);
    //    } else if(data == kCCMenuItemActivated) {
    //        executeJSFunctionFromReservedSpot(this->cx, p, dataVal, retval);
    //    }
    
    return 1;
}

static void getTouchesFuncName(int eventType, std::string &funcName) {
    switch(eventType) {
        case CCTOUCHBEGAN:
            funcName = "onTouchesBegan";
            break;
        case CCTOUCHENDED:
            funcName = "onTouchesEnded";
            break;
        case CCTOUCHMOVED:
            funcName = "onTouchesMoved";
            break;
        case CCTOUCHCANCELLED:
            funcName = "onTouchesCancelled";
            break;
    }
    
}

static void getTouchFuncName(int eventType, std::string &funcName) {
    switch(eventType) {
        case CCTOUCHBEGAN:
            funcName = "onTouchBegan";
            break;
        case CCTOUCHENDED:
            funcName = "onTouchEnded";
            break;
        case CCTOUCHMOVED:
            funcName = "onTouchMoved";
            break;
        case CCTOUCHCANCELLED:
            funcName = "onTouchCancelled";
            break;
    }
    
}


static void getJSTouchObject(JSContext *cx, CCTouch *x, jsval &jsret) {    
    js_type_class_t *classType;
    TypeTest<cocos2d::CCTouch> t;
    uint32_t typeId = t.s_id();
    HASH_FIND_INT(_js_global_type_ht, &typeId, classType);
    assert(classType);
    JSObject *_tmp = JS_NewObject(cx, classType->jsclass, classType->proto, classType->parentProto);
    js_proxy_t *proxy;
    JS_NEW_PROXY(proxy, x, _tmp);
    jsret = OBJECT_TO_JSVAL(_tmp);
}


int ScriptingCore::executeTouchesEvent(int nHandler, int eventType, 
                                       CCSet *pTouches, CCNode *self) {
    
    jsval retval;
    
    std::string funcName;
    getTouchesFuncName(eventType, funcName);
    
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
    executeJSFunctionWithName(this->cx, lP->obj, funcName.c_str(), jsretArrVal, retval);
    
    return 1;
}

int ScriptingCore::executeCustomTouchesEvent(int eventType, 
                                       CCSet *pTouches, JSObject *obj)
{
    
    jsval retval;
    std::string funcName;
    getTouchesFuncName(eventType, funcName);
    
    JSObject *jsretArr = JS_NewArrayObject(this->cx, 0, NULL);
    int count = 0;
    for(CCSetIterator it = pTouches->begin(); it != pTouches->end(); ++it, ++count) {
        jsval jsret;
        getJSTouchObject(this->cx, (CCTouch *) *it, jsret);
        if(!JS_SetElement(this->cx, jsretArr, count, &jsret)) {
            break;
        }
    }
    
    jsval jsretArrVal = OBJECT_TO_JSVAL(jsretArr);
    executeJSFunctionWithName(this->cx, obj, funcName.c_str(), jsretArrVal, retval);
    
    return 1;
}


int ScriptingCore::executeCustomTouchEvent(int eventType, 
                                           CCTouch *pTouch, JSObject *obj) {
    jsval retval;
    std::string funcName;
    getTouchFuncName(eventType, funcName);
    
    jsval jsTouch;
    getJSTouchObject(this->cx, pTouch, jsTouch);
    
    executeJSFunctionWithName(this->cx, obj, funcName.c_str(), jsTouch, retval);
    return 1;
    
}  


int ScriptingCore::executeCustomTouchEvent(int eventType, 
                                           CCTouch *pTouch, JSObject *obj,
                                           jsval &retval) {

    std::string funcName;
    getTouchFuncName(eventType, funcName);
    
    jsval jsTouch;
    getJSTouchObject(this->cx, pTouch, jsTouch);

    executeJSFunctionWithName(this->cx, obj, funcName.c_str(), jsTouch, retval);
    return 1;
    
}  


int ScriptingCore::executeSchedule(int nHandler, float dt, CCNode *self) {
    
    executeFunctionWithFloatData(nHandler, dt, self);
    return 1;
}
