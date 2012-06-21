#!/usr/bin/env python

from clang import cindex
import sys
import pdb
import ConfigParser
import yaml

def parse_headers(headers, classes, args):
	# print("will parse: " + str(headers) + " " + str(args))
	index = cindex.Index.create()
	tu = index.parse(headers, args)
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
	deep_iterate(tu.cursor, classes)

def deep_iterate(cursor, classes=[], depth=0, force=False):
	# get the canonical type
	c = cursor.canonical
	if c.kind == cindex.CursorKind.NAMESPACE or c.kind == cindex.CursorKind.CLASS_DECL:
		if force or c.displayname in classes:
			print cursor.location
			if not force:
				depth = 0
			force = True
			print("%s %s - %s" % (">" * depth, c.displayname, c.kind))
			# print("**** green lighted")
	elif force:
		print("> " * depth + str(c.displayname) + " - " + str(c.kind))

	for node in cursor.get_children():
		deep_iterate(node, classes, depth+1, force)

def main():
	from clang import cindex
	from optparse import OptionParser, OptionGroup

	global opts

	parser = OptionParser("usage: %prog [options] {configfile}")
	parser.add_option("-s", action="store", type="string", dest="section", help="sets a specific section to be converted")
	(opts, args) = parser.parse_args()

	if len(args) == 0:
		parser.error('invalid number of arguments')

	config = ConfigParser.SafeConfigParser()
	config.read(args[0])

	# only parse a specific section
	# pdb.set_trace()
	if opts.section:
		headers = config.get(opts.section, 'headers')
		classes = config.get(opts.section, 'classes')
		clang_args = (config.get(opts.section, 'extra_arguments') or "").split(" ")
		clang_args += headers.split(" ")
		parse_headers(None, classes.split(" "), clang_args)

if __name__ == '__main__':
	main()
