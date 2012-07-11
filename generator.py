#!/usr/bin/env python

from clang import cindex
import sys
import pdb
import ConfigParser
import yaml
import re
from Cheetah.Template import Template
import os
from os import path

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

def native_name_from_kind(ntype):
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

def build_namespace(cursor, name = []):
	'''
	build the full namespace for a specific cursor
	'''
	if cursor:
		parent = cursor.semantic_parent
		if parent and parent.kind == cindex.CursorKind.NAMESPACE:
			name += [parent.displayname]
			build_namespace(parent, name)
	return "::".join(name)

class NativeType(object):
	def __init__(self, ntype):
		self.type = ntype
		self.is_pointer = False
		if ntype.kind == cindex.TypeKind.POINTER:
			pointee = ntype.get_pointee()
			self.is_pointer = True
			if pointee.kind == cindex.TypeKind.RECORD:
				decl = pointee.get_declaration()
				ns = build_namespace(decl)
				if len(ns) > 0:
					self.name = ns + "::" + decl.displayname
				else:
					self.name = decl.displayname
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
		tpl = Template(file=path.join(generator.target, "templates", "function.h"),
						searchList=[current_class, self, {"generator": generator}])
		generator.head_file.write(str(tpl))
		if self.static:
			if config['definitions'].has_key('sfunction'):
				tpl = Template(config['definitions']['sfunction'],
									 searchList=[current_class, self, {"generator": generator}])
				self.signature_name = str(tpl)
			tpl = Template(file=path.join(generator.target, "templates", "sfunction.c"),
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
			tpl = Template(file=path.join(generator.target, "templates", "ifunction.c"),
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
		tpl = Template(file=path.join(generator.target, "templates", "function.h"),
						searchList=[current_class, self, {"generator": generator}])
		generator.head_file.write(str(tpl))
		if static:
			if config['definitions'].has_key('sfunction'):
				tpl = Template(config['definitions']['sfunction'],
								searchList=[current_class, self, {"generator": generator}])
				self.signature_name = str(tpl)
			tpl = Template(file=path.join(generator.target, "templates", "sfunction_overloaded.c"),
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
			tpl = Template(file=path.join(generator.target, "templates", "ifunction_overloaded.c"),
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
		prelude_h = Template(file=path.join(generator.target, "templates", "prelude.h"),
							 searchList=[{"generator": generator}, self])
		prelude_c = Template(file=path.join(generator.target, "templates", "prelude.c"),
							 searchList=[{"generator": generator}, self])
		generator.head_file.write(str(prelude_h))
		generator.impl_file.write(str(prelude_c))
		list_of_skips = generator.skip.split(" ")
		for m in self.methods_clean(generator):
			m['impl'].generate_code(generator, self)
		for m in self.static_methods_clean(generator):
			m['impl'].generate_code(generator, self)
		# generate register section
		register = Template(file=path.join(generator.target, "templates", "register.c"),
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
		self.outdir = opts['outdir']
		self.prefix = opts['prefix']
		self.headers = opts['headers']
		self.classes = opts['classes']
		self.clang_args = opts['clang_args']
		self.target = "targets/" + opts['target']
		self.skip = opts['skip'] or ''
		self.impl_file = None
		self.head_file = None

	def generate_code(self):
		# must read the yaml file first
		stream = file(path.join(self.target, "conversions.yaml"), "r")
		data = yaml.load(stream)
		self.config = data
		implfilepath = os.path.join(self.outdir, self.prefix + ".cpp")
		headfilepath = os.path.join(self.outdir, self.prefix + ".hpp")
		self.impl_file = open(implfilepath, "w+")
		self.head_file = open(headfilepath, "w+")
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
	from optparse import OptionParser, OptionGroup

	parser = OptionParser("usage: %prog [options] {configfile}")
	parser.add_option("-s", action="store", type="string", dest="section",
						help="sets a specific section to be converted")
	parser.add_option("-t", action="store", type="string", dest="target",
						help="specifies the target vm. Will search for TARGET.yaml")
	parser.add_option("-o", action="store", type="string", dest="outdir",
						help="specifies the target vm. Will search for TARGET.yaml")

	(opts, args) = parser.parse_args()

	if len(args) == 0:
		parser.error('invalid number of arguments')

	config = ConfigParser.SafeConfigParser()
	config.read(args[0])

	if (0 == len(config.sections())):
		raise Exception("No sections defined in config file")

	sections = []
	if opts.section:
		if (opts.section in config.sections()):
			sections = []
			sections.append(opts.section)
		else:
			raise Exception("Section not found in config file")
	else:
		print("processing all sections")
		sections = config.sections()

	# find available targets
	targets = []
	if (os.path.isdir("targets")):
		targets = [entry for entry in os.listdir("targets")
				   if (os.path.isdir(os.path.join("targets", entry)))]
		if 0 == len(targets):
			raise Exception("No targets defined")

	if opts.target:
		if (opts.target in targets):
			targets = []
			targets.append(opts.target)

	if opts.outdir:
		outdir = opts.outdir
	else:
		outdir = "gen"
	if not os.path.exists(outdir):
		os.makedirs(outdir)

	for t in targets:
		print "\n.... Generating bindings for target", t
		for s in sections:
			print "\n.... .... Processing section", s, "\n"
			gen_opts = {
				'prefix': config.get(s, 'prefix'),
				'headers': config.get(s, 'headers'),
				'classes': config.get(s, 'classes').split(' '),
				'clang_args': (config.get(s, 'extra_arguments') or "").split(" "),
				'target': t,
				'outdir': outdir,
				'skip': config.get(s, 'skip')
				}
			generator = Generator(gen_opts)
			generator.generate_code()

if __name__ == '__main__':
	main()
