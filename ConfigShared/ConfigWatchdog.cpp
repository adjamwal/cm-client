#include <functional>
#include <list>
#include "ConfigWatchdog.hpp"

namespace ConfigShared
{

ConfigWatchdog& ConfigWatchdog::getConfigWatchdog()
{
    static ConfigWatchdog cw;
    return cw;
}

void ConfigWatchdog::addSubscriber(std::function<void()> subscriber)
{
    if(subscriber)
        subscribers.push_back(subscriber);
}

void ConfigWatchdog::removeAllSubscribers()
{
    subscribers.clear();
}

void ConfigWatchdog::detectedConfigChanges()
{
    notify_subscribers();
}


void ConfigWatchdog::notify_subscribers()
{
    for (auto& subscriber : subscribers)
        subscriber();
}

}
