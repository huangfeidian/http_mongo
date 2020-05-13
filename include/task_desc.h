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
		modify_update,
		modify_delete
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
		std::string hint;
		std::string sort;
		std::string fields;


		read_prefer_mode read_prefer = read_prefer_mode::secondary;
		json to_json() const;
		bool from_json(const json& data);

	};
	class find_task: public base_task
	{
	protected:
		find_option _option;
		std::string _query;
	public:
		find_task(const base_task& base,
			const find_option& in_option,
			const std::string& in_query,
			const std::string& in_fields);
		find_task(const base_task& base);
		const find_option& option() const;
		const std::string& query() const;
		json to_json() const;
		bool from_json(const json& data);
		static std::shared_ptr<find_task> find_one(
			const base_task& base, 
			const json::object_t& query,
			std::unordered_map<std::string, bool>  fields = {},
			read_prefer_mode read_prefer = read_prefer_mode::secondary,
			std::unordered_map<std::string, bool> sort = {},
			std::unordered_map<std::string, bool>  hint = {}
		);
		static std::shared_ptr<find_task> find_multi(
			const base_task& base,
			const json::object_t& query,
			std::uint32_t limit,
			std::unordered_map<std::string, bool>  fields = {},
			std::uint32_t skip = 0,
			read_prefer_mode read_prefer = read_prefer_mode::secondary,
			std::unordered_map<std::string, bool> sort = {},
			std::unordered_map<std::string, bool>  hint = {}
		);

	};

	class count_task: public base_task
	{
		protected:
		find_option _option;
		std::string _query;
	public:
		count_task(const base_task& base,
			const find_option& in_option,
			const std::string& in_query,
			const std::string& in_fields);
		count_task(const base_task& base);
		const find_option& option() const;
		const std::string& query() const;
		json to_json() const;
		bool from_json(const json& data);
		static std::shared_ptr<count_task> count(
			const base_task& base,
			const json::object_t& query,
			read_prefer_mode read_prefer = read_prefer_mode::secondary,
			std::unordered_map<std::string, bool>  hint = {}
		);
	};

	class update_task: public base_task
	{
	protected:
		bool _multi = false;
		bool _upset = false;
		std::string _doc;
		std::string _query;

	public:
		json to_json() const;
		bool from_json(const json& data);

		update_task(const base_task& in_base,
			const std::string& in_doc,
			bool in_multi,
			bool in_upset);
		bool is_multi() const;
		bool is_upset() const;
		const std::string& doc() const;
		const std::string& query() const;
		update_task(const base_task& in_base);
		static std::shared_ptr<update_task> update_one(
			const base_task& base,
			const json::object_t& query,
			const json::object_t& doc,
			bool upset
		);
		static std::shared_ptr<update_task> update_multi(
			const base_task& base,
			const json::object_t& query,
			const json::object_t& doc
		);
	};

	class modify_task: public base_task
	{
	protected:
		bool _upset = false;
		std::string _doc;
		bool _remove = false;
		std::string _query;
		bool _return_new = false;
		find_option _option;


	public:
		json to_json() const;
		bool from_json(const json& data);

		modify_task(const base_task& in_base,
			const std::string& in_doc,
			const std::string& in_query,
			bool in_remove,
			bool in_upset,
			bool in_return_new);
		bool is_remove() const;
		bool is_upset() const;
		bool is_return_new() const;
		const find_option& option() const;
		const std::string& query() const;

		const std::string& doc() const;
		modify_task(const base_task& in_base);
		static std::shared_ptr<modify_task> modify_one(
			const base_task& base,
			const json::object_t& query,
			const json::object_t& doc,
			bool upset,
			bool return_new,
			std::unordered_map<std::string, bool>  fields = {},
			std::unordered_map<std::string, bool> sort = {}
		);
		static std::shared_ptr<modify_task> delete_one(
			const base_task& base,
			const json::object_t& query,
			std::unordered_map<std::string, bool>  fields = {},
			std::unordered_map<std::string, bool> sort = {}
		);
	};

	class delete_task: public base_task
	{
	protected:
		std::string _query;
		bool _limit_one = true;//false for delete many
	public:
		json to_json() const;
		bool from_json(const json& data);
		delete_task(const base_task& in_base,
			const std::string& in_query,
			bool in_only_one);
		bool is_limit_one() const;
		const std::string& query() const;
		delete_task(const base_task& in_base);
		static std::shared_ptr<delete_task> delete_one(
			const base_task& base,
			const json::object_t& query
		);
		static std::shared_ptr<delete_task> delete_multi(
			const base_task& base,
			const json::object_t& query
		);
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