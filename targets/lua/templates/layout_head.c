\#include "scripting/lua-bindings/auto/${out_file}.hpp"
#if $macro_judgement
$macro_judgement
#end if
#for header in $headers
    #if os.path.normcase(os.path.commonprefix([header, $search_path])) == $search_path
\#include "${os.path.relpath(header, $search_path).replace(os.path.sep, '/')}"
    #else
\#include "${os.path.basename(header)}"
    #end if
#end for
\#include "scripting/lua-bindings/manual/tolua_fix.h"
\#include "scripting/lua-bindings/manual/LuaBasicConversions.h"
#if $cpp_headers
#for header in $cpp_headers
\#include "${header}"
#end for
#end if
