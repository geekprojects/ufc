//
// Created by Ian Parker on 19/08/2025.
//

#include "lnmnavdata.h"

using namespace std;
using namespace UFC;

bool LittleNavMapData::init()
{
    sqlite3_initialize();

    string m_path = "/Users/ian/.config/ABarthel/little_navmap_db/little_navmap_navigraph.sqlite";
    m_database = make_shared<Database>(m_path);
    return m_database->open();
}

std::shared_ptr<UFC::Airports> LittleNavMapData::getAirports()
{
    return NavDataSource::getAirports();
}

std::shared_ptr<UFC::NavAids> LittleNavMapData::getNavAids()
{
    return NavDataSource::getNavAids();
}

bool LNMAirports::init()
{
    auto db = ((LittleNavMapData*)m_navDataSource)->getDatabase();
    m_findByCodeStatement = db->prepareStatement("select * from airport where ident = ?");
    return true;
}

std::shared_ptr<UFC::Airport> LNMAirports::findNearest(UFC::Coordinate point) const
{
    return nullptr;
}

std::shared_ptr<UFC::Airport> LNMAirports::findByCode(const std::string &code)
{
    //auto db = ((LittleNavMapData*)m_navDataSource)->getDatabase():

    m_findByCodeStatement->bindString(1, code);
    m_findByCodeStatement->executeQuery();
    if (m_findByCodeStatement->step())
    {
        m_findByCodeStatement->getString(1);
    }

    return nullptr;
}
