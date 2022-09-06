#pragma once

#include <task_channel/task_channel.h>
#include <spdlog/spdlog.h>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <mongocxx/client.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/instance.hpp>

#include "db_task.h"


namespace spiritsaway::http_mongo::server
{
	using logger_t = std::shared_ptr<spdlog::logger>;
	using namespace spiritsaway;
	struct mongo_config
	{
		std::string host;
		std::uint16_t port;
		std::string user;
		std::string passwd;
		std::string dbname;
	};

	class worker
	{
		const mongo_config m_config;
		logger_t logger;
		concurrency::task_channels<db_task, true>& m_task_source;
		db_task::channel_type pre_channel;
		std::uint32_t worker_id = 0;

		std::shared_ptr<mongocxx::client> m_client;
		std::shared_ptr<mongocxx::database> m_db;

		std::uint32_t retry_base_milliseconds = 100;
		std::uint32_t retry_max_fold = 16;
		std::uint32_t cur_retry_time = 1;
	public:
		worker(const mongo_config& config, concurrency::task_channels<db_task, true>& task_source, logger_t in_logger);
		worker(const worker& other) = delete;
		void run();
		void set_executor_id(std::uint32_t executor_id);
	protected:
		bool connect();
		bool poll();
		bool ping();
		bool should_reconnect(const std::string& error) const;
		bool should_retry(const std::string& error) const;

	public:
		virtual ~worker();
	};

}