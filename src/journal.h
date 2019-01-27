#pragma once

#include "TypedData.h"
#include <functional>
#include <string>

struct JournalEntry
{
    JournalEntry() = default;

    JournalEntry(char const*const str, TypedData* d)
    : name(str), data(d)
    {
    }
    JournalEntry(const std::string& str, TypedData* d)
    : name(str), data(d)
    {
    }
    JournalEntry(const JournalEntry& rh) = delete;
    JournalEntry(JournalEntry&& rh) noexcept
    {
        std::swap(rh.name, name);
        std::swap(rh.data, data);
    }
    ~JournalEntry() { delete data; }

    JournalEntry& operator= (JournalEntry&& rh) noexcept
    {
        std::swap(rh.name, name);
        std::swap(rh.data, data);
        return *this;
    } 

    std::string name;
    TypedData* data = nullptr;
};

struct Journal
{
    struct Transaction
    {
        std::string name;
        std::function<void()> action;
        std::function<void()> undo;
    };

    void commit(Transaction&& e)
    {
        std::lock_guard<std::mutex> lock(records_mutex);

        // if index is in the middle of the undo/redo history, pop the undo history
        // to size.  A future version of Journal will instead branch.

        // increment to where the new transaction will be emplaced
        last_action_index++; 

        // discard any subsequent actions
        while (last_action_index > records.size())
            records.pop_back();
        records.emplace_back(std::move(e));
    }

    void undo()
    {
        if (last_action_index < 0)
            return;

        std::lock_guard<std::mutex> lock(records_mutex);
        records[last_action_index].undo();
        --last_action_index;
    }

    void redo()
    {
        if ((last_action_index + 1) < records.size())
        {
            ++last_action_index;
            records[last_action_index].action();
        }
    }

    int last_action_index = -1;
    std::mutex records_mutex;
    std::vector<Transaction> records;
};
