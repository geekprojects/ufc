//
// Created by Ian Parker on 14/05/2024.
//

#ifndef UFC_DEVICE_H
#define UFC_DEVICE_H

#include <ufc/aircraftstate.h>

#include <string>
#include <map>
#include <memory>

#include "logger.h"

namespace UFC
{

class Device;
class FlightConnector;

class DeviceInit
{
 public:
    DeviceInit() = default;
    virtual ~DeviceInit() = default;

    virtual std::shared_ptr<Device> create(FlightConnector* app) { return nullptr; }

    virtual std::string getName() { return ""; }
};

class DeviceRegistry
{
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
        std::shared_ptr<Device> create(UFC::FlightConnector* app) override \
        { \
            return std::make_shared<_class>(app); \
        } \
    }; \
    _name##Init* const _name##Init::init = new _name##Init();

class Device : public Logger
{
    FlightConnector* m_flightConnector;
    std::string m_name;

 protected:
    [[nodiscard]] FlightConnector* getFlightConnector() const { return m_flightConnector; }

 public:
    explicit Device(FlightConnector* flightConnector, const std::string &name);
    ~Device() override = default;

    virtual bool detect() = 0;
    virtual bool init() { return true; }
    virtual void close()
    {
        // Does nothing by default
    }

    virtual std::string getName() { return m_name; }

    virtual void update(std::shared_ptr<AircraftState> state) = 0;
};

}

#endif
