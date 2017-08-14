#include "console.h"
#include "../tv/tv.h"
#include "../tv/tv_channel.h"
#include "../tv/tv_item.h"
#include "../tv/tv_frame_audio.h"
#include "../tv/tv_frame_numerical.h"
#include "../tv/tv_window.h"
#include "../util.h"

#include "../tv/sink/tv_sink.h"
#include "../tv/sink/audio/tv_sink_audio_hardware.h"
#include "../tv/sink/numerical/tcp/tv_sink_numerical_tcp_accept.h"

#include "../tv/transcode/tv_transcode.h"
#include "../tv/transcode/tv_transcode_audio.h"
#include "../tv/transcode/tv_transcode_opus.h"

#include "../file.h"

#include "../id/tier/id_tier.h"

/*
  Somewhat hacky, but far better interface for actual BasicTV functionality.
 */

std::string sink_id_to_readable(
	id_t_ sink_id){
	tv_sink_state_t *sink_state_ptr =
		PTR_DATA(sink_id,
			 tv_sink_state_t);
	ASSERT(sink_state_ptr != nullptr, P_ERR);
	std::string retval;
	switch(sink_state_ptr->get_medium()){
	case TV_SINK_MEDIUM_AUDIO_HARDWARE:
		retval += "Speakers (AUDIO_HARDWARE)";
		break;
	case TV_SINK_MEDIUM_NUMERICAL_TCP_ACCEPT:
		retval += "TCP Sink (NUMERICAL_TCP_ACCEPT)";
		break;
	default:
		print("invalid medium", P_ERR);
	}
	retval += "/";
	switch(sink_state_ptr->get_frame_type()){
	case TV_FRAME_TYPE_AUDIO:
		retval += "Audio";
		break;
	case TV_FRAME_TYPE_NUMERICAL:
		retval += "Numerical";
		break;
	default:
		print("invalid frame_type", P_ERR);
	}
	retval += "/";
	switch(sink_state_ptr->get_flow_direction()){
	case TV_SINK_MEDIUM_FLOW_DIRECTION_IN:
		retval += "In";
		break;
	case TV_SINK_MEDIUM_FLOW_DIRECTION_OUT:
		retval += "Out";
		break;
	default:
		print("invalid flow direction", P_ERR);
	}
	return retval;
}

std::vector<uint8_t> console_tv_load_opus_file(
	std::string file){
	std::vector<uint8_t> raw_samples;
	int32_t ogg_opus_error;
	OggOpusFile *opus_file =
		op_open_file(
			file.c_str(),
			&ogg_opus_error);
	if(opus_file == nullptr){
		print("couldn't open the OGG Opus file, error code " + std::to_string(ogg_opus_error), P_ERR);
	}
	opus_int16 pcm[5760*2];
	int samples_read = 0;
	while((samples_read =
	       op_read(opus_file,
		       &(pcm[0]),
		       5760*2,
		       nullptr)) > 0){
		raw_samples.insert(
			raw_samples.end(),
			(uint8_t*)(&(pcm[0])),
			(uint8_t*)(&(pcm[0])+samples_read));
	}
	op_free(opus_file);
	opus_file = nullptr;
	return raw_samples;
}

std::vector<uint8_t> console_tv_load_wave_file(
	std::string filename){
	tv_audio_prop_t wave_audio_prop;
	wave_audio_prop.set_format(
		TV_AUDIO_FORMAT_WAV);
	// TODO: use a second read, make it fail, get the metadata,
	// and plug it into here (or just make a more full suite for raw
	// file interfacing?)
	wave_audio_prop.set_sampling_freq(
		48000);
	wave_audio_prop.set_bit_depth(
		16);
	wave_audio_prop.set_channel_count(
		1);
	std::vector<std::vector<uint8_t> > codec =
		std::vector<std::vector<uint8_t> >(
			{file::read_file_vector(filename)});
	uint32_t sampling_freq =
		48000;
	uint8_t bit_depth =
		16;
	uint8_t channel_count =
		1;
	std::vector<uint8_t> retval =
		transcode::audio::codec::to_raw(
			&codec,
			&wave_audio_prop,
			&sampling_freq,
			&bit_depth,
			&channel_count);
	return retval;
}

