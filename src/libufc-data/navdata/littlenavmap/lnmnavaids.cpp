//
// Created by Ian Parker on 17/03/2026.
//

#include "ufc/data/lnmnavdata.h"

using namespace std;
using namespace UFC;

bool LNMNavAids::init()
{
    m_findNavStatement = getDatabase()->prepareStatement(
        "SELECT ns.lonx, ns.laty, ns.nav_type, ns.name, wp.waypoint_id FROM nav_search ns "
        "left join waypoint wp on wp.waypoint_id = ns.waypoint_id or wp.nav_id = ns.vor_id "
        "WHERE ns.ident=?");
    return true;
}

vector<shared_ptr<NavAid>> LNMNavAids::findById(const string &id)
{
    vector<shared_ptr<NavAid>> navAids;

    m_findNavStatement->bindString(1, id);
    while (m_findNavStatement->step())
    {
        auto navAid = make_shared<NavAid>();
        navAid->setId(id);
        navAid->setLocation(Coordinate(
            m_findNavStatement->getDouble(1),
            m_findNavStatement->getDouble(0)));
        string type = m_findNavStatement->getString(2);
        navAid->setName(m_findNavStatement->getString(3));
        navAid->setSourceId(m_findNavStatement->getInt64(4));

        if (type.find('W') != string::npos)
        {
            navAid->setType(NavAidType::WAY_POINT);
        }
        else if (type.find('V') != string::npos)
        {
            navAid->setType(NavAidType::VOR);
        }
        else
        {
            // Not ideal
            navAid->setType(NavAidType::WAY_POINT);
        }

        navAids.push_back(navAid);
    }
    m_findNavStatement->reset();

    return navAids;
}
