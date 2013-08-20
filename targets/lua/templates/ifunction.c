## ===== instance function implementation template
static int ${signature_name}(lua_State* tolua_S)
{
    int argc = 0;
    ${namespaced_class_name}* cobj = nullptr;
    bool ok  = true;

\#if COCOS2D_DEBUG >= 1
    tolua_Error tolua_err;
\#endif

#if not $is_constructor
\#if COCOS2D_DEBUG >= 1
    if (!tolua_isusertype(tolua_S,1,"$class_name",0,&tolua_err)) goto tolua_lerror;
\#endif

    cobj = (${namespaced_class_name}*)tolua_tousertype(tolua_S,1,0);

\#if COCOS2D_DEBUG >= 1
    if (!cobj) 
    {
        tolua_error(tolua_S,"invalid 'cobj' in function '${signature_name}'", NULL);
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
                                      "level": 2,
                                      "arg":$arg,
                                      "ntype": $arg.name.replace("*", "")})};
            #set $arg_array += ["arg"+str(count)]
            #set $count = $count + 1
        #end while
        #if $arg_idx >= 0
        if(!ok)
            return 0;
        #end if
        #set $arg_list = ", ".join($arg_array)
        #if $is_constructor
        cobj = new ${namespaced_class_name}($arg_list);
#if not $generator.script_control_cpp
        if (nullptr != dynamic_cast<cocos2d::Object *>(cobj)) 
        {
            cobj->autorelease();
            int ID = (cobj) ? (int)cobj->_ID : -1;
            int* luaID = (cobj) ? &cobj->_luaID : NULL;
            toluafix_pushusertype_ccobject(tolua_S, ID, luaID, (void*)cobj,"$class_name");
        }
        else
        {
            tolua_pushusertype(tolua_S,(void*)cobj,"$class_name");
            tolua_register_gc(tolua_S,lua_gettop(tolua_S));
        }
#else
        tolua_pushusertype(tolua_S,(void*)cobj,"$class_name");
        tolua_register_gc(tolua_S,lua_gettop(tolua_S));
#end if
        return 1;
        #else
            #if $ret_type.name != "void"
                #if $ret_type.is_enum
        int ret = (int)cobj->${func_name}($arg_list);
                #else
        ${ret_type} ret = cobj->${func_name}($arg_list);
                #end if
        ${ret_type.from_native({"generator": $generator,
                                "in_value": "ret",
                                "out_value": "ret",
                                "ntype": $ret_type.name.replace("*", ""),
                                "class_name": $class_name,
                                "level": 2})};
        return 1;
                #else
        cobj->${func_name}($arg_list);
        return 0;
                #end if
        #end if         
    }
        #set $arg_idx = $arg_idx + 1
    #end while
#end if
    CCLOG("%s has wrong number of arguments: %d, was expecting %d \n", "${func_name}",argc, ${min_args});
    return 0;
\#if COCOS2D_DEBUG >= 1
    tolua_lerror:
    tolua_error(tolua_S,"#ferror in function '${signature_name}'.",&tolua_err);
\#endif
    return 0;
}
