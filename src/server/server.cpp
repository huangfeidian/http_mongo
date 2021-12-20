#include <server.h>
#include <nlohmann/json.hpp>
#include <spdlog/fmt/ostr.h>
#include <bsoncxx/json.hpp>

using namespace spiritsaway;


using namespace spiritsaway::http_mongo;
using namespace spiritsaway::http_mongo::server;
using json = nlohmann::json;

mongo_session::mongo_session(tcp::socket&& socket,
	logger_t in_logger,
	std::uint32_t in_expire_time,
	concurrency::task_channels<db_task>& in_task_dest)
	: http_utils::server::session(std::move(socket), std::move(in_logger), in_expire_time)
	, _task_dest(in_task_dest)
{

}
bool mongo_session::from_json_to_bson_str(std::string& data)
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
		return false;
	}

}
std::string mongo_session::check_request()
{
	auto check_error = http_utils::server::session::check_request();
	if (!check_error.empty())
	{
		return check_error;
	}
	if (!req_.target().starts_with("/mongo/post/"))
	{
		return "request target must start with /mongo/post/";
	}
	if (!json::accept(req_.body()))
	{
		//logger->info("request is {}", req_);
		return "body must be json";
	}
	auto json_body = json::parse(req_.body());
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
void mongo_session::route_request()
{
	logger->debug("mongo_session accept new request {} header {}", request_id, cur_task->debug_info());

	auto self = std::dynamic_pointer_cast<mongo_session>(shared_from_this());
	auto cur_task_lambda = [=](const task_desc::task_reply& reply)
	{
		return self->finish_task(reply);
	};
	callback = std::make_shared<db_task::callback_t>(cur_task_lambda);
	auto cur_db_task = std::make_shared<db_task>(cur_task, callback, logger);
	beast::get_lowest_layer(stream_).expires_after(
		std::chrono::seconds(expire_time));
	expire_timer = std::make_shared<boost::asio::steady_timer>(stream_.get_executor(), std::chrono::seconds(expire_time / 2));
	expire_timer->async_wait([self](const boost::system::error_code& e) {
		self->on_timeout(e);
	});
	_task_dest.add_task(cur_db_task);

}
void mongo_session::on_timeout(const boost::system::error_code& e)
{
	if (e == boost::asio::error::operation_aborted)
	{
		return;
	}
	expire_timer.reset();
	callback.reset();

	do_write(http_utils::common::create_response::bad_request("timeout", req_));
}

void mongo_session::finish_task(const task_desc::task_reply& reply)
{
	expire_timer.reset();
	callback.reset();
	logger->debug("finish task {}", request_id);
	http::response<http::string_body> res{ http::status::ok, req_.version() };
	res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
	res.set(http::field::content_type, "text/html");
	res.keep_alive(req_.keep_alive());
	res.body() = reply.to_json().dump();
	res.prepare_payload();
	do_write(std::move(res));
}

mongo_listener::mongo_listener(net::io_context& ioc,
	tcp::endpoint endpoint,
	logger_t in_logger,
	std::uint32_t expire_time,
	concurrency::task_channels<db_task>& task_dest)
	: http_utils::server::listener(ioc, std::move(endpoint), std::move(in_logger), expire_time)
	, _task_dest(task_dest)
{

}

std::shared_ptr<http_utils::server::session> mongo_listener::make_session(tcp::socket&& socket)
{
	return std::make_shared<mongo_session>(std::move(socket), logger, expire_time, _task_dest);
}