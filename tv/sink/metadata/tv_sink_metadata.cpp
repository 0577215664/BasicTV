#include "tv_sink_metadata.h"
#include "../../tv.h"
#include "../../tv_item.h"
#include "../../tv_channel.h"

#include "../../../id/id_api.h"
#include "../../../id/id.h"

static std::vector<tv_sink_metadata_medium_t> tv_sink_metadata_medium;

tv_sink_metadata_state_t::tv_sink_metadata_state_t() : id(this, TYPE_TV_SINK_METADATA_STATE_T){
}

tv_sink_metadata_state_t::~tv_sink_metadata_state_t(){
}
