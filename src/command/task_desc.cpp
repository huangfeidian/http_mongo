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
std::string base_task::from_json(const json& data)
{
	if(!data.is_object())
	{
		return "data is not object";
	}
	auto col_iter = data.find("collection");
	if(col_iter == data.end())
	{
		return "cant find key collection";
		
	}
	if(!col_iter->is_string())
	{
		return "value for collection is not str";
	}
	_collection = (*col_iter).get<std::string>();

	auto channel_iter = data.find("channel");
	if(channel_iter == data.end())
	{
		return "cant find key channel";
		
	}
	if(!channel_iter->is_string())
	{
		return "value for channel is not string";
	}
	_channel = (*channel_iter).get<std::string>();

	auto request_iter = data.find("request_id");
	if(request_iter == data.end())
	{
		return "cant find key request_id";
	}
	if(!request_iter->is_string())
	{
		return "value for request_id is not str";
	}
	_request_id = (*request_iter).get<std::string>();
	auto op_type_iter = data.find("op_type");
	if(op_type_iter == data.end())
	{
		return "cant fint key op_type";
		
	}
	if(!op_type_iter->is_string())
	{
		return "value for key op_type is not string";
	}
	auto cur_type_str = (*op_type_iter).get<std::string>();
	auto opt_type = magic_enum::enum_cast<task_op>(cur_type_str);
	if(!opt_type)
	{
		return "value for key op_type is not valid task_op";
	}
	_op_type = opt_type.value();
	auto extra_iter = data.find("extra");
	if(extra_iter != data.end())
	{
		if(!extra_iter->is_object())
		{
			return "value for extra is not object";
		}
		extra = (*extra_iter).get<json::object_t>();
	}
	return "";
}

std::string base_task::to_bson(const std::function<bool(std::string&)>& bson_func)
{
	return "";
}

