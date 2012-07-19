## ===== instance function implementation template
JSBool ${signature_name}(JSContext *cx, uint32_t argc, jsval *vp)
{
#if $min_args > 0
	jsval *argv = JS_ARGV(cx, vp);
#end if
#if not $is_constructor
	JSObject *obj = JS_THIS_OBJECT(cx, vp);
	js_proxy_t *proxy; JS_GET_NATIVE_PROXY(proxy, obj);
	${namespaced_class_name}* cobj = (${namespaced_class_name} *)(proxy ? proxy->ptr : NULL);
	TEST_NATIVE_OBJECT(cx, cobj)
#end if


#set arg_list = ""
#set arg_array = []
#if $min_args > 0
	#set $count = 0
	#for $arg in $arguments
	${arg.to_string($generator)} arg${count};
	#set $count = $count + 1
	#end for
	if (argc == ${min_args}) {
	#set $count = 0
	#for $arg in $arguments
		${arg.to_native({"generator": $generator,
						 "in_value": "argv[" + str(count) + "]",
						 "out_value": "arg" + str(count),
						 "class_name": $class_name,
						 "level": 2,
						 "ntype": str($arg)})};
		#set $arg_array += ["arg"+str(count)]
		#set $count = $count + 1
	#end for
	} else {
		return JS_FALSE;
	}
	#set $arg_list = ", ".join($arg_array)
#end if
#if $is_constructor
	${namespaced_class_name}* cobj = new ${namespaced_class_name}($arg_list);
	js_type_class_t *typeClass;
	const char* type = cobj->getObjectType();
	HASH_FIND_STR(_js_global_type_ht, type, typeClass);
	assert(typeClass);
	JSObject *obj = JS_NewObject(cx, typeClass->jsclass, typeClass->proto, typeClass->parentProto);
\#ifdef COCOS2D_JAVASCRIPT
	JS_AddObjectRoot(cx, &obj);
\#endif
	JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj));
	// link the native object with the javascript object
	js_proxy_t *p;
	JS_NEW_PROXY(p, cobj, obj);
#else
	#if $ret_type.name != "void"
	${ret_type} ret = cobj->${func_name}($arg_list);
	jsval jsret;
	${ret_type.from_native({"generator": $generator,
							"in_value": "ret",
							"out_value": "jsret",
							"level": 0})};
	JS_SET_RVAL(cx, vp, jsret);
	#else
	cobj->${func_name}($arg_list);
	#end if
#end if
	return JS_TRUE;
}


