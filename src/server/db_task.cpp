#include <db_task.h>
#include <bsoncxx/json.hpp>
using namespace spiritsaway::http_mongo::server;
using namespace spiritsaway::http_mongo;
using json = nlohmann::json;
db_task::db_task(std::shared_ptr<const task_desc::base_task> in_task_desc,
	std::weak_ptr<task_desc::reply_callback_t> in_callback, logger_t in_logger)
	: _task_desc(std::move(in_task_desc))
	, _callback(std::move(in_callback))
	, logger(std::move(in_logger))
	, begin_time(std::chrono::steady_clock::now())
{
}

mongocxx::read_preference::read_mode db_task::read_mode(task_desc::read_prefer_mode in_read_mode)const
{
	switch (in_read_mode)
	{
	case task_desc::read_prefer_mode::primary:
		return mongocxx::read_preference::read_mode::k_primary;
	case task_desc::read_prefer_mode::primary_prefer:
		return mongocxx::read_preference::read_mode::k_primary_preferred;
		break;
	case task_desc::read_prefer_mode::secondary:
		return mongocxx::read_preference::read_mode::k_secondary;
	case task_desc::read_prefer_mode::secondary_prefer:
		return mongocxx::read_preference::read_mode::k_secondary_preferred;
	case task_desc::read_prefer_mode::nearest:
		return mongocxx::read_preference::read_mode::k_nearest;
	default:
		return mongocxx::read_preference::read_mode::k_secondary;
	}
}


std::shared_ptr<const task_desc::base_task> db_task::task_desc() const
{
	return _task_desc;
}

void db_task::finish(const std::string& error)
{
	_reply.error = error;
	auto cur_callback = _callback.lock();
	_callback.reset();
	if (!cur_callback)
	{
		return;
	}
	cur_callback->operator()(_reply);
}

const std::string& db_task::channel_id() const
{
	return _task_desc->channel();
}
void db_task::run(mongocxx::database& db)
{
	run_begin_time = std::chrono::steady_clock::now();

	run_impl(db);
	run_end_time = std::chrono::steady_clock::now();
	auto milliseconds_cost = (run_end_time - run_begin_time).count() / (1000 * 1000);
	if (milliseconds_cost > 100)
	{
		logger->warn("db task: {} run cost {} ms", _task_desc->debug_info(), milliseconds_cost);
	}
	_reply.cost = milliseconds_cost;
	
}
void db_task::run_impl(mongocxx::database& db)
{
	switch (_task_desc->op_type())
	{
	case task_desc::task_op::find_one:
	case task_desc::task_op::find_multi:
	{
		run_find_task(db);
		break;
	}
	case task_desc::task_op::count:
	{
		run_count_task(db);
		break;
	}
	case task_desc::task_op::insert_one:
	case task_desc::task_op::insert_multi:
	{
		run_insert_task(db);
		break;
	}
	case task_desc::task_op::update_one:
	case task_desc::task_op::update_multi:
	{
		run_update_task(db);
		break;
	}
	case task_desc::task_op::delete_one:
	case task_desc::task_op::delete_multi:
	{
		run_delete_task(db);
		break;
	}
	case task_desc::task_op::modify_delete:
	case task_desc::task_op::modify_update:
	{
		run_modify_task(db);
		break;
	}

	default:
		_reply.error = "invalid op_type";
		logger->error("invalid op_type for task {}", _task_desc->debug_info());
		break;
	}

}
void db_task::run_find_task(mongocxx::database& db)
{
	auto cur_find_task = std::dynamic_pointer_cast<const task_desc::find_task>(_task_desc);
	if (!cur_find_task)
	{
		logger->error("fail to convert task to find_task detail is: {}", _task_desc->debug_info());
		return;
	}
	const auto& task_find_opt = cur_find_task->option();
	mongocxx::options::find opt;
	opt.limit(task_find_opt.limit);
	opt.skip(task_find_opt.skip);
	mongocxx::read_preference read_pre;
	read_pre.mode(read_mode(task_find_opt.read_prefer));
	opt.read_preference(std::move(read_pre));

	if (!task_find_opt.sort.is_null())
	{
		const auto& sort_str = task_find_opt.sort.dump();
		opt.sort(bsoncxx::from_json(bsoncxx::stdx::string_view(sort_str.data(), sort_str.size())));
	}
	if (!task_find_opt.fields.is_null())
	{
		const auto& fields_str = task_find_opt.fields.dump();
		opt.projection(bsoncxx::from_json(bsoncxx::stdx::string_view(fields_str.data(), fields_str.size())));
	}
	if (!task_find_opt.hint.is_null())
	{
		const auto& hint_str = task_find_opt.hint.dump();
		opt.hint(mongocxx::hint(bsoncxx::from_json(bsoncxx::stdx::string_view(hint_str.data(), hint_str.size()))));
	}
	const auto& query_str = cur_find_task->query().dump();

	mongocxx::cursor cursor = db[cur_find_task->collection()].find(bsoncxx::from_json(bsoncxx::stdx::string_view(query_str.data(), query_str.size())), opt);
	for (auto& one_doc: cursor)
	{
		_reply.content.push_back(bsoncxx::to_json(one_doc));
	}
	_reply.count = _reply.content.size();
}

