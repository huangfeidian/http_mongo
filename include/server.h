#pragma once

#include "db_task.h"

#include "worker.h"

#include <http_utils/http_server.h>

#include <nlohmann/json.hpp>

namespace spiritsaway::http_mongo::server
{


	class mongo_server : public http_utils::http_server
	{
	public:
		mongo_server(asio::io_context& ioc
			, std::shared_ptr<spdlog::logger> in_logger, const std::string& address, const std::string& port, spiritsaway::concurrency::task_channels<db_task, true>& task_dest);
		bool from_json_to_bson_str(std::string& data) const;
	protected:
		void handle_request(const http_utils::request& req, http_utils::reply_handler rep_cb) override;
		std::string check_request(const http_utils::request& cur_req, std::shared_ptr<task_desc::base_task> cur_task) const;
		void finish_task(const http_utils::request& cur_req, const task_desc::task_reply& reply, http_utils::reply_handler rep_cb);
	protected:
		concurrency::task_channels<db_task, true>& m_task_dest;
	};
}