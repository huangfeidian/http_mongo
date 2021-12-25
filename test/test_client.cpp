#include <task_desc.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/fmt/ostr.h>
#include <http_utils/http_client.h>

using namespace spiritsaway::http_mongo::task_desc;
using namespace spiritsaway;
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
using json = nlohmann::json;
using logger_t = std::shared_ptr<spdlog::logger>;


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
std::vector<std::string> create_cmds()
{
	std::vector<std::string> result;
	base_task _base(task_op::invalid, "test", 111, "test");
	json::object_t query;
	query["hehe"] = 1;

	json::object_t doc_impl;
	doc_impl["hehe"] = 1;
	doc_impl["haha"] = 2;
	json::object_t doc;
	doc["$set"] = doc_impl;

	auto task_1 = find_task::find_one(_base, query);
	result.push_back(task_1->to_json().dump());

	auto task_2 = find_task::find_multi(_base, query, 2);
	result.push_back(task_2->to_json().dump());

	auto task_3 = delete_task::delete_multi(_base, query);
	result.push_back(task_3->to_json().dump());

	auto task_4 = update_task::update_one(_base, query, doc, false);
	result.push_back(task_4->to_json().dump());
	
	auto task_5 = update_task::update_multi(_base, query, doc);
	result.push_back(task_5->to_json().dump());

	auto task_6 = update_task::update_one(_base, query, doc, true);
	result.push_back(task_6->to_json().dump());

	auto task_7 = modify_task::modify_one(_base, query, doc, true, true);
	result.push_back(task_7->to_json().dump());
	auto task_8 = count_task::count(_base, query);
	result.push_back(task_8->to_json().dump());
	
	auto task_9 = modify_task::delete_one(_base, query);
	result.push_back(task_9->to_json().dump());

	return result;

}
void make_request(net::io_context& ioc, logger_t cur_logger,
	const std::string& host, const std::uint16_t& port, const std::uint32_t& expire_time,
	std::uint32_t& next_cmd_idx, const std::vector<std::string>& cmds, std::shared_ptr< http_utils::common::callback_t>& pre_callback)
{
	if (next_cmd_idx >= cmds.size())
	{
		return;
	}
	http_utils::common::request_data cur_request;
	cur_request.host = "127.0.0.1";
	cur_request.port = "8090";
	cur_request.target = "/mongo/post/";
	cur_request.version = http_utils::common::http_version::v1_1;
	cur_request.method = http::verb::post;
	
	cur_request.data = cmds[next_cmd_idx];
	auto result_lambda = [&, cur_logger](http_utils::common::error_pos ec, const http::response<http::string_body>& response) mutable
	{
		if (ec != http_utils::common::error_pos::ok)
		{
			cur_logger->info("request error {}", magic_enum::enum_name(ec));
		}
		cur_logger->info("cmd {} reply {}", cmds[next_cmd_idx - 1], response.body());
		make_request(ioc, cur_logger, host, port, expire_time, next_cmd_idx, cmds, pre_callback);
	};
	auto cur_callback = std::make_shared<http_utils::common::callback_t>(result_lambda);
	pre_callback = cur_callback;
	auto cur_session = std::make_shared<http_utils::client::session>(ioc, cur_request, cur_logger, cur_callback, expire_time);

	cur_logger->info("make request index {} content {}", next_cmd_idx, cmds[next_cmd_idx]);
	next_cmd_idx++;

	cur_session->run();
}

int main(int argc, const char** argv)
{
	std::string argv_info = "args format: host port expire_time";
	//if (argc != 4)
	//{
	//	std::cout << argv_info << std::endl;
	//	return 0;
	//}
	std::string host;
	std::uint16_t port;
	std::uint32_t expire_time;
	//host = std::string(argv[1]);
	//port = std::stoi(argv[2]);
	//expire_time = std::stoi(argv[3]);
	host = "127.0.0.1";
	port = 8080;
	expire_time = 10;
	std::vector<std::string> detail_cmds = create_cmds();

	net::io_context ioc;
	auto cur_logger = create_logger("mongo_client");
	std::shared_ptr< http_utils::common::callback_t> client_callback;
	std::uint32_t next_cmd_idx = 0;
	make_request(ioc, cur_logger, host, port, expire_time, next_cmd_idx, detail_cmds, client_callback);
	ioc.run();

}