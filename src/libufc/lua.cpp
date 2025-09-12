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

bool UFCDataMetaObject::Exists(const std::string &name)
{
    float value;
    return m_flightConnector->getDataSource()->getDataFloat(name, value);
}

std::shared_ptr<LuaType> UFCDataMetaObject::getValue(std::string &name)
{
    float value;
    bool res = m_flightConnector->getDataSource()->getDataFloat(name, value);
    if (!res)
    {
        return std::make_shared<LuaTNil>();
    }
    return std::make_shared<LuaTNumber>(value);
}

 void UFCDataMetaObject::setValue(std::string &name, std::shared_ptr<LuaType> val)
{
    if (val->getTypeId() != LUA_TNUMBER)
    {
        return;
    }

    LuaTNumber* number = dynamic_cast<LuaTNumber*>(val.get());

    m_flightConnector->getDataSource()->setData(name, number->getValue());
}

int UFCCommandMetaObject::Execute(LuaState &L)
{
    int n = lua_gettop(L);

    printf("UFCCommandMetaObject::Execute: arguments=%d\n", n);

    if (!lua_isstring(L, 2))
    {
        lua_pushliteral(L, "incorrect argument");
        lua_error(L);
    }

    auto commandStr = lua_tostring(L, 2);
    printf("UFCCommandMetaObject::Execute: command=%s\n", commandStr);

    CommandDefinition commandDefinition = {};
    m_flightConnector->getDataSource()->executeCommand(commandStr, commandDefinition);

    return 0;
}

UFCLua::UFCLua(FlightConnector* flightConnector) : m_flightConnector(flightConnector)
{
    m_ufcDataMetaObject = std::make_shared<UFCDataMetaObject>(m_flightConnector);
    m_lua.AddGlobalVariable("data", m_ufcDataMetaObject);

    m_ufcCommandMetaObject = std::make_shared<UFCCommandMetaObject>(m_flightConnector);
    m_lua.AddGlobalVariable("command", m_ufcCommandMetaObject);

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
