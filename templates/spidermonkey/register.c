JSBool ${generator.prefix}_${class_name}_constructor(JSContext *cx, uint32_t argc, jsval *vp) {
	#if len($methods) > 0
	return JS_TRUE;
	#else
	return JS_FALSE;
	#end if
}

void ${generator.prefix}_${class_name}_finalize(JSContext *cx, JSObject *obj) {
}

void register_${generator.prefix}_${class_name}(JSContext *cx, JSObject *global, const char *name) {
	${generator.prefix}_${class_name}_class = (JSClass *)calloc(1, sizeof(JSClass));
	${generator.prefix}_${class_name}_class->name = name;
	${generator.prefix}_${class_name}_class->addProperty = JS_PropertyStub;
	${generator.prefix}_${class_name}_class->delProperty = JS_PropertyStub;
	${generator.prefix}_${class_name}_class->getProperty = JS_PropertyStub;
	${generator.prefix}_${class_name}_class->setProperty = JS_StrictPropertyStub;
	${generator.prefix}_${class_name}_class->enumerate = JS_EnumerateStub;
	${generator.prefix}_${class_name}_class->resolve = JS_ResolveStub;
	${generator.prefix}_${class_name}_class->convert = JS_ConvertStub;
	${generator.prefix}_${class_name}_class->finalize = ${generator.prefix}_${class_name}_finalize;
	${generator.prefix}_${class_name}_class->flags = JSCLASS_HAS_PRIVATE;

	#if len($fields) > 0
	static JSPropertySpec properties[] = {
		{0, 0, 0, 0, 0}
	};
	#else
	JSPropertySpec *properties = NULL;
	#end if

	#set methods = $methods_clean($generator)
	#if len(methods) > 0
	static JSFunctionSpec funcs[] = {
		#for m in methods
		#set fn = m['impl'][0]
		JS_FN("${m['name']}", ${fn.signature_name}, ${fn.min_args}, JSPROP_PERMANENT | JSPROP_SHARED),
		#end for
		JS_FS_END
	};
	#else
	JSFunctionSpec *funcs = NULL;
	#end if

	#set st_methods = $static_methods_clean($generator)
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

	${generator.prefix}_${class_name}_prototype = JS_InitClass(
		cx, global,
		NULL, // parent proto
		${generator.prefix}_${class_name}_class,
		${generator.prefix}_${class_name}_constructor, 0, // constructor
		properties,
		funcs,
		NULL, // no static properties
		st_funcs);
}
