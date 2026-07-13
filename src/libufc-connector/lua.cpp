//
// Created by Ian Parker on 27/05/2025.
//

#include <memory>

#include "ufc/flightconnector.h"
#include "lua.h"

#include "Engine/LuaTTable.hpp"
#include "ufc/utils/utils.h"

using namespace std;
using namespace UFC;
using namespace LuaCpp;
using namespace LuaCpp::Registry;
using namespace LuaCpp::Engine;

bool UFCDataMetaObject::Exists(const string &name)
{
    return m_flightConnector->getState()->isSet(name);
}

shared_ptr<LuaType> UFCDataMetaObject::getValue(string &name)
{
    if (m_flightConnector->getDataSource() == nullptr)
    {
        return make_shared<LuaTNil>();
    }

    auto value = m_flightConnector->getState()->getValue(name);
    if (value == nullptr)
    {
        return make_shared<LuaTNil>();
    }

    switch (value->getType())
    {
        case DataRefType::BOOLEAN:
        case DataRefType::INTEGER:
        case DataRefType::FLOAT:
        case DataRefType::UNKNOWN:
            return make_shared<LuaTNumber>(value->getFloat());
        case DataRefType::STRING:
        {
            auto table = make_shared<LuaTTable>();
            for (int idx = 0; idx < value->getString().size(); ++idx)
            {
                table->setValue(Table::Key(idx + 1), make_shared<LuaTNumber>(value->getString()[idx]));
            }
            return table;
        }
    }
}

 void UFCDataMetaObject::setValue(string &name, shared_ptr<LuaType> val)
{
    if (val->getTypeId() == LUA_TNUMBER)
    {
        auto number = static_cast<LuaTNumber*>(val.get());
        if (number == nullptr)
        {
            printf("UFCDataMetaObject::setValue: name=%s value is null?\n", name.c_str());
            return;
        }
        m_flightConnector->getDataSource()->setData(name, AircraftValue((float)number->getValue()));
    }
    else if (val->getTypeId() == LUA_TSTRING)
    {
        auto str = static_cast<LuaTString*>(val.get());
        m_flightConnector->getState()->set(name, utf82wstring(str->getValue().c_str()));
    }
    else if (val->getTypeId() == LUA_TTABLE)
    {
        auto strTable = static_cast<LuaTTable*>(val.get());

        wstring str(strTable->getValues().size(), ' ');
        for (auto valuePair : strTable->getValues())
        {
            int idx = valuePair.first.getIntValue();
            wchar_t value = static_cast<LuaTNumber*>(valuePair.second.get())->getValue();
            str[idx - 1] = value;
        }

        m_flightConnector->getState()->set(name, str);
    }
    else
    {
        printf("UFCDataMetaObject::setValue: Unhandled type: %d\n", val->getTypeId());
    }
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
    m_ufcDataMetaObject = make_shared<UFCDataMetaObject>(m_flightConnector);
    m_lua.AddGlobalVariable("data", m_ufcDataMetaObject);

    m_ufcCommandMetaObject = make_shared<UFCCommandMetaObject>(m_flightConnector);
    m_lua.AddGlobalVariable("command", m_ufcCommandMetaObject);

    m_stateTable = make_shared<LuaTTable>();
    m_lua.AddGlobalVariable("state", m_stateTable);
}

void UFCLua::execute(string str)
{
    m_lua.CompileStringAndRun(str);
}

float UFCLua::execute(const string &name, const string &str, string variable, float value)
{
    map<string, AircraftValue> values;
    values[name] = value;
    return execute(name, str, values);
}

float UFCLua::execute(const string &name, const string &str, map<string, AircraftValue> values)
{
    m_lua.CompileString(name, str);

    LuaEnvironment env;
    shared_ptr<LuaTNumber> returnValue = nullptr;

    for (pair<string, AircraftValue> value : values)
    {
        string valueName = value.first;
        auto luaValue = make_shared<LuaTNumber>(value.second.getFloat());
        env[valueName] = luaValue;
        if (valueName == "value")
        {
            returnValue = luaValue;
        }
    }

    if (returnValue == nullptr)
    {
        returnValue = make_shared<LuaTNumber>(0.0);
        env["value"] = returnValue;
    }

    env["data"] = m_ufcDataMetaObject;
    env["command"] = m_ufcCommandMetaObject;
    env["state"] = m_stateTable;

    m_lua.RunWithEnvironment(name, env);

    return returnValue->getValue();
}
