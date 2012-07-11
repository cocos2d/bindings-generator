\#include "ScriptingCore.h"
\#include "jstypedarray.h"
\#include "${generator.prefix}.hpp"
#for header in $generator.headers
\#include "${os.path.basename(header)}"
#end for

JSClass  *js_${generator.prefix}_${class_name}_class;
JSObject *js_${generator.prefix}_${class_name}_prototype;

