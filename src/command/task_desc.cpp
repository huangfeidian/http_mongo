#include <task_desc.h>
using namespace spiritsaway::http_mongo::task_desc;

json base_task::to_json()const
{
	json result;
	result["collection"] = _collection;
	result["channel"] = _channel;
	result["request_id"] = _request_id;
	result["op_type"] = op_name();
	if(!extra.empty())
	{
		result["extra"] = extra;
	}
	return result;
}

std::string_view base_task::op_name() const
{
	return magic_enum::enum_name(_op_type);
}
task_op base_task::op_type() const
{
	return _op_type;
}
const std::string& base_task::channel() const
{
	return _channel;
}
const std::string& base_task::request_id() const
{
	return _request_id;
}
const std::string& base_task::collection() const
{
	return _collection;
}
base_task::base_task(task_op in_op_type,
	const std::string& in_channel,
	const std::string& in_request_id,
	const std::string& in_collection)
	: _collection(in_collection)
	, _channel(in_channel)
	, _request_id(in_request_id)
	, _op_type(in_op_type)

{
	
}
base_task::base_task()
{

}
bool base_task::from_json(const json& data)
{
	if(!data)
	{
		return false;
	}
	auto col_iter = data.find("collection");
	if(col_iter == data.end())
	{
		return false;
		
	}
	if(!col_iter->is_string())
	{
		return false;
	}
	_collection = (*col_iter).get<std::string>();

	auto channel_iter = data.find("channel");
	if(channel_iter == data.end())
	{
		return false;
		
	}
	if(!channel_iter->is_string())
	{
		return false;
	}
	_channel = (*channel_iter).get<std::string>();

	auto request_iter = data.find("request_id");
	if(request_iter == data.end())
	{
		return false;
	}
	if(!request_iter->is_string())
	{
		return false;
	}
	_request_id = (*request_iter).get<std::string>();
	auto op_type_iter = data.find("op_type");
	if(op_type_iter == data.end())
	{
		return false;
		
	}
	if(!op_type_iter->is_string())
	{
		return false;
	}
	auto cur_type_str = (*op_type_iter).get<std::string>();
	auto opt_type = magic_enum::enum_cast<task_op>(cur_type_str);
	if(!opt_type)
	{
		return false;
	}
	_op_type = opt_type.value();
	auto extra_iter = data.find("extra");
	if(extra_iter != data.end())
	{
		if(!extra_iter->is_object())
		{
			return false;
		}
		extra = (*extra_iter).get<json::object_t>();
	}
	return true;
}


json find_option::to_json() const
{
	json result;
	if(limit)
	{
		result["limit"] = limit;
	}
	if(skip)
	{
		result["skip"] = skip;
	}
	if(!hint.empty())
	{
		result["hint"] = hint;
	}
	if(read_prefer != read_prefer_mode::primary)
	{
		result["read_prefer"] = magic_enum::enum_name(read_prefer);
	}
	if(!sort.empty())
	{
		result["sort"] = sort;
	}
	if(!fields.empty())
	{
		result["fields"] = fields;
	}

	return result;
}
bool find_option::from_json(const json& data)
{
	if(!data)
	{
		return true;
	}
	auto limit_iter = data.find("limit");
	if(limit_iter != data.end())
	{
		if(!limit_iter->is_number_unsigned())
		{
			return false;
		}
		limit = (*limit_iter).get<std::uint32_t>();
	}
	auto skip_iter = data.find("skip");
	if(skip_iter != data.end())
	{
		if(!skip_iter->is_number_unsigned())
		{
			return false;
		}
		skip = (*skip_iter).get<std::uint32_t>();
	}


	auto hint_iter = data.find("hint");
	if(hint_iter != data.end())
	{
		if(!hint_iter->is_object())
		{
			return false;
		}
		for(const auto& one_hint: (*hint_iter).items())
		{
			if(!one_hint.value().is_boolean())
			{
				return false;
			}
			hint[one_hint.key()] = one_hint.value().get<bool>();
		}
	}
	auto sort_iter = data.find("sort");
	if(sort_iter != data.end())
	{
		if(!sort_iter->is_object())
		{
			return false;
		}
		for(const auto& one_sort: (*sort_iter).items())
		{
			if(!one_sort.value().is_boolean())
			{
				return false;
			}
			sort[one_sort.key()] = one_sort.value().get<bool>();
		}
	}

	auto fields_iter = data.find("fields");
	if(fields_iter != data.end())
	{
		if(!fields_iter->is_object())
		{
			return false;
		}
		for(const auto& one_fields: (*fields_iter).items())
		{
			if(!one_fields.value().is_boolean())
			{
				return false;
			}
			fields[one_fields.key()] = one_fields.value().get<bool>();
		}
	}

	auto read_pref_iter = data.find("read_pref");
	if(read_pref_iter != data.end())
	{
		if(!read_pref_iter->is_string())
		{
			return false;
		}
		auto cur_read_str = (*read_pref_iter).get<std::string>();
		auto opt_pref = magic_enum::enum_cast<read_prefer_mode>(cur_read_str);
		if(!opt_pref)
		{
			return false;
		}
		read_prefer = opt_pref.value();
	}
	return true;
}


