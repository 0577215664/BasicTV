#include "console.h"
#include "../tv/tv.h"
#include "../tv/tv_channel.h"
#include "../tv/tv_item.h"
#include "../tv/tv_audio.h"
#include "../tv/tv_frame_audio.h"
#include "../tv/tv_window.h"
#include "../tv/transcode/tv_transcode.h"
#include "../util.h"

#include "../file.h"

/*
  tv_window_t handlers

  TODO: I changed tv_channel_t internally. Instead of just commenting out
  the offending blocks and confusing me later, I should redefine these
  commands so they more accurately reflect the internals (create commands
  directly for tv_item_t and binding commands for tv_channel_t)
 */

DEC_CMD(tv_window_set_channel_id){
	id_t_ tv_window_id =
		convert::array::id::from_hex(
			registers[0]);
	id_t_ tv_item_id =
		convert::array::id::from_hex(
			registers[1]);
	tv_window_t *window =
		PTR_DATA(tv_window_id,
			 tv_window_t);
	if(window == nullptr){
		print("window is a nullptr", P_ERR);
	}
	window->set_item_id(tv_item_id);
}

DEC_CMD(tv_window_list_active_streams){
	id_t_ tv_window_id =
		convert::array::id::from_hex(
			registers[0]);
	tv_window_t *window =
		PTR_DATA(tv_window_id,
			 tv_window_t);
	if(window == nullptr){
		print("window is a nullptr", P_ERR);
	}
	output_table = 
		console_generate_generic_id_table(
			window->get_active_streams());
	ADD_COLUMN_TO_TABLE(
		output_table, 0, get_item_id, convert::array::id::to_hex, tv_window_t);
}

DEC_CMD(tv_window_clear_active_streams){
}

DEC_CMD(tv_window_add_active_stream){
}

DEC_CMD(tv_window_del_active_stream){
}

DEC_CMD(tv_window_create){
	tv_window_t *window =
		new tv_window_t;
	output_table =
		console_generate_generic_id_table(
			std::vector<id_t_>({window->id.get_id()}));
}

DEC_CMD(tv_window_set_to_time){
	tv_window_t *window =
		PTR_DATA(
			convert::array::id::from_hex(registers[0]),
			tv_window_t);
	if(window == nullptr){
		print("window is a nullptr", P_ERR);
	}
	uint64_t time = std::stoull(registers[1]);
	window->set_timestamp_offset(time-get_time_microseconds());
}

DEC_CMD(tv_window_set_timestamp_offset){
	tv_window_t *window =
		PTR_DATA(
			convert::array::id::from_hex(registers[0]),
			tv_window_t);
	if(window == nullptr){
		print("window is a nullptr", P_ERR);
	}
	uint64_t time = std::stoull(registers[1]);
	window->set_timestamp_offset(time);
}

/*
  tv_channel_t handlers
 */

DEC_CMD(tv_channel_create){
	tv_channel_t *channel =
		new tv_channel_t;
	output_table =
		console_generate_generic_id_table(
			std::vector<id_t_>({channel->id.get_id()}));
}

/*
  tv_item_t handlers
 */

DEC_CMD(tv_item_create){
	output_table =
		console_generate_generic_id_table(
	{(new tv_item_t)->id.get_id()});
}

DEC_CMD(tv_item_get_stream_list){
	tv_item_t *item =
	 	PTR_DATA(convert::array::id::from_hex(registers[0]),
	 		 tv_item_t);
	if(item == nullptr){
		print("item is a nullptr", P_ERR);
	}
	print("this just fetches the first ID in each frame list for simplicity", P_NOTE);
	// I wouldn't mind changing it over when I get helper functions
	
	std::vector<std::vector<id_t_> > frame_set =
		item->get_frame_id_vector();
	std::vector<id_t_> fixed_id_vector;
	for(uint64_t i = 0;i < frame_set.size();i++){
		if(frame_set[i].size() == 0){
			print("empty frame set, skipping", P_NOTE);
			continue;
		}
		if(frame_set[i].size() > 1){
			print("truncating possibly useful data", P_NOTE);
		}
		fixed_id_vector.push_back(frame_set[i][0]);
	}
	// console_generate_generic_id_table only works on one-dimensional
	// data, and since tv_item_t is the only known major exception to
	// this (as it pertains to getting output to the table), i'm not too
	// worried about getting a helper function. Maybe for something more
	// mission critical though...
	output_table =
		console_generate_generic_id_table(
			fixed_id_vector);
}

DEC_CMD(tv_audio_load_wav){
	print("again, no more tv_audio_load_wav, do it right this time", P_CRIT);
	// id_t_ channel_id =
	// 	convert::array::id::from_hex(registers.at(0));
	// uint64_t start_time_micro_s =
	// 	std::stoull(registers.at(1));
	// std::string file = registers.at(2);
	// ::tv_audio_load_wav(
	// 	channel_id,
	// 	start_time_micro_s,
	// 	file);			
}

/*
  These test commands are self contained systems, so they create
  and use their own variables. The program should be smart enough
  to correctly handle multiple channels and windows by this point
  (and if it doesn't that should be fixed).

  The two parameters this takes is the offset (in microseconds) that the
  broadcast will start at, and the path to the file (absolute or from
  the directory of BasicTV, not the directory of where the telnet client
  is being ran, obviously)
 */

DEC_CMD(tv_test_audio){
	const std::string file =
		registers.at(1);
	tv_window_t *window =
		new tv_window_t;
	tv_channel_t *channel =
		new tv_channel_t;
	tv_item_t *item =
		new tv_item_t;
	ID_DATA_MAKE_TMP(channel->id);
	ID_DATA_MAKE_TMP(window->id);
	ID_DATA_MAKE_TMP(item->id);
	channel->set_description(
		convert::string::to_bytes("BasicTV Audio Test"));
	std::vector<id_t_> all_frame_audios =
		console_tv_load_samples_to_frames(
			console_tv_load_samples_from_file(
				file),
			TV_AUDIO_FORMAT_WAVE);
	item->set_tv_channel_id(
		channel->id.get_id());
	item->add_frame_id(
		all_frame_audios);
	window->set_item_id(
		item->id.get_id());
	window->add_active_stream_id(
		all_frame_audios.at(0));
	output_table =
		console_generate_generic_id_table(
			all_frame_audios);
}

DEC_CMD(tv_test_menu){
	print_socket("not implemented yet\n");
}

DEC_CMD(tv_test_card){
	print_socket("not implemented yet\n");
}
