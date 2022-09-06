#include <worker.h>
#include <spdlog/fmt/fmt.h>
#include <bsoncxx/builder/stream/document.hpp>

using namespace spiritsaway;
using namespace spiritsaway::http_mongo::server;
worker::worker(const mongo_config& config,
	concurrency::task_channels<db_task, true>& task_source,
	logger_t in_logger)
	: m_config(config)
	, m_task_source(task_source)
	, logger(in_logger)

{

}

bool worker::connect()
{
	m_db.reset();
	m_client.reset();
	std::string uri_str;

	try
	{
		if (m_config.user.empty())
		{
			uri_str = fmt::format("mongodb://{}:{}/{}", m_config.host, m_config.port, m_config.dbname);
		}
		else
		{
			uri_str = fmt::format("mongodb://{}:{}@{}:{}/?authSource={}", m_config.user, m_config.passwd, m_config.host, m_config.port, m_config.dbname);
		}
		mongocxx::uri uri(uri_str);

		m_client = std::make_shared<mongocxx::client>(uri);
		m_db = std::make_shared<mongocxx::database>(m_client->database(m_config.dbname));
		return true;
	}
	catch (std::exception& e)
	{
		logger->error("failt to make client for {} error is {}", uri_str, e.what());
		return false;
	}


}
bool worker::ping()
{
	try
	{
		m_db->run_command(bsoncxx::builder::stream::document{} <<
			"ping" << 1 << bsoncxx::builder::stream::finalize);
		return true;
	}
	catch (std::exception& e)
	{
		logger->error("ping with error {}", e.what());
		return false;
	}
}
void worker::run()
{
	while (true)
	{
		while (!connect())
		{
			logger->warn("connect to db {} host {} port {} with user {} passwd {} fail, wait 1 second", m_config.dbname, m_config.host, m_config.port, m_config.user, m_config.passwd);
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		logger->warn("connect to db {} host {} port {} with user {} passwd {} suc", m_config.dbname, m_config.host, m_config.port, m_config.user, m_config.passwd);
		while (true)
		{
			auto temp_result = poll();
			if (!temp_result)
			{
				break;
			}
		}
	}
}

bool worker::should_reconnect(const std::string& error) const
{
	if (error.find("socket error or timeout") != std::string::npos)
	{
		return true;
	}
	if (error.find("connection refused calling ismaster") != std::string::npos)
	{
		return true;
	}
	return false;
}
bool worker::should_retry(const std::string& error) const
{
	const static std::vector<std::string> retry_errors = {
		"no master found",
		"error querying server",
		"transport error",
		"could not contact primary",
		"write results unavailable",
		"could not enforce write concern",
		"not master",
		"interrupted at shutdown",
		"No suitable servers found",
		"could not find host matching read preference",
		"unable to target",
		"waiting for replication timed out",
		"can't connect to new replica set master",
		"socket exception",
		"Couldn't get a connection within the time limit",
		"Operation timed out",
	};
	for (const auto& one_err : retry_errors)
	{
		if (error.find(one_err) != std::string::npos)
		{
			return true;
		}
	}
	return false;
}

bool worker::poll()
{
	auto cur_task = m_task_source.poll_one_task(pre_channel, worker_id);
	if (!cur_task)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		if (!ping())
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	try
	{
		cur_task->run(*m_db);
		pre_channel = cur_task->channel_id();
		cur_retry_time = 1;
		cur_task->finish("");
		return true;
	}
	catch (std::exception& e)
	{
		std::string cur_err = e.what();
		if (should_reconnect(cur_err))
		{
			logger->error("db error: {} reconnect for task {} ", cur_err, cur_task->task_desc()->debug_info());
			m_task_source.add_task(cur_task, true);
			pre_channel = "";
			return false;
		}
		if (should_retry(cur_err))
		{
			m_task_source.add_task(cur_task, true);
			logger->error("db error: {} retry for task {} ", cur_err, cur_task->task_desc()->debug_info());
			pre_channel = "";
			cur_retry_time = std::min(retry_max_fold, cur_retry_time * 2);
			std::this_thread::sleep_for(std::chrono::milliseconds(cur_retry_time * retry_base_milliseconds));
			return true;
		}
		cur_task->finish(cur_err);
		return true;
	}

}
worker::~worker()
{

}