#pragma once

#include <atomic>
#include <chrono>
#include <map>
#include <string>
#include <vector>

using Clock = std::chrono::system_clock;

class ModbusLogFetcher
{
public:
    struct DevInfo
    {
        struct Record
        {
            std::string data;
            Clock::time_point timestamp;

            friend
            std::ostream &operator<< (std::ostream &os, const Record &);
        };

        using RecordSeq = std::vector<Record>;

        int addr;
        RecordSeq recordSeq;

        friend
        std::ostream &operator<< (std::ostream &os, const DevInfo &);

        DevInfo(int i): addr{i}
        {}
    };

    using DevInfoSeq = std::vector<DevInfo>;

    std::map<std::string, DevInfoSeq> data_;
private:
    std::string brokerAddr_;
   // std::atomic<bool> stopMonitor_{false};
    std::atomic<bool> stopExec_{false};

    void dispatch(const std::string &);
public:
    ModbusLogFetcher(const std::string &brokerAddr);
    void monitor(const std::string &, const std::vector<int> &);
    void exec();
    void dump(std::ostream &);
};
