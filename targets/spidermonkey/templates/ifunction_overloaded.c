## ===== static function implementation template - for overloaded functions
JSBool ${signature_name}(JSContext *cx, uint32_t argc, jsval *vp)
{
	jsval *argv = JS_ARGV(cx, vp);
	JSObject *obj;
	${namespaced_class_name}* cobj;
#if not $is_constructor
	obj = JS_THIS_OBJECT(cx, vp);
	js_proxy_t *proxy; JS_GET_NATIVE_PROXY(proxy, obj);
	cobj = (${namespaced_class_name} *)(proxy ? proxy->ptr : NULL);
	TEST_NATIVE_OBJECT(cx, cobj)
#end if

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
	#if $is_constructor
		cobj = new ${namespaced_class_name}(${arg_list});
\#ifdef COCOS2D_JAVASCRIPT
		cocos2d::CCObject *_ccobj = dynamic_cast<cocos2d::CCObject *>(cobj);
		if (_ccobj) {
			_ccobj->autorelease();
		}
\#endif
		TypeTest<${namespaced_class_name}> t;
		js_type_class_t *typeClass;
		uint32_t typeId = t.s_id();
		HASH_FIND_INT(_js_global_type_ht, &typeId, typeClass);
		assert(typeClass);
		obj = JS_NewObject(cx, typeClass->jsclass, typeClass->proto, typeClass->parentProto);
		js_proxy_t *proxy;
		JS_NEW_PROXY(proxy, cobj, obj);
\#ifdef COCOS2D_JAVASCRIPT
		JS_AddNamedObjectRoot(cx, &proxy->obj, "${namespaced_class_name}");
\#endif
	#else
		#if str($func.ret_type) != "void"
		${func.ret_type} ret = cobj->${func.func_name}($arg_list);
		jsval jsret; ${func.ret_type.from_native({"generator": $generator,
												  "in_value": "ret",
												  "out_value": "jsret",
												  "ntype": str($func.ret_type),
												  "level": 2})};
		JS_SET_RVAL(cx, vp, jsret);
		#else
		cobj->${func.func_name}($arg_list);
		#end if
		return JS_TRUE;
	#end if
	}
#end for
#if $is_constructor
	if (cobj) {
		JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj));
		return JS_TRUE;
	}
#end if
	return JS_FALSE;
}
