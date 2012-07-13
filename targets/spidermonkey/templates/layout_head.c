\#include "jsapi.h"
\#include "jstypedarray.h"
\#include "${prefix}.hpp"
#for header in $headers
\#include "${os.path.basename(header)}"
#end for

JSBool dummy_constructor(JSContext *cx, uint32_t argc, jsval *vp);
JSBool dummy_constructor(JSContext *cx, uint32_t argc, jsval *vp) {
	return JS_FALSE;
}

