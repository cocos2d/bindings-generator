#set has_constructor = False
#set generator = $current_class.generator
#set methods = $current_class.methods_clean()
#set st_methods = $current_class.static_methods_clean()
#set public_fields = $current_class.public_fields
#if $current_class.methods.has_key('constructor')
#set has_constructor = True
#set constructor = $current_class.methods.constructor
${current_class.methods.constructor.generate_code($current_class)}
#end if

#if $generator.in_listed_extend_classed($current_class.class_name) and $has_constructor
#if not $constructor.is_overloaded
    ${constructor.generate_code($current_class, None, False, True)}
#else
    ${constructor.generate_code($current_class, False, True)}
#end if
#end if

#if len($current_class.parents) > 0
extern se::Object* __jsb_${current_class.parents[0].underlined_class_name}_proto;
#end if

#if $has_constructor
bool js_${current_class.underlined_class_name}_finalize(se::State& s)
{
    if (s.nativeThisObject() != nullptr)
    {
        cocos2d::log("jsbindings: finalizing JS object %p (${current_class.namespaced_class_name})", s.nativeThisObject());
        ${current_class.namespaced_class_name}* cobj = (${current_class.namespaced_class_name}*)s.nativeThisObject();
        #if $current_class.is_ref_class
        if (cobj->getReferenceCount() == 1)
            cobj->autorelease();
        else
            cobj->release();
        #else
        delete cobj;
        #end if
    }
    return true;
}
SE_BIND_FINALIZE_FUNC(js_${current_class.underlined_class_name}_finalize)
#end if

bool js_register_${generator.prefix}_${current_class.class_name}(se::Object* obj)
{
#if has_constructor
    #if len($current_class.parents) > 0
    auto cls = se::Class::create("${current_class.target_class_name}", obj, __jsb_${current_class.parents[0].underlined_class_name}_proto, _SE(js_${generator.prefix}_${current_class.class_name}_constructor));
    #else
    auto cls = se::Class::create("${current_class.target_class_name}", obj, nullptr, _SE(js_${generator.prefix}_${current_class.class_name}_constructor));
    #end if
#else
    #if len($current_class.parents) > 0
    auto cls = se::Class::create("${current_class.target_class_name}", obj, __jsb_${current_class.parents[0].underlined_class_name}_proto, nullptr);
    #else
    auto cls = se::Class::create("${current_class.target_class_name}", obj, nullptr, nullptr);
    #end if
#end if

#for m in public_fields
    #if $generator.should_bind_field($current_class.class_name, m.name)
    cls->defineProperty("${m.name}", _SE(${m.signature_name}_get_${m.name}), _SE(${m.signature_name}_set_${m.name}));
    #end if
#end for
#for m in methods
    #set fn = m['impl']
    cls->defineFunction("${m['name']}", _SE(${fn.signature_name}));
#end for
#if $generator.in_listed_extend_classed($current_class.class_name) and $has_constructor
    cls->defineFunction("ctor", _SE(js_${generator.prefix}_${current_class.class_name}_ctor));
#end if
#if len(st_methods) > 0
    #for m in st_methods
    #set fn = m['impl']
    cls->defineStaticFunction("${m['name']}", _SE(${fn.signature_name}));
    #end for
#end if
#if $has_constructor
    cls->defineFinalizedFunction(_SE(js_${current_class.underlined_class_name}_finalize));
#end if
    cls->install();
    JSBClassType::registerClass<${current_class.namespaced_class_name}>(cls);

    __jsb_${current_class.underlined_class_name}_proto = cls->getProto();
    __jsb_${current_class.underlined_class_name}_class = cls;

#if $generator.in_listed_extend_classed($current_class.class_name) and not $current_class.is_abstract
    se::ScriptEngine::getInstance()->executeScriptBuffer("(function () { ${generator.target_ns}.${current_class.target_class_name}.extend = cc.Class.extend; })()");
#end if
    se::ScriptEngine::getInstance()->clearException();
    return true;
}

