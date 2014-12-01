void register_all_${prefix}(JSContext* cx, JS::HandleObject obj) {
    #if $target_ns
    // first, try to get the ns
    JS::RootedValue nsval(cx);
    JS::RootedObject ns(cx);
    JS_GetProperty(cx, obj, "${target_ns}", &nsval);
    if (nsval == JSVAL_VOID) {
        JS::RootedObject proto(cx, JSVAL_TO_OBJECT(JSVAL_NULL));
        JS::RootedObject parent(cx, JSVAL_TO_OBJECT(JSVAL_NULL));
        ns = JS_NewObject(cx, NULL, proto, parent);
        nsval = OBJECT_TO_JSVAL(ns);
        JS_SetProperty(cx, obj, "${target_ns}", nsval);
    } else {
        JS_ValueToObject(cx, nsval, &ns);
    }
    //obj = ns;
    #end if

    #for jsclass in $sorted_classes
    #if $in_listed_classes(jsclass)
    js_register_${prefix}_${jsclass}(cx, ns);
    #end if
    #end for
}

