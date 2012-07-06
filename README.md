# Requirements

* python2.7
* py-yaml
* cheetah (for target language templates)
* libclang, from clang 3.1

If using macports, you can easily install all requirements above.

    sudo port install python27 py27-yaml py27-cheetah clang-3.1 llvm-3.1

You should also modify the generator script, to point to the directory where libclang.dyld is
located.

# Usage

    Usage: generator.py [options] {configfile}
    
    Options:
      -h, --help   show this help message and exit
      -s SECTION   sets a specific section to be converted
      -t TARGET    specifies the target vm. Will search for TARGET.yaml

Basically, you specify a target vm (spidermonkey is the only current target vm) and the section from
the `.ini` file you want to generate code for.

# The `.ini` file

The `.ini` file is a simple text file specifying the settings for the code generator. Here's the
default one, used for cocos2d-x

    [cocos2d-x]
    name = cocos2dx
    events  = CCNode#onEnter CCNode#onExit
    extra_arguments = -I../../cocos2dx/include -I../../cocos2dx/platform -I../../cocos2dx/platform/ios -I../../cocos2dx -I../../cocos2dx/kazmath/include -arch i386 -DTARGET_OS_IPHONE -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator5.1.sdk -x c++
    headers = ../../cocos2dx/include/cocos2d.h
    classes = CCSprite
    structs_as_objects = ccColor3B

## Required sections

* name: the name of the project. Must be a valid identifier for the language of the target vm. Most
  of the time, the name will be intermixed between the class name and the function name, since all
  generated (probably) will be free functions, we do that in order to avoid name collition.
* events: a list of identifiers in the form of ClassName#functionName that are events to be called
  from the native world to the target vm.
* extra_arguments: extra arguments to pass to the clang interface. Basically you can think of this
  as the arguments to pass to the "compiler", so add as many as you need here. If you're targetting
  C++, make sure you add "-x c++" as the last argument to force C++ mode on a ".h" file. Otherwise,
  name your header files as ".hpp".
* headers: list of headers to parse. Usually you add a single header that in turn `#include`s the
  rest of the files.
* classes: the classes that will be parsed.

# The templates
