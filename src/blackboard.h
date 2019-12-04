
#include <map>
#include <mutex>
#include <string>

#include "TypedData.h"

struct Blackboard
{
    std::mutex bb_mutex;
    int next_id = 1;
    std::map<int, TypedData*> values;
};

TypedData* blackboard_get(Blackboard* b, int id)
{
    if (!b)
        return 0;

    std::lock_guard<std::mutex> lock(b->bb_mutex);
    auto it = b->values.find(id);
    if (it == b->values.end())
        return {};

    auto r = it->second;
    b->values.erase(it);
    return r;
}

int blackboard_new_entry(Blackboard* b, TypedData* d)
{
    if (!b)
        return 0;

    std::lock_guard<std::mutex> lock(b->bb_mutex);
    b->values[b->next_id++] = d;
    return b->next_id - 1;
}
