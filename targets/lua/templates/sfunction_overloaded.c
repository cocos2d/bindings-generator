## ===== static function implementation template - for overloaded functions
int ${signature_name}(lua_State* tolua_S)
{
    int argc = 0;
    bool ok  = true;
\#if COCOS2D_DEBUG >= 1
    tolua_Error tolua_err;
\#endif

\#if COCOS2D_DEBUG >= 1
    if (!tolua_isusertable(tolua_S,1,"$class_name",0,&tolua_err)) goto tolua_lerror;
\#endif

    argc = lua_gettop(tolua_S)-1;

    #for func in $implementations   
    #if len($func.arguments) >= $func.min_args
    #set arg_count = len($func.arguments)
    #set arg_idx = $func.min_args
    #while $arg_idx <= $arg_count
    do 
    {
        if (argc == ${arg_idx})
        {
            #set arg_list = ""
            #set arg_array = []
            #set count = 0
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
            #if str($func.ret_type) != "void"
                #if $func.ret_type.is_enum
            int ret = (int)${namespaced_class_name}::${func.func_name}($arg_list);
                #else
            ${func.ret_type} ret = ${namespaced_class_name}::${func.func_name}($arg_list);
                #end if
            ${func.ret_type.from_native({"generator": $generator,
                                         "in_value": "ret",
                                         "out_value": "jsret",
                                         "ntype": $func.ret_type.name.replace("*", ""),
                                         "class_name": $class_name,
                                         "level": 2})};
            return 1;
            #else
            ${namespaced_class_name}::${func.func_name}($arg_list);
            return 0;
            #end if
        }
        #set $arg_idx = $arg_idx + 1
    } while (0);
    #end while
    #end if
    #end for
    CCLOG("%s has wrong number of arguments: %d, was expecting %d", "${func.func_name}",argc, ${func.min_args});
    return 0;
\#if COCOS2D_DEBUG >= 1
    tolua_lerror:
    tolua_error(tolua_S,"#ferror in function '${signature_name}'.",&tolua_err);
\#endif
    return 0;
}