std::vector<id_t_> console_tv_load_samples_to_frames(
	std::vector<uint8_t> raw_samples,
	uint8_t frame_format){
	// Intermediate (raw to codec)
	tv_audio_prop_t int_audio_prop;
	int_audio_prop.set_format(
		frame_format);
	int_audio_prop.set_flags(
		TV_AUDIO_PROP_FORMAT_ONLY);
	int_audio_prop.set_sampling_freq(
		48000);
	int_audio_prop.set_bit_rate(
		65536); // only read by Opus
	int_audio_prop.set_channel_count(
		1);
	int_audio_prop.set_bit_depth(
		16);
	
	// Final frame output
	tv_audio_prop_t frame_audio_prop;
	frame_audio_prop.set_format(
		frame_format);
	frame_audio_prop.set_flags(
		TV_AUDIO_PROP_FORMAT_ONLY);
	
	// standard output properties for Opus to raw samples
	const uint32_t sampling_freq =
		48000;
	const uint8_t bit_depth =
		16;
	const uint8_t channel_count =
		1;
	
	/*
	  to_frames with the same format SHOULD repacketize each individual
	  frame, one at a time, until everything is finished, and do no
	  conversions whatsoever if the outputs can be valid as the inputs
	  (i.e. no specified output sampling freq, bit depth, or special encoder
	  jargon)

	  Right now it just decodes and encodes everything, which is OK for
	  small loads, but becomes unreasonable very quickly
	 */

	std::vector<std::vector<uint8_t> > packetized_codec_data =
		transcode::audio::raw::to_codec(
			&raw_samples,
			sampling_freq,
			bit_depth,
			channel_count,
			&int_audio_prop);
	if(packetized_codec_data.size() == 0){
		print("packetized_codec_data is empty", P_ERR);
	}
	std::vector<id_t_> retval =
		transcode::audio::codec::to_frames(
			&packetized_codec_data,
			&int_audio_prop,
			&frame_audio_prop,
			1000*1000);
	id_api::linked_list::link_vector(
		retval, 10);
	PRINT_AUDIO_PROP(int_audio_prop);
	PRINT_AUDIO_PROP(frame_audio_prop);
	const uint64_t snippet_duration =
		frame_audio_prop.get_snippet_duration_micro_s();
	ASSERT(snippet_duration != 0, P_ERR);
	const uint64_t true_start_time =
		get_time_microseconds();
	uint64_t start_time =
		true_start_time;
	for(uint64_t i = 0;i < retval.size();i++){
		tv_frame_audio_t *frame_audio =
			PTR_DATA(retval[i],
				 tv_frame_audio_t);
		if(frame_audio == nullptr){
			print("frame_audio is a nullptr", P_ERR);
		}
		ASSERT(frame_audio->get_packet_set().size() != 0, P_ERR);
		frame_audio->set_start_time_micro_s(
			start_time);
		start_time += snippet_duration*frame_audio->get_packet_set().size();
		P_V(start_time, P_NOTE);
	}
	P_V((start_time-true_start_time)/(1000000), P_NOTE);
	if(retval.size() == 0){
		print("frame vector is empty", P_ERR);
	}
	return retval;
}

std::vector<uint8_t> console_tv_load_samples_from_file(
	std::string file){
	if(file.substr(file.size()-4, 4) == "opus"){
		print("interpreted as Opus", P_NOTE);
		return console_tv_load_opus_file(
			file);
	}
	if(file.substr(file.size()-3, 3) == "wav"){
		print("interpreted as WAVE", P_NOTE);
		return console_tv_load_wave_file(
			file);
	}
	print("not a supported type", P_ERR);
	return std::vector<uint8_t>({});
}


std::string console_t::tv_manager_read_string(
	net_socket_t *console_inbound_socket){
	std::string retval;
	bool in_loop = true;
	while(in_loop){
		std::vector<uint8_t> tmp_vector =
			console_inbound_socket->recv(1, NET_SOCKET_RECV_NO_HANG);
		if(tmp_vector.size() != 0){
			if(tmp_vector[0] != '\r' && tmp_vector[0] != '\n'){
				retval += std::string(1, tmp_vector[0]);
			}else{
				if(retval.size() != 0){
					in_loop = false;
				}
			}
		}
	}
	console_inbound_socket->send(retval + "\n");
	return retval;
}

