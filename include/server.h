#pragma once

#include "db_task.h"

#include "worker.h"

#include <http_utils/http_server.h>

#include <nlohmann/json.hpp>

namespace spiritsaway::http_mongo::server
{
	using namespace spiritsaway;
	using json = nlohmann::json;
	namespace beast = boost::beast;         // from <boost/beast.hpp>
	namespace http = beast::http;           // from <boost/beast/http.hpp>
	namespace net = boost::asio;            // from <boost/asio.hpp>
	using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
	using logger_t = std::shared_ptr<spdlog::logger>;


	class mongo_session : public http_utils::server::session
	{
		concurrency::task_channels<db_task>& _task_dest;
		std::string request_id;
		std::vector<std::string> mongo_cmds;
		std::string channel;
		std::string check_request() override;
		void route_request() override;
		void finish_task(const task_desc::task_reply& reply);
		std::shared_ptr<db_task::callback_t> callback;
		std::shared_ptr<task_desc::base_task> cur_task;
		std::shared_ptr<boost::asio::steady_timer> expire_timer;
		void on_timeout(const boost::system::error_code& e);
		static bool from_json_to_bson_str(std::string& data);

	public:
		mongo_session(tcp::socket&& socket,
			logger_t in_logger,
			std::uint32_t in_expire_time,
			concurrency::task_channels<db_task>& in_task_dest);

	};

	class mongo_listener : public http_utils::server::listener
	{
	protected:
		concurrency::task_channels<db_task>& _task_dest;

	public:
		mongo_listener(net::io_context& ioc,
			tcp::endpoint endpoint,
			logger_t in_logger,
			std::uint32_t expire_time,
			concurrency::task_channels<db_task>& task_dest);

		std::shared_ptr<http_utils::server::session> make_session(tcp::socket&& socket) override;
	};
}