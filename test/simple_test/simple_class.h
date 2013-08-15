#ifndef __SIMPLE_CLASS_H__
#define __SIMPLE_CLASS_H__

#include <string>
#include <stdint.h>
#include <memory>
#include <vector>

//#define TEST_COCOS2D

#ifdef TEST_COCOS2D
#include "cocos2d.h"
#endif

using namespace std;

enum someThingEnumerated {
	kValue1 = 1,
	kValue2,
	kValue3,
	kValue4
};

class SimpleNativeClass
{
protected:
	int m_someField;
	int m_someOtherField;
	char* m_anotherMoreComplexField;

public:
    static const uint32_t OBJECT_TYPE = 0x777;
    virtual uint32_t getObjectType() {
        return SimpleNativeClass::OBJECT_TYPE;
    };

    SimpleNativeClass();
	SimpleNativeClass(int m) : m_someField(m) {};
	SimpleNativeClass(int m1, int m2) : m_someField(m1), m_someOtherField(m2) {};
	~SimpleNativeClass();

	// these methods are simple, can be defined inline
	int getSomeField() {
		return m_someField;
	}
	int getSomeOtherField() {
		return m_someOtherField;
	}
	const char *getAnotherMoreComplexField() {
		return m_anotherMoreComplexField;
	}
	void setSomeField(int f) {
		m_someField = f;
	}
	void setSomeField() {

	}
	void setSomeOtherField(int f) {
		m_someOtherField = f;
	}
	void setAnotherMoreComplexField(const char *str);

	long long thisReturnsALongLong();

	static void func();
	static void func(int a);
	static void func(int a, float b);

	long long receivesLongLong(long long someId);
	std::string returnsAString();
	const char *returnsACString();

	int doSomeProcessing(std::string arg1, std::string arg2);
};

namespace SomeNamespace {
class AnotherClass {
protected:
	int justOneField;

public:
    static const uint32_t OBJECT_TYPE = 0x778;
    virtual uint32_t getObjectType() {
        return AnotherClass::OBJECT_TYPE;
    };
	int aPublicField;

	AnotherClass();
	~AnotherClass();

	// also simple methods, can be defined inline
	int getJustOneField() {
		return justOneField;
	}
	// wrong setter - won't work (needs ONLY one parameter in order to work)
	void setJustOneField() {
		justOneField = 999;
	}

	void doSomethingSimple();
};
};

#ifndef TEST_COCOS2D
namespace cocos2d {

class Object
{
public:
    virtual ~Object(){};
};

}
#endif

namespace cocos2d_ptr_test {

class Node_T : public cocos2d::Object
{
public:
    virtual ~Node_T()
    {
        printf("In the destruction of Node_T...\n");
    };
    
    static shared_ptr<Node_T> create()
    {
        return shared_ptr<Node_T>(new Node_T());
    }
    
    void setPostion()
    {
        printf("In Node::setPosition...\n");
    };

    void addChild(std::shared_ptr<Node_T> child, int zOrder, int tag)
    {
        printf("begin Node::addChild ....\n");
        _children.push_back(child);
        printf("after Node::addChild ....\n");
    };

private:
    std::vector<std::shared_ptr<Node_T>> _children;
};

class Sprite_T : public Node_T
{
public:
    typedef std::shared_ptr<Sprite_T> Ptr;
    static Ptr create(const char* filename)
    {
        Sprite_T* ret = new Sprite_T();
        ret->init(filename);
        return shared_ptr<Sprite_T>(ret);
    }

    virtual ~Sprite_T()
    {
        printf("destruction of Sprite_T....\n");
    }

    bool init(const char* filename)
    {
        return true;
    }
};

} // namespace cocos2d {

#endif