static id_t_ frame_id_to_state_id(
	id_t_ frame_id,
	uint8_t flow_direction){
	uint8_t medium = 0;
	switch(get_id_type(frame_id)){
	case TYPE_TV_FRAME_AUDIO_T:
		medium = TV_SINK_MEDIUM_AUDIO_HARDWARE;
		break;
	case TYPE_TV_FRAME_NUMERICAL_T:
		medium = TV_SINK_MEDIUM_NUMERICAL_TCP_ACCEPT;
		break;
	default:
		print("invalid frame type", P_ERR);
	}
	std::vector<id_t_> sink_state_vector =
		ID_TIER_CACHE_GET(
			TYPE_TV_SINK_STATE_T);
	for(uint64_t i = 0;i < sink_state_vector.size();i++){
		tv_sink_state_t *sink_state_ptr =
			PTR_DATA(sink_state_vector[i],
				 tv_sink_state_t);
		CONTINUE_IF_NULL(sink_state_ptr, P_WARN);
		if(sink_state_ptr->get_medium() == medium &&
		   sink_state_ptr->get_flow_direction() == flow_direction){
			print("found valid TV sink", P_DEBUG);
			return sink_state_vector[i];
		}
	}
	print("can't find a valid sink", P_ERR);
	return ID_BLANK_ID;
}

void console_t::tv_manager_load_item_to_channel(
	net_socket_t *console_inbound_socket){
	print_socket("Channel ID:");
	const std::string channel_id =
		tv_manager_read_string(
			console_inbound_socket);
	print_socket("Start Time Offset (microseconds):");
	const std::string start_time_micro_s_offset =
		tv_manager_read_string(
			console_inbound_socket);
	tv_item_t *new_item =
		new tv_item_t;
	print_socket("Item Name:");
	std::string name =
		tv_manager_read_string(
			console_inbound_socket);
	new_item->set_name(
		std::vector<uint8_t>(
			name.c_str(),
			name.c_str()+name.size()));
	print_socket("Item Desc:");
	std::string desc =
		tv_manager_read_string(
			console_inbound_socket);
	new_item->add_param(
		VORBIS_COMMENT_PARAM_DESCRIPTION,
		desc,
		true);
	print_socket("Item Type (1: Audio, 2: Numerical)");
	const std::string item_type =
		tv_manager_read_string(
			console_inbound_socket);
	print_socket("interpreted channel ID as " +
		     convert::array::id::to_hex(
			     convert::array::id::from_hex(
				     channel_id)) + "\n");
	tv_channel_t *channel_ptr =
		PTR_DATA(convert::array::id::from_hex(channel_id),
			 tv_channel_t);
	new_item->set_tv_channel_id(
		convert::array::id::from_hex(channel_id));
	if(item_type == "1"){
		print_socket("File Path (WAV only):");
		const std::string file_path =
			tv_manager_read_string(
				console_inbound_socket);
		if(channel_ptr == nullptr){
			print_socket("channel_ptr is a nullptr");
		}
		print_socket("Frame Format (OPUS or WAVE):");
		std::string frame_format =
			tv_manager_read_string(
				console_inbound_socket);
		uint8_t frame_format_byte =
			0;
		if(frame_format == "OPUS"){
			frame_format_byte = TV_AUDIO_FORMAT_OPUS;
		}else if(frame_format == "WAVE"){
			frame_format_byte = TV_AUDIO_FORMAT_WAVE;
		}
		std::vector<id_t_> frame_id_vector =
			console_tv_load_samples_to_frames(
				console_tv_load_samples_from_file(
					file_path),
				frame_format_byte);
		tv_frame_audio_t *frame_audio_end_ptr =
			PTR_DATA(frame_id_vector[frame_id_vector.size()-1],
				 tv_frame_audio_t);
		ASSERT(frame_audio_end_ptr, P_ERR);
		new_item->add_frame_id(
			frame_id_vector);
		new_item->set_tv_channel_id(
			convert::array::id::from_hex(channel_id));
		new_item->set_start_time_micro_s(
			get_time_microseconds()+std::stoi(
				start_time_micro_s_offset));
		new_item->set_end_time_micro_s(
			frame_audio_end_ptr->get_end_time_micro_s());
		print_socket("added data properly\n");
	}else{
		print_socket("numerical streams are stream only\n"
			     "open a TCP connection to the BasicTV node (port 59051) "
			     "and send data in the following format\n"
			     "[UNIQUE NUMBER PER NUMERICAL DEVICE] [VALUE] [UNIT] [TIMESTAMP MICRO S]\n"
			     "Viewers, once they bind the type to a window, should connect to their BasicTV nodes (port 59050)\n");
		tv_frame_numerical_t *frame_numerical_ptr =
			new tv_frame_numerical_t;
		// this is only here so there is a common pointer that both
		// the tv_item_t has access to (on the playback side) and the
		// tv_window_t has (on the recording side)
		// technically having the same start and end times should work
		// since we have a frame_entry incrementor for each frame
		frame_numerical_ptr->set_start_time_micro_s(
			get_time_microseconds());
		frame_numerical_ptr->set_ttl_micro_s(
			1);
		new_item->add_frame_id(
			{frame_numerical_ptr->id.get_id()});
		print_socket("added data properly\n");
	}
	output_table =
		console_generate_generic_id_table(
			std::vector<id_t_>(
				{new_item->id.get_id(),
						channel_ptr->id.get_id()}));
}

