//
// Created by Ian Parker on 29/04/2026.
//

#include "navdata.h"

#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/mman.h>

#include "ufc/utils/utils.h"

using namespace std;
using namespace UFC;

std::shared_ptr<NavFile> NavFile::open(const std::string& fileName)
{
    auto navFile = make_shared<NavFile>();

    navFile->fd = ::open(fileName.c_str(), O_RDONLY);
    if (navFile->fd == -1)
    {
        fprintf(stderr, "Error opening nav file %s\n", fileName.c_str());
        return nullptr;
    }

    navFile->size = lseek(navFile->fd, 0, SEEK_END);
    if (navFile->size == -1)
    {
        fprintf(stderr, "Error getting size of nav file %s\n", fileName.c_str());
        return nullptr;
    }

    navFile->mmapData = static_cast<char*>(mmap(nullptr, navFile->size, PROT_READ, MAP_SHARED, navFile->fd, 0));
    if (navFile->mmapData == MAP_FAILED)
    {
        fprintf(stderr, "Error mapping nav file %s\n", fileName.c_str());
        return nullptr;
    }

    navFile->ptr = navFile->mmapData;
    navFile->end = navFile->mmapData + navFile->size;

    return navFile;
}

void NavFile::close()
{
    munmap(mmapData, size);
    ::close(fd);

    mmapData = nullptr;
    ptr = nullptr;
    end = nullptr;
    size = 0;
    fd = -1;
}

std::wstring NavFile::readLine()
{
    string buffer;
    while (ptr < end)
    {
        auto c = *ptr++;
        if (c == '\r')
        {
            if (ptr < end)
            {
                char next = *ptr;
                if (next == '\n')
                {
                    ptr++;
                }
            }
            break;
        }
        if (c == '\n')
        {
            break;
        }

        buffer += c;
    }
    return utf82wstring(buffer.c_str());
    /*
    char lineBuffer[2048];
    char const* res = fgets(lineBuffer, 2048, fd);
    if (res == nullptr)
    {
        return L"";
    }

    auto len = (int)strnlen(lineBuffer, 2048);
    if (len == 0)
    {
        return L"";
    }
    while (len > 0 && (lineBuffer[len-1] == '\r' || lineBuffer[len-1] == '\n'))
    {
        lineBuffer[len - 1] = 0;
        len--;
    }

    if (len <= 0)
    {
        return L"";
    }

    return utf82wstring(lineBuffer);
    */
}
