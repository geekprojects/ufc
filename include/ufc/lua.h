//
// Created by Ian Parker on 27/05/2025.
//

#ifndef LUA_H
#define LUA_H

#include <LuaCpp.hpp>

namespace UFC
{
class FlightConnector;

class UFCMetaObject : public LuaCpp::LuaMetaObject
{
    UFC::FlightConnector* m_flightConnector;

public:
    UFCMetaObject(UFC::FlightConnector* flightConnector) : m_flightConnector(flightConnector) {}

    bool Exists(const std::string &name);
    std::shared_ptr<LuaType> getValue(std::string &name);
    void setValue(std::string &name, std::shared_ptr<LuaType> val);
};

class UFCLua
{
 protected:
    UFC::FlightConnector* m_flightConnector;

    LuaCpp::LuaContext m_lua;
    std::shared_ptr<UFCMetaObject> m_ufcMetaObject;
    std::shared_ptr<LuaCpp::Engine::LuaTTable> m_stateTable;

 public:
    UFCLua(UFC::FlightConnector* flightConnector) : m_flightConnector(flightConnector) {}
    ~UFCLua() = default;

    bool init();
    void execute(std::string str);
};

}

#endif //LUA_H
