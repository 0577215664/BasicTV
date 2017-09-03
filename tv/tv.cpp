#include "tv.h"
#include "sink/tv_sink.h"

#include "tv_window.h"
#include "tv_frame_audio.h"
#include "tv_frame_video.h"
#include "tv_frame_standard.h"
#include "tv_frame_numerical.h"

#include "../util.h"
#include "../id/id_api.h"
#include "../settings.h"

void tv_init(){
	// auto-initializes the PortAudio system since this is the first
	// right now we pull this from ID_TIER_CACHE_GET
	if(settings::get_setting("audio") == "true"){
		tv::sink::state::init(
			TV_SINK_MEDIUM_AUDIO_HARDWARE,
			TV_SINK_MEDIUM_FLOW_DIRECTION_OUT);
	}
	tv::sink::state::init(
		TV_SINK_MEDIUM_NUMERICAL_TCP_ACCEPT,
		TV_SINK_MEDIUM_FLOW_DIRECTION_IN); // writing
	tv::sink::state::init(
		TV_SINK_MEDIUM_NUMERICAL_TCP_ACCEPT,
		TV_SINK_MEDIUM_FLOW_DIRECTION_OUT); // reading
}

#define FRAME_TYPE_SEEK(frame_type) if(true){frame_type *ptr = PTR_DATA(std::get<0>(stream), frame_type);PRINT_IF_NULL(ptr, P_UNABLE);latest_id = tv_frame_scroll_to_time(ptr, get_time_microseconds()+window_offset);}break;

// Playback towards outward sinks
static void tv_loop_sink_outward_flow(
	std::tuple<id_t_, id_t_, std::vector<uint8_t> > stream,
	id_t_ *last_pushed_id, 
	tv_window_t *window_ptr){

	const int64_t window_offset =
		window_ptr->get_timestamp_offset_micro_s();
	// should work fine, but haven't had a use case yet, so haven't tested
	id_t_ latest_id = ID_BLANK_ID;
	switch(get_id_type(std::get<0>(stream))){
	case TYPE_TV_FRAME_AUDIO_T:
		FRAME_TYPE_SEEK(tv_frame_audio_t);
	case TYPE_TV_FRAME_NUMERICAL_T:
		FRAME_TYPE_SEEK(tv_frame_numerical_t);
	case TYPE_TV_FRAME_VIDEO_T:
		print("no formal video support exists yet", P_WARN);
		return;
	default:
		print("ID type given isn't a valid frame type", P_WARN);
		return;
	}
	if(latest_id != ID_BLANK_ID){
		/*
		  TODO: standardize how the data is actually leaving the 
		  program: instantly or pushing for a callback.

		  Right now audio is the only system that uses a callback,
		  so it decompresses the data in the main thread ahead of
		  time, offloads to a vector, and the callback takes care
		  of the rest.
		*/

		if(*last_pushed_id != ID_BLANK_ID &&
		   *last_pushed_id == latest_id){
			return;
		}

		std::get<0>(stream) = latest_id;
		window_ptr->set_active_streams(
			{stream});
		tv_sink_state_t *sink_state_ptr =
			PTR_DATA(std::get<1>(stream),
				 tv_sink_state_t);
		PRINT_IF_NULL(sink_state_ptr, P_ERR);
		std::vector<id_t_> ids_to_push;
		if(get_id_type(std::get<0>(stream)) == TV_FRAME_TYPE_AUDIO){
			data_id_t *data_id_ptr =
				PTR_ID(latest_id, );
			ids_to_push =
				data_id_ptr->get_linked_list().second;
			ids_to_push.insert(
				ids_to_push.begin(),
				latest_id);
			uint64_t old_size = ~0;
			// finds last ID in the list we have an ID for, and
			// gets the end of that ID until we have no more growth
			// sanity checks on whether we have the individual IDs
			// are handled inside of tv::sink::state::push (more
			// specifically, ATM, the audio_hardware push)
			const uint64_t tv_forward_buffer =
				settings::get_setting_unsigned_def(
					"tv_forward_buffer", 10);

			while(ids_to_push.size() < tv_forward_buffer &&
			      ids_to_push.size() != old_size){
				old_size = ids_to_push.size();
				data_id_t *frame_id_ptr =
					nullptr;
				for(int64_t c = ids_to_push.size()-1;c >= 0;c--){
					frame_id_ptr =
						PTR_ID(ids_to_push[c], );
					if(frame_id_ptr != nullptr){
						break;
					}
				}
				std::vector<id_t_> linked_list_forward =
					frame_id_ptr->get_linked_list().second;
				ids_to_push.insert(
					ids_to_push.end(),
					linked_list_forward.begin(),
					linked_list_forward.end());
			}
			if(ids_to_push.size() > tv_forward_buffer){
				ids_to_push.erase(
					ids_to_push.begin()+tv_forward_buffer,
					ids_to_push.end());
			}
		}else{
			ids_to_push =
				std::vector<id_t_>({latest_id});
		}
		tv::sink::state::push(
			sink_state_ptr,
			window_offset,
			ids_to_push);
		*last_pushed_id =
			ids_to_push[ids_to_push.size()-1];
	}
	
}

