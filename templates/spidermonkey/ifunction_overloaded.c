## ===== static function implementation template - for overloaded functions
JSBool ${signature_name}(JSContext *cx, uint32_t argc, jsval *vp)
{
	JSObject *obj = JS_THIS_OBJECT(cx, vp);
	jsval *argv = JS_ARGV(cx, vp);
	${class_name}* native_self = (${class_name} *)JS_GetInstancePrivate(cx, obj, &js_${project}_${class_name}_class, argv);
	#for func in $implementations
		#set arg_list = ""
		#set arg_array = []
		#if $func.min_args > 0
	if (argc == ${func.min_args}) {
		#set count = 0
		#for $arg in $func.arguments
		${arg} arg${count};
		${arg.to_native($generator, "argv[" + str(count) + "]", "arg" + str(count), 2)};
		#set $arg_array += ["arg"+str(count)]
		#set $count = $count + 1
		#end for
		#set $arg_list = ", ".join($arg_array)
		#end if
		#if str($func.ret_type) != "void"
		${func.ret_type} ret = native_self->${func.func_name}($arg_list);
		jsval jsret; ${func.ret_type.from_native($generator, "ret", "jsret", 2)};
		JS_SET_RVAL(cx, vp, jsret);
		#else
		native_self->${func.func_name}($arg_list);
		#end if
		return JS_TRUE;
	}
	#end for
	return JS_FALSE;
}
