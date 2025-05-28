//
// Created by Ian Parker on 27/05/2025.
//

#include <memory>

#include "ufc/flightconnector.h"
#include "ufc/lua.h"

#include "Engine/LuaTTable.hpp"

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

bool UFCLua::init()
{
    m_ufcMetaObject = std::make_shared<UFCMetaObject>(m_flightConnector);
    m_lua.AddGlobalVariable("data", m_ufcMetaObject);

    m_stateTable = std::make_shared<LuaTTable>();
    m_lua.AddGlobalVariable("state", m_stateTable);

    return true;
}

void UFCLua::execute(std::string str)
{
    printf("UFCLua::execute: %s\n", str.c_str());
    m_lua.CompileStringAndRun(str);
}

