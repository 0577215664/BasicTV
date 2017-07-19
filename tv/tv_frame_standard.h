#ifndef TV_FRAME_STANDARD_H
#define TV_FRAME_STANDARD_H
#include "../id/id_api.h"
#include "../net/proto/net_proto_api.h"
#define TV_FRAME_DEFAULT_FREQ 60
#define TV_FRAME_DEFAULT_TTL (1/TV_FRAME_DEFAULT_FREQ)

// starts encoder state on stateful codecs
#define TV_FRAME_STANDARD_START_PACKET (1 << 1)
// ends encoder state on stateful codecs
#define TV_FRAME_STANDARD_END_PACKET (1 << 2)

/*
  codec_state_ref is a random reference number (not to be confused with the
  ID subsystem) that is created for every new encoded segment of a
  tv_frame_standard_t.

  Opus allows me to start giving it packets in order and it'll figure it out,
  but for other codecs that either don't allow that or don't adapt as well,
  the decoder can go back in the linked list until it finds the first entry
  that has the codec_state_ref AND the TV_FRAME_STANDARD_START_PACKET flipped.
  Then it would create a proper decoding state (tv_transcode_state_t) with the
  same identifier (and hash of the tv_frame_*_t as well) and load the
  information chronologically from there, fast forward to the current time, and
  do whatever.
*/

class tv_frame_standard_t{
private:
	uint64_t start_time_micro_s = 0;
	uint32_t ttl_micro_s = 0;
	uint64_t frame_entry = 0;
	uint64_t codec_state_ref = 0;
	uint64_t flags = 0;
public:
	tv_frame_standard_t();
	~tv_frame_standard_t();
	GET_SET_S(codec_state_ref, uint64_t);
	GET_SET_S(flags, uint64_t);
	GET_SET_S(start_time_micro_s, uint64_t);
	GET_SET_S(ttl_micro_s, uint32_t);
	GET_SET_S(frame_entry, uint64_t);
	void list_virtual_data(data_id_t *id);
	void set_standard(uint64_t start_time_micro_s_,
			  uint32_t ttl_micro_s_,
			  uint64_t frame_entry_);
	void get_standard(uint64_t *start_time_micro_s_,
			  uint32_t *ttl_micro_s,
			  uint64_t *frame_entry_);
	bool valid(uint64_t timestamp_micro_s);
	/* uint64_t get_start_time_micro_s(){return start_time_micro_s;} */
	/* uint64_t get_ttl_micro_s(){return ttl_micro_s;} */
	uint64_t get_end_time_micro_s(){return get_start_time_micro_s()+get_ttl_micro_s();}
};

// TODO: protect this against infinite loops

template<typename T>
id_t_ tv_frame_scroll_to_time(T data, uint64_t play_time){
	std::vector<id_t_> request_vector;
	while(data != nullptr){
		const uint64_t start_time_micro_s =
			data->get_start_time_micro_s();
		const uint64_t end_time_micro_s =
			data->get_end_time_micro_s(); // includes ttl_micro_s
		const bool stay =
			BETWEEN(start_time_micro_s,
				play_time,
				end_time_micro_s);
		id_t_ new_id = ID_BLANK_ID;
		if(stay){
			break;
		}else{
			const bool go_forward =
				(play_time > end_time_micro_s);
			std::pair<std::vector<id_t_>, std::vector<id_t_> > linked_list =
				data->id.get_linked_list();
			if(go_forward){
				if(linked_list.second.size() >= 1){
					new_id = linked_list.second[0];
				}else{
					new_id = ID_BLANK_ID;
				}
			}else{
				if(linked_list.first.size() >= 1){
					new_id = linked_list.first[0];
				}else{
					new_id = ID_BLANK_ID;
				}
			}
		} /*  */
		data_id_t *new_id_ptr =
			PTR_ID(new_id, );
		if(new_id_ptr != nullptr){
			std::pair<std::vector<id_t_>, std::vector<id_t_> >  id_pair =
				new_id_ptr->get_linked_list();
			id_pair.first.insert(
				id_pair.first.end(),
				id_pair.second.begin(),
				id_pair.second.end());
			for(uint64_t i = 0;i < id_pair.first.size();i++){
				if(std::find(
					   request_vector.begin(),
					   request_vector.end(),
					   id_pair.first[i]) == request_vector.end()){
					request_vector.push_back(
						id_pair.first[i]);
				}
			}
			data = (T)new_id_ptr->get_ptr();
		}else{
			data = nullptr;
		}
	}
	// only out that isn't valid is data = PTR_ID...
	net_proto::request::add_id(
		request_vector);
	if(data != nullptr){
		return data->id.get_id();
	}
	return ID_BLANK_ID;
}
#endif
