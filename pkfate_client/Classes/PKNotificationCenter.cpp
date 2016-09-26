//
//  PKNotificationCenter.cpp
//  PKFate
//
//  Created by 茁 袁 on 15/10/29.
//
//
#include "PKNotificationCenter.h"
static PKNotificationCenter *_notification_center = nullptr;
PKNotificationCenter* PKNotificationCenter::getInstance() {
        if (!_notification_center) {
            _notification_center = new PKNotificationCenter;
        }
        return _notification_center;
    }

bool PKNotificationCenter::IsExist(const std::string &name) {
        return _callbacks.find(name) != _callbacks.end();
    }

    void PKNotificationCenter::addObserver(Ref *target, SEL_CallFuncO selector,
                     const std::string& name, Ref *sender)
    {
        CCLOG("addObserver: %s", name.c_str());
       // _m.lock();
        CCLOG("addObserver: %s locked", name.c_str());
        
        // assert(!IsExist(name));

		PkCallback *cb = new PkCallback;

        cb->target = target;
        cb->selector = selector;
        cb->name = name;
        cb->sender = sender;
        _callbacks[name] = cb;
        //_m.unlock();

    }
    
    void PKNotificationCenter::removeObserver(Ref *target, const std::string& name) {
        CCLOG("removeObserver: %s", name.c_str());
       // _m.lock();
        CCLOG("removeObserver: %s locked", name.c_str());
        auto cbit = _callbacks.find(name);
        if (cbit == _callbacks.end()) {
            CCLOG("not supported message: %s", name.c_str());
           // _m.unlock();
            return;
        }
        
		PkCallback *cb = cbit->second;
        _callbacks.erase(cbit);
        delete cb;
       // _m.unlock();
    }
	int PKNotificationCenter::removeAllObservers(Ref *target){
		if (!target)
			return 0;
		int ret = 0;
		//_m.lock();
		CCLOG("removeAllObservers");
		std::map<std::string, PkCallback*>::iterator it = _callbacks.begin();
		while (it != _callbacks.end()){
			PkCallback *cb = it->second;
			if (cb->target == target){
				_callbacks.erase(it++);
				delete cb;
				ret++;
			}
			else
				it++;
		}
		//_m.unlock();
		return ret;
	}
    void PKNotificationCenter::postNotification(const std::string& name, Ref *sender) {
        //_m.lock();
        auto cbit = _callbacks.find(name);
        if (cbit == _callbacks.end()) {
            CCLOG("not supported message: %s", name.c_str());
            //_m.unlock();
            return;
        }

		PkCallback *cb = cbit->second;

        if (!sender) {
            sender = cb->sender;
        }

        Ref* target = cb->target;
      //  _m.unlock();

        if (target) {
            (target->*cb->selector)(sender);
        }
    }