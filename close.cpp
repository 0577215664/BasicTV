#include "main.h"
#include "close.h"

#include "encrypt/encrypt.h"
#include "tv/tv.h"
#include "input/input.h"
#include "net/proto/net_proto.h"
#include "console/console.h"
#include "id/id_api.h"
#include "settings.h"

void close(){
	closing = true;
	tv_close();
	input_close();
	net_proto_close();
	console_close();
	id_tier_close();
	id_tier::operation::del_id_from_state(
		{id_tier::state_tier::only_state_of_tier(
				0, 0)},
		id_tier::lookup::ids::from_tier(
			std::vector<std::pair<uint8_t, uint8_t> >({
				std::make_pair(
					ID_TIER_MAJOR_MEM, 0)})));
	ENGINE_cleanup();
	EVP_cleanup();
	ERR_free_strings();
	CRYPTO_cleanup_all_ex_data();
	// SDL_Init implicitly calls subsystems, regardless of what
	// is being passed. This is meant to solve that only
	SDL_Quit();
}
