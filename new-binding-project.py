#!/usr/bin/python
# -*- coding: utf-8 -*-
import os
import stat
import shutil
import errno


class bcolors:

    HEADER = '\033[1m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'

    def disable(self):
        self.HEADER = ''
        self.OKBLUE = ''
        self.OKGREEN = ''
        self.WARNING = ''
        self.FAIL = ''
        self.ENDC = ''


print bcolors.HEADER + 'Welcome to the generator for JS Bindings!' \
    + bcolors.ENDC
print 'Please answer the following questions to get started'

# this is the path to the original js bindings dir

bindings_base_dir = os.getcwd()


def step(number):
    print bcolors.HEADER + '\nStep ' + str(number) + bcolors.ENDC


def default_raw_input(msg, default):
    return raw_input(bcolors.OKGREEN + msg + bcolors.ENDC + ' '
                     + bcolors.WARNING + '[' + default + ']'
                     + bcolors.ENDC + ': ') or default


def copy_and_overwrite(from_path, to_path):
    if os.path.exists(to_path):
        shutil.rmtree(to_path)
    shutil.copytree(from_path, to_path)


def make_sure_path_exists(path):
    try:
        os.makedirs(path)
    except OSError, exception:
        if exception.errno != errno.EEXIST:
            raise


def write_new_file(path, text):
    target = open(path, 'w')
    target.truncate()

    target.write(text)

    target.close()


step(1)

name = default_raw_input('What is the name for your project?',
                         'my_bindings_project')

step(2)

print 'Please specify a output directory - if the directory doesnt exists we create it'
output_dir_name = name + '-bindings'
output_dir = default_raw_input('Output directory',
                               os.path.join(os.getcwd(),
                               output_dir_name))

step(3)

print 'Please specify a class name'
class_name = default_raw_input('Class name', name.title() + 'Bindings')

print 'Copying base binding project to output directory'
copy_and_overwrite(bindings_base_dir, output_dir)

print "Creating empty project '" + name + "'"
make_sure_path_exists(os.path.join(output_dir, name))

print 'Creating required files'

test_ini = \
    """
[testandroid]
name = {name}
prefix = autogentestbindings
classes = {class_name}

android_headers = -I%(androidndkdir)s/platforms/android-14/arch-arm/usr/include -I%(androidndkdir)s/sources/cxx-stl/gnu-libstdc++/4.7/libs/armeabi-v7a/include -I%(androidndkdir)s/sources/cxx-stl/gnu-libstdc++/4.7/include
android_flags = -D_SIZE_T_DEFINED_

clang_headers = -I%(clangllvmdir)s/lib/clang/3.3/include
clang_flags = -nostdinc -x c++ -std=c++11

simple_test_headers = -I%(cxxgeneratordir)s/{name}

extra_arguments = %(android_headers)s %(clang_headers)s %(android_flags)s %(clang_flags)s %(simple_test_headers)s %(extra_flags)s

headers = %(cxxgeneratordir)s/{name}/classes/{class_name}.h

target_namespace =
remove_prefix =
skip = 
base_objects =
abstract_classes =
classes_have_type_info = no
rename =
rename_functions =
rename_classes =
# classes for which there will be no "parent" lookup
classes_have_no_parents =

# base classes which will be skipped when their sub-classes found them.
base_classes_to_skip =

# Determining whether to use script object(js object) to control the lifecycle of native(cpp) object or the other way around. Supported values are 'yes' or 'no'.
script_control_cpp = yes
""".format(**{'name': name,
        'output_dir_name': output_dir_name, 'class_name': class_name})

write_new_file(os.path.join(output_dir, name, name + '.ini'), test_ini)

