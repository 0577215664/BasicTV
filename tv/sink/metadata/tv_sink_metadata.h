#ifndef TV_SINK_METADATA_H
#define TV_SINK_METADATA_H
#include "../../../id/id.h"
#include "../../../id/id_api.h"

#include "../../../state.h"

/*
  Interface for generating and sending metadata out to the clearnet. 

  The current (and superior) system in place is Atom 1.0.

  Any metadata sink is generated on the fly based on data we have,
  along with any information given to us (implementation-specific).

  In the case of Atom, we can use the HTTP POST header to pull
  channels, hashes, and whatever (and RSS if I/somebody else makes
  an implementation for it)
 */

#define TV_SINK_METADATA_ATOM 1

struct tv_sink_metadata_state_t : public state_t{
private:
	/*
	  TODO: create one HTTP server that can have tv_sink_metadata_state_t
	  and other state stuff bind to it (on an "only one needed per
	  program instance" basis)

	  Dedicated sockets isn't too bad at the moment
	 */
	id_t_ connection_socket_id = ID_BLANK_ID;
	std::vector<id_t_> socket_vector;
	std::vector<id_t_> channel_set;
public:
	data_id_t id;
	tv_sink_metadata_state_t();
	~tv_sink_metadata_state_t();
	GET_SET(channel_set, std::vector<id_t_>);
};

#define TV_SINK_METADATA_INIT(medium) tv_sink_metadata_state_t *tv_sink_metadata_##medium##_init()
#define TV_SINK_METADATA_CLOSE(medium) void tv_sink_metadata_##medium##_close(tv_sink_metadata_state_t *metadata_state_ptr)
#define TV_SINK_METADATA_UPDATE(medium) void tv_sink_metadata_##medium##_update(tv_sink_metadata_state_t *metadata_state_ptr)

struct tv_sink_metadata_medium_t{
public:
	uint8_t medium = 0;
	
	tv_sink_metadata_state_t* (*init) =
		nullptr;
	void (*close)(tv_sink_metadata_state_t* metadata_state_ptr) =
		nullptr;
	void (*update)(tv_sink_metadata_state_t* metadata_state_ptr) =
		nullptr;
};

extern tv_sink_metadata_medium_t tv_sink_metadata_get_medium(uint8_t medium);

#endif
