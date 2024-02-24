#include "server.hpp"

#include <boost/program_options.hpp>

asio::awaitable<void>
Listener(uint16_t port, size_t size) {
    BulkContext context;
    auto printer = std::make_shared<PrintComposite>();
    printer->Add(std::make_shared<ThreadedPrintable<FilePrintFunctor, 2>>());
    printer->Add(std::make_shared<ThreadedPrintable<ConsolePrintFunctor>>());

    auto processor = std::make_shared<CommandProcessor>(size, printer);
    const auto executor = co_await asio::this_coro::executor;

    otus::Log::Get().Info("Listing TCP v4 port {}, bulk size {}", port, size);
    tcp::acceptor acceptor{executor, {tcp::v4(), port}};

    size_t hits = 0;
    for (;;) {
        auto socket = co_await acceptor.async_accept(asio::use_awaitable);
        otus::Log::Get().Info("New client conencted, hit no {}", ++hits);
        std::make_shared<BulkSession>(std::move(socket), context, processor,
                                      printer)
            ->start();
    }
}

void
Run(uint16_t port, size_t size) {
    otus::Log::Get().Info("Running server...");
    asio::io_context ctx;

    asio::signal_set signals{ctx, SIGINT, SIGTERM};
    signals.async_wait([&](auto, auto) { ctx.stop(); });

    asio::co_spawn(ctx, Listener(port, size), asio::detached);

    ctx.run();
    otus::Log::Get().Info("Server stopped.");
}

int
main(int argc, char *argv[]) {
    size_t size = 3;
    uint16_t port = 9000;
    otus::Log::Get().SetSeverity(otus::Log::INFO);

    namespace po = boost::program_options;
    po::options_description desc("Bulk asio server");
    desc.add_options()("help", "Produce this help message")(
        "size,s", po::value<size_t>(),
        "Bulk size")("port,p", po::value<uint16_t>(), "TCP Port")(
        "debug,d", po::value<bool>(), "Enable debug output");

    try {
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            desc.print(std::cout);
            return 1;
        }

        if (vm.count("debug")) {
            otus::Log::Get().SetSeverity(otus::Log::DEBUG);
        }

        if (vm.count("size") && vm.count("port")) {
            size = vm["size"].as<size_t>();
            port = vm["port"].as<uint16_t>();
        }
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
        desc.print(std::cout);
    }

    try {
        Run(port, size);
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
