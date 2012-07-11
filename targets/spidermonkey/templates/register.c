#set has_constructor = False
#if $methods.has_key('constructor')
#set has_constructor = True
${methods.constructor.generate_code($generator, {"namespaced_class_name": $namespaced_class_name, "class_name": $class_name})}
#else
JSBool dummy_constructor(JSContext *cx, uint32_t argc, jsval *vp);
JSBool dummy_constructor(JSContext *cx, uint32_t argc, jsval *vp) {
	return JS_FALSE;
}
#end if

#set methods = $methods_clean($generator)
#set st_methods = $static_methods_clean($generator)

void ${generator.prefix}_${class_name}_finalize(JSContext *cx, JSObject *obj) {
}

void register_${generator.prefix}_${class_name}(JSContext *cx, JSObject *global, const char *name) {
	js_${generator.prefix}_${class_name}_class = (JSClass *)calloc(1, sizeof(JSClass));
	js_${generator.prefix}_${class_name}_class->name = name;
	js_${generator.prefix}_${class_name}_class->addProperty = JS_PropertyStub;
	js_${generator.prefix}_${class_name}_class->delProperty = JS_PropertyStub;
	js_${generator.prefix}_${class_name}_class->getProperty = JS_PropertyStub;
	js_${generator.prefix}_${class_name}_class->setProperty = JS_StrictPropertyStub;
	js_${generator.prefix}_${class_name}_class->enumerate = JS_EnumerateStub;
	js_${generator.prefix}_${class_name}_class->resolve = JS_ResolveStub;
	js_${generator.prefix}_${class_name}_class->convert = JS_ConvertStub;
	js_${generator.prefix}_${class_name}_class->finalize = ${generator.prefix}_${class_name}_finalize;
	js_${generator.prefix}_${class_name}_class->flags = JSCLASS_HAS_PRIVATE;

	#if len($fields) > 0
	static JSPropertySpec properties[] = {
		{0, 0, 0, 0, 0}
	};
	#else
	JSPropertySpec *properties = NULL;
	#end if

	#if len(methods) > 0
	static JSFunctionSpec funcs[] = {
		#for m in methods
		#set fn = m['impl']
		JS_FN("${m['name']}", ${fn.signature_name}, ${fn.min_args}, JSPROP_PERMANENT | JSPROP_SHARED),
		#end for
		JS_FS_END
	};
	#else
	JSFunctionSpec *funcs = NULL;
	#end if

	#if len(st_methods) > 0
	static JSFunctionSpec st_funcs[] = {
		#for m in st_methods
		#set fn = m['impl']
		JS_FN("${m['name']}", ${fn.signature_name}, ${fn.min_args}, JSPROP_PERMANENT | JSPROP_SHARED),
		#end for
		JS_FS_END
	};
	#else
	JSFunctionSpec *st_funcs = NULL;
	#end if

	js_${generator.prefix}_${class_name}_prototype = JS_InitClass(
		cx, global,
		NULL, // parent proto
		js_${generator.prefix}_${class_name}_class,
#if has_constructor
		js_${generator.prefix}_${class_name}_constructor, 0, // constructor
#else
		dummy_constructor, 0, // no constructor
#end if
		properties,
		funcs,
		NULL, // no static properties
		st_funcs);
}
