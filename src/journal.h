#pragma once

#include "TypedData.h"
#include <string>

struct JournalEntry
{
    JournalEntry(char const*const str, TypedData* d)
    : name(str), data(d)
    {
    }
    JournalEntry(const std::string& str, TypedData* d)
    : name(str), data(d)
    {
    }
    JournalEntry(JournalEntry&& rh) noexcept
    {
        std::swap(rh.name, name);
        std::swap(rh.data, data);
    }
    ~JournalEntry() { delete data; }
    std::string name;
    TypedData* data = nullptr;
};
