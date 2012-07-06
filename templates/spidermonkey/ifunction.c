## ===== instance function implementation template
JSBool ${signature_name}(JSContext *cx, uint32_t argc, jsval *vp)
{
	JSObject *obj = JS_THIS_OBJECT(cx, vp);
	jsval *argv = JS_ARGV(cx, vp);
	${class_name}* native_self = (${class_name} *)JS_GetInstancePrivate(cx, obj, &js_${project}_${class_name}_class, argv);
	#set arg_list = ""
	#set arg_array = []
	#if $min_args > 0
	if (argc >= ${min_args}) {
		#set $count = 0
		#for $arg in $arguments
		${arg} arg${count};
		${arg.to_native("argv[" + str(count) + "]", "arg" + str(count))};
		#set $arg_array += ["arg"+str($count)]
		#set $count = $count + 1
		#end for
	}
	#set $arg_list = ", ".join($arg_array)
	#end if
	#if $ret_type.name != "void"
	${ret_type} ret = native_self->${func_name}($arg_list);
	JS_SET_RVAL(cx, vp, ${ret_type.from_native("ret")});
	#else
	native_self->${func_name}($arg_list);
	#end if
	return JS_TRUE;
}
