#!/usr/bin/env python

from clang import cindex
import sys
import pdb
import ConfigParser
import yaml
import re
from Cheetah.Template import Template
from os import path

def native_name_from_kind(ntype):

    type_map = {
        cindex.TypeKind.VOID        : "void",
        cindex.TypeKind.BOOL        : "bool",
        cindex.TypeKind.CHAR_U      : "unsigned char",
        cindex.TypeKind.UCHAR       : "unsigned char",
        cindex.TypeKind.CHAR16      : "char",
        cindex.TypeKind.CHAR32      : "char",
        cindex.TypeKind.USHORT      : "unsigned short",
        cindex.TypeKind.UINT        : "unsigned int",
        cindex.TypeKind.ULONG       : "unsigned long",
        cindex.TypeKind.ULONGLONG   : "unsigned long long",
        cindex.TypeKind.CHAR_S      : "char",
        cindex.TypeKind.SCHAR       : "char",
        cindex.TypeKind.WCHAR       : "wchar_t",
        cindex.TypeKind.SHORT       : "short",
        cindex.TypeKind.INT         : "int",
        cindex.TypeKind.LONG        : "long",
        cindex.TypeKind.LONGLONG    : "long long",
        cindex.TypeKind.FLOAT       : "float",
        cindex.TypeKind.DOUBLE      : "double",
        cindex.TypeKind.LONGDOUBLE  : "long double",
        cindex.TypeKind.NULLPTR     : "NULL",
        cindex.TypeKind.OBJCID      : "id",
        cindex.TypeKind.OBJCCLASS   : "class",
        cindex.TypeKind.OBJCSEL     : "SEL",
        cindex.TypeKind.ENUM        : "int",
    }

    kind = ntype.kind
    if kind in type_map:
        return type_map[kind]
    elif kind == cindex.TypeKind.UNEXPOSED:
        # might be an std::string
        decl = ntype.get_declaration()
        parent = decl.semantic_parent
        if decl.spelling == "string" and parent and parent.spelling == "std":
            return "std::string"
    else:
        print >> sys.stderr, "Unknown type: " + str(kind)
        return "??"
		# pdb.set_trace()

class NativeType(object):
	def __init__(self, ntype):
		self.type = ntype
		self.is_pointer = False
		if ntype.kind == cindex.TypeKind.POINTER:
			pointee = ntype.get_pointee()
			self.is_pointer = True
			if pointee.kind == cindex.TypeKind.RECORD:
				self.name = pointee.get_declaration().displayname
			else:
				self.name = native_name_from_kind(pointee)
			self.name += "*"
		else:
			self.name = native_name_from_kind(ntype)

	def from_native(self, generator, in_value, out_value, indent_level=0):
		if generator.config['conversions']['from_native'].has_key(self.name):
			tpl = generator.config['conversions']['from_native'][self.name]
			tpl = Template(tpl, searchList=[{"in_value": in_value,
											 "out_value": out_value,
											 "level": indent_level}])
			return str(tpl).rstrip()
		return "#pragma error NO CONVERSION FROM NATIVE FOR " + self.name

	def to_native(self, generator, in_value, out_value, indent_level=0):
		if generator.config['conversions']['to_native'].has_key(self.name):
			tpl = generator.config['conversions']['to_native'][self.name]
			tpl = Template(tpl, searchList=[{"in_value": in_value,
											 "out_value": out_value,
											 "level": indent_level}])
			return str(tpl).rstrip()
		return "#pragma error NO CONVERSION TO NATIVE FOR " + self.name

	def __str__(self):
		return self.name

class NativeField(object):
	def __init__(self, cursor):
		cursor = cursor.canonical
		self.cursor = cursor
		self.name = cursor.displayname
		self.kind = cursor.type.kind
		self.location = cursor.location
		member_field_re = re.compile('m_(\w+)')
		match = member_field_re.match(self.name)
		if match:
			self.pretty_name = match.group(1)
		else:
			self.pretty_name = self.name

class NativeFunction(object):
	def __init__(self, cursor):
		self.cursor = cursor
		self.func_name = cursor.spelling
		self.signature_name = self.func_name
		self.arguments = []
		self.static = cursor.kind == cindex.CursorKind.CXX_METHOD and cursor.is_method_static()
		self.implementations = []
		self.is_constructor = False
		result = cursor.result_type
		# get the result
		if result.kind == cindex.TypeKind.LVALUEREFERENCE:
			result = result.get_pointee()
		self.ret_type = NativeType(cursor.result_type)
		# parse the arguments
		for arg in cursor.type.argument_types():
			self.arguments += [NativeType(arg)]
		self.min_args = len(self.arguments)

	def generate_code(self, generator, current_class=None):
		config = generator.config
		tpl = Template(file=path.join("templates", generator.target, "function.h"),
					   searchList=[current_class, self, {"generator": generator}])
		generator.head_file.write(str(tpl))
		if self.static:
			if config['definitions'].has_key('sfunction'):
				tpl = Template(config['definitions']['sfunction'],
							   searchList=[current_class, self, {"generator": generator}])
				self.signature_name = str(tpl)
			tpl = Template(file=path.join("templates", generator.target, "sfunction.c"),
						   searchList=[current_class, self, {"generator": generator}])
		else:
			if not self.is_constructor:
				if config['definitions'].has_key('ifunction'):
					tpl = Template(config['definitions']['ifunction'],
							       searchList=[current_class, self, {"generator": generator}])
					self.signature_name = str(tpl)
			else:
				if config['definitions'].has_key('constructor'):
					tpl = Template(config['definitions']['constructor'],
								   searchList=[current_class, self, {"generator": generator}])
					self.signature_name = str(tpl)
			tpl = Template(file=path.join("templates", generator.target, "ifunction.c"),
						   searchList=[current_class, self, {"generator": generator}])
		generator.impl_file.write(str(tpl))

