#include <atomic>
#include <fstream>
#include <iostream>
#include <vector>

#include <unistd.h>

#include <nlohmann/json.hpp>

#include "Ensure.h"
#include "HijackSignal.h"
#include "ModbusLogFetcher.h"

namespace {

using json = nlohmann::json;

constexpr auto SERVICE = "service";
constexpr auto ADDR = "addr";

void help(const char *argv0, const char *message = nullptr)
{
    if(message) std::cout << "WARNING: " << message << '\n';

    std::cout
        << argv0
        << " -a broker_address"
        << " -i input"
        << std::endl;
}

std::atomic<ModbusLogFetcher *> fetcher;

void dumpOnSignal(int signalNo)
{
    if(!fetcher) goto raise;

    fetcher.load()->dump(std::cout);
raise:
    if(SIGUSR1 != signalNo) ::raise(signalNo);

    if(
        SIGSTOP == signalNo
        || SIGTSTP == signalNo
        || SIGUSR1 == signalNo)
    {
        hijackSignal(signalNo, dumpOnSignal);
    }
}

} /* namespace */

int main(int argc, char *const argv[])
{
    std::string brokerAddress;
    std::string input;

    for(int c; -1 != (c = ::getopt(argc, argv, "ha:i:"));)
    {
        switch(c)
        {
            case 'h':
                help(argv[0]);
                return EXIT_SUCCESS;
                break;
            case 'a':
                brokerAddress = optarg ? optarg : "";
                break;
            case 'i':
                input = optarg ? optarg : "";
                break;
            case ':':
            case '?':
            default:
                help(argv[0], "geopt() failure");
                return EXIT_FAILURE;
                break;
        }
    }

    if(
        brokerAddress.empty()
        || input.empty())
    {
        help(argv[0], "missing/invalid required arguments");
        return EXIT_FAILURE;
    }

    try
    {
        json cfg;
        std::ifstream{input} >> cfg;

        ENSURE(cfg.is_array(), RuntimeError);

        hijackSignal(SIGINT, dumpOnSignal);
        hijackSignal(SIGSTOP, dumpOnSignal);
        hijackSignal(SIGTERM, dumpOnSignal);
        hijackSignal(SIGTSTP, dumpOnSignal);
        hijackSignal(SIGUSR1, dumpOnSignal);

        ModbusLogFetcher modbusLogFetcher(brokerAddress);
        fetcher = &modbusLogFetcher;

        for(const auto &i : cfg)
        {
            ENSURE(i.count(SERVICE), RuntimeError);
            ENSURE(i[SERVICE].is_string(), RuntimeError);
            ENSURE(i.count(ADDR), RuntimeError);
            ENSURE(i[ADDR].is_array(), RuntimeError);

            modbusLogFetcher.monitor(
                i[SERVICE].get<std::string>(),
                i[ADDR].get<std::vector<int>>());
        }

        modbusLogFetcher.exec();
        fetcher = nullptr;
    }
    catch(const std::exception &except)
    {
        std::cerr << "std exception " << except.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch(...)
    {
        std::cerr << "unsupported exception" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