void console_t::tv_manager_bind_item_to_window(
	net_socket_t *console_inbound_socket){
	print_socket("Item ID:");
	const std::string item_id =
		tv_manager_read_string(
			console_inbound_socket);
	print_socket("Sink ID:");
	const std::string sink_id =
		tv_manager_read_string(
			console_inbound_socket);
	print_socket("Offset from Live TV (0 for Live, - for Rewind, + for Forwards):");
	const std::string time_offset_micro_s_str =
		tv_manager_read_string(
			console_inbound_socket);
	tv_item_t *item_ptr =
		PTR_DATA(convert::array::id::from_hex(item_id),
			 tv_item_t);
	if(item_ptr == nullptr){
		print_socket("item_ptr is a nullptr");	
		print("item_ptr is a nullptr", P_ERR);
	}
	std::vector<id_t_> window_vector =
		ID_TIER_CACHE_GET(
			TYPE_TV_WINDOW_T);
	tv_window_t *window_ptr =
		new tv_window_t;
	// NEED to use a new window_ptr for pretty much everything at this
	// point, since timestamp_offset_micro_s applies to everything
	tv_sink_state_t *sink_state_ptr =
		PTR_DATA(convert::array::id::from_hex(sink_id),
			 tv_sink_state_t);
	if(sink_state_ptr == nullptr){
		print_socket("sink_state_ptr is a nullptr\n");
		print("sink_state_ptr is a nullptr", P_ERR);
	}
	switch(sink_state_ptr->get_flow_direction()){
	case TV_SINK_MEDIUM_FLOW_DIRECTION_IN:
		print_socket("an input sink is specified, will be recording, encoding, and transmitting live\n");
		break;
	case TV_SINK_MEDIUM_FLOW_DIRECTION_OUT: // only mediums can have BOTH
		print_socket("an output sink is specified, will follow the linked list of the frame\n");
		break;
	default:
		print_socket("invalid sink state direction\n");
		print("invalid sink state direction", P_ERR);
		break;
	}
	const uint64_t timestamp_offset =
		std::stoi(time_offset_micro_s_str);
	print_socket("interpreted timestamp offset as " + std::to_string(timestamp_offset) + "\n");
	window_ptr->set_timestamp_offset_micro_s(
		timestamp_offset);
	window_ptr->set_item_id(
		item_ptr->id.get_id());
	std::vector<std::tuple<id_t_, id_t_, std::vector<uint8_t> > > streams =
		window_ptr->get_active_streams();;
	streams.push_back(
		std::make_tuple(
			item_ptr->get_frame_id_vector()[0][0],
			sink_state_ptr->id.get_id(),
			std::vector<uint8_t>({})));
	window_ptr->set_active_streams(
		streams);
	print_socket("everything should be loaded nicely now, right?\n");
}

void console_t::tv_manager_list_window_streams(){
	std::vector<id_t_> window_vector =
		ID_TIER_CACHE_GET(
			TYPE_TV_WINDOW_T);
	// we can have more, but it simplifies the UI
	output_table.clear();
	output_table.push_back(
		std::vector<std::string>(
			{"Current Frame ID",
					"Sink"}));
	for(uint64_t i = 0;i < window_vector.size();i++){
		tv_window_t *window_ptr =
			PTR_DATA(window_vector[i],
				 tv_window_t);
		if(window_ptr == nullptr){
			print_socket("window_ptr is a nullptr\n");
			print("window_ptr is a nullptr", P_ERR);
		}
		std::vector<std::tuple<id_t_, id_t_, std::vector<uint8_t> > > active_streams =
			    window_ptr->get_active_streams();	    
		for(uint64_t c = 0;c < active_streams.size();c++){
			std::string id_str =
				convert::array::id::to_hex(
					std::get<0>(active_streams[c]));
			std::string sink_str =
				sink_id_to_readable(
					std::get<1>(active_streams[c]));
			output_table.push_back(
				{id_str, sink_str});
		}
	}
}

