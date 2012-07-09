## ===== static function implementation template
JSBool ${signature_name}(JSContext *cx, uint32_t argc, jsval *vp)
{
	#set arg_list = ""
	#set arg_array = []
	#if $min_args > 0
	jsval *argv = JS_ARGV(cx, vp);
	if (argc >= ${min_args}) {
		#set $count = 0
		#for $arg in $arguments
		${arg} arg${count};
		${arg.to_native($generator, "argv[" + str(count) + "]", "arg" + str(count), 2)};
		#set $arg_array += ["arg"+str($count)]
		#set $count = $count + 1
		#end for
		#set $arg_list = ", ".join($arg_array)
		#if str($ret_type) != "void"
		${ret_type} ret = ${class_name}::${func_name}($arg_list);
		jsval jsret;
		${ret_type.from_native($generator, "ret", "jsret", 2)};
		JS_SET_RVAL(cx, vp, jsret);
		#else
		${class_name}::${func_name}($arg_list);
		#end if
	}
	#else
	#if str($ret_type) != "void"
	${ret_type} ret = ${class_name}::${func_name}($arg_list);
	jsval jsret;
	${ret_type.from_native($generator, "ret", "jsret", 2)};
	JS_SET_RVAL(cx, vp, jsret);
	#else
	${class_name}::${func_name}($arg_list);
	#end if
	#end if
	return JS_TRUE;
}
