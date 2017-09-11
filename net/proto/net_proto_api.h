#ifndef NET_PROTO_API_H
#define NET_PROTO_API_H

#include "algorithm"

namespace net_proto{
	namespace peer{
		void set_self_peer_id(id_t_ self_peer_id_);
		// IP/URL and port, easy wrapper. Assume default if not called
		void set_self_as_peer(std::string ip, uint16_t port);
		id_t_ get_self_as_peer();
		id_t_ random_peer_id();
		id_t_ optimal_peer_for_id(id_t_ id);

		std::string get_breakdown(id_t_ peer_id_);
	};
};

#include "net_proto.h"

#endif
