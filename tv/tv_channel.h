#ifndef TV_CHANNEL_H
#define TV_CHANNEL_H

#include "../main.h"
#include "tv.h"

#include "tv_meta.h"
/*
  tv_channel_t:
  TV channel is a description of what a channel is. The actual stream
  content has been offset to tv_item_t.

  It has a description of the channel, a reference to a wallet set, 
  official website, thumbnail image (?), and other stuff as well.

  TV window NO LONGER NEEDS to directly refer tv_channel_t. Instead, refer
  TV item directly to TV window, and load TV channel's data just inside of
  the 'info' button or description or whatever
 */

/*
  TODO: make sure this works with all Unicode stuff
 */

struct tv_channel_t : public tv_meta_t{
private:
	id_t_ wallet_set_id = ID_BLANK_ID;
public:
	data_id_t id;
	tv_channel_t();
	~tv_channel_t();
	GET_SET_ID(wallet_set_id);
};

#endif
