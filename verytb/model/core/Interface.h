#pragma once
// direct include
// C system headers
// C++ standard library headers
#include <memory>
#include <type_traits>
#include <utility>
// Other libraries' .h files.
#include <spdlog/spdlog.h>
// Your project's .h files.

namespace verytb::model {

class Interface {};

template <typename DataType>
class ValidReadyOut : public Interface {
public:
    virtual bool can_write() = 0;
    virtual void write(const DataType &&data) = 0;
    virtual void write(const std::unique_ptr<DataType> &data) = 0;
};

template <typename DataType>
class ValidReadyIn : public Interface {
public:
    virtual bool can_read() = 0;
    virtual DataType read() = 0;
};

template <typename DataType>
class ValidReady : public ValidReadyOut<DataType>, public ValidReadyIn<DataType> {
    std::optional<DataType> buf;

    bool can_write() override {
        return not buf.has_value();
    }

    bool can_read() override {
        return buf.has_value();
    }

    void write(const DataType &&data) override {
        buf = data;
    }

    void write(const std::unique_ptr<DataType> &data) override {
        buf = *data;
    }

    DataType read() override {
        DataType ret = *buf;
        buf.reset();
        return ret;
    }
};

} // namespace verytb::model