void db_task::run_count_task(mongocxx::database& db)
{
	auto cur_count_task = std::dynamic_pointer_cast<const task_desc::count_task>(_task_desc);
	if (!cur_count_task)
	{
		logger->error("fail to convert task to count_task detail is: {}", _task_desc->debug_info());
		return;
	}
	const auto& task_count_opt = cur_count_task->option();

	mongocxx::options::count opt = mongocxx::options::count{};
	if (!task_count_opt.hint.is_null())
	{
		const auto& hint_str = task_count_opt.hint.dump();
		opt.hint(mongocxx::hint(bsoncxx::from_json(bsoncxx::stdx::string_view(hint_str.data(), hint_str.size()))));
	}
	const auto& query_str = cur_count_task->query().dump();

	_reply.count = db[cur_count_task->collection()].count_documents(bsoncxx::from_json(bsoncxx::stdx::string_view(query_str.data(), query_str.size())), opt);
}

void db_task::run_delete_task(mongocxx::database& db)
{
	auto cur_del_task = std::dynamic_pointer_cast<const task_desc::delete_task>(_task_desc);
	if (!cur_del_task)
	{
		logger->error("fail to convert task to delete_task detail is: {}", _task_desc->debug_info());
		return;
	}
	

	
	const auto& query_str = cur_del_task->query().dump();

	bsoncxx::stdx::optional<mongocxx::result::delete_result> cur_del_result;
	if (cur_del_task->is_limit_one())
	{
		cur_del_result = db[cur_del_task->collection()].delete_one(bsoncxx::from_json(bsoncxx::stdx::string_view(query_str.data(), query_str.size())));
	}
	else
	{
		cur_del_result = db[cur_del_task->collection()].delete_many(bsoncxx::from_json(bsoncxx::stdx::string_view(query_str.data(), query_str.size())));
	}
	if (cur_del_result)
	{
		const auto& real_result = cur_del_result.value();
		_reply.count = real_result.deleted_count();
	}
}
void db_task::run_insert_task(mongocxx::database& db)
{
	auto cur_insert_task = std::dynamic_pointer_cast<const task_desc::insert_task>(_task_desc);
	if (!cur_insert_task)
	{
		logger->error("fail to convert task to insert_task detail is: {}", _task_desc->debug_info());
		return;
	}
	mongocxx::options::insert opt = mongocxx::options::insert{};
	
	const auto& docs = cur_insert_task->docs();
	if (task_desc()->op_type() == task_desc::task_op::insert_one)
	{
		auto doc_str = docs[0].dump();
		bsoncxx::stdx::optional<mongocxx::result::insert_one> cur_insert_result;
		cur_insert_result = db[cur_insert_task->collection()].insert_one(bsoncxx::from_json(bsoncxx::stdx::string_view(doc_str.data(), doc_str.size())), opt);
		const auto& real_result = cur_insert_result.value();

		auto cur_inserted_id = real_result.inserted_id();
		if (cur_inserted_id.type() == bsoncxx::type::k_string)
		{
			_reply.content.push_back(std::string(cur_inserted_id.get_string().value));
			_reply.count = 1;
		}
		else if (cur_inserted_id.type() == bsoncxx::type::k_oid)
		{
			_reply.content.push_back(cur_inserted_id.get_oid().value.to_string());
			_reply.count = 1;
		}
		else
		{
			_reply.count = 0;
		}
	}
	else
	{
		std::vector<bsoncxx::document::value> doc_vals;
		doc_vals.reserve(docs.size());
		std::vector< bsoncxx::document::view> doc_views;
		doc_views.reserve(docs.size());
		for (auto& one_doc : docs)
		{
			auto one_doc_str = one_doc.dump();
			doc_vals.push_back(bsoncxx::from_json(bsoncxx::stdx::string_view(one_doc_str.data(), one_doc_str.size())));
		}
		for (const auto& one_doc : doc_vals)
		{
			doc_views.push_back(bsoncxx::document::view(one_doc));
		}
		bsoncxx::stdx::optional<mongocxx::result::insert_many> cur_insert_result;
		cur_insert_result = db[cur_insert_task->collection()].insert_many(doc_views, opt);
		const auto& real_result = cur_insert_result.value();

		_reply.count = real_result.inserted_count();
		for (const auto& one_insert_id : real_result.inserted_ids())
		{
			if (one_insert_id.second.type() == bsoncxx::type::k_string)
			{
				_reply.content.push_back(std::string(one_insert_id.second.get_string().value));
			}
			else if (one_insert_id.second.type() == bsoncxx::type::k_oid)
			{
				_reply.content.push_back(one_insert_id.second.get_oid().value.to_string());
			}
			else
			{
				_reply.content.push_back({});
			}
		}
	}
	
}

