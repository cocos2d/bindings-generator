#ifndef __JS_COCOS2D_X_SPECIFICS_H__
#define __JS_COCOS2D_X_SPECIFICS_H__

#include "jsapi.h"
#include "ScriptingCore.h"

void register_cocos2dx_js_extensions();

class JSTouchDelegate: public CCTouchDelegate, public CCNode {
    public:
        void setJSObject(JSObject *obj);
        void registerStandardDelegate();
        void registerTargettedDelegate(int priority, bool swallowsTouches);
    
    bool ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent) {
        CC_UNUSED_PARAM(pTouch); 
        CC_UNUSED_PARAM(pEvent); 
        jsval retval; 
        ScriptingCore::getInstance()->executeCustomTouchEvent(CCTOUCHBEGAN, 
                                                        pTouch, _mObj, retval);
        if(JSVAL_IS_BOOLEAN(retval)) {
            return JSVAL_TO_BOOLEAN(retval);
        } return false;
    };
    // optional
    
    void ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent) {
        CC_UNUSED_PARAM(pTouch); 
        CC_UNUSED_PARAM(pEvent);
        jsval retval; 
        ScriptingCore::getInstance()->executeCustomTouchEvent(CCTOUCHMOVED, 
                                                              pTouch, _mObj);
    }
    
    void ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent) {
        CC_UNUSED_PARAM(pTouch); 
        CC_UNUSED_PARAM(pEvent);
        ScriptingCore::getInstance()->executeCustomTouchEvent(CCTOUCHENDED, 
                                                              pTouch, _mObj);
    }
    
    void ccTouchCancelled(CCTouch *pTouch, CCEvent *pEvent) {
        CC_UNUSED_PARAM(pTouch); 
        CC_UNUSED_PARAM(pEvent);
        ScriptingCore::getInstance()->executeCustomTouchEvent(CCTOUCHCANCELLED, 
                                                              pTouch, _mObj);
    }
    
    // optional
    void ccTouchesBegan(CCSet *pTouches, CCEvent *pEvent) {
        CC_UNUSED_PARAM(pTouches); 
        CC_UNUSED_PARAM(pEvent);
        ScriptingCore::getInstance()->executeCustomTouchesEvent(CCTOUCHBEGAN, 
                                                              pTouches, _mObj);
    }
    
    void ccTouchesMoved(CCSet *pTouches, CCEvent *pEvent) {
        CC_UNUSED_PARAM(pTouches); 
        CC_UNUSED_PARAM(pEvent);
        ScriptingCore::getInstance()->executeCustomTouchesEvent(CCTOUCHMOVED, 
                                                                pTouches, _mObj);        
    }
    void ccTouchesEnded(CCSet *pTouches, CCEvent *pEvent) {
        CC_UNUSED_PARAM(pTouches); 
        CC_UNUSED_PARAM(pEvent);
        ScriptingCore::getInstance()->executeCustomTouchesEvent(CCTOUCHENDED, 
                                                                pTouches, _mObj);
    }
    void ccTouchesCancelled(CCSet *pTouches, CCEvent *pEvent) {
        CC_UNUSED_PARAM(pTouches); 
        CC_UNUSED_PARAM(pEvent);
        ScriptingCore::getInstance()->executeCustomTouchesEvent(CCTOUCHCANCELLED, 
                                                                pTouches, _mObj);
    }
    private:
        JSObject *_mObj;    
};



#endif