find_task::find_task(const base_task& base,
	find_option in_option,
	json::object_t in_query,
	std::unordered_map<std::string, bool> in_fields)
	: base_task(base)
	, _option(in_option)
	, _query(in_query)
{
	
}
find_task::find_task(const base_task& base)
: base_task(base)
{

}
json find_task::to_json() const
{
	auto result = base_task::to_json();
	result["find_option"] = _option.to_json();
	result["query"] = _query;
	return result;
}
bool find_task::from_json(const json& data)
{
	if(!_option.from_json(data["find_option"]))
	{
		return false;
	}
	auto query_iter = data.find("query");
	if(query_iter == data.end())
	{
		return false;
	}
	if(!query_iter->is_object())
	{
		return false;
	}
	_query = (*query_iter).get<json::object_t>();
	return true;
}

const find_option& find_task::option() const
{
	return _option;
}
const json::object_t& find_task::query() const
{
	return _query;
}



count_task::count_task(const base_task& base,
	find_option in_option,
	json::object_t in_query,
	std::unordered_map<std::string, bool> in_fields)
	: base_task(base)
	, _option(in_option)
	, _query(in_query)
{
	
}
count_task::count_task(const base_task& base)
: base_task(base)
{

}
json count_task::to_json() const
{
	auto result = base_task::to_json();
	result["find_option"] = _option.to_json();
	result["query"] = _query;
	return result;
}
bool count_task::from_json(const json& data)
{
	if(!_option.from_json(data["find_option"]))
	{
		return false;
	}
	auto query_iter = data.find("query");
	if(query_iter == data.end())
	{
		return false;
	}
	if(!query_iter->is_object())
	{
		return false;
	}
	_query = (*query_iter).get<json::object_t>();
	return true;
}

const find_option& count_task::option() const
{
	return _option;
}
const json::object_t& count_task::query() const
{
	return _query;
}


json update_task::to_json() const
{
	json result = base_task::to_json();
	result["upset"] = _upset;
	result["multi"] = _multi;
	result["doc"] = _doc;
	result["query"] = _query;
	return result;
}
bool update_task::from_json(const json& data)
{
	if (data.is_object())
	{
		return false;
	}
	auto upset_iter = data.find("upset");
	if(upset_iter == data.end())
	{
		return false;
	}
	if(!upset_iter->is_boolean())
	{
		return false;
	}
	_upset = (*upset_iter).get<bool>();

	auto multi_iter = data.find("multi");
	if(multi_iter == data.end())
	{
		return false;
	}
	if(!multi_iter->is_boolean())
	{
		return false;
	}
	_multi = (*multi_iter).get<bool>();

	auto doc_iter = data.find("doc");
	if(doc_iter == data.end())
	{
		return false;
	}
	if(!doc_iter->is_object())
	{
		return false;
	}
	_doc = (*doc_iter).get<json::object_t>();

	auto query_iter = data.find("query");
	if (query_iter == data.end())
	{
		return false;
	}
	if (!query_iter->is_object())
	{
		return false;
	}
	_query = (*query_iter).get<json::object_t>();

	return true;
}

update_task::update_task(const base_task& in_base,
	const json::object_t& in_doc,
	bool in_multi,
	bool in_upset)
	: base_task(in_base)
	, _doc(in_doc)
	, _multi(in_multi)
	, _upset(in_upset)
{

}

update_task::update_task(const base_task& in_base)
: base_task(in_base)
{

}

bool update_task::is_multi() const
{
	return _multi;
}
bool update_task::is_upset() const
{
	return _upset;
}
const json::object_t& update_task::doc() const
{
	return _doc;
}
const json::object_t& update_task::query() const
{
	return _query;
}