void db_task::run_update_task(mongocxx::database& db)
{
	auto cur_update_task = std::dynamic_pointer_cast<const task_desc::update_task>(_task_desc);
	if (!cur_update_task)
	{
		logger->error("fail to convert task to update_task detail is: {}", _task_desc->debug_info());
		return;
	}

	mongocxx::options::update opt = mongocxx::options::update{};
	opt.upsert(cur_update_task->is_upset());
	const auto& query_str = cur_update_task->query().dump();


	const auto& doc_str = cur_update_task->doc().dump();
	bsoncxx::stdx::optional<mongocxx::result::update> cur_update_result;

	if(cur_update_task->is_multi())
	{
		cur_update_result = db[cur_update_task->collection()].update_many(bsoncxx::from_json(bsoncxx::stdx::string_view(query_str.data(), query_str.size())), bsoncxx::from_json(bsoncxx::stdx::string_view(doc_str.data(), doc_str.size())), opt);
	}
	else
	{
		cur_update_result = db[cur_update_task->collection()].update_one(bsoncxx::from_json(bsoncxx::stdx::string_view(query_str.data(), query_str.size())), bsoncxx::from_json(bsoncxx::stdx::string_view(doc_str.data(), doc_str.size())), opt);
		
	}
	if (cur_update_result)
	{
		
		const auto& real_result = cur_update_result.value();

		_reply.count = real_result.modified_count();
		const auto& upset_id = real_result.upserted_id();
		if (_reply.count == 0 && upset_id)
		{
			_reply.count = 1;
		}
		

	}

}

void db_task::run_modify_task(mongocxx::database& db)
{
	auto cur_modify_task = std::dynamic_pointer_cast<const task_desc::modify_task>(_task_desc);
	if (!cur_modify_task)
	{
		logger->error("fail to convert task to modify_task detail is: {}", _task_desc->debug_info());
		return;
	}
	const auto& task_modify_opt = cur_modify_task->option();
	const auto& query_str = cur_modify_task->query().dump();
	auto query_doc = bsoncxx::from_json(bsoncxx::stdx::string_view(query_str.data(), query_str.size()));

	auto query_view = bsoncxx::document::view(query_doc);

	if (cur_modify_task->is_remove())
	{
		mongocxx::options::find_one_and_delete opt = mongocxx::options::find_one_and_delete{};
		if (!task_modify_opt.fields.is_null())
		{
			const auto& fields_str = task_modify_opt.fields.dump();
			opt.projection(bsoncxx::from_json(bsoncxx::stdx::string_view(fields_str.data(), fields_str.size())));
		}
		if (!task_modify_opt.sort.empty())
		{
			const auto& sort_str = task_modify_opt.sort.dump();
			opt.sort(bsoncxx::from_json(bsoncxx::stdx::string_view(sort_str.data(), sort_str.size())));
		}
		auto cur_result = db[cur_modify_task->collection()].find_one_and_delete(query_view, opt);
		if (cur_result)
		{
			_reply.content.push_back(bsoncxx::to_json(cur_result.value()));
			_reply.count = 1;
		}


	}
	else
	{
		
		mongocxx::options::find_one_and_update opt = mongocxx::options::find_one_and_update{};
		if (!task_modify_opt.fields.is_null())
		{
			const auto& fields_str = task_modify_opt.fields.dump();
			opt.projection(bsoncxx::from_json(bsoncxx::stdx::string_view(fields_str.data(), fields_str.size())));
		}
		if (!task_modify_opt.sort.empty())
		{
			const auto& sort_str = task_modify_opt.sort.dump();
			opt.sort(bsoncxx::from_json(bsoncxx::stdx::string_view(sort_str.data(), sort_str.size())));
		}
		opt.upsert(cur_modify_task->is_upset());

		opt.return_document(cur_modify_task->is_return_new() ? mongocxx::options::return_document::k_after : mongocxx::options::return_document::k_before);

		const auto& doc_str = cur_modify_task->doc().dump();


		auto cur_result = db[cur_modify_task->collection()].find_one_and_update(query_view, bsoncxx::from_json(bsoncxx::stdx::string_view(doc_str.data(), doc_str.size())), opt);
		if (cur_result)
		{
			_reply.content.push_back(bsoncxx::to_json(cur_result.value()));
			_reply.count = 1;
		}

	}
}
