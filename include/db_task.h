#pragma once
#include "task_desc.h"
#include <memory>
#include <chrono>
#include <spdlog/logger.h>
#include <mongocxx/database.hpp>

namespace spiritsaway::http_mongo
{
	using cost_time_t = std::chrono::time_point<std::chrono::steady_clock>;
	using logger_t = std::shared_ptr<spdlog::logger>;
	class db_task
	{
		std::shared_ptr<const task_desc::base_task> _task_desc;
		std::weak_ptr<task_desc::reply_callback_t> _callback;
		task_desc::task_reply _reply;
		const cost_time_t begin_time;
		cost_time_t run_begin_time;
		cost_time_t run_end_time;
		logger_t logger;
		std::uint32_t retry_base_milliseconds = 100;
		std::uint32_t retry_max_fold = 16;
		std::uint32_t cur_retry_time = 1;
	public:
		db_task(std::shared_ptr<const task_desc::base_task> in_task_desc,
			std::weak_ptr<task_desc::reply_callback_t> in_callback, logger_t in_logger);
		db_task(const db_task& other) = delete;
		void run(mongocxx::database& db);
		mongocxx::read_preference::read_mode read_mode(task_desc::read_prefer_mode in_read_mode) const;
		void finish();
	protected:
		void run_find_task(mongocxx::database& db);
		void run_count_task(mongocxx::database& db);
		void run_update_task(mongocxx::database& db);
		void run_find_and_modify_task(mongocxx::database& db);
		void run_delete_task(mongocxx::database& db);
		bool should_reconnect() const;
		bool should_retry() const;
		bool run_impl(mongocxx::database& db);

	};
}
