#include "tv.h"
#include "tv_video.h"
#include "sink/tv_sink.h"

#include "tv_window.h"
#include "tv_frame_audio.h"
#include "tv_frame_video.h"
#include "tv_frame_standard.h"

#include "../util.h"
#include "../id/id_api.h"
#include "../settings.h"

void tv_init(){
	// auto-initializes the PortAudio system since this is the first
	// right now we pull this from ID_TIER_CACHE_GET
	tv::sink::state::init(
		TV_SINK_MEDIUM_AUDIO_HARDWARE,
		TV_SINK_MEDIUM_FLOW_DIRECTION_OUT);
	tv::sink::state::init(
		TV_SINK_MEDIUM_NUMERICAL_TCP_ACCEPT,
		TV_SINK_MEDIUM_FLOW_DIRECTION_IN); // writing
	tv::sink::state::init(
		TV_SINK_MEDIUM_NUMERICAL_TCP_ACCEPT,
		TV_SINK_MEDIUM_FLOW_DIRECTION_OUT); // reading
}

#define TV_LOOP_FRAME_SEARCH_SEEK_SINK(format)\
	void tv_##format##_search_seek_sink(){	\
		std::vector<id_t_> window_vector =	\
		ID_API_CACHE_GET(			\
			TYPE_TV_WINDOW_T);		\
		for(uint64_t i  = 0;i < window_vector.size();i++	\
	}						\
	
void tv_loop(){
	std::vector<id_t_> window_vector =
		ID_TIER_CACHE_GET(
			TYPE_TV_WINDOW_T);
	const uint64_t tv_forward_buffer =
		settings::get_setting_unsigned_def(
			"tv_forward_buffer", 10);
	for(uint64_t i = 0;i < window_vector.size();i++){
		tv_window_t *window_ptr =
			PTR_DATA(window_vector[i],
				 tv_window_t);	
		CONTINUE_IF_NULL(window_ptr, P_WARN);
		const int64_t window_offset =
			window_ptr->get_timestamp_offset_micro_s();
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
		id_t_ latest_id = ID_BLANK_ID;
		switch(get_id_type(std::get<0>(active_streams[0]))){
		case TYPE_TV_FRAME_AUDIO_T:
			if(true){
				tv_frame_audio_t *frame_audio_ptr =
					PTR_DATA(std::get<0>(active_streams[0]),
						 tv_frame_audio_t);
				CONTINUE_IF_NULL(frame_audio_ptr, P_NOTE);
				latest_id =
					tv_frame_scroll_to_time(
						frame_audio_ptr,
						get_time_microseconds() + window_offset);
			}
			break;
		case TYPE_TV_FRAME_VIDEO_T:
			print("no formal video support exists yet", P_WARN);
			continue;
			break;
		default:
			print("ID type given isn't a valid frame type", P_WARN);
			continue;
		}
		if(latest_id != ID_BLANK_ID){
			std::tuple<id_t_, id_t_, std::vector<uint8_t> > new_stream_data =
				active_streams[0];
			window_ptr->set_active_streams(
				std::vector<std::tuple<id_t_, id_t_, std::vector<uint8_t> > >({new_stream_data}));
			tv_sink_state_t *sink_state_ptr =
				PTR_DATA(std::get<1>(active_streams[0]),
					 tv_sink_state_t);
			ASSERT(sink_state_ptr->get_frame_type() ==
			       TV_FRAME_TYPE_AUDIO, P_ERR);
			CONTINUE_IF_NULL(sink_state_ptr, P_WARN);
			data_id_t *data_id_ptr =
				PTR_ID(latest_id, );
			std::vector<id_t_> ids_to_push =
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
			net_proto::request::add_id(
				ids_to_push);
			tv::sink::state::push(
				sink_state_ptr,
				window_offset,
				ids_to_push);
		}
		
	}
}

void tv_close(){
}
