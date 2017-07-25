#include "tv_sink_metadata_atom.h"
#include "../tv_sink_metadata.h"
#include "../../tv_sink.h"

#include <string>

/*
  Atom shouldn't worry about populating the channel_vector right now, but it
  should sometime in the future.

  The BasicTV Atom feed allows for downloading different bitrate audio/video
  files from the node, which don't inherently have to exist at those bitrates,
  but can be generated on the fly
 */

static std::string atom_get_prefix(){
	return "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
}

static std::string atom_wrap_to_feed(std::string data){
	return "<feed xmlns=\"http://www.w3.org/2005/Atom\">" + data + "</feed>";
}

static std::string atom_wrap_title(std::string data){
	return "<title>" + data + "</title>";
}

TV_SINK_METADATA_INIT(atom){	
	tv_sink_metadata_state_t *metadata_state_ptr =
		new tv_sink_metadata_state_t;
	tv_sink_metadata_atom_state_t *atom_state_ptr =
		new tv_sink_metadata_atom_state_t;
	metadata_state_ptr->set_state_ptr(
		reinterpret_cast<void*>(atom_state_ptr));
	return metadata_state_ptr;
}

TV_SINK_METADATA_CLOSE(atom){
	
}

TV_SINK_METADATA_UPDATE(atom){
}