class NativeOverloadedFunction(object):
	def __init__(self, func_array):
		self.implementations = func_array
		self.func_name = func_array[0].func_name
		self.signature_name = self.func_name
		self.min_args = 100
		self.is_constructor = False
		for m in func_array:
			self.min_args = min(self.min_args, m.min_args)

	def append(self, func):
		self.min_args = min(self.min_args, func.min_args)
		self.implementations += [func]

	def generate_code(self, generator, current_class=None):
		config = generator.config
		static = self.implementations[0].static
		tpl = Template(file=path.join("templates", generator.target, "function.h"),
					   searchList=[current_class, self, {"generator": generator}])
		generator.head_file.write(str(tpl))
		if static:
			if config['definitions'].has_key('sfunction'):
				tpl = Template(config['definitions']['sfunction'],
							   searchList=[current_class, self, {"generator": generator}])
				self.signature_name = str(tpl)
			tpl = Template(file=path.join("templates", generator.target, "sfunction_overloaded.c"),
						   searchList=[current_class, self, {"generator": generator}])
		else:
			if not self.is_constructor:
				if config['definitions'].has_key('ifunction'):
					tpl = Template(config['definitions']['ifunction'],
							       searchList=[current_class, self, {"generator": generator}])
					self.signature_name = str(tpl)
			else:
				if config['definitions'].has_key('constructor'):
					tpl = Template(config['definitions']['constructor'],
								   searchList=[current_class, self, {"generator": generator}])
					self.signature_name = str(tpl)
			tpl = Template(file=path.join("templates", generator.target, "ifunction_overloaded.c"),
						   searchList=[current_class, self, {"generator": generator}])
		generator.impl_file.write(str(tpl))


class NativeClass(object):
	def __init__(self, cursor):
		# the cursor to the implementation
		self.cursor = cursor
		self.class_name = cursor.displayname
		self.parents = []
		self.fields = []
		self.methods = {}
		self.static_methods = {}
		self.parse()

	def parse(self):
		'''
		parse the current cursor, getting all the necesary information
		'''
		self._deep_iterate(self.cursor)

	def methods_clean(self, generator):
		'''
		clean list of methods (without the ones that should be skipped)
		'''
		ret = []
		list_of_skips = generator.skip.split(" ")
		for name, impl in self.methods.iteritems():
			should_skip = False
			for it in list_of_skips:
				if it.find("::") > 0:
					klass_name, method_name = it.split("::")
					if klass_name == self.class_name and method_name == name:
						should_skip = True
						break
			if not should_skip:
				ret += [{"name": name, "impl": impl}]
		return ret

	def static_methods_clean(self, generator):
		'''
		clean list of static methods (without the ones that should be skipped)
		'''
		ret = []
		list_of_skips = generator.skip.split(" ")
		for name, impl in self.static_methods.iteritems():
			should_skip = False
			for it in list_of_skips:
				if it.find("::") > 0:
					klass_name, method_name = it.split("::")
					if klass_name == self.class_name and method_name == name:
						should_skip = True
						break
			if not should_skip:
				ret += [{"name": name, "impl": impl}]
		return ret

	def generate_code(self, generator):
		'''
		actually generate the code. it uses the current target templates/rules in order to
		generate the right code

		@param: generator the generator
		'''
		config = generator.config
		prelude_h = Template(file=path.join("templates", generator.target, "prelude.h"),
							 searchList=[{"generator": generator}, self])
		prelude_c = Template(file=path.join("templates", generator.target, "prelude.c"),
							 searchList=[{"generator": generator}, self])
		generator.head_file.write(str(prelude_h))
		generator.impl_file.write(str(prelude_c))
		list_of_skips = generator.skip.split(" ")
		for m in self.methods_clean(generator):
			m['impl'].generate_code(generator, self)
		for m in self.static_methods_clean(generator):
			m['impl'].generate_code(generator, self)
		# generate register section
		register = Template(file=path.join("templates", generator.target, "register.c"),
							searchList=[{"generator": generator}, self])
		generator.impl_file.write(str(register))
		# FIXME: this should be in a footer.h
		generator.head_file.write("\n#endif\n")

	def _deep_iterate(self, cursor=None):
		for node in cursor.get_children():
			if self._process_node(node):
				self._deep_iterate(node)

	def _process_node(self, cursor):
		'''
		process the node, depending on the type. If returns true, then it will perform a deep
		iteration on its children. Otherwise it will continue with its siblings (if any)

		@param: cursor the cursor to analyze
		'''
		if cursor.kind == cindex.CursorKind.CXX_BASE_SPECIFIER:
			parent = cursor.get_definition()
			if parent:
				self.parents += [NativeClass(parent)]
		elif cursor.kind == cindex.CursorKind.FIELD_DECL:
			self.fields += [NativeField(cursor)]
		elif cursor.kind == cindex.CursorKind.CXX_METHOD:
			# skip if variadic
			if not cursor.type.is_function_variadic():
				m = NativeFunction(cursor)
				if m.static:
					if not self.static_methods.has_key(m.func_name):
						self.static_methods[m.func_name] = m
					else:
						previous_m = self.static_methods[m.func_name]
						if isinstance(previous_m, NativeOverloadedFunction):
							previous_m.append(m)
						else:
							self.static_methods[m.func_name] = NativeOverloadedFunction([m, previous_m])
				else:
					if not self.methods.has_key(m.func_name):
						self.methods[m.func_name] = m
					else:
						previous_m = self.methods[m.func_name]
						if isinstance(previous_m, NativeOverloadedFunction):
							previous_m.append(m)
						else:
							self.methods[m.func_name] = NativeOverloadedFunction([m, previous_m])
		elif cursor.kind == cindex.CursorKind.CONSTRUCTOR:
			m = NativeFunction(cursor)
			m.is_constructor = True
			if not self.methods.has_key('constructor'):
				self.methods['constructor'] = m
			else:
				previous_m = self.methods['constructor']
				if isinstance(previous_m, NativeOverloadedFunction):
					previous_m.append(m)
				else:
					m = NativeOverloadedFunction([m, previous_m])
					m.is_constructor = True
					self.methods['constructor'] = m
		else:
			print >> sys.stderr, "unknown cursor: %s - %s" % (cursor.kind, cursor.displayname)