test_sh = \
    """
#!/bin/bash
# 
# Usage:
#   export NDK_ROOT=/path/to/NDK-r9b
#   ./test.sh

# exit this script if any commmand fails
set -e

# read user.cfg if it exists and is readable

_CFG_FILE=$(dirname "$0")"/user.cfg"
if [ -f "$_CFG_FILE" ];
then
    if [ ! -r "$_CFG_FILE" ]; then
       echo "Fatal Error: $_CFG_FILE exists but is unreadable"
       exit 1
    fi
fi

# paths

if [ -z "${{NDK_ROOT+aaa}}" ]; then
# ... if NDK_ROOT is not set, use "$HOME/bin/android-ndk"
    NDK_ROOT="$HOME/bin/android-ndk"
fi

if [ -z "${{PYTHON_BIN+aaa}}" ]; then
# ... if PYTHON_BIN is not set, use "/usr/bin/python2.7"
    PYTHON_BIN="/usr/bin/python2.7"
fi

# find current dir
DIR="$( cd "$( dirname "${{BASH_SOURCE[0]}}" )" && pwd )"

# paths with defaults hardcoded to relative paths

if [ -z "${{CXX_GENERATOR_ROOT+aaa}}" ]; then
    CXX_GENERATOR_ROOT="$DIR/.."
fi


echo "Paths"
echo "    NDK_ROOT: $NDK_ROOT"
echo "    PYTHON_BIN: $PYTHON_BIN"
echo "    CXX_GENERATOR_ROOT: $CXX_GENERATOR_ROOT"
echo "    TO_JS_ROOT: $TO_JS_ROOT"

# check NDK version, must be r9b
if ! grep -q r9b $NDK_ROOT/RELEASE.TXT
then
    echo " Fatal Error: NDK r9b must be required!"
    exit 1
fi

# check clang include path
OS_NAME=$('uname')
NDK_LLVM_ROOT=$NDK_ROOT/toolchains/llvm-3.3/prebuilt
case "$OS_NAME" in
    Darwin | darwin)
        echo "in darwin"
        if [ -d "$NDK_LLVM_ROOT/darwin-x86_64" ]; then
            NDK_LLVM_ROOT=$NDK_LLVM_ROOT/darwin-x86_64
        elif [ -d "$NDK_LLVM_ROOT/darwin-x86" ]; then
            NDK_LLVM_ROOT=$NDK_LLVM_ROOT/darwin-x86
        else
            echo $NDK_LLVM_ROOT
            echo " Fatal Error: $NDK_LLVM_ROOT doesn't contains prebuilt llvm 3.3"
            exit 1
        fi
        ;;
    Linux | linux)
        echo "in linux"
        if [ -d "$NDK_LLVM_ROOT/linux-x86_64" ]; then
            NDK_LLVM_ROOT=$NDK_LLVM_ROOT/linux-x86_64
        elif [ -d "$NDK_LLVM_ROOT/linux-x86" ]; then
            NDK_LLVM_ROOT=$NDK_LLVM_ROOT/linux-x86
        else
            echo " Fatal Error: $NDK_LLVM_ROOT doesn't contains prebuilt llvm 3.3"
            exit 1
        fi
        ;;
    *)
        echo " Fatal Error: Please run this script in linux or mac osx."
        exit 1
        ;;
esac


# write userconf.ini

_CONF_INI_FILE="$PWD/userconf.ini"
if [ -f "$_CONF_INI_FILE" ]
then
    rm "$_CONF_INI_FILE"
fi

_CONTENTS=""
_CONTENTS+="[DEFAULT]"'
'
_CONTENTS+="androidndkdir=$NDK_ROOT"'
'
_CONTENTS+="clangllvmdir=$NDK_LLVM_ROOT"'
'
_CONTENTS+="cxxgeneratordir=$CXX_GENERATOR_ROOT"'
'
_CONTENTS+="extra_flags="'
'

echo 
echo "generating userconf.ini..."
echo ---
echo -e "$_CONTENTS"
echo -e "$_CONTENTS" > "$_CONF_INI_FILE"
echo ---

# Generate bindings for cocos2dx
echo "Generating bindings for cocos2dx..."
set -x

LD_LIBRARY_PATH=${{CXX_GENERATOR_ROOT}}/libclang $PYTHON_BIN ${{CXX_GENERATOR_ROOT}}/generator.py ${{CXX_GENERATOR_ROOT}}/{name}/{name}.ini -t spidermonkey -s testandroid -o ./{name}_bindings
""".format(**{'name': name,
        'output_dir_name': output_dir_name, 'class_name': class_name})

sh_path = os.path.join(output_dir, name, 'build_' + name + '.sh')
write_new_file(sh_path, test_sh)
st = os.stat(sh_path)
os.chmod(sh_path, st.st_mode | stat.S_IEXEC)