bool base_task::str_or_object(json::const_iterator& iter, std::string& dest)
{
	if (iter->is_string())
	{
		dest = (*iter).get<std::string>();
		return true;
	}
	if (iter->is_object())
	{
		dest = (*iter).dump();
		return true;
	}
	return false;

}
std::string base_task::convert_bool_map_to_str(const std::unordered_map<std::string, bool>& data)
{
	std::unordered_map<std::string, int> result;
	for (auto& [k, v] : data)
	{
		if (v)
		{
			result[k] = 1;
		}
		else
		{
			result[k] = -1;
		}
	}
	return json(result).dump();
}
std::string base_task::debug_info() const
{
	return base_task::to_json().dump();
}
json find_option::to_json() const
{
	json::object_t result;
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
std::string find_option::from_json(const json& data)
{
	if(!data.is_object())
	{
		return "data is not object";
	}
	auto limit_iter = data.find("limit");
	if(limit_iter != data.end())
	{
		if(!limit_iter->is_number_unsigned())
		{
			return "value for limit is not unsigned";
		}
		limit = (*limit_iter).get<std::uint32_t>();
	}
	auto skip_iter = data.find("skip");
	if(skip_iter != data.end())
	{
		if(!skip_iter->is_number_unsigned())
		{
			return "value for skip is not unsigned";
		}
		skip = (*skip_iter).get<std::uint32_t>();
	}


	auto hint_iter = data.find("hint");
	if(hint_iter != data.end())
	{
		if (!base_task::str_or_object(hint_iter, hint))
		{
			return "value for hint is not valid object";
		}
	}
	auto sort_iter = data.find("sort");
	if(sort_iter != data.end())
	{
		if (!base_task::str_or_object(sort_iter, sort))
		{
			return "value for sort is not valid object";
		}
	}

	auto fields_iter = data.find("fields");
	if(fields_iter != data.end())
	{
		if (!base_task::str_or_object(fields_iter, fields))
		{
			return "value for fields is not valid object";
		}
	}

	auto read_pref_iter = data.find("read_pref");
	if(read_pref_iter != data.end())
	{
		if(!read_pref_iter->is_string())
		{
			return "value for read_pref is not string";
		}
		auto cur_read_str = (*read_pref_iter).get<std::string>();
		auto opt_pref = magic_enum::enum_cast<read_prefer_mode>(cur_read_str);
		if(!opt_pref)
		{
			return "value for read_pref is not valid read_prefer_mode";
		}
		read_prefer = opt_pref.value();
	}
	return "";
}

std::string find_option::to_bson(const std::function<bool(std::string&)>& bson_func)
{
	if (!fields.empty())
	{
		if (!bson_func(fields))
		{
			return "fields";
		}
	}
	if (!sort.empty())
	{
		if (!bson_func(sort))
		{
			return "sort";
		}
	}
	if (!hint.empty())
	{
		if (!bson_func(hint))
		{
			return "hint";
		}
	}
	return "";
}


find_task::find_task(const base_task& base,
	const find_option& in_option,
	const std::string& in_query,
	const std::string& in_fields)
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
std::string find_task::from_json(const json& data)
{
	if (!data.is_object())
	{
		return "data is not object";
	}
	auto option_iter = data.find("find_option");
	if (option_iter == data.end())
	{
		return "cant find key find_option";
	}
	auto option_error = _option.from_json(*option_iter);
	if (!option_error.empty())
	{
		return "value for option is not valid, detail is: " + option_error;
	}
	
	auto query_iter = data.find("query");
	if(query_iter == data.end())
	{
		return "cant find key query";
	}
	if (!str_or_object(query_iter, _query))
	{
		return "value for query is not object";
	}
	return "";
}

std::string find_task::to_bson(const std::function<bool(std::string&)>& bson_func)
{
	auto base_error = base_task::to_bson(bson_func);

	if (!base_error.empty())
	{
		return base_error;
	}
	auto option_error = _option.to_bson(bson_func);
	if(!option_error.empty())
	{
		return "option:" + option_error;
	}
	if (!bson_func(_query))
	{
		return "query";
	}
	return "";
}

const find_option& find_task::option() const
{
	return _option;
}
const std::string& find_task::query() const
{
	return _query;
}

std::shared_ptr<find_task> find_task::find_one(
	const base_task& base,
	const json::object_t& query,
	std::unordered_map<std::string, bool>  fields,
	read_prefer_mode read_prefer,
	std::unordered_map<std::string, bool> sort,
	std::unordered_map<std::string, bool>  hint
)
{
	auto result = std::make_shared<find_task>(base);
	result->_query = json(query).dump();
	if (!fields.empty())
	{
		result->_option.fields = convert_bool_map_to_str(fields);
	}
	if (!sort.empty())
	{
		result->_option.fields = convert_bool_map_to_str(sort);
	}
	if (!hint.empty())
	{
		result->_option.fields = convert_bool_map_to_str(hint);
	}
	result->_option.read_prefer = read_prefer;
	result->_op_type = task_op::find_one;
	result->_option.limit = 1;
	return result;
}

std::shared_ptr<find_task> find_task::find_multi(
	const base_task& base,
	const json::object_t& query,
	std::uint32_t limit,
	std::unordered_map<std::string, bool>  fields,
	std::uint32_t skip,
	read_prefer_mode read_prefer,
	std::unordered_map<std::string, bool> sort,
	std::unordered_map<std::string, bool>  hint
)
{

	auto result = std::make_shared<find_task>(base);
	result->_query = json(query).dump();
	if (!fields.empty())
	{
		result->_option.fields = convert_bool_map_to_str(fields);
	}
	if (!sort.empty())
	{
		result->_option.sort = convert_bool_map_to_str(sort);
	}
	if (!hint.empty())
	{
		result->_option.hint = convert_bool_map_to_str(hint);
	}
	result->_option.read_prefer = read_prefer;
	result->_op_type = task_op::find_multi;
	result->_option.limit = limit;
	return result;
}

count_task::count_task(const base_task& base,
	const find_option& in_option,
	const std::string& in_query,
	const std::string& in_fields)
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
std::string count_task::from_json(const json& data)
{
	if (!data.is_object())
	{
		return "data is not object";
	}
	auto option_iter = data.find("find_option");
	if (option_iter == data.end())
	{
		return "cant find key find_option";
	}
	auto option_error = _option.from_json(*option_iter);
	if (!option_error.empty())
	{
		return "value for option is not valid, detail is: " + option_error;
	}
	auto query_iter = data.find("query");
	if(query_iter == data.end())
	{
		return "cant find key query";
	}
	if (!str_or_object(query_iter, _query))
	{
		return "value for query is not object";
	}
	return "";
}

std::string count_task::to_bson(const std::function<bool(std::string&)>& bson_func)
{
	auto base_error = base_task::to_bson(bson_func);

	if (!base_error.empty())
	{
		return base_error;
	}
	auto option_error = _option.to_bson(bson_func);
	if (!option_error.empty())
	{
		return "option:" + option_error;
	}
	if (!bson_func(_query))
	{
		return "query";
	}
	return "";
}

const find_option& count_task::option() const
{
	return _option;
}
const std::string& count_task::query() const
{
	return _query;
}

std::shared_ptr<count_task> count_task::count(
	const base_task& base,
	const json::object_t& query,
	read_prefer_mode read_prefer,
	std::unordered_map<std::string, bool>  hint
)
{
	auto result = std::make_shared<count_task>(base);
	result->_op_type = task_op::count;
	result->_query = json(query).dump();
	result->_option.read_prefer = read_prefer;
	if (!hint.empty())
	{
		result->_option.hint = convert_bool_map_to_str(hint);
	}
	return result;
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
std::string update_task::from_json(const json& data)
{
	if (!data.is_object())
	{
		return "data is not object";
	}
	auto upset_iter = data.find("upset");
	if(upset_iter == data.end())
	{
		return "cant find key upset";
	}
	if(!upset_iter->is_boolean())
	{
		return "value for key upset is not bool";
	}
	_upset = (*upset_iter).get<bool>();

	auto multi_iter = data.find("multi");
	if(multi_iter == data.end())
	{
		return "cant find key multi";
	}
	if(!multi_iter->is_boolean())
	{
		return "value for key multi is not bool";
	}
	_multi = (*multi_iter).get<bool>();

	auto doc_iter = data.find("doc");
	if(doc_iter == data.end())
	{
		return "cant find key doc";
	}
	if (!str_or_object(doc_iter, _doc))
	{
		return "value for doc is not valid object";
	}

	auto query_iter = data.find("query");
	if (query_iter == data.end())
	{
		return "cant find key query";
	}
	if (!str_or_object(query_iter, _query))
	{
		return "value for key query is not valid object";
	}

	return "";
}

std::string update_task::to_bson(const std::function<bool(std::string&)>& bson_func)
{
	auto base_error = base_task::to_bson(bson_func);

	if (!base_error.empty())
	{
		return base_error;
	}

	if (!bson_func(_doc))
	{
		return "doc";
	}
	if (!bson_func(_query))
	{
		return "query";
	}
	return "";
}


update_task::update_task(const base_task& in_base,
	const std::string& in_doc,
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
const std::string& update_task::doc() const
{
	return _doc;
}
const std::string& update_task::query() const
{
	return _query;
}


json modify_task::to_json() const
{
	json result = base_task::to_json();
	result["upset"] = _upset;
	result["doc"] = _doc;
	result["remove"] = _remove;
	result["query"] = _query;
	result["return_new"] = _return_new;
	return result;
}
std::string modify_task::from_json(const json& data)
{
	if (!data.is_object())
	{
		return "data is not object";
	}
	auto upset_iter = data.find("upset");
	if(upset_iter == data.end())
	{
		return "cant find key upset";
	}
	if(!upset_iter->is_boolean())
	{
		return "value for key upset is not bool";
	}
	_upset = (*upset_iter).get<bool>();

	auto remove_iter = data.find("remove");
	if(remove_iter == data.end())
	{
		return "cant find key remove";
	}
	if(!remove_iter->is_boolean())
	{
		return "value for key remove is not bool";
	}
	_remove = (*remove_iter).get<bool>();

	auto return_new_iter = data.find("return_new");
	if (return_new_iter != data.end())
	{
		if (!return_new_iter->is_boolean())
		{
			return "value for return_new is not bool";
		}
		_return_new = (*return_new_iter).get<bool>();
	}


	auto doc_iter = data.find("doc");
	if(doc_iter == data.end())
	{
		return "cant find key doc";
	}
	if (!str_or_object(doc_iter, _doc))
	{
		return "value for key doc is not valid object";
	}
	auto query_iter = data.find("query");
	if (query_iter == data.end())
	{
		return "cant find key query";
	}
	if (!str_or_object(query_iter, _query))
	{
		return "value for query is not valid object";
	}

	return "";
}
std::string modify_task::to_bson(const std::function<bool(std::string&)>& bson_func)
{
	auto base_error = base_task::to_bson(bson_func);

	if (!base_error.empty())
	{
		return base_error;
	}
	if (op_type() == task_op::modify_update)
	{
		if (!bson_func(_doc))
		{
			return "doc";
		}
	}
	
	if (!bson_func(_query))
	{
		return "query";
	}
	return "";
}

std::shared_ptr<update_task> update_task::update_one(
	const base_task& base,
	const json::object_t& query,
	const json::object_t& doc,
	bool upset
)
{
	auto result = std::make_shared<update_task>(base);
	result->_query = json(query).dump();
	result->_doc = json(doc).dump();
	result->_upset = upset;
	result->_op_type = task_op::update_one;
	result->_multi = false;
	return result;
}
std::shared_ptr<update_task> update_task::update_multi(
	const base_task& base,
	const json::object_t& query,
	const json::object_t& doc
)
{
	auto result = std::make_shared<update_task>(base);
	result->_query = json(query).dump();
	result->_doc = json(doc).dump();
	result->_op_type = task_op::update_multi;
	result->_multi = true;
	return result;
}

modify_task::modify_task(const base_task& in_base,
	const std::string& in_doc,
	const std::string& in_query,
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

modify_task::modify_task(const base_task& in_base)
: base_task(in_base)
{

}
bool modify_task::is_remove() const
{
	return _remove;
}

bool modify_task::is_upset() const
{
	return _upset;
}

bool modify_task::is_return_new() const
{
	return _return_new;
}

const std::string& modify_task::doc() const
{
	return _doc;
}

const std::string& modify_task::query() const
{
	return _query;
}
const find_option& modify_task::option() const
{
	return _option;
}

std::shared_ptr<modify_task> modify_task::modify_one(
	const base_task& base,
	const json::object_t& query,
	const json::object_t& doc,
	bool upset,
	bool return_new,
	std::unordered_map<std::string, bool>  fields,
	std::unordered_map<std::string, bool> sort
)
{
	auto result = std::make_shared<modify_task>(base);
	result->_query = json(query).dump();
	result->_doc = json(doc).dump();
	result->_op_type = task_op::modify_update;
	result->_upset = upset;
	result->_return_new = return_new;
	if (!fields.empty())
	{
		result->_option.fields = convert_bool_map_to_str(fields);
	}
	if (!sort.empty())
	{
		result->_option.sort = convert_bool_map_to_str(sort);
	}
	return result;
}
std::shared_ptr<modify_task> modify_task::delete_one(
	const base_task& base,
	const json::object_t& query,
	std::unordered_map<std::string, bool>  fields,
	std::unordered_map<std::string, bool> sort
)
{
	auto result = std::make_shared<modify_task>(base);
	result->_query = json(query).dump();
	result->_op_type = task_op::modify_delete;
	if (!fields.empty())
	{
		result->_option.fields = convert_bool_map_to_str(fields);
	}
	if (!sort.empty())
	{
		result->_option.sort = convert_bool_map_to_str(sort);
	}
	return result;
}


bool delete_task::is_limit_one() const
{
	return _limit_one;
}

const std::string& delete_task::query() const
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
std::string delete_task::from_json(const json& data)
{
	if (!data.is_object())
	{
		return "data is not object";
	}
	auto query_iter = data.find("query");
	if(query_iter == data.end())
	{
		return "cant find key query";
	}
	if (!str_or_object(query_iter, _query))
	{
		return "value for key query is not valid object";
	}
	auto limit_one_iter = data.find("limit_one");
	if(limit_one_iter == data.end())
	{
		return "cant find key limit_one";
	}
	if(!limit_one_iter->is_boolean())
	{
		return "value for limit_one is not bool";
	}
	_limit_one = (*limit_one_iter).get<bool>();

	
	return "";
}
std::string delete_task::to_bson(const std::function<bool(std::string&)>& bson_func)
{
	auto base_error = base_task::to_bson(bson_func);
	if(!base_error.empty())
	{
		return base_error;
	}
	if (!bson_func(_query))
	{
		return "query";
	}
	return "";
}


delete_task::delete_task(const base_task& in_base,
	const std::string& in_query,
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
std::shared_ptr<delete_task> delete_task::delete_one(
	const base_task& base,
	const json::object_t& query
)
{
	auto result = std::make_shared<delete_task>(base);
	result->_op_type = task_op::delete_one;
	result->_query = json(query).dump();
	result->_limit_one = true;
	return result;
}
std::shared_ptr<delete_task> delete_task::delete_multi(
	const base_task& base,
	const json::object_t& query
)
{
	auto result = std::make_shared<delete_task>(base);
	result->_op_type = task_op::delete_multi;
	result->_query = json(query).dump();
	result->_limit_one = false;
	return result;
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

std::string task_reply::from_json(const json& data)
{
	auto count_iter = data.find("count");
	if(count_iter == data.end())
	{
		return "cant find key count";
	}
	if(count_iter->is_number_unsigned())
	{
		return "value for count is not unsigned";
	}

	
	count = (*count_iter).get<std::uint32_t>();

	auto cost_iter = data.find("cost");
	if(cost_iter == data.end())
	{
		return "cant find key cost";
	}
	if(cost_iter->is_number_unsigned())
	{
		return "value for key cost is not unsigned";
	}

	
	cost = (*cost_iter).get<std::uint32_t>();

	auto error_iter = data.find("error");
	if(error_iter == data.end())
	{
		return "cant find key error";
	}
	if(!error_iter->is_string())
	{
		return "value for error is not string";
	}
	error = (*error_iter).get<std::string>();

	auto content_iter = data.find("content");
	if(content_iter == data.end())
	{
		return "cant find key content";
	}
	if(!content_iter->is_array())
	{
		return "value for content is not array";
	}
	for (const auto& one_item : *content_iter)
	{
		if (!one_item.is_string())
		{
			return "item in content is not string";
		}
		content.push_back(one_item.get<std::string>());
	}
	return "";
}



