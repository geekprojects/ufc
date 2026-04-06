//
// Created by Ian Parker on 25/03/2026.
//

#include "gtest/gtest.h"
#include <ufc/data/procedures.h>
#include <ufc/data/lnmnavdata.h>

using namespace std;
using namespace UFC;

TEST(LNMProcedures, Departure1)
{
    auto data = make_shared<LittleNavMapData>(nullptr);
    ASSERT_TRUE(data->init());
    auto procedures = data->getProcedures();
    //procedures->getProcedures("EGKK");
    auto departure = procedures->getDeparture("EGKK", "SAM3W");
    ASSERT_NE(departure, nullptr);
}

TEST(LNMProcedures, RunwayDepartures)
{
    auto data = make_shared<LittleNavMapData>(nullptr);
    ASSERT_TRUE(data->init());
    auto procedures = data->getProcedures();
    //procedures->getProcedures("EGKK");
    auto departures = procedures->getProceduresForRunway(ProcedureType::DEPARTURE, "LIML", "17");
    for (auto departure : departures)
    {
        printf("%s: runway=%s\n", departure->ident.c_str(), departure->runway.c_str());
    }
}

TEST(LNMProcedures, Arrival1)
{
    auto data = make_shared<LittleNavMapData>(nullptr);
    ASSERT_TRUE(data->init());
    auto procedures = data->getProcedures();
    procedures->getArrival("EGHI", "ELDA1S");
}
