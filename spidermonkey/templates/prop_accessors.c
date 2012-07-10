JSBool js_${project}_${class_name}_property_get(JSContext *cx, JSObject *obj, jsid _id, jsval *val) {
	if (JSID_IS_INT(_id)) {
		int32_t propId = JSID_TO_INT(_id);
		${class_name}* cobj;
		PROXY_GET_NATIVE(${class_name}, obj);
	}
}
