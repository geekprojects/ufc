//
// Created by Ian Parker on 16/10/2024.
//

#ifndef CLOCK_H
#define CLOCK_H

#include <ufc/datasource.h>

class ClockDataSource : public UFC::DataSource
{
 private:
    bool m_24Hour = true;
    bool m_showYear = false;
    time_t m_showYearTime = 0;
    bool m_showSeconds = false;
    time_t m_showSecondsTime = 0;

 public:
    ClockDataSource(UFC::FlightConnector* flightConnector);

    bool connect() override;
    void disconnect() override;
    bool update() override;

    void executeCommand(const std::string& command, const CommandDefinition& commandDefinition) override;
};

#endif //CLOCK_H
