bool register_all_${prefix}(se::Object* obj)
{
#if $target_ns
    // Get the ns
    se::Value nsVal;
    if (!obj->getProperty("${target_ns}", &nsVal))
    {
        se::Object* jsobj = se::Object::createPlainObject(false);
        nsVal.setObject(jsobj);
        obj->setProperty("${target_ns}", nsVal);
        jsobj->release();
    }
    se::Object* ns = nsVal.toObject();
#else 
    // Get the global ns
    se::Object* ns = ScriptEngine::getInstance()->getGlobalObject();
#end if

#for jsclass in $sorted_classes
    #if $in_listed_classes(jsclass)
    js_register_${prefix}_${jsclass}(ns);
    #end if
#end for
    return true;
}

#if $macro_judgement
\#endif //$macro_judgement
#end if