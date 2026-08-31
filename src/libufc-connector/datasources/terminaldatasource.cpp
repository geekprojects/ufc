//
// Created by Ian Parker on 17/07/2026.
//

#include "terminaldatasource.h"

#include <thread>
#include <unistd.h>

#include "ufc/flightconnector.h"
#include "ufc/utils/utils.h"

#if defined(__APPLE__) && defined(__MACH__)
#include <util.h>
#else
#include <pty.h>
#endif

using namespace std;
using namespace UFC;

UFC_DATA_SOURCE(Terminal, TerminalDataSource)

TerminalDataSource::TerminalDataSource(UFC::FlightConnector* flightConnector) : DataSource(flightConnector, "Terminal", "")
{
}

bool TerminalDataSource::connect()
{
    return true;
}

void TerminalDataSource::disconnect()
{
}

bool TerminalDataSource::update()
{
    int master;
    int slave;
    char name[1024];

    auto state = getFlightConnector()->getState();
    reset();

    int res = openpty(&master, &slave, name, NULL, NULL);
    if (res != 0)
    {
        log(ERROR, "update: Failed to open PTY: %d", errno);
        return false;
    }
    pid_t pid = fork();

    if (pid > 0)
    {
        // Parent
        close(slave);

        m_childPid = pid;
        m_childOut = master;
        m_childIn = master;

        char buffer[4097];
        while (isRunning())
        {
            state = getFlightConnector()->getState();
            res = read(m_childOut, buffer, 4096);
            if (res <= 0 || res > 4096)
            {
                break;
            }
#if 1
            log(DEBUG, "update: (Parent) Read %d bytes", res);
#endif

            buffer[res] = 0;
            //m_terminal->receiveChars(buffer, res);

            wstring line = utf82wstring(buffer);
            for (size_t pos = 0; pos < line.length(); pos++)
            {
                printChar(state, line[pos]);
            }

            for (int i = 1; i <= 14; i++)
            {
                state->set("fmc/0/line" + to_string(i) + "/text", getLine(i, BufferType::TEXT));

                auto textColour = getLine(i, BufferType::TEXT_COLOUR);
                auto backgroundColour = getLine(i, BufferType::BACKGROUND_COLOUR);
                if (i == m_curRow)
                {
                    auto swap = textColour[m_curCol - 1];
                    textColour[m_curCol - 1] = backgroundColour[m_curCol - 1];
                    backgroundColour[m_curCol - 1] = swap;
                }

                state->set("fmc/0/line" + to_string(i) + "/textColour", textColour);
                state->set("fmc/0/line" + to_string(i) + "/backgroundColour", backgroundColour);
            }
        }
        log(DEBUG, "update: (Parent) Child exited");
    }
    else if (pid == 0)
    {
        dup2(slave, STDOUT_FILENO);
        dup2(slave, STDIN_FILENO);
        dup2(slave, STDERR_FILENO);

        char* argv[4096];
        unsigned int i = 0;
        argv[i++] = strdup("/bin/dash");
        //argv[i++] = strdup("-i");
        /*
        for (i = 0; i < m_args.size(); i++)
        {
            argv[i + 1] = m_args[i];
        }
        */
        argv[i] = 0;

        char* envp[4096];
        i = 0;
        /*
        for (i = 0; i < m_env.size(); i++)
        {
            envp[i] = m_env[i];
        }
        */
        envp[i++] = strdup((string("HOME=") + getenv("HOME")).c_str());
        envp[i++] = strdup("TERM=xterm-256color");
        envp[i] = 0;

        execve(argv[0], argv, envp);

        perror("execl() failed");
        _exit(errno);
    }
    return true;
}

void TerminalDataSource::printChar(shared_ptr<AircraftState>& state, wchar_t c)
{
    if (m_state == TerminalState::NORMAL)
    {
        if (c == '\n')
        {
            m_curRow++;
            m_curCol = 1;
        }
        else if (c == '\r')
        {
            // Ignore
        }
        else if (c == 8)
        {
            // Backspace
            if (m_curCol > 1)
            {
                m_curCol--;
                auto line =  getLine(m_curRow);
                line[m_curCol - 1] = ' ';
                setLine(m_curRow, line);
            }
        }
        else if (c == 0x1b)
        {
            m_state = TerminalState::ESC;
            return;
        }
        else if (iswprint(c))
        {
            setChar(state, m_curRow, m_curCol++, c);
        }
        else
        {
            log(WARN, "Unhandled character: %d", c);
        }

        if (m_curCol > 24)
        {
            m_curCol = 1;
            m_curRow++;
        }
        if (m_curRow > 14)
        {
            log(DEBUG, "printChar: scrolling up!");
            for (int i = 2; i <= 14; i++)
            {
                wstring line = getLine(i);
                setLine(i - 1, line);
            }

            setLine(14, wstring(24, ' '));
            m_curRow = 14;
        }
    }
    else if (m_state == TerminalState::ESC)
    {
        m_command = L"";
        if (c == '[')
        {
            m_state = TerminalState::CSI;
        }
        else if (c == ']')
        {
            m_state = TerminalState::OSC;
        }
        else
        {
            m_state = TerminalState::NORMAL;
        }
    }
    else if (m_state == TerminalState::CSI)
    {
        if (iswalpha(c))
        {
            auto params = splitString(m_command, ';');

            wstring paramStr;
            for (auto param : params)
            {
                if (!paramStr.empty())
                {
                    paramStr += L", ";
                }
                paramStr += L"[" + param + L"]";
            }

            long param1 = -1;
            long param2 = -1;
            if (params.size() > 0)
            {
                param1 = wcstol(params.at(0).c_str(), nullptr, 10);
                if (params.size() > 1)
                {
                    param2 = wcstol(params.at(1).c_str(), nullptr, 10);
                }
            }

            switch (c)
            {
                case 'd':
                    if (param1 == -1)
                    {
                        param1 = 1;
                    }
                    log(DEBUG, "printChar: CSI d: row=%d", param1);
                    m_curRow = param1;
                    m_curCol = 1;
                    break;
                case 'n':
                    if (param1 == 6)
                    {
                        log(DEBUG, "printChar: CSI 6n: Requested position: %d, %d", m_curRow, m_curCol);
                        string response = "\x1b[" + to_string(m_curRow) + ";" + to_string(m_curCol) + "R";
                        log(DEBUG, "printChar: CSI 6n:  -> %s", response.c_str());
                        write(m_childIn, response.c_str(), response.length());
                    }
                    break;
                case 'A':
                    if (param1 == -1)
                    {
                        param1 = 1;
                    }
                    log(DEBUG, "printChar: CSI: A: row -= %ld", param1);
                    m_curRow -= param1;
                    break;
                case 'B':
                    if (param1 == -1)
                    {
                        param1 = 1;
                    }
                    log(DEBUG, "printChar: CSI: B: row += %ld", param1);
                    m_curRow += param1;
                    break;
                case 'C':
                    if (param1 == -1)
                    {
                        param1 = 1;
                    }
                    log(DEBUG, "printChar: CSI: C: col += %ld", param1);
                    m_curCol += param1;
                    break;
                case 'D':
                    if (param1 == -1)
                    {
                        param1 = 1;
                    }
                    log(DEBUG, "printChar: CSI: D: col -= %ld", param1);
                    m_curCol -= param1;
                    break;
                case 'H':
                    if (param1 == -1)
                    {
                        param1 = 1;
                    }
                    if (param2 == -1)
                    {
                        param2 = 1;
                    }
                    log(DEBUG, "printChar: CSI: H: Cursor Position: row=%ld, col=%ld", param1, param2);
                    m_curRow = param1;
                    m_curCol = param2;
                    break;
                case 'J':
                {
                    int start = 1;
                    int end = 14;
                    if (param1 == -1 || param1 == 0)
                    {
                        start = m_curRow + 1;
                    }
                    else if (param1 == 1)
                    {
                        end = m_curRow - 1;
                    }
                    else if (param1 == 2)
                    {
                        // All
                    }
                    else
                    {
                    log(WARN, "printChar: CSI: J Unsupported param: %d", param1);
                    }
                    wstring blankLine(24, L' ');
                    for (int i = start; i <= end; i++)
                    {
                        setLine(i, blankLine);
                    }
                    break;
                }
                case 'K':
                {
                    auto line = getLine(m_curRow);
                    int start = 1;
                    int end = 24;
                    if (param1 == -1 || param1 == 0)
                    {
                        start = m_curCol + 1;
                    }
                    else if (param1 == 1)
                    {
                        end = m_curCol - 1;
                    }
                    for (int i = start; i <= end; i++)
                    {
                        line[i - 1] = ' ';
                    }
                    log(DEBUG, "printChar: CSI: K (%d): start=%d, end=%d)", param1, start, end);
                    setLine(m_curRow, line);
                    break;
                }
                default:
                    log(WARN, "printChar: CSI: unhandled command: %ls%lc (params=%ls)", m_command.c_str(), c, paramStr.c_str());
                    break;
            }

            if (m_curRow < 1)
            {
                m_curRow = 1;
            }
            else if (m_curRow > 14)
            {
                m_curRow = 14;
            }
            if (m_curCol < 1)
            {
                m_curCol = 1;
            }
            else if (m_curCol > 24)
            {
                m_curCol = 24;
            }

            m_state = TerminalState::NORMAL;
        }
        else
        {
            m_command += c;
        }
    }
    else if (m_state == TerminalState::OSC)
    {
        if (c == 0x07)
        {
            log(DEBUG, "printChar: OSC: command=%ls", m_command.c_str());
            m_state = TerminalState::NORMAL;
        }
        else
        {
            m_command += c;
        }
    }
}

void TerminalDataSource::command(const std::string& command)
{
    if (!command.starts_with("fmc/0/"))
    {
        return;
    }

    wchar_t key = 0;
    if (command.starts_with("fmc/0/key"))
    {
        if (command == "fmc/0/keyPlusMinus")
        {
            key = '-';
        }
        else if (command.starts_with("fmc/0/key"))
        {
            key = command.at(9);
            key = tolower(key);
        }
    }
    else if (command == "fmc/0/space")
    {
        key = ' ';
    }
    else if (command == "fmc/0/slash")
    {
        key = '/';
    }
    else if (command == "fmc/0/clear")
    {
        key = 8;
    }
    else if (command == "fmc/0/blank1")
    {
        key = '\n';
    }

    if (key > 0)
    {
        write(m_childIn, &key, 1);
    }
}

void TerminalDataSource::reset()
{
    for (int i = 1; i <= 14; i++)
    {
        setLine(i, wstring(24, L' '));
        setLine(i, wstring(24, L'w'), BufferType::TEXT_COLOUR);
        setLine(i, wstring(24, L'b'), BufferType::BACKGROUND_COLOUR);
    }
    m_curRow = 1;
    m_curCol = 1;
}

std::wstring& TerminalDataSource::getLine(int row, BufferType type)
{
    auto& buffer = getBuffer(type);
    wstring& line = buffer[row - 1];
    if (line.length() < 24)
    {
        line += wstring(24 - line.length(), L' ');
    }

    return line;
}

void TerminalDataSource::setLine(int row, wstring line, BufferType type)
{
    auto& buffer = getBuffer(type);
    buffer[row - 1] = line;
}

wchar_t TerminalDataSource::getChar(int row, int col, BufferType type)
{
    auto& line = getLine(row, type);
    return line[col - 1];
}

void TerminalDataSource::setChar(
    std::shared_ptr<AircraftState> &state,
    int row,
    int col,
    wchar_t c,
    BufferType type)
{
    auto& line = getLine(row, type);
    line[col - 1] = c;
}
