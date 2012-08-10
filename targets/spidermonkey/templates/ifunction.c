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
	if (argc == ${min_args}) {
#set $count = 0
#for $arg in $arguments
		${arg.to_string($generator)} arg${count};
#set $count = $count + 1
#end for
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
#set $arg_list = ", ".join($arg_array)
#if $is_constructor
		${namespaced_class_name}* cobj = new ${namespaced_class_name}($arg_list);
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
		JSObject *obj = JS_NewObject(cx, typeClass->jsclass, typeClass->proto, typeClass->parentProto);
		JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj));
		// link the native object with the javascript object
		js_proxy_t *p;
		JS_NEW_PROXY(p, cobj, obj);
\#ifdef COCOS2D_JAVASCRIPT
		JS_AddNamedObjectRoot(cx, &p->obj, "${namespaced_class_name}");
\#endif
#else
	#if $ret_type.name != "void"
		${ret_type} ret = cobj->${func_name}($arg_list);
		jsval jsret;
		${ret_type.from_native({"generator": $generator,
								"in_value": "ret",
								"out_value": "jsret",
								"ntype": str($ret_type),
								"level": 2})};
		JS_SET_RVAL(cx, vp, jsret);
	#else
		cobj->${func_name}($arg_list);
	#end if
#end if
		return JS_TRUE;
	}
	JS_ReportError(cx, "wrong number of arguments: %d, was expecting %d", argc, ${min_args});
	return JS_FALSE;
}
