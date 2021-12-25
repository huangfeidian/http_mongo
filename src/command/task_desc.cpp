#include <task_desc.h>
using namespace spiritsaway::http_mongo::task_desc;

json base_task::to_json()const
{
	json result;
	result["collection"] = m_collection;
	result["channel"] = m_channel;
	result["request_id"] = m_request_id;
	result["op_type"] = op_name();
	if(!m_extra.empty())
	{
		result["extra"] = m_extra;
	}
	return result;
}

std::string_view base_task::op_name() const
{
	return magic_enum::enum_name(m_op_type);
}
task_op base_task::op_type() const
{
	return m_op_type;
}
const std::string& base_task::channel() const
{
	return m_channel;
}
std::uint64_t base_task::request_id() const
{
	return m_request_id;
}
const std::string& base_task::collection() const
{
	return m_collection;
}
base_task::base_task(task_op in_op_type,
	const std::string& in_channel,
	const std::uint64_t in_request_id,
	const std::string& in_collection)
	: m_collection(in_collection)
	, m_channel(in_channel)
	, m_request_id(in_request_id)
	, m_op_type(in_op_type)

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
	m_collection = (*col_iter).get<std::string>();

	auto channel_iter = data.find("channel");
	if(channel_iter == data.end())
	{
		return "cant find key channel";
		
	}
	if(!channel_iter->is_string())
	{
		return "value for channel is not string";
	}
	m_channel = (*channel_iter).get<std::string>();

	auto request_iter = data.find("request_id");
	if(request_iter == data.end())
	{
		return "cant find key request_id";
	}
	if(!request_iter->is_number_unsigned())
	{
		return "value for request_id is not str";
	}
	m_request_id = (*request_iter).get<std::uint64_t>();
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
	m_op_type = opt_type.value();
	auto extra_iter = data.find("extra");
	if(extra_iter != data.end())
	{
		if(!extra_iter->is_object())
		{
			return "value for extra is not object";
		}
		m_extra = (*extra_iter).get<json::object_t>();
	}
	return "";
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
		hint = *hint_iter;
	}
	auto sort_iter = data.find("sort");
	if(sort_iter != data.end())
	{
		sort = *sort_iter;
	}

	auto fields_iter = data.find("fields");
	if(fields_iter != data.end())
	{
		fields = *fields_iter;
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




find_task::find_task(const base_task& base,
	const find_option& in_option,
	const json& in_query,
	const json& in_fields)
	: base_task(base)
	, m_option(in_option)
	, m_query(in_query)
{
	
}
find_task::find_task(const base_task& base)
: base_task(base)
{

}
json find_task::to_json() const
{
	auto result = base_task::to_json();
	result["find_option"] = m_option.to_json();
	result["query"] = m_query;
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
	auto option_error = m_option.from_json(*option_iter);
	if (!option_error.empty())
	{
		return "value for option is not valid, detail is: " + option_error;
	}
	
	auto query_iter = data.find("query");
	if(query_iter == data.end())
	{
		return "cant find key query";
	}
	m_query = *query_iter;
	return "";
}



const find_option& find_task::option() const
{
	return m_option;
}
const json& find_task::query() const
{
	return m_query;
}

std::shared_ptr<find_task> find_task::find_one(
	const base_task& base,
	const json& query,
	json  fields,
	read_prefer_mode read_prefer,
	json sort,
	json  hint
)
{
	auto result = std::make_shared<find_task>(base);
	result->m_query = query;
	result->m_option.fields = fields;
	result->m_option.sort = sort;
	result->m_option.hint = hint;
	
	result->m_option.read_prefer = read_prefer;
	result->m_op_type = task_op::find_one;
	result->m_option.limit = 1;
	return result;
}

std::shared_ptr<find_task> find_task::find_multi(
	const base_task& base,
	const json& query,
	std::uint32_t limit,
	json  fields,
	std::uint32_t skip,
	read_prefer_mode read_prefer,
	json sort,
	json  hint
)
{

	auto result = std::make_shared<find_task>(base);
	result->m_query = query;
	result->m_option.fields = fields;
	result->m_option.sort = sort;
	result->m_option.hint = hint;
	result->m_option.read_prefer = read_prefer;
	result->m_op_type = task_op::find_multi;
	result->m_option.limit = limit;
	return result;
}

count_task::count_task(const base_task& base,
	const find_option& in_option,
	const json& in_query,
	const json& in_fields)
	: base_task(base)
	, m_option(in_option)
	, m_query(in_query)
{
	
}
count_task::count_task(const base_task& base)
: base_task(base)
{

}
json count_task::to_json() const
{
	auto result = base_task::to_json();
	result["find_option"] = m_option.to_json();
	result["query"] = m_query;
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
	auto option_error = m_option.from_json(*option_iter);
	if (!option_error.empty())
	{
		return "value for option is not valid, detail is: " + option_error;
	}
	auto query_iter = data.find("query");
	if(query_iter == data.end())
	{
		return "cant find key query";
	}
	m_query = *query_iter;
	return "";
}



const find_option& count_task::option() const
{
	return m_option;
}
const json& count_task::query() const
{
	return m_query;
}

std::shared_ptr<count_task> count_task::count(
	const base_task& base,
	const json& query,
	read_prefer_mode read_prefer,
	json  hint
)
{
	auto result = std::make_shared<count_task>(base);
	result->m_op_type = task_op::count;
	result->m_query = json(query).dump();
	result->m_option.read_prefer = read_prefer;
	result->m_option.hint = hint;

	return result;
}

json insert_task::to_json() const
{
	json result = base_task::to_json();
	result["docs"] = m_docs;
	return result;
}

std::string insert_task::from_json(const json& data)
{
	if (!data.is_object())
	{
		return "data is not object";
	}
	auto doc_iter = data.find("docs");
	if (doc_iter == data.end())
	{
		return "cant find key docs";
	}
	if (!doc_iter->is_array())
	{
		return "doc value is not array";
	}
	m_docs = doc_iter->get<std::vector<json>>();
	return {};
}


insert_task::insert_task(const base_task& in_base, const std::vector<json>& in_docs)
	: base_task(in_base)
	, m_docs(in_docs)
{

}
std::shared_ptr<insert_task> insert_task::insert_one(const base_task& base,
	const json& doc)
{
	std::vector<json> doc_vec;
	doc_vec.push_back(doc);
	auto cur_result = std::make_shared<insert_task>(base, doc_vec);
	cur_result->m_op_type = task_op::insert_one;
	return cur_result;
}

std::shared_ptr<insert_task> insert_task::insert_multi(const base_task& base,
	const std::vector<json>& docs)
{
	
	auto cur_result = std::make_shared<insert_task>(base, docs);
	cur_result->m_op_type = task_op::insert_multi;
	return cur_result;
}

insert_task::insert_task(const base_task& in_base)
	: base_task(in_base)
{

}

json update_task::to_json() const
{
	json result = base_task::to_json();
	result["upset"] = m_upset;
	result["multi"] = m_multi;
	result["doc"] = m_doc;
	result["query"] = m_query;
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
	m_upset = (*upset_iter).get<bool>();

	auto multi_iter = data.find("multi");
	if(multi_iter == data.end())
	{
		return "cant find key multi";
	}
	if(!multi_iter->is_boolean())
	{
		return "value for key multi is not bool";
	}
	m_multi = (*multi_iter).get<bool>();

	auto doc_iter = data.find("doc");
	if(doc_iter == data.end())
	{
		return "cant find key doc";
	}
	m_doc = *doc_iter;

	auto query_iter = data.find("query");
	if (query_iter == data.end())
	{
		return "cant find key query";
	}
	m_query = *query_iter;

	return "";
}




update_task::update_task(const base_task& in_base,
	const json& in_doc,
	bool in_multi,
	bool in_upset)
	: base_task(in_base)
	, m_doc(in_doc)
	, m_multi(in_multi)
	, m_upset(in_upset)
{

}

update_task::update_task(const base_task& in_base)
: base_task(in_base)
{

}

bool update_task::is_multi() const
{
	return m_multi;
}
bool update_task::is_upset() const
{
	return m_upset;
}
const json& update_task::doc() const
{
	return m_doc;
}
const json& update_task::query() const
{
	return m_query;
}


json modify_task::to_json() const
{
	json result = base_task::to_json();
	result["upset"] = m_upset;
	result["doc"] = m_doc;
	result["remove"] = m_remove;
	result["query"] = m_query;
	result["return_new"] = m_return_new;
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
	m_upset = (*upset_iter).get<bool>();

	auto remove_iter = data.find("remove");
	if(remove_iter == data.end())
	{
		return "cant find key remove";
	}
	if(!remove_iter->is_boolean())
	{
		return "value for key remove is not bool";
	}
	m_remove = (*remove_iter).get<bool>();

	auto return_new_iter = data.find("return_new");
	if (return_new_iter != data.end())
	{
		if (!return_new_iter->is_boolean())
		{
			return "value for return_new is not bool";
		}
		m_return_new = (*return_new_iter).get<bool>();
	}


	auto doc_iter = data.find("doc");
	if(doc_iter == data.end())
	{
		return "cant find key doc";
	}
	m_doc = *doc_iter;
	auto query_iter = data.find("query");
	if (query_iter == data.end())
	{
		return "cant find key query";
	}
	m_query = *query_iter;

	return "";
}


std::shared_ptr<update_task> update_task::update_one(
	const base_task& base,
	const json& query,
	const json& doc,
	bool upset
)
{
	auto result = std::make_shared<update_task>(base);
	result->m_query = query;
	result->m_doc = doc;
	result->m_upset = upset;
	result->m_op_type = task_op::update_one;
	result->m_multi = false;
	return result;
}
std::shared_ptr<update_task> update_task::update_multi(
	const base_task& base,
	const json& query,
	const json& doc
)
{
	auto result = std::make_shared<update_task>(base);
	result->m_query = query;
	result->m_doc = doc;
	result->m_op_type = task_op::update_multi;
	result->m_multi = true;
	return result;
}

modify_task::modify_task(const base_task& in_base,
	const json& in_doc,
	const json& in_query,
	bool in_remove,
	bool in_upset,
	bool in_return_new)
	: base_task(in_base)
	, m_doc(in_doc)
	, m_upset(in_upset)
	, m_remove(in_remove)
	, m_query(in_query)
	, m_return_new(in_return_new)
{

}

modify_task::modify_task(const base_task& in_base)
: base_task(in_base)
{

}
bool modify_task::is_remove() const
{
	return m_remove;
}

bool modify_task::is_upset() const
{
	return m_upset;
}

bool modify_task::is_return_new() const
{
	return m_return_new;
}

const json& modify_task::doc() const
{
	return m_doc;
}

const json& modify_task::query() const
{
	return m_query;
}
const find_option& modify_task::option() const
{
	return m_option;
}

std::shared_ptr<modify_task> modify_task::modify_one(
	const base_task& base,
	const json& query,
	const json& doc,
	bool upset,
	bool return_new,
	json  fields,
	json sort
)
{
	auto result = std::make_shared<modify_task>(base);
	result->m_query = query;
	result->m_doc = doc;
	result->m_op_type = task_op::modify_update;
	result->m_upset = upset;
	result->m_return_new = return_new;
	result->m_option.fields = fields;
	result->m_option.sort = sort;

	return result;
}
std::shared_ptr<modify_task> modify_task::delete_one(
	const base_task& base,
	const json& query,
	json  fields,
	json sort
)
{
	auto result = std::make_shared<modify_task>(base);
	result->m_query = query;
	result->m_op_type = task_op::modify_delete;
	result->m_option.fields = fields;
	result->m_option.sort = sort;
	return result;
}


bool delete_task::is_limit_one() const
{
	return m_limit_one;
}

const json& delete_task::query() const
{
	return m_query;
}

json delete_task::to_json() const
{
	auto result = base_task::to_json();
	result["limit_one"] = m_limit_one;
	result["query"] = m_query;
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
	m_query = *query_iter;
	auto limit_one_iter = data.find("limit_one");
	if(limit_one_iter == data.end())
	{
		return "cant find key limit_one";
	}
	if(!limit_one_iter->is_boolean())
	{
		return "value for limit_one is not bool";
	}
	m_limit_one = (*limit_one_iter).get<bool>();

	
	return "";
}



delete_task::delete_task(const base_task& in_base,
	const json& in_query,
	bool in_limit_one)
	: base_task(in_base)
	, m_query(in_query)
	, m_limit_one(in_limit_one)
{

}

delete_task::delete_task(const base_task& in_base)
: base_task(in_base)
{

}
std::shared_ptr<delete_task> delete_task::delete_one(
	const base_task& base,
	const json& query
)
{
	auto result = std::make_shared<delete_task>(base);
	result->m_op_type = task_op::delete_one;
	result->m_query = query;
	result->m_limit_one = true;
	return result;
}
std::shared_ptr<delete_task> delete_task::delete_multi(
	const base_task& base,
	const json& query
)
{
	auto result = std::make_shared<delete_task>(base);
	result->m_op_type = task_op::delete_multi;
	result->m_query = query;
	result->m_limit_one = false;
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



