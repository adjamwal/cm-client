/**************************************************************************
 *       Copyright (c) 2023, Cisco Systems, All Rights Reserved
 ***************************************************************************
 *
 *  @file:    ConfigWatchdog.hpp
 *
 ***************************************************************************
 * @desc  Class for spreading reloading configuration throught the system.
 ***************************************************************************/
#pragma once
#include "config_shared.hpp"

#include <functional>
#include <list>

namespace ConfigShared
{

class  CONFIGSHARED_MODULE_API ConfigWatchdog
{
public:
    ConfigWatchdog(const ConfigWatchdog &) = delete;
    ConfigWatchdog &operator=(const ConfigWatchdog &) = delete;
    ConfigWatchdog(ConfigWatchdog &&) = delete;

    static ConfigWatchdog& getConfigWatchdog();
    void addSubscriber(std::function<void()> subscriber);
    void removeAllSubscribers();
    //this function could be used to get information about changes in config file
    void detectedConfigChanges();

private:
    ConfigWatchdog() = default;
    void notify_subscribers();

    virtual ~ConfigWatchdog()
    {
        removeAllSubscribers();
    }

    std::list<std::function<void()>> subscribers;

};

}
