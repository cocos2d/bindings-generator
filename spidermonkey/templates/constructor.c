js_${project}_${class_name}_constructor(JSContext *cx, uintN argc, jsval *vp) {
	JSObject *obj = JS_NewObject(cx, ${project}_${class_name}_class, ${project}_${class_name}_prototype, ${parent});
	// create the native object here
	JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj));
	return JS_TRUE;
}
