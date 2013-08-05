## ===== static function implementation template
static int ${signature_name}(lua_State* tolua_S)
{
	int argc = 0;
	bool ok  = true;

\#if COCOS2D_DEBUG >= 1
	tolua_Error tolua_err;
\#endif

\#if COCOS2D_DEBUG >= 1
	if (!tolua_isusertable(tolua_S,1,"$class_name",0,&tolua_err)) goto tolua_lerror;
\#endif

	argc = lua_gettop(tolua_S) - 1;

#if len($arguments) >= $min_args
	#set arg_count = len($arguments)
	#set arg_idx = $min_args
	#while $arg_idx <= $arg_count
	if (argc == ${arg_idx}) {
		#set arg_list = ""
		#set arg_array = []
		#set $count = 0
		#while $count < $arg_idx
			#set $arg = $arguments[$count]
		${arg.to_string($generator)} arg${count};
			#set $count = $count + 1
		#end while
		#set $count = 0
		#while $count < $arg_idx
			#set $arg = $arguments[$count]
		${arg.to_native({"generator": $generator,
                         "in_value": "argv[" + str(count) + "]",
                         "out_value": "arg" + str(count),
                         "arg_idx": $count+2,
                         "class_name": $class_name,
                         "level": 2,
                         "arg":$arg,
                         "ntype": $arg.name.replace("*", "")})};
	        #set $arg_array += ["arg"+str($count)]
	        #set $count = $count + 1
		#end while
		#if $arg_idx >= 0
		if(!ok)
			return 0;
		#end if
		#set $arg_list = ", ".join($arg_array)
	#if $ret_type.name != "void"
		#if $ret_type.is_enum
		int ret = (int)${namespaced_class_name}::${func_name}($arg_list);
		#else
		${ret_type} ret = ${namespaced_class_name}::${func_name}($arg_list);
		#end if
		${ret_type.from_native({"generator": $generator,
                            "in_value": "ret",
                            "out_value": "ret",
                            "ntype": $ret_type.name.replace("*", ""),
                            "class_name": $class_name,
                            "level": 2})};
	    return 1;
	#else
		${namespaced_class_name}::${func_name}($arg_list);
		return 0;
	#end if
	}
		#set $arg_idx = $arg_idx + 1
	#end while
#end if
	printf("wrong number of arguments: %d, was expecting %d", argc, ${min_args});
	return 0;
\#if COCOS2D_DEBUG >= 1
	tolua_lerror:
	tolua_error(tolua_S,"#ferror in function '${signature_name}'.",&tolua_err);
\#endif
	return 0;
}

