#pragma once
#include <functional>
#include <list>

namespace PackageManager
{

class ConfigWatchdog
{
public:
    ConfigWatchdog(const ConfigWatchdog &) = delete;
    ConfigWatchdog &operator=(const ConfigWatchdog &) = delete;
    ConfigWatchdog(ConfigWatchdog &&) = delete;

    static ConfigWatchdog& getConfigWatchdog()
    {
        static ConfigWatchdog cw;
        return cw;
    }

    void addSubscriber(std::function<void()> subscriber)
    {
        if(subscriber)
            subscribers.push_back(subscriber);
    }

    void removeAllSubscribers()
    {
        subscribers.clear();
    }

    //this function could be used to get information about changes in config file
    void detectedConfigChanges()
    {
        notify_subscribers();
    }

private:
    ConfigWatchdog() = default;

    virtual ~ConfigWatchdog()
    {
        removeAllSubscribers();
    }

    void notify_subscribers()
    {
        for (auto& subscriber : subscribers)
            subscriber();
        }

    std::list<std::function<void()>> subscribers;

};

}