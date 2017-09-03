#ifndef ID_TIER_MOVE_NETWORK_H
#define ID_TIER_MOVE_NETWORK_H

// fills requests to other peers by copying from the first ID (non-network) to
// the second ID (networked)

// TODO: maybe make a standardization of which tiers can shift with each other
// for latency reasons (or just a cost between a shift maybe?)
extern std::vector<std::tuple<id_t_, id_t_, id_t_, uint8_t> > tier_network_move_push_logic(
	id_t_ first_id,
	id_t_ second_id);
#endif
