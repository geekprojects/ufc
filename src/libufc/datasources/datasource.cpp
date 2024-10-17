//
// Created by Ian Parker on 14/10/2024.
//

#include <ufc/datasource.h>

using namespace std;
using namespace UFC;

static DataSourceRegistry* g_dataSourceRegistry = nullptr;

DataSourceRegistry* DataSourceRegistry::getDataSourceRegistry()
{
    if (g_dataSourceRegistry == nullptr)
    {
        g_dataSourceRegistry = new DataSourceRegistry();
    }
    return g_dataSourceRegistry;
}

void DataSourceRegistry::registerDataSource(DataSourceInit* dataSource)
{
    getDataSourceRegistry()->m_dataSources.insert(make_pair(dataSource->getName(), dataSource));
}
