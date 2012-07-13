void register_all_${prefix}() {
	JSContext *cx = ScriptingCore::getInstance().getGlobalContext();
	JSObject *obj = JS_GetGlobalObject(cx);
	#if $target_ns
	JSObject *ns = JS_NewObject(cx, NULL, NULL, NULL);
	jsval nsval = OBJECT_TO_JSVAL(ns);
	JS_SetProperty(cx, obj, "${target_ns}", &nsval);
	obj = ns;
	#end if

	#for jsclass in $classes
	js_register_${prefix}_${jsclass}(cx, obj);
	#end for
}

