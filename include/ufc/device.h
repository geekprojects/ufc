//
// Created by Ian Parker on 14/05/2024.
//

#ifndef DEVICE_H
#define DEVICE_H

#include <ufc/aircraftstate.h>

#include <string>
#include <map>

#include "logger.h"

namespace UFC
{

class Device;
class FlightConnector;

class DeviceInit
{
 private:
 public:
    DeviceInit() = default;
    virtual ~DeviceInit() = default;

    virtual Device* create(FlightConnector* app) { return nullptr; }

    virtual std::string getName() { return ""; }
};

class DeviceRegistry
{
 private:
    std::map<std::string, DeviceInit*> m_devices;

 public:
    static DeviceRegistry* getDeviceRegistry();
    static void registerDevice(DeviceInit* device);

    const std::map<std::string, DeviceInit*>& getDevices() { return m_devices; }
};

#define UFC_DEVICE(_name, _class)  \
    class _name##Init : public UFC::DeviceInit \
    { \
     private: \
        static _name##Init* const init; \
     public: \
        _name##Init() \
        { \
            UFC::DeviceRegistry::registerDevice(this); \
        } \
        ~_name##Init() override \
        { \
        } \
        std::string getName() override \
        { \
            return #_name; \
        } \
        _class* create(UFC::FlightConnector* app) override \
        { \
            return new _class(app); \
        } \
    }; \
    _name##Init* const _name##Init::init = new _name##Init();

class Device : public Logger
{
 protected:
    FlightConnector* m_flightConnector;
    std::string m_name;

 public:
    explicit Device(FlightConnector* flightConnector, const std::string &name);
    ~Device() override = default;

    virtual bool detect() = 0;
    virtual bool init() { return true; }

    virtual std::string getName() { return m_name; }

    virtual void update(const AircraftState& state) = 0;
};

}

#endif //DEVICE_H
