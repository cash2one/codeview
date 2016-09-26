//
//  PKNotificationCenter.h
//  PKFate
//
//  Created by 茁 袁 on 15/10/29.
//
//

#pragma once

#include <stdio.h>
#include <mutex>
#include "cocos2d.h"

using namespace cocos2d;

class PKNotificationCenter {

    typedef struct structPkCallback {
        Ref* target;
        SEL_CallFuncO selector;
        std::string name;
        Ref* sender;
    } PkCallback;

private:
	std::map<std::string, PkCallback*> _callbacks;
    
   // std::mutex _m;

public:

    static PKNotificationCenter* getInstance();
    bool IsExist(const std::string &name);
    void addObserver(Ref *target, SEL_CallFuncO selector, const std::string& name, Ref *sender);
    void removeObserver(Ref *target, const std::string& name);
    int removeAllObservers(Ref *target);
    void postNotification(const std::string& name, Ref *sender=nullptr);
};