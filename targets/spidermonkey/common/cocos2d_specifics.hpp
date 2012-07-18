#ifndef __JS_COCOS2D_X_SPECIFICS_H__
#define __JS_COCOS2D_X_SPECIFICS_H__

#include "jsapi.h"
#include "ScriptingCore.h"

JSBool js_cocos2dx_CCMenuItem_create(JSContext *cx, uint32_t argc, jsval *vp);
JSBool js_cocos2dx_CCMenuItemSprite_create(JSContext *cx, uint32_t argc, jsval *vp);
JSBool js_cocos2dx_CCMenuItemImage_create(JSContext *cx, uint32_t argc, jsval *vp);
JSBool js_cocos2dx_CCMenuItemLabel_create(JSContext *cx, uint32_t argc, jsval *vp);
JSBool js_cocos2dx_CCMenuItemAtlasFont_create(JSContext *cx, uint32_t argc, jsval *vp);
JSBool js_cocos2dx_CCMenuItemFont_create(JSContext *cx, uint32_t argc, jsval *vp);
JSBool js_cocos2dx_CCMenuItemToggle_create(JSContext *cx, uint32_t argc, jsval *vp);

#endif
