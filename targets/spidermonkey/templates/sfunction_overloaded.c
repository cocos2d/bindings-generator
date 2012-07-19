## ===== static function implementation template - for overloaded functions
JSBool ${signature_name}(JSContext *cx, uint32_t argc, jsval *vp)
{
	jsval *argv = JS_ARGV(cx, vp);
	#for func in $implementations
		#set arg_list = ""
		#set arg_array = []
		#if $func.min_args >= 0
	if (argc == ${func.min_args}) {
		#set count = 0
		#for $arg in $func.arguments
		${arg.to_string($generator)} arg${count};
		${arg.to_native({"generator": $generator,
						 "in_value": "argv[" + str(count) + "]",
						 "out_value": "arg" + str(count),
						 "class_name": $class_name,
						 "level": 2,
						 "ntype": str($arg)})};
		#set $arg_array += ["arg"+str(count)]
		#set $count = $count + 1
		#end for
		#set $arg_list = ", ".join($arg_array)
		#end if
		#if str($func.ret_type) != "void"
		${func.ret_type} ret = ${namespaced_class_name}::${func.func_name}($arg_list);
		jsval jsret;
		${func.ret_type.from_native({"generator": $generator,
									 "in_value": "ret",
									 "out_value": "jsret",
									 "ntype": str($func.ret_type),
									 "level": 2})};
		JS_SET_RVAL(cx, vp, jsret);
		#else
		${namespaced_class_name}::${func.func_name}($arg_list);
		#end if
		return JS_TRUE;
	}
	#end for
	return JS_FALSE;
}
