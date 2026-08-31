//
// Created by Ian Parker on 17/07/2026.
//

#ifndef UNIVERSALFLIGHTCONNECTOR_TERMINALDATASOURCE_H
#define UNIVERSALFLIGHTCONNECTOR_TERMINALDATASOURCE_H

#include "ufc/datasource.h"

enum class TerminalState
{
    NORMAL,
    ESC,
    CSI,
    OSC
};

struct TerminalBuffer
{

};

enum class BufferType
{
    TEXT,
    TEXT_COLOUR,
    BACKGROUND_COLOUR
};

class TerminalDataSource : public UFC::DataSource
{
    pid_t m_childPid;
    int m_childOut;
    int m_childIn;

    TerminalState m_state = TerminalState::NORMAL;
    int m_curCol = 1;
    int m_curRow = 1;
    std::wstring m_command;

    std::map<BufferType, std::vector<std::wstring>> m_buffer;

    void printChar(std::shared_ptr<UFC::AircraftState> &state, wchar_t c);

    void command(const std::string &command) override;

    void reset();

    std::vector<std::wstring>& getBuffer(BufferType type)
    {
        if (!m_buffer.contains(type))
        {
            m_buffer[type] = std::vector<std::wstring>(14);
        }
        if (m_buffer[type].size() < 14)
        {
            m_buffer[type].resize(14);
        }
        return m_buffer[type];
    }

    std::wstring& getLine(int row, BufferType type = BufferType::TEXT);
    void setLine(int row, std::wstring line, BufferType type = BufferType::TEXT);

    wchar_t getChar(int row, int col, BufferType type = BufferType::TEXT);
    void setChar(std::shared_ptr<UFC::AircraftState> &state, int row, int col, wchar_t c, BufferType type = BufferType::TEXT);

public:
    explicit TerminalDataSource(UFC::FlightConnector* flightConnector);
    ~TerminalDataSource() override = default;

    bool connect() override;

    void disconnect() override;

    bool update() override;

};


#endif //UNIVERSALFLIGHTCONNECTOR_TERMINALDATASOURCE_H
