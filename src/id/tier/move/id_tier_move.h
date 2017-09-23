#ifndef ID_TIER_MOVE_H
#define ID_TIER_MOVE_H

#define COPY_UP 1
#define COPY_DOWN 2

#include "../../id.h"

extern std::vector<std::tuple<id_t_, id_t_, id_t_, uint8_t> > tier_move_logic(
	id_t_ first_id,
	id_t_ second_id);
#endif
