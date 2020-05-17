
#include <server.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/fmt/ostr.h>
using namespace spiritsaway::http_mongo::server;
using namespace http_utils;



std::shared_ptr<spdlog::logger> create_logger(const std::string& name)
{
	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	console_sink->set_level(spdlog::level::debug);
	std::string pattern = "[thread %t] %+";
	console_sink->set_pattern(pattern);

	auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(name + ".log", true);
	file_sink->set_level(spdlog::level::trace);
	file_sink->set_pattern(pattern);
	auto logger = std::make_shared<spdlog::logger>(name, spdlog::sinks_init_list{ console_sink, file_sink });
	logger->set_level(spdlog::level::trace);
	return logger;
}

int main()
{
	std::string db_host = "192.168.1.160";
	std::string db_user = "test";
	std::string db_passwd = "test";
	std::string db_name = "test_db";
	std::uint16_t db_port = 27017;
	std::uint16_t listen_port = 8090;
	std::string address = "0.0.0.0";
	std::uint32_t task_worker_num = 2;
	std::uint32_t io_worker_num = 2;
	std::uint32_t expire_time = 10;
	mongo_config _db_config;
	_db_config.dbname = db_name;
	_db_config.host = db_host;
	_db_config.passwd = db_passwd;
	_db_config.user = db_user;
	_db_config.port = db_port;
	auto listen_address = net::ip::make_address(address);
	mongocxx::instance mongo_instance;
	net::io_context ioc(static_cast<int>(io_worker_num));
	concurrency::task_channels<db_task> task_queue;
	auto cur_logger = create_logger("mongo_http_server");
	auto cur_listener = std::make_shared<mongo_listener>(ioc,
		tcp::endpoint(listen_address, listen_port), cur_logger, expire_time, task_queue);
	cur_listener->run();
	std::vector<std::thread> io_worker_threads;
	std::vector<std::thread> task_worker_threads;
	std::vector<std::shared_ptr<worker>> task_workers;
	for (int i = 0; i < task_worker_num; i++)
	{
		auto cur_task_worker = std::make_shared<worker>(_db_config, task_queue, cur_logger);
		task_workers.push_back(cur_task_worker);
		task_worker_threads.emplace_back([cur_task_worker]()
		{
			cur_task_worker->run();
		});
	}
	for (int i = 0; i + 1 < io_worker_num; i++)
	{
		io_worker_threads.emplace_back([&ioc]()
		{
			ioc.run();
		});
	}
	ioc.run();

}