void console_t::tv_manager_list_channels_and_items(){
	std::vector<id_t_> item_vector =
		ID_TIER_CACHE_GET(
			TYPE_TV_ITEM_T);
	// uses output_table
	output_table.clear();
	output_table.push_back(
		{"Item ID",
				"Name",
				"Desc",
				"Live?",
				"Channel ID"});
	for(uint64_t i = 0;i < item_vector.size();i++){
		tv_item_t *item_ptr =
			PTR_DATA(item_vector[i],
				 tv_item_t);
		if(item_ptr == nullptr){
			print_socket("NULL ITEM\n");
			continue;
		}
		tv_channel_t *channel =
			PTR_DATA(item_ptr->get_tv_channel_id(),
				 tv_channel_t);
		if(channel == nullptr){
			print_socket("NULL CHANNEL\n");
			continue;
		}
		std::vector<std::string> datum;
		try{
			datum = std::vector<std::string>(
				{convert::array::id::to_hex(item_vector[i]),
						item_ptr->search_for_param(VORBIS_COMMENT_PARAM_TITLE).at(0),
						item_ptr->search_for_param(VORBIS_COMMENT_PARAM_DESCRIPTION).at(0),
						(BETWEEN(item_ptr->get_start_time_micro_s(), get_time_microseconds(), item_ptr->get_end_time_micro_s())) ? "Yes" : "No",
						convert::array::id::to_hex(item_ptr->get_tv_channel_id())
						// "Wallet Set ID: " + convert::array::id::to_hex(channel->get_wallet_set_id()), // temporary
						});
			output_table.push_back(
				datum);
		}catch(...){}
	}
}

void console_t::tv_manager_list_sinks(){
	std::vector<id_t_> sink_vector =
		ID_TIER_CACHE_GET(
			TYPE_TV_SINK_STATE_T);
	output_table.clear();
	output_table.push_back(
		{"Sink ID",
				"Medium Type",
				"Frame Type",
				"Flow Direction"});
	for(uint64_t i = 0;i < sink_vector.size();i++){
		tv_sink_state_t *sink_state_ptr =
			PTR_DATA(sink_vector[i],
				 tv_sink_state_t);
		if(sink_state_ptr == nullptr){
			print_socket("sink_state_ptr is a nullptr\n");
			print("sink_state_ptr is a nullptr", P_WARN);
		}
		std::string sink_id =
			convert::array::id::to_hex(
				sink_vector[i]);
		std::string medium_type;
		switch(sink_state_ptr->get_medium()){
		case TV_SINK_MEDIUM_AUDIO_HARDWARE:
			medium_type = "Speakers (AUDIO_HARDWARE)";
			break;
		case TV_SINK_MEDIUM_NUMERICAL_TCP_ACCEPT:
			medium_type = "TCP Sink (NUMERICAL_TCP_ACCEPT)";
			break;
		default:
			print_socket("invalid medium\n");
			print("invalid medium", P_ERR);
		}
		std::string frame_type;
		switch(sink_state_ptr->get_frame_type()){
		case TV_FRAME_TYPE_AUDIO:
			frame_type = "Audio";
			break;
		case TV_FRAME_TYPE_NUMERICAL:
			frame_type = "Numerical";
			break;
		default:
			print_socket("invalid frame_type\n");
			print("invalid frame_type", P_ERR);
		}
		std::string flow_direction;
		switch(sink_state_ptr->get_flow_direction()){
		case TV_SINK_MEDIUM_FLOW_DIRECTION_IN:
			flow_direction = "In";
			break;
		case TV_SINK_MEDIUM_FLOW_DIRECTION_OUT:
			flow_direction= "Out";
			break;
		default:
			print_socket("invalid flow direction\n");
			print("invalid flow direction", P_ERR);
		}
		output_table.push_back(
			std::vector<std::string>(
				{sink_id,
						medium_type,
						frame_type,
						flow_direction}));
	}
}

