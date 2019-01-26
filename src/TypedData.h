#pragma once

#include <typeindex>
#include <sstream>
#include <string>

class TypedData 
{
public:
    TypedData() : type(typeid(TypedData)) { }
    TypedData(std::type_index t) : type(t) { }
    virtual ~TypedData() { }

    virtual void copy(const TypedData*) = 0;
    virtual TypedData* clone() = 0;
    virtual std::string to_string() = 0;

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

    virtual void copy(const TypedData* rhs) override 
    {
        if (type == rhs->type) 
        {
            const Data* rhsData = reinterpret_cast<const Data*>(rhs);
            _data = rhsData->_data;
        }
    }

    virtual TypedData* clone() override
    {
        Data* result = new Data();
        result->_data = _data;
        return result;
    }

    virtual std::string to_string() override
    {
        std::stringstream str;
        str << _data;
        return str.str();
    }

private:
    T _data;
};

