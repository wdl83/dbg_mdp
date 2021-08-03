#include <ctime>

#include "Ensure.h"
#include "ModbusLogFetcher.h"
#include "Trace.h"
#include "mdp/Client.h"
#include "mdp/MDP.h"

#include "tlog.h"

using json = nlohmann::json;

namespace {

struct StopGuard
{
    std::atomic<bool> &stop_;

    StopGuard(std::atomic<bool> &stop): stop_{stop} {}
    ~StopGuard() {stop_ =true;}
};

const char *const FCODE = "fcode";

constexpr const uint8_t FCODE_USER1_OFFSET = 65;
constexpr const uint8_t FCODE_RD_BYTES = FCODE_USER1_OFFSET + 0;

}

ModbusLogFetcher::ModbusLogFetcher(const std::string &brokerAddr):
    brokerAddr_{std::move(brokerAddr)}
{}

void ModbusLogFetcher::dispatch(const std::string &service)
{
    TRACE(TraceLevel::Debug, service);

    auto &seq = data_[service];

    for(auto &info : seq)
    {
        json payload
        {
            // tlog
            {
                {tlog::SLAVE, info.addr},
                {FCODE, FCODE_RD_BYTES},
                {tlog::ADDR, 4648},
                {tlog::COUNT, 249}
            },
            {
                {tlog::SLAVE, info.addr},
                {FCODE, FCODE_RD_BYTES},
                {tlog::ADDR, 4897},
                {tlog::COUNT, 7}
            }
        };

        Client client;

        const auto replyPayload =
            client.exec(brokerAddr_, service, {payload.dump()});

        ENSURE(2 == int(replyPayload.size()), RuntimeError);
        ENSURE(MDP::Broker::Signature::statusSucess == replyPayload[0], RuntimeError);

        const auto reply = json::parse(replyPayload.back());

        ENSURE(reply.is_array(), RuntimeError);
        ENSURE(2 == reply.size(), RuntimeError);
        ENSURE(reply[0].count(tlog::SLAVE), RuntimeError);
        ENSURE(info.addr == reply[0][tlog::SLAVE], RuntimeError);
        ENSURE(reply[1].count(tlog::SLAVE), RuntimeError);
        ENSURE(info.addr == reply[1][tlog::SLAVE], RuntimeError);

        auto data = tlog::toStr(reply[0]);
        data += tlog::toStr(reply[1]);

        if(!info.recordSeq.empty() && info.recordSeq.back().data == data)
        {
            info.recordSeq.back().timestamp = Clock::now();
            TRACE(TraceLevel::Debug, service, ", addr ", info.addr, ", update skipped");
            continue;
        }

        TRACE(TraceLevel::Debug, service, ", addr ", info.addr, ", data\n", data);

        info.recordSeq.push_back({std::move(data), Clock::now()});
    }
}

void ModbusLogFetcher::monitor(const std::string &service, const std::vector<int> &devices)
{
    auto &seq = data_[service];


    for(auto addr : devices)
    {
        auto match = [addr](const DevInfo &info) {return info.addr == addr;};

        if(std::end(seq) == std::find_if(std::begin(seq), std::end(seq), match))
        {
            seq.emplace_back(addr);
            TRACE(TraceLevel::Info, service, ", addr ", addr);
        }
        else
        {
            TRACE(TraceLevel::Warning, service, ", addr ", addr, ", skipped");
        }
    }
}

void ModbusLogFetcher::exec()
{
    while(!stopExec_)
    {
        try
        {
            std::this_thread::sleep_for(std::chrono::seconds{10});

            for(const auto &i : data_)
            {
                dispatch(i.first);
            }
        }
        catch(const std::exception &except)
        {
            TRACE(TraceLevel::Error, except.what());
        }
        catch(...)
        {
            TRACE(TraceLevel::Error, "unsupported exception");
            stopExec_ = true;
        }
    }
}

void formatTimestamp(Clock::time_point timestamp, std::ostream &os)
{
    const auto time = Clock::to_time_t(timestamp);

    os << std::put_time(std::localtime(&time), "%C/%m/%d %X");
}

void ModbusLogFetcher::dump(std::ostream &os)
{
    for(const auto &i : data_)
    {
        const auto &service = i.first;
        const auto &seq = i.second;

        for(const auto &info : seq)
        {
            const auto addr = info.addr;

            for(const auto &record : info.recordSeq)
            {
                formatTimestamp(record.timestamp, os);

                os << ' ' << service << ' ' << addr << ' ';
                os << '\n';
                os << record.data;
            }
        }
    }
}
