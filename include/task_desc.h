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
		insert_one,
		insert_multi,
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
		std::string m_collection = "";
		std::string m_channel = "";
		std::string m_request_id = "";
		task_op m_op_type = task_op::invalid;
		json::object_t m_extra;
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
		virtual std::string from_json(const json& data);
		std::string debug_info() const;

	};
	class find_option
	{
	public:
		std::uint32_t limit = 1;
		std::uint32_t skip = 0;
		json hint;
		json sort;
		json fields;


		read_prefer_mode read_prefer = read_prefer_mode::secondary;
		json to_json() const;
		std::string from_json(const json& data);
	};
	class find_task: public base_task
	{
	protected:
		find_option m_option;
		json m_query;
	public:
		find_task(const base_task& base,
			const find_option& in_option,
			const json& in_query,
			const json& in_fields);
		find_task(const base_task& base);
		const find_option& option() const;
		const json& query() const;
		json to_json() const override;
		std::string from_json(const json& data) override;

		static std::shared_ptr<find_task> find_one(
			const base_task& base, 
			const json& query,
			json  fields = {},
			read_prefer_mode read_prefer = read_prefer_mode::secondary,
			json sort = {},
			json  hint = {}
		);
		static std::shared_ptr<find_task> find_multi(
			const base_task& base,
			const json& query,
			std::uint32_t limit,
			json  fields = {},
			std::uint32_t skip = 0,
			read_prefer_mode read_prefer = read_prefer_mode::secondary,
			json sort = {},
			json  hint = {}
		);

	};

	class count_task: public base_task
	{
		protected:
		find_option m_option;
		json m_query;
	public:
		count_task(const base_task& base,
			const find_option& in_option,
			const json& in_query,
			const json& in_fields);
		count_task(const base_task& base);
		const find_option& option() const;
		const json& query() const;
		json to_json() const override;
		std::string from_json(const json& data) override;

		static std::shared_ptr<count_task> count(
			const base_task& base,
			const json& query,
			read_prefer_mode read_prefer = read_prefer_mode::secondary,
			json  hint = {}
		);
	};

	class insert_task : public base_task
	{
	protected:
		std::vector<json> m_docs;
	public:
		insert_task(const base_task& in_base);
		insert_task(const base_task& in_base, const std::vector<json>& in_docs);
		static std::shared_ptr<insert_task> insert_one(const base_task& base,
			const json& doc);
		static std::shared_ptr<insert_task> insert_multi(const base_task& base,
			const std::vector<json>& docs);
		json to_json() const override;
		const std::vector<json>& docs() const
		{
			return m_docs;
		}
		std::string from_json(const json& data) override;
	};

	class update_task: public base_task
	{
	protected:
		bool m_multi = false;
		bool m_upset = false;
		json m_doc;
		json m_query;

	public:
		json to_json() const override;
		std::string from_json(const json& data) override;

		update_task(const base_task& in_base,
			const json& in_doc,
			bool in_multi,
			bool in_upset);
		bool is_multi() const;
		bool is_upset() const;
		const json& doc() const;
		const json& query() const;
		update_task(const base_task& in_base);
		static std::shared_ptr<update_task> update_one(
			const base_task& base,
			const json& query,
			const json& doc,
			bool upset
		);
		static std::shared_ptr<update_task> update_multi(
			const base_task& base,
			const json& query,
			const json& doc
		);
	};

	class modify_task: public base_task
	{
	protected:
		bool m_upset = false;
		json m_doc;
		bool m_remove = false;
		json m_query;
		bool m_return_new = false;
		find_option m_option;


	public:
		json to_json() const override;
		std::string from_json(const json& data) override;

		modify_task(const base_task& in_base,
			const json& in_doc,
			const json& in_query,
			bool in_remove,
			bool in_upset,
			bool in_return_new);
		bool is_remove() const;
		bool is_upset() const;
		bool is_return_new() const;
		const find_option& option() const;
		const json& query() const;

		const json& doc() const;
		modify_task(const base_task& in_base);
		static std::shared_ptr<modify_task> modify_one(
			const base_task& base,
			const json& query,
			const json& doc,
			bool upset,
			bool return_new,
			json  fields = {},
			json sort = {}
		);
		static std::shared_ptr<modify_task> delete_one(
			const base_task& base,
			const json& query,
			json  fields = {},
			json sort = {}
		);
	};

	class delete_task: public base_task
	{
	protected:
		json m_query;
		bool m_limit_one = true;//false for delete many
	public:
		json to_json() const override;
		std::string from_json(const json& data) override;

		delete_task(const base_task& in_base,
			const json& in_query,
			bool in_only_one);
		bool is_limit_one() const;
		const json& query() const;
		delete_task(const base_task& in_base);
		static std::shared_ptr<delete_task> delete_one(
			const base_task& base,
			const json& query
		);
		static std::shared_ptr<delete_task> delete_multi(
			const base_task& base,
			const json& query
		);
	};
	class task_reply
	{
	public:
		std::string error;
		std::uint32_t count = 0;
		std::vector<std::string> content;
		std::uint32_t cost = 0;// cost for miliseconds
		json to_json() const;
		std::string from_json(const json& data);

	};
	using reply_callback_t = std::function<void(const task_desc::task_reply&)>;

}