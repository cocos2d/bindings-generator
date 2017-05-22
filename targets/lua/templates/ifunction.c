## ===== instance function implementation template
int ${signature_name}(lua_State* tolua_S)
{
    int argc = 0;
    ${namespaced_class_name}* cobj = nullptr;
    bool ok  = true;

\#if COCOS2D_DEBUG >= 1
    tolua_Error tolua_err;
\#endif


#if not $is_constructor
\#if COCOS2D_DEBUG >= 1
    if (!tolua_isusertype(tolua_S,1,"${generator.scriptname_from_native($namespaced_class_name, $namespace_name)}",0,&tolua_err)) goto tolua_lerror;
\#endif

    cobj = (${namespaced_class_name}*)tolua_tousertype(tolua_S,1,0);

\#if COCOS2D_DEBUG >= 1
    if (!cobj) 
    {
        tolua_error(tolua_S,"invalid 'cobj' in function '${signature_name}'", nullptr);
        return 0;
    }
\#endif
#end if

    argc = lua_gettop(tolua_S)-1;
#if len($arguments) >= $min_args
    #set arg_count = len($arguments)
    #set arg_idx = $min_args
    #while $arg_idx <= $arg_count
    if (argc == ${arg_idx}) 
    {
        #set $count = 0
        #while $count < $arg_idx
            #set $arg = $arguments[$count]
        ${arg.to_string($generator)} arg${count};
            #set $count = $count + 1
        #end while
        #set $count = 0
        #set arg_list = ""
        #set arg_array = []
        #while $count < $arg_idx
            #set $arg = $arguments[$count]

        ${arg.to_native({"generator": $generator,
                                      "in_value": "argv[" + str(count) + "]",
                                      "out_value": "arg" + str(count),
                                      "arg_idx": $count+2,
                                      "class_name": $class_name,
                                      "lua_namespaced_class_name": $generator.scriptname_from_native($namespaced_class_name, $namespace_name),
                                      "func_name": $func_name,
                                      "level": 2,
                                      "arg":$arg,
                                      "ntype": $arg.namespaced_name.replace("*", ""),
                                      "scriptname": $generator.scriptname_from_native($arg.namespaced_name, $arg.namespace_name)})};
            #set arg_type = arg.to_string($generator)
            #if arg.is_pointer or arg.is_numeric or arg.is_enum or arg_type == "bool"
                #set $arg_array += ["arg"+str(count)]
            #else
                #set $arg_array += ["std::move(arg"+str(count)+")"]
            #end if
            #set $count = $count + 1
        #end while
        #if $arg_idx >= 0
        if(!ok)
        {
            tolua_error(tolua_S,"invalid arguments in function '${signature_name}'", nullptr);
            return 0;
        }
        #end if
        #set $arg_list = ", ".join($arg_array)
        #if $is_constructor
        cobj = new ${namespaced_class_name}($arg_list);
#if not $generator.script_control_cpp
    #if $is_ref_class
        cobj->autorelease();
        int ID =  (int)cobj->_ID ;
        int* luaID =  &cobj->_luaID ;
        toluafix_pushusertype_ccobject(tolua_S, ID, luaID, (void*)cobj,"${generator.scriptname_from_native($namespaced_class_name, $namespace_name)}");
    #else
        tolua_pushusertype(tolua_S,(void*)cobj,"${generator.scriptname_from_native($namespaced_class_name, $namespace_name)}");
        tolua_register_gc(tolua_S,lua_gettop(tolua_S));
    #end if
#else
        tolua_pushusertype(tolua_S,(void*)cobj,"${generator.scriptname_from_native($namespaced_class_name, $namespace_name)}");
        tolua_register_gc(tolua_S,lua_gettop(tolua_S));
#end if
        return 1;
        #else
            #if $ret_type.name != "void"
                #if $ret_type.is_enum
        int ret = (int)cobj->${func_name}($arg_list);
                #else
        ${ret_type.get_whole_name($generator)} ret = cobj->${func_name}($arg_list);
                #end if
        ${ret_type.from_native({"generator": $generator,
                                "in_value": "ret",
                                "out_value": "ret",
                                "type_name": $ret_type.namespaced_name.replace("*", ""),
                                "ntype": $ret_type.get_whole_name($generator),
                                "class_name": $class_name,
                                "level": 2,
                                "scriptname": $generator.scriptname_from_native($ret_type.namespaced_name, $ret_type.namespace_name)})};
        return 1;
                #else
        cobj->${func_name}($arg_list);
        lua_settop(tolua_S, 1);
        return 1;
                #end if
        #end if         
    }
        #set $arg_idx = $arg_idx + 1
    #end while
#end if
    luaL_error(tolua_S, "%s has wrong number of arguments: %d, was expecting %d \n", "${generator.scriptname_from_native($namespaced_class_name, $namespace_name)}:${func_name}",argc, ${min_args});
    return 0;

\#if COCOS2D_DEBUG >= 1
#if not $is_constructor
    tolua_lerror:
#end if
    tolua_error(tolua_S,"#ferror in function '${signature_name}'.",&tolua_err);
\#endif

    return 0;
}
