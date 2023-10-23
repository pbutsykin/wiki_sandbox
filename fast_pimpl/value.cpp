#include <memory>
#include <iostream>

#include "value.hpp"
#include "third_party.hpp"

using namespace thirdparty::super_havy;

Value::Value() : _data(10) {}
Value::Value(Value&& other) = default;
Value::Value(const Value& other) = default;
Value& Value::operator=(Value&& other) = default;
Value& Value::operator=(const Value& other) = default;
Value::~Value() = default;

size_t Value::Size() const {
    return _data->size();
}
