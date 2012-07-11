\#ifndef __${generator.prefix}_h__
\#define __${generator.prefix}_h__

\#include "ScriptingCore.h"

#if len($fields) > 0
enum js_fields_${generator.prefix}_${class_name} {
#for $field in $fields
	k${field.name.capitalize()},
#end for
};
#end if

extern JSClass  *js_${generator.prefix}_${class_name}_class;
extern JSObject *js_${generator.prefix}_${class_name}_prototype;

JSBool ${generator.prefix}_${class_name}_constructor(JSContext *cx, uint32_t argc, jsval *vp);
void ${generator.prefix}_${class_name}_finalize(JSContext *cx, JSObject *obj);
void register_${generator.prefix}_${class_name}(JSContext *cx, JSObject *global, const char *name);

