#pragma once
#include <string>
#include <unordered_map>

#include <nlohmann/json.hpp>
#include <magic_enum.hpp>
#include <functional>

namespace spiritsaway::http_mongo::task_desc
{
	using json = nlohmann::json;
	enum class task_op
	{
		invalid = 0,
		find_one,
		find_multi,
		count,
		update_one,
		update_multi,
		delete_one,
		delete_multi,
		find_and_modify,
	};
	enum class read_prefer_mode
	{
		primary = 0,
		primary_prefer,
		secondary,
		secondary_prefer,
		nearest
	};
	class base_task
	{
	protected:
		std::string _collection = "";
		std::string _channel = "";
		std::string _request_id = "";
		task_op _op_type = task_op::invalid;
		json::object_t extra;
	public:
		using channel_type = std::string;

		virtual json to_json() const;
		std::string_view op_name() const;
		task_op op_type() const;
		const std::string& channel() const;
		const std::string& request_id() const;
		const std::string& collection() const;
		base_task(task_op in_op_type,
			const std::string& in_channel,
			const std::string& in_request_id,
			const std::string& in_collection);
		base_task::base_task();
		virtual bool from_json(const json& data);
		std::string debug_info() const;
	};
	class find_option
	{
	public:
		std::uint32_t limit = 1;
		std::uint32_t skip = 0;
		std::unordered_map<std::string, bool> hint;
		std::unordered_map<std::string, bool> sort;
		std::unordered_map<std::string, bool> fields;


		read_prefer_mode read_prefer = read_prefer_mode::secondary;
		json to_json() const;
		bool from_json(const json& data);

	};
	class find_task: public base_task
	{
	protected:
		find_option _option;
		json::object_t _query;
	public:
		find_task(const base_task& base,
			find_option in_option,
			json::object_t in_query,
			std::unordered_map<std::string, bool> in_fields);
		find_task(const base_task& base);
		const find_option& option() const;
		const json::object_t& query() const;
		json to_json() const;
		bool from_json(const json& data);

	};

	class count_task: public base_task
	{
		protected:
		find_option _option;
		json::object_t _query;
	public:
		count_task(const base_task& base,
			find_option in_option,
			json::object_t in_query,
			std::unordered_map<std::string, bool> in_fields);
		count_task(const base_task& base);
		const find_option& option() const;
		const json::object_t& query() const;
		json to_json() const;
		bool from_json(const json& data);
	};

	class update_task: public base_task
	{
	protected:
		bool _multi = false;
		bool _upset = false;
		json::object_t _doc;
		json::object_t _query;

	public:
		json to_json() const;
		bool from_json(const json& data);

		update_task(const base_task& in_base,
			const json::object_t& in_doc,
			bool in_multi,
			bool in_upset);
		bool is_multi() const;
		bool is_upset() const;
		const json::object_t& doc() const;
		const json::object_t& query() const;
		update_task(const base_task& in_base);
	};

	class find_and_modify_task: public base_task
	{
	protected:
		bool _upset = false;
		json::object_t _doc;
		bool _remove = false;
		json::object_t _query;
		bool _return_new = false;


	public:
		json to_json() const;
		bool from_json(const json& data);

		find_and_modify_task(const base_task& in_base,
			const json::object_t& in_doc,
			const json::object_t& in_query,
			bool in_remove,
			bool in_upset,
			bool in_return_new);
		bool is_remove() const;
		bool is_upset() const;
		bool is_return_new() const;
		const find_option& option() const;
		const json::object_t& query() const;

		const json::object_t& doc() const;
		find_and_modify_task(const base_task& in_base);
	};

	class delete_task: public base_task
	{
	protected:
		json::object_t _query;
		bool _limit_one = true;//false for delete many
	public:
		json to_json() const;
		bool from_json(const json& data);
		delete_task(const base_task& in_base,
			const json::object_t& in_query,
			bool in_only_one);
		bool is_limit_one() const;
		const json::object_t& query() const;
		delete_task(const base_task& in_base);
	};
	class task_reply
	{
	public:
		std::string error;
		std::uint32_t count;
		std::vector<std::string> content;
		std::uint32_t cost;// cost for miliseconds
		json to_json() const;
		bool from_json(const json& data);

	};
	using reply_callback_t = std::function<void(const task_desc::task_reply&)>;

}