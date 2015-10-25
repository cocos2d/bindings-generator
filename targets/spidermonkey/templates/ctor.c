## ===== ctor function implementation template

static bool ${signature_name}(JSContext *cx, uint32_t argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject obj(cx, args.thisv().toObjectOrNull());
#if len($arguments) >= $min_args
    #set arg_count = len($arguments)
    #set arg_idx = $min_args
    #set $count = 0
    #if $arg_idx > 0
    bool ok = true;
    #end if
    #while $count < $arg_idx
        #set $arg = $arguments[$count]
        #if $arg.is_numeric
    ${arg.to_string($generator)} arg${count} = 0;
        #elif $arg.is_pointer
    ${arg.to_string($generator)} arg${count} = nullptr;
        #else
    ${arg.to_string($generator)} arg${count};
        #end if
        #set $count = $count + 1
    #end while
    #set $count = 0
    #set arg_list = ""
    #set arg_array = []
    #while $count < $arg_idx
        #set $arg = $arguments[$count]
    ${arg.to_native({"generator": $generator,
                         "in_value": "args.get(" + str(count) + ")",
                         "out_value": "arg" + str(count),
                         "class_name": $class_name,
                         "level": 2,
                         "ntype": str($arg)})};
        #set $arg_array += ["arg"+str(count)]
        #set $count = $count + 1
    #end while
    #if $arg_idx > 0
    JSB_PRECONDITION2(ok, cx, false, "js_${underlined_class_name}_ctor : Error processing arguments");
    #end if
    #set $arg_list = ", ".join($arg_array)
    ${namespaced_class_name} *nobj = new (std::nothrow) ${namespaced_class_name}($arg_list);
    #if $is_ref_class
    nobj->autorelease();
        #if $generator.script_control_cpp
    nobj->retain();
    retainCount++;
    CCLOG("++++++RETAINED++++++ %d ref count: %d", retainCount, nobj->getReferenceCount());
    JS::RootedObject hook(cx, JS_NewObject(cx, jsb_FinalizeHook_class, JS::RootedObject(cx, jsb_FinalizeHook_prototype), JS::NullPtr()));
    JS_SetProperty(cx, hook, "owner", JS::RootedValue(cx, args.thisv()));
    JS_SetProperty(cx, obj, "__hook", JS::RootedValue(cx, OBJECT_TO_JSVAL(hook)));
        #end if
    #end if
#if not $generator.script_control_cpp
    js_proxy_t* p = jsb_new_proxy(nobj, obj);
    AddNamedObjectRoot(cx, &p->obj, "${namespaced_class_name}");
#else
    jsb_new_proxy(nobj, obj);
#end if
    bool isFound = false;
    if (JS_HasProperty(cx, obj, "_ctor", &isFound) && isFound)
        ScriptingCore::getInstance()->executeFunctionWithOwner(OBJECT_TO_JSVAL(obj), "_ctor", args);
    args.rval().setUndefined();
    return true;
#end if
}