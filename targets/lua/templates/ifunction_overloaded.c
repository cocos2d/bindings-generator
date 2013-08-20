## ===== instance function implementation template - for overloaded functions
static int ${signature_name}(lua_State* tolua_S)
{
    int argc = 0;
    ${namespaced_class_name}* cobj = NULL;
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
#for func in $implementations
#if len($func.arguments) >= $func.min_args
    #set arg_count = len($func.arguments)
    #set arg_idx = $func.min_args
    #while $arg_idx <= $arg_count
    #set arg_list = ""
    #set arg_array = []
    do{
        #if $func.min_args >= 0
        if (argc == ${arg_idx}) {
            #set $count = 0
            #while $count < $arg_idx
            #set $arg = $func.arguments[$count]
            ${arg.to_string($generator)} arg${count};
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

            #if $arg_idx >= 0 
            if (!ok) { ok = true; break; }
            #end if
            #end while
            #set $arg_list = ", ".join($arg_array)
        #end if
        #if $is_constructor
            cobj = new ${namespaced_class_name}($arg_list);
#if not $generator.script_control_cpp
            if (NULL != dynamic_cast<cocos2d::Object *>(cobj) ) 
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
            #if $func.ret_type.name != "void"
                #if $func.ret_type.is_enum
            int ret = (int)cobj->${func.func_name}($arg_list);
                #else
            ${func.ret_type} ret = cobj->${func.func_name}($arg_list);
                #end if
            ${func.ret_type.from_native({"generator": $generator,
                                                      "in_value": "ret",
                                                      "out_value": "ret",
                                                      "ntype": $func.ret_type.name.replace("*", ""),
                                                      "class_name": $class_name,
                                                      "level": 2})};
            return 1;
            #else
            cobj->${func.func_name}($arg_list);
            return 0;
            #end if
        #end if
        }
    }while(0);
    #set $arg_idx = $arg_idx + 1
    #end while
#end if
#end for
    CCLOG("%s has wrong number of arguments: %d, was expecting %d \n", "${func.func_name}",argc, ${func.min_args});
    return 0;
\#if COCOS2D_DEBUG >= 1
    tolua_lerror:
    tolua_error(tolua_S,"#ferror in function '${signature_name}'.",&tolua_err);
\#endif
    return 0;
}