header_h = \
    """
#ifndef __{class_name_upper}_H__
#define __{class_name_upper}_H__

#include <string>
#include <stdint.h>

enum someThingEnumerated {{
	kValue1 = 1,
	kValue2,
	kValue3,
	kValue4
}};

class {class_name}
{{
protected:
	int m_someField;
	int m_someOtherField;
	char* m_anotherMoreComplexField;

public:
    static const uint32_t OBJECT_TYPE = 0x777;
    virtual uint32_t getObjectType() {{
        return {class_name}::OBJECT_TYPE;
    }};

    {class_name}();
	{class_name}(int m) : m_someField(m) {{}};
	{class_name}(int m1, int m2) : m_someField(m1), m_someOtherField(m2) {{}};
	~{class_name}();

	// these methods are simple, can be defined inline
	int getSomeField() {{
		return m_someField;
	}}
	int getSomeOtherField() {{
		return m_someOtherField;
	}}
	const char *getAnotherMoreComplexField() {{
		return m_anotherMoreComplexField;
	}}
	void setSomeField(int f) {{
		m_someField = f;
	}}
	void setSomeField() {{

	}}
	void setSomeOtherField(int f) {{
		m_someOtherField = f;
	}}
	void setAnotherMoreComplexField(const char *str);

	long long thisReturnsALongLong();

	static void func();
	static void func(int a);
	static void func(int a, float b);

	long long receivesLongLong(long long someId);
	std::string returnsAString();
	const char *returnsACString();

	int doSomeProcessing(std::string arg1, std::string arg2);
}};

namespace SomeNamespace {{
class AnotherClass {{
protected:
	int justOneField;

public:
    static const uint32_t OBJECT_TYPE = 0x778;
    virtual uint32_t getObjectType() {{
        return AnotherClass::OBJECT_TYPE;
    }};
	int aPublicField;

	AnotherClass();
	~AnotherClass();

	// also simple methods, can be defined inline
	int getJustOneField() {{
		return justOneField;
	}}
	// wrong setter - won't work (needs ONLY one parameter in order to work)
	void setJustOneField() {{
		justOneField = 999;
	}}

	void doSomethingSimple();
}};
}};

#endif
""".format(**{
    'name': name,
    'output_dir_name': output_dir_name,
    'class_name': class_name,
    'class_name_upper': class_name.upper(),
    })

h_path = os.path.join(output_dir, name, 'classes')
make_sure_path_exists(h_path)
write_new_file(os.path.join(h_path, class_name + '.h'), header_h)

header_cpp = \
    """
/**
 * Simple example of a C++ class that can be binded using the
 * automatic script generator
 */

#include "{class_name}.h"

{class_name}::{class_name}()
{{
	// just set some fields
	m_someField = 0;
	m_someOtherField = 10;
	m_anotherMoreComplexField = NULL;
}}

// empty destructor
{class_name}::~{class_name}()
{{
}}

long long {class_name}::thisReturnsALongLong() {{
	static long long __id = 0;
	return __id++;
}}

void {class_name}::func() {{
}}

void {class_name}::func(int a) {{
}}

void {class_name}::func(int a, float b) {{
}}

long long {class_name}::receivesLongLong(long long someId) {{
	return someId + 1;
}}

std::string {class_name}::returnsAString() {{
	std::string myString = "my std::string";
	return myString;
}}

const char *{class_name}::returnsACString() {{
	return "this is a c-string";
}}

// just a very simple function :)
int {class_name}::doSomeProcessing(std::string arg1, std::string arg2)
{{
	return arg1.length() + arg2.length();
}}

void {class_name}::setAnotherMoreComplexField(const char *str)
{{
	if (m_anotherMoreComplexField) {{
		free(m_anotherMoreComplexField);
	}}
	size_t len = strlen(str);
	m_anotherMoreComplexField = (char *)malloc(len);
	memcpy(m_anotherMoreComplexField, str, len);
}}

namespace SomeNamespace
{{
AnotherClass::AnotherClass()
{{
	justOneField = 1313;
	aPublicField = 1337;
}}
// empty destructor
AnotherClass::~AnotherClass()
{{
}}

void AnotherClass::doSomethingSimple() {{
	fprintf(stderr, "just doing something simple
");
}}
}};
""".format(**{'name': name,
        'output_dir_name': output_dir_name, 'class_name': class_name})

write_new_file(os.path.join(output_dir, name, 'classes', class_name
               + '.cpp'), header_cpp)
print bcolors.HEADER + '*************' + bcolors.ENDC
print bcolors.HEADER + '* All done! *' + bcolors.ENDC
print bcolors.HEADER + '*************' + bcolors.ENDC

print 'You can now edit the example classes in ' \
    + os.path.join(output_dir, name, 'classes')
print '\n'
print bcolors.HEADER + 'You can build you JS bindings by running: ' \
    + bcolors.ENDC
print 'cd "' + os.path.join(output_dir, name) + '"'
print '.' + sh_path

			