
#include <map>
#include <mutex>
#include <string>
#include <typeindex>

class TypedData 
{
public:
    TypedData() : type(typeid(TypedData)) { }
    TypedData(std::type_index t) : type(t) { }
    virtual ~TypedData() { }

    virtual void copy(const TypedData*) = 0;

    const std::type_index type;
};

template <typename T>
class Data : public TypedData 
{
public:
    Data() : TypedData(typeid(T)) {}
    Data(const T& data) : TypedData(typeid(T)), _data(data) {}
    virtual ~Data() {}
    virtual const T& value() const { return _data; }
    virtual void setValue(const T& i) { _data = i; }

    virtual void copy(const TypedData* rhs) override {
        if (type == rhs->type) {
            const Data* rhsData = reinterpret_cast<const Data*>(rhs);
            _data = rhsData->_data;
        }
    }

private:
    T _data;
};


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
