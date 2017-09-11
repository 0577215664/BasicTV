#include "tv_sink_video_hardware.h"
#include "../../tv_sink.h"
#include "../../../../state.h"

TV_SINK_MEDIUM_INIT(video_hardware){
	ASSERT(flow_direction == TV_SINK_MEDIUM_FLOW_DIRECTION_OUT, P_ERR);
	STD_STATE_INIT(tv_sink_state_t,
		       state_ptr,
		       tv_sink_video_hardware_state_t,
		       video_hardware_state_ptr);
}

TV_SINK_MEDIUM_CLOSE(video_hardware){
	STD_STATE_CLOSE(state_ptr, tv_sink_video_hardware_state_t);
}

TV_SINK_MEDIUM_PULL(video_hardware){
	ASSERT(state_ptr != nullptr, P_ERR);
	print("there is no hardware video pushing, we aren't that good", P_ERR);
	return std::vector<id_t_>({});
}

TV_SINK_MEDIUM_PUSH(video_hardware){
}