// Live streams from inward sinks
static void tv_loop_sink_inward_flow(
	std::tuple<id_t_, id_t_, std::vector<uint8_t> > stream,
	tv_window_t *window_ptr){
	std::vector<id_t_> new_frames =
		tv::sink::state::pull(
			std::get<1>(stream),
			0);
	if(new_frames.size() > 0){
		print("created " + std::to_string(new_frames.size()) + " frames of type " +
		      convert::type::from(get_id_type(new_frames.at(0))), P_DEBUG);
		id_api::linked_list::link_vector(
			new_frames,
			10);
		data_id_t *old_frame_id =
			PTR_ID(std::get<0>(stream), );
		ASSERT(old_frame_id != nullptr, P_ERR);
		old_frame_id->set_linked_list(
			std::make_pair(
				old_frame_id->get_linked_list().first,
				new_frames));
		std::get<0>(stream) =
			new_frames[new_frames.size()-1];
		window_ptr->set_active_streams(
			{stream});
	}
}

void tv_loop(){
	std::vector<id_t_> window_vector =
		ID_TIER_CACHE_GET(
			TYPE_TV_WINDOW_T);
	for(uint64_t i = 0;i < window_vector.size();i++){
		tv_window_t *window_ptr =
			PTR_DATA(window_vector[i],
				 tv_window_t);	
		CONTINUE_IF_NULL(window_ptr, P_WARN);
		const std::vector<
			std::tuple<
				id_t_,
				id_t_,
				std::vector<uint8_t> > > active_streams =
			window_ptr->get_active_streams();
		CONTINUE_IF_TRUE(active_streams.size() == 0);
		if(active_streams.size() > 1){
			print("more than one active stream at one time, weird...", P_WARN);
		}
		for(uint64_t c = 0;c < active_streams.size();c++){
			tv_sink_state_t *sink_state_ptr =
				PTR_DATA(std::get<1>(active_streams[c]),
					 tv_sink_state_t);
			if(window_ptr->get_last_pushed_frame_mapping().size() !=
			   active_streams.size()){
				window_ptr->set_last_pushed_frame_mapping(
					std::vector<id_t_>(
						active_streams.size(),
						ID_BLANK_ID));
			}
			switch(sink_state_ptr->get_flow_direction()){
			case TV_SINK_MEDIUM_FLOW_DIRECTION_IN:
				tv_loop_sink_inward_flow(
					active_streams[c],
					window_ptr);
				break;
			case TV_SINK_MEDIUM_FLOW_DIRECTION_OUT:
				tv_loop_sink_outward_flow(
					active_streams[c],
					&(window_ptr->get_last_pushed_frame_mapping()[c]),
					window_ptr);
				break;
			default:
				print("invalid flow direction for tv sink", P_ERR);
			}
		}
		
	}
}

void tv_close(){
}
