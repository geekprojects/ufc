//
// Created by Ian Parker on 27/05/2025.
//

#ifndef LUA_H
#define LUA_H

#include <LuaCpp.hpp>

namespace UFC
{
class FlightConnector;

class UFCDataMetaObject : public LuaCpp::LuaMetaObject
{
    UFC::FlightConnector* m_flightConnector;

public:
    UFCDataMetaObject(UFC::FlightConnector* flightConnector) : m_flightConnector(flightConnector) {}

    bool Exists(const std::string &name);
    std::shared_ptr<LuaType> getValue(std::string &name);
    void setValue(std::string &name, std::shared_ptr<LuaType> val);
};

class UFCCommandMetaObject : public LuaCpp::LuaMetaObject
{
    UFC::FlightConnector* m_flightConnector;

public:
    UFCCommandMetaObject(UFC::FlightConnector* flightConnector) : m_flightConnector(flightConnector) {}

    int Execute(LuaCpp::Engine::LuaState &L) override;
};

class UFCLua
{
 protected:
    UFC::FlightConnector* m_flightConnector;

    LuaCpp::LuaContext m_lua;
    std::shared_ptr<UFCDataMetaObject> m_ufcDataMetaObject;
    std::shared_ptr<UFCCommandMetaObject> m_ufcCommandMetaObject;
    std::shared_ptr<LuaCpp::Engine::LuaTTable> m_stateTable;

 public:
    UFCLua(UFC::FlightConnector* flightConnector);
    ~UFCLua() = default;

    bool init();
    void execute(std::string str);

    float execute(const std::string &str, std::string variable, float value);
};

}

#endif //LUA_H