json find_and_modify_task::to_json() const
{
	json result = base_task::to_json();
	result["upset"] = _upset;
	result["doc"] = _doc;
	result["remove"] = _remove;
	result["query"] = _query;
	result["return_new"] = _return_new;
	return result;
}
bool find_and_modify_task::from_json(const json& data)
{
	if (data.is_object())
	{
		return false;
	}
	auto upset_iter = data.find("upset");
	if(upset_iter == data.end())
	{
		return false;
	}
	if(!upset_iter->is_boolean())
	{
		return false;
	}
	_upset = (*upset_iter).get<bool>();

	auto remove_iter = data.find("remove");
	if(remove_iter == data.end())
	{
		return false;
	}
	if(!remove_iter->is_boolean())
	{
		return false;
	}
	_remove = (*remove_iter).get<bool>();

	auto return_new_iter = data.find("return_new");
	if (return_new_iter != data.end())
	{
		if (!return_new_iter->is_boolean())
		{
			return false;
		}
		_return_new = (*return_new_iter).get<bool>();
	}


	auto doc_iter = data.find("doc");
	if(doc_iter == data.end())
	{
		return false;
	}
	if(!doc_iter->is_object())
	{
		return false;
	}
	_doc = (*doc_iter).get<json::object_t>();

	auto query_iter = data.find("query");
	if (query_iter == data.end())
	{
		return false;
	}
	if (!query_iter->is_object())
	{
		return false;
	}
	_query = (*query_iter).get<json::object_t>();

	return true;
}

find_and_modify_task::find_and_modify_task(const base_task& in_base,
	const json::object_t& in_doc,
	const json::object_t& in_query,
	bool in_remove,
	bool in_upset,
	bool in_return_new)
	: base_task(in_base)
	, _doc(in_doc)
	, _upset(in_upset)
	, _remove(in_remove)
	, _query(in_query)
	, _return_new(in_return_new)
{

}

find_and_modify_task::find_and_modify_task(const base_task& in_base)
: base_task(in_base)
{

}
bool find_and_modify_task::is_remove() const
{
	return _remove;
}

bool find_and_modify_task::is_upset() const
{
	return _upset;
}

bool find_and_modify_task::is_return_new() const
{
	return _return_new;
}

const json::object_t& find_and_modify_task::doc() const
{
	return _doc;
}

const json::object_t& find_and_modify_task::query() const
{
	return _query;
}


bool delete_task::is_limit_one() const
{
	return _limit_one;
}

const json::object_t& delete_task::query() const
{
	return _query;
}

json delete_task::to_json() const
{
	auto result = base_task::to_json();
	result["limit_one"] = _limit_one;
	result["query"] = _query;
	return result;
}
bool delete_task::from_json(const json& data)
{
	auto query_iter = data.find("query");
	if(query_iter == data.end())
	{
		return false;
	}
	if(!query_iter->is_object())
	{
		return false;
	}
	_query = (*query_iter).get<json::object_t>();
	auto limit_one_iter = data.find("limit_one");
	if(limit_one_iter == data.end())
	{
		return false;
	}
	if(!limit_one_iter->is_boolean())
	{
		return false;
	}
	_limit_one = (*limit_one_iter).get<bool>();

	
	return true;
}
delete_task::delete_task(const base_task& in_base,
	const json::object_t& in_query,
	bool in_limit_one)
	: base_task(in_base)
	, _query(in_query)
	, _limit_one(in_limit_one)
{

}

delete_task::delete_task(const base_task& in_base)
: base_task(in_base)
{

}

json task_reply::to_json() const
{
	json result;
	result["error"] = error;
	result["count"] = count;
	result["content"] = content;
	result["cost"] = cost;
	return result;
}

bool task_reply::from_json(const json& data)
{
	auto count_iter = data.find("count");
	if(count_iter == data.end())
	{
		return false;
	}
	if(count_iter->is_number_unsigned())
	{
		return false;
	}

	
	count = (*count_iter).get<std::uint32_t>();

	auto cost_iter = data.find("cost");
	if(cost_iter == data.end())
	{
		return false;
	}
	if(cost_iter->is_number_unsigned())
	{
		return false;
	}

	
	cost = (*cost_iter).get<std::uint32_t>();

	auto error_iter = data.find("error");
	if(error_iter == data.end())
	{
		return false;
	}
	if(!error_iter->is_string())
	{
		return false;
	}
	error = (*error_iter).get<std::string>();

	auto content_iter = data.find("content");
	if(content_iter == data.end())
	{
		return false;
	}
	if(!content_iter->is_array())
	{
		return false;
	}
	content = (*content_iter).get<json::array_t>();
	return true;
}



