#pragma once

#include "log.hpp"

#include <chrono>
#include <cstdlib>
#include <format>
#include <fstream>
#include <iostream>
#include <list>
#include <string>

class IPrintable {
  public:
    virtual void write(const std::string &) = 0;
    virtual ~IPrintable() {}
};

class PrintComposite : public IPrintable {
  protected:
    std::list<std::shared_ptr<IPrintable>> children_;

  public:
    ~PrintComposite() {
        otus::Log::Get().Debug("dtor {}", otus::get_type_name<decltype(this)>());
    }

    void Add(std::shared_ptr<IPrintable> component) {
        children_.emplace_back(std::move(component));
    }

    void Remove(std::shared_ptr<IPrintable> component) {
        children_.remove(component);
    }

    void write(const std::string &data) final {
        for (auto &&c : children_) {
            c->write(data);
        }
    }
};

class ConsolePrint : public IPrintable {
    public:
    void write(const std::string &data) final {
        std::cout << data;
    };
};

class FilePrint : public IPrintable {
    public:
    void write(const std::string &data) final {
        using namespace std::chrono;
        const auto stamp = current_zone()->to_local(system_clock::now());
        const auto unixStamp =
            duration_cast<seconds>(stamp.time_since_epoch()).count();
        std::string fileName =
            std::format("bulk{}.log", std::to_string(unixStamp));

        std::ofstream file(fileName);
        file << data;
        file.close();
    };
};