class Generator(object):
	def __init__(self, opts):
		self.index = cindex.Index.create()
		self.prefix = opts['prefix']
		self.headers = opts['headers']
		self.classes = opts['classes']
		self.clang_args = opts['clang_args']
		self.target = opts['target']
		self.skip = opts['skip'] or ''
		self.impl_file = None
		self.head_file = None

	def generate_code(self):
		# must read the yaml file first
		stream = file(self.target + ".yaml", "r")
		data = yaml.load(stream)
		self.config = data
		self.impl_file = open(self.prefix + ".cpp", "w+")
		self.head_file = open(self.prefix + ".hpp", "w+")
		self._parse_headers()
		self.impl_file.close()
		self.head_file.close()

	def _parse_headers(self):
		# print("will parse: " + str(headers) + " " + str(args))
		tu = self.index.parse(self.headers, self.clang_args)
		if len(tu.diagnostics) > 0:
			is_fatal = False
			for d in tu.diagnostics:
				if d.severity >= cindex.Diagnostic.Error:
					is_fatal = True
				print(d.category_name + ": " + str(d.location))
				print("  " + d.spelling)
			if is_fatal:
				print("*** Found errors - can not continue")
				return
		self._deep_iterate(tu.cursor)

	def _deep_iterate(self, cursor, depth=0, force=False):
		# get the canonical type
		is_class = False
		if cursor.kind == cindex.CursorKind.CLASS_DECL:
			if force or (cursor == cursor.canonical and cursor.displayname in self.classes):
				nclass = NativeClass(cursor)
				nclass.generate_code(self)
				return

		for node in cursor.get_children():
			# print("%s %s - %s" % (">" * depth, node.displayname, node.kind))
			self._deep_iterate(node, depth+1, force)

def main():
	from clang import cindex
	from optparse import OptionParser, OptionGroup

	global opts

	parser = OptionParser("usage: %prog [options] {configfile}")
	parser.add_option("-s", action="store", type="string", dest="section",
					  help="sets a specific section to be converted")
	parser.add_option("-t", action="store", type="string", dest="target",
					  help="specifies the target vm. Will search for TARGET.yaml")
	(opts, args) = parser.parse_args()

	if not opts.target:
		parser.error("Target is required")

	if not opts.section:
		parser.error("Section is required")

	if len(args) == 0:
		parser.error('invalid number of arguments')

	config = ConfigParser.SafeConfigParser()
	config.read(args[0])

	# generate_code(classes.split(" "), clang_args, opts.target)
	# parse_headers(None, classes.split(" "), clang_args)
	gen_opts = {
		'prefix': config.get(opts.section, 'prefix'),
		'headers': config.get(opts.section, 'headers'),
		'classes': config.get(opts.section, 'classes').split(' '),
		'clang_args': (config.get(opts.section, 'extra_arguments') or "").split(" "),
		'target': opts.target,
		'skip': config.get(opts.section, 'skip')
	}
	generator = Generator(gen_opts)
	generator.generate_code()

if __name__ == '__main__':
	main()
