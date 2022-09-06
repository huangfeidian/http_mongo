#include <server.h>
#include <nlohmann/json.hpp>
#include <spdlog/fmt/ostr.h>
#include <bsoncxx/json.hpp>

using namespace spiritsaway;


using namespace spiritsaway::http_mongo;
using namespace spiritsaway::http_mongo::server;
using json = nlohmann::json;


bool mongo_server::from_json_to_bson_str(std::string& data) const
{
	try
	{
		auto cur_bson = bsoncxx::from_json(data);
		auto cur_view = cur_bson.view();
		data = std::string(reinterpret_cast<const char*>(cur_view.data()), cur_view.length());
		return true;
	}
	catch (std::exception& e)
	{
		m_logger->error("fail to convert json data {} to bson with err {}", data, e.what());
		return false;
	}

}
std::string mongo_server::check_request(const http_utils::request& cur_req, std::shared_ptr<task_desc::base_task> cur_task) const
{

	if (cur_req.uri.find("/mongo/post/") != 0)
	{
		return "request target must start with /mongo/post/";
	}
	if (!json::accept(cur_req.body))
	{
		//logger->info("request is {}", req_);
		return "body must be json";
	}
	auto json_body = json::parse(cur_req.body);
	if (!json_body.is_object())
	{
		return "body must be json object";

	}
	task_desc::base_task cur_base;
	std::string json_convert_error;
	std::string bson_convert_error;
	json_convert_error = cur_base.from_json(json_body);
	if (!json_convert_error.empty())
	{
		return "cant get base task from request: " + json_convert_error;
	}
	switch (cur_base.op_type())
	{
	case task_desc::task_op::invalid:
		return "invalid task op";
	case task_desc::task_op::find_one:
	case task_desc::task_op::find_multi:
	{
		auto cur_find_task = std::make_shared<task_desc::find_task>(cur_base);
		json_convert_error = cur_find_task->from_json(json_body);
		if (!json_convert_error.empty())
		{
			return "cant construct find_task from request: " + json_convert_error;
		}

		cur_task = cur_find_task;
		return "";
	}
	case task_desc::task_op::insert_one:
	case task_desc::task_op::insert_multi:
	{
		auto cur_insert_task = std::make_shared<task_desc::insert_task>(cur_base);
		json_convert_error = cur_insert_task->from_json(json_body);
		if (!json_convert_error.empty())
		{
			return "cant construct cur_update_task from request: " + json_convert_error;
		}

		cur_task = cur_insert_task;
		return "";
	}
	case task_desc::task_op::update_one:
	case task_desc::task_op::update_multi:
	{
		auto cur_update_task = std::make_shared<task_desc::update_task>(cur_base);
		json_convert_error = cur_update_task->from_json(json_body);
		if (!json_convert_error.empty())
		{
			return "cant construct cur_update_task from request: " + json_convert_error;
		}

		cur_task = cur_update_task;
		return "";
	}
	case task_desc::task_op::delete_one:
	case task_desc::task_op::delete_multi:
	{
		auto cur_delete_task = std::make_shared<task_desc::delete_task>(cur_base);
		json_convert_error = cur_delete_task->from_json(json_body);
		if (!json_convert_error.empty())
		{
			return "cant construct cur_delete_task from request: " + json_convert_error;
		}
		cur_task = cur_delete_task;
		return "";
	}
	case task_desc::task_op::count:
	{
		auto cur_count_task = std::make_shared<task_desc::count_task>(cur_base);
		json_convert_error = cur_count_task->from_json(json_body);
		if (!json_convert_error.empty())
		{
			return "cant construct cur_count_task from request: " + json_convert_error;
		}

		cur_task = cur_count_task;
		return "";
	}
	case task_desc::task_op::modify_update:
	case task_desc::task_op::modify_delete:
	{
		auto cur_modify_task = std::make_shared<task_desc::modify_task>(cur_base);
		json_convert_error = cur_modify_task->from_json(json_body);
		if (!json_convert_error.empty())
		{
			return "cant construct cur_modify_task from request: " + json_convert_error;
		}

		cur_task = cur_modify_task;
		return "";
	}
	default:
		return "invalid task operation";
	}

	return "";
}
void mongo_server::handle_request(const http_utils::request& cur_req, http_utils::reply_handler rep_cb)
{
	std::shared_ptr<task_desc::base_task> cur_task_desc;
	std::string check_err = check_request(cur_req, cur_task_desc);
	if (!check_err.empty())
	{
		m_logger->debug("redis_session parse  body {} fail {}", cur_req.body, check_err);
		return;
	}


	auto cur_callback = std::make_shared<db_task::callback_t>([=](const task_desc::task_reply& reply)
		{
			return finish_task(cur_req, reply, rep_cb);
		});
	auto cur_db_task = std::make_shared<db_task>(cur_task_desc, cur_callback, m_logger);

	m_task_dest.add_task(cur_db_task);

}


void mongo_server::finish_task(const http_utils::request& cur_req, const task_desc::task_reply& reply, http_utils::reply_handler rep_cb)
{
	http_utils::reply cur_rep;
	cur_rep.status_code = 200;
	cur_rep.status_detail = "OK";
	cur_rep.content = reply.to_json().dump();
	cur_rep.headers.emplace_back(http_utils::header{ "ContentType", "text/json" });

	rep_cb(cur_rep);
}

mongo_server::mongo_server(asio::io_context& ioc
	, std::shared_ptr<spdlog::logger> in_logger, const std::string& address, const std::string& port, spiritsaway::concurrency::task_channels<db_task, true>& task_dest)
	: http_utils::http_server(ioc, in_logger, address, port)
	, m_task_dest(task_dest)
{

}