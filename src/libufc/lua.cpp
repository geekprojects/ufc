//
// Created by Ian Parker on 27/05/2025.
//

#include <memory>

#include "ufc/flightconnector.h"
#include "lua.h"

#include "Engine/LuaTTable.hpp"

using namespace std;
using namespace UFC;
using namespace LuaCpp;
using namespace LuaCpp::Registry;
using namespace LuaCpp::Engine;

bool UFCMetaObject::Exists(const std::string &name)
{
    float value;
    return m_flightConnector->getDataSource()->getDataFloat(name, value);
}

std::shared_ptr<LuaType> UFCMetaObject::getValue(std::string &name)
{
    float value;
    bool res = m_flightConnector->getDataSource()->getDataFloat(name, value);
    if (!res)
    {
        return std::make_shared<LuaTNil>();
    }
    return std::make_shared<LuaTNumber>(value);
}

 void UFCMetaObject::setValue(std::string &name, std::shared_ptr<LuaType> val)
{
    if (val->getTypeId() != LUA_TNUMBER)
    {
        return;
    }

    LuaTNumber* number = dynamic_cast<LuaTNumber*>(val.get());

    m_flightConnector->getDataSource()->setData(name, number->getValue());
}

UFCLua::UFCLua(FlightConnector* flightConnector) : m_flightConnector(flightConnector)
{
    m_ufcMetaObject = std::make_shared<UFCMetaObject>(m_flightConnector);
    m_lua.AddGlobalVariable("data", m_ufcMetaObject);

    m_stateTable = std::make_shared<LuaTTable>();
    m_lua.AddGlobalVariable("state", m_stateTable);
}

void UFCLua::execute(std::string str)
{
    m_lua.CompileStringAndRun(str);
}

float UFCLua::execute(const std::string &str, std::string variable, float value)
{
    m_lua.CompileString("ufcexec", str);

    auto luaValue = make_shared<LuaTNumber>(value);
    LuaEnvironment env;
    env[variable] = luaValue;
    m_lua.RunWithEnvironment("ufcexec", env);

    return luaValue->getValue();
}