void console_t::tv_manager_create_tv_channel(
	net_socket_t *console_inbound_socket){
	print_socket("Channel Name: ");
	const std::string name_str =
		tv_manager_read_string(
			console_inbound_socket);
	print_socket("Channel Description: ");
	const std::string desc_str =
		tv_manager_read_string(
			console_inbound_socket);
	print_socket("Channel Wallet Set ID (NULL for nothing): ");
	const std::string wallet_set_id_str =
		tv_manager_read_string(
			console_inbound_socket);
	tv_channel_t *channel_ptr = nullptr;
	try{
		channel_ptr = new tv_channel_t;
		channel_ptr->add_param(
			VORBIS_COMMENT_PARAM_TITLE,
			name_str,
			true); // replace
		channel_ptr->add_param(
			VORBIS_COMMENT_PARAM_DESCRIPTION,
			desc_str,
			true);
		if(wallet_set_id_str != "NULL"){
			channel_ptr->set_wallet_set_id(
				convert::array::id::from_hex(
					wallet_set_id_str));
		}
	}catch(...){
		print_socket("unable to interpret input\n");
		if(channel_ptr != nullptr){
			ID_TIER_DESTROY(channel_ptr->id.get_id());
			channel_ptr = nullptr;
		}
		return;
	}
	output_table =
		console_generate_generic_id_table(
			std::vector<id_t_>(
			{channel_ptr->id.get_id()}));
}

void console_t::tv_manager_play_loaded_item_live(
	net_socket_t *console_inbound_socket){
	print_socket("Item ID:");
	const std::string item_id =
		tv_manager_read_string(
			console_inbound_socket);
	tv_item_t *item_ptr =
		PTR_DATA(convert::array::id::from_hex(item_id),
			 tv_item_t);
	if(item_ptr == nullptr){
		print_socket("item_ptr is a nullptr");	
		print("item_ptr is a nullptr", P_ERR);
	}
	tv_channel_t *channel_ptr =
		PTR_DATA(item_ptr->get_tv_channel_id(),
			 tv_channel_t);
	// technically we don't need channel_ptr, but printing out metadata
	// is something i'd like to do real soon
	if(channel_ptr == nullptr){
		print_socket("channel_ptr is a nullptr\n");
		print("channel_ptr is a nullptr", P_ERR);
	}
	std::vector<id_t_> window_vector =
		ID_TIER_CACHE_GET(
			TYPE_TV_WINDOW_T);
	for(uint64_t i = 0;i < window_vector.size();i++){
		ID_TIER_DESTROY(window_vector[i]);
	}
	tv_window_t *window_ptr = 
		new tv_window_t;
	const int64_t timestamp_offset =
		item_ptr->get_start_time_micro_s()-get_time_microseconds()-(10*1000*1000);
	print_socket("interpreted timestamp offset as " + std::to_string(timestamp_offset) + "\n");
	window_ptr->set_timestamp_offset_micro_s(
		timestamp_offset);
	window_ptr->set_item_id(
		item_ptr->id.get_id());
	window_ptr->set_active_streams(
		std::vector<std::tuple<id_t_, id_t_, std::vector<uint8_t> > >({
				std::make_tuple(
					item_ptr->get_frame_id_vector().at(0).at(0),
					frame_id_to_state_id(
						item_ptr->get_frame_id_vector().at(0).at(0),
						TV_SINK_MEDIUM_FLOW_DIRECTION_OUT),
					std::vector<uint8_t>({}))
					}));
	print_socket("everything should be loaded nicely now, right?\n");

}

