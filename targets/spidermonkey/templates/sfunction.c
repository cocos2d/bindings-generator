## ===== static function implementation template
JSBool ${signature_name}(JSContext *cx, uint32_t argc, jsval *vp)
{
	#set arg_list = ""
	#set arg_array = []
#if $min_args > 0
	jsval *argv = JS_ARGV(cx, vp);
	#set $count = 0
	#for $arg in $arguments
	${arg.to_string($generator)} arg${count};
		#set $count = $count + 1
	#end for
	assert(argc >= ${min_args});

	#set $count = 0
	#for $arg in $arguments
	${arg.to_native({"generator": $generator,
		"in_value": "argv[" + str(count) + "]",
		"out_value": "arg" + str(count),
		"class_name": $class_name,
		"level": 2,
		"ntype": str($arg)})};
        #set $arg_array += ["arg"+str($count)]
        #set $count = $count + 1
	#end for
	#set $arg_list = ", ".join($arg_array)
#end if
#if str($ret_type) != "void"
	${ret_type} ret = ${namespaced_class_name}::${func_name}($arg_list);
	jsval jsret;
	${ret_type.from_native({"generator": $generator,
							"in_value": "ret",
							"out_value": "jsret",
							"ntype": str($ret_type),
							"level": 1})};
	JS_SET_RVAL(cx, vp, jsret);
#else
	${namespaced_class_name}::${func_name}($arg_list);
#end if
	return JS_TRUE;
}

