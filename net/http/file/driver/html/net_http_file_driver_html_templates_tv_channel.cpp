#include "net_http_file_driver_html_templates_tv_channel.h"
#include "net_http_file_driver_html_templates.h"

#include "../../../../../tv/tv_channel.h"

static std::string comma_vorbis_comments(
	std::vector<std::string> tmp){
	std::string retval;
	for(uint64_t i = 0;i < tmp.size();i++){
		retval += tmp[i] + ",";
	}
	if(retval.size() > 0){
		retval.erase(retval.end());
	}
	return retval;
}

HTML_TEMP(create_tv_channel){
	
}

HTML_TEMP(list_tv_channel){
}

HTML_TEMP(destroy_tv_channel){
}

HTML_LOGIC(create_tv_channel){
	const GET_STR(create_tv_channel_title);
	const GET_STR(create_tv_channel_desc);
	const GET_STR(create_tv_channel_wallet_set_id);

	tv_channel_t *channel_ptr =
		new tv_channel_t;
	channel_ptr->add_param(
		VORBIS_COMMENT_PARAM_TITLE,
		create_tv_channel_title,
		false);
	channel_ptr->add_param(
		VORBIS_COMMENT_PARAM_DESCRIPTION,
		create_tv_channel_desc,
		false);
	channel_ptr->set_wallet_set_id(
		convert::array::id::from_hex(
			create_tv_channel_wallet_set_id));

	return "Created TV Channel " + convert::array::id::to_hex(channel_ptr->id.get_id());
}

HTML_LOGIC(list_tv_channel){
	std::vector<id_t_> tv_channels =
		ID_TIER_CACHE_GET(
			TYPE_TV_CHANNEL_T);
	std::vector<std::vector<std::string> > channel_data_table(
		{
			std::vector<std::string>({
					"ID",
						"Title",
						"Desc",
						"Wallet ID"})
				});
	for(uint64_t i = 0;i < tv_channels.size();i++){
		tv_channel_t *channel_ptr =
			PTR_DATA(tv_channels[i],
				 tv_channel_t);
		CONTINUE_IF_NULL(channel_ptr, P_WARN);
		try{
			channel_data_table.push_back(
				std::vector<std::string>({
						convert::array::id::to_hex(tv_channels[i]),
							comma_vorbis_comments(
								channel_ptr->search_for_param(VORBIS_COMMENT_PARAM_TITLE)),
							comma_vorbis_comments(
								channel_ptr->search_for_param(VORBIS_COMMENT_PARAM_DESCRIPTION)),
							convert::array::id::to_hex(channel_ptr->get_wallet_set_id())
							}));
		}catch(...){
			channel_data_table.push_back(
				std::vector<std::string>({
						"EXCEPTION (simplified catch, so not all data is unknown",
							"EXCEPTION",
							"EXCEPTION",
							"EXCEPTION"}));
		}
	}
	return http::tags::html_table(
		channel_data_table);
}

HTML_LOGIC(destroy_tv_channel){
	const GET_STR(destroy_tv_channel_id);
	const id_t_ tmp_id =
		convert::array::id::from_hex(
			destroy_tv_channel_id);
	id_tier::operation::del_id_from_state(
		{tmp_id},
		id_tier::state_tier::optimal_state_vector_of_tier_vector(
			all_tiers));
	// id_api::destroy(tmp_id);
	return "Destroyed TV Channel " + convert::array::id::to_hex(tmp_id);
}