void console_t::tv_manager_create_sink(net_socket_t *console_inbound_socket){
	print_socket("Medium (AUDIO_HARDWARE or NUMERICAL_TCP_ACCEPT): ");
	const std::string medium_type_str =
		tv_manager_read_string(
			console_inbound_socket);
	print_socket("Flow Direction (IN or OUT): ");
	const std::string flow_direction_str =
		tv_manager_read_string(
			console_inbound_socket);
	uint8_t medium = 0;
	uint8_t flow_direction = 0;
	if(medium_type_str == "AUDIO_HARDWARE"){
		medium = TV_SINK_MEDIUM_AUDIO_HARDWARE;
	}else if(medium_type_str == "NUMERICAL_TCP_ACCEPT"){
		medium = TV_SINK_MEDIUM_NUMERICAL_TCP_ACCEPT;
	}else{
		print_socket("invalid medium\n");
		print("invalid medium", P_ERR);
	}

	if(flow_direction_str == "IN"){
		flow_direction = TV_SINK_MEDIUM_FLOW_DIRECTION_IN;
	}else if(flow_direction_str == "OUT"){
		flow_direction = TV_SINK_MEDIUM_FLOW_DIRECTION_OUT;
	}else{
		print_socket("invalid flow direction\n");
		print("invalid flow direction", P_ERR);
	}
	tv_sink_state_t *sink_state_ptr =
		tv::sink::state::init(
			medium,
			flow_direction);
	if(sink_state_ptr == nullptr){
		print_socket("sink_state_ptr is a nullptr\n");
		print("sink_state_ptr is a nullptr", P_ERR);
	}
	print_socket("Queueing up medium-specific settings\n");
	switch(medium){
	case TV_SINK_MEDIUM_AUDIO_HARDWARE:
		print_socket("no settings for AUDIO_HARDWARE\n");
		break;
	case TV_SINK_MEDIUM_NUMERICAL_TCP_ACCEPT:
		if(true){
			// TODO: maybe allow whitelisting IP address ranges?
			print_socket("Incoming Port: ");
			const std::string incoming_port_str =
				tv_manager_read_string(
					console_inbound_socket);
			uint64_t incoming_port =
				std::stoi(incoming_port_str);
			if(incoming_port > 65535){
				print_socket("port falls outside of 16-bit range\n");
				print("port falls outside of 16-bit range", P_ERR);
			}
			tv_sink_numerical_tcp_accept_state_t *tcp_accept_state_ptr =
				reinterpret_cast<tv_sink_numerical_tcp_accept_state_t*>(
					sink_state_ptr->get_state_ptr());
			if(tcp_accept_state_ptr == nullptr){
				print_socket("tcp_accept_state_ptr is a nullptr\n");
				print("tcp_accept_state_ptr is a nullptr", P_ERR);
			}
			net_socket_t *socket_ptr =
				PTR_DATA(tcp_accept_state_ptr->get_conn_socket_id(),
					 net_socket_t);
			if(socket_ptr == nullptr){
				print_socket("socket_ptr is a nullptr\n");
				print("socket_ptr is a nullptr", P_ERR);
			}
			socket_ptr->set_net_ip(
				"", static_cast<uint16_t>(incoming_port));
		}
		break;
	default:
		print_socket("invalid medium (after proper sink creation?)\n");
	}
}

void console_t::tv_manager_print_options(){
	const std::string tmp =
		"(1) Load TV Item to Channel\n"
		"(2) Bind TV Item to Window\n"
		"(3) Play Loaded TV Item in 10 Seconds\n"
		"(4) List TV Window Streams\n"
		"(5) List TV Items\n"
		"(6) List TV Sinks\n"
		"(7) Create a Sink\n"
		"(8) Create TV Channel\n"
		"(9) Exit TV Manager\n"
		"] ";
	print_socket(tmp);
}

DEC_CMD(tv_manager){
	net_socket_t* console_inbound_socket =
		PTR_DATA(get_socket_id(),
			 net_socket_t);
	if(console_inbound_socket == nullptr){
		print_socket("console_inbound_socket is a nullptr\n");
		print("console_inbound_socket is a nullptr", P_ERR);
	}
	bool tv_manager_loop = true;
	while(tv_manager_loop){
		print_socket("BasicTV TV Manager\nWARNING: tv_manager currently blocks the main thread of execution, exit when you aren't doing something useful\n");
		tv_manager_print_options();
		std::string read_string =
			tv_manager_read_string(
				console_inbound_socket);
		const uint64_t option =
			std::stoi(read_string);
		print_socket("interpreted input as " + std::to_string(option) + "\n");
		output_table.clear();
		switch(option){
		case 1:
			tv_manager_load_item_to_channel(console_inbound_socket);
			break;
		case 2:
			tv_manager_bind_item_to_window(console_inbound_socket);
			break;
		case 3:
			tv_manager_play_loaded_item_live(console_inbound_socket);
			break;
		case 4:
			tv_manager_list_window_streams();
			break;
		case 5:
			tv_manager_list_channels_and_items();
			break;
		case 6:
			tv_manager_list_sinks();
			break;
		case 7:
			tv_manager_create_sink(console_inbound_socket);
			break;
		case 8:
			tv_manager_create_tv_channel(console_inbound_socket);
			break;
		case 9:
			print_socket("closing TV manager\n");
			tv_manager_loop = false;
			break;
		default:
			print_socket("invalid option\n");
			return;
		}
		print_output_table();
	}
}
