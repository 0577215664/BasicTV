#ifndef ID_TIER_DEFINE_H
#define ID_TIER_DEFINE_H
// useful macro functions for all medium

#define GET_ALL_STATE_PTR(medium_)				\
	id_tier_state_t *tier_state_ptr =			\
		PTR_DATA(state_id,				\
			 id_tier_state_t);			\
	ASSERT(tier_state_ptr != nullptr, P_ERR);		\
	id_tier_##medium_##_state_t* medium_##_state_ptr =	\
		(id_tier_##medium_##_state_t*)(			\
			tier_state_ptr->get_payload());		\
	ASSERT(medium_##_state_ptr != nullptr, P_ERR);


#define ID_TIER_LOOP_STANDARD(add_func, get_func)			\
	for(uint64_t i = 0;i < tier_state_ptr->inbound_transport.size();i++){ \
		if(tier_state_ptr->inbound_transport[i].get_payload().size() == 0){ \
			bool complete = false;				\
			try{						\
				tier_state_ptr->inbound_transport[i].set_payload( \
					get_func(			\
						state_id,		\
						tier_state_ptr->inbound_transport[i].get_payload_id())); \
				complete = true;			\
			}catch(...){					\
			}						\
			if(complete){					\
				ASSERT(tier_state_ptr->inbound_transport[i].get_payload().size() != 0, P_ERR); \
			}						\
		}							\
	}								\
	for(uint64_t i = 0;i < tier_state_ptr->outbound_transport.size();i++){ \
		if(tier_state_ptr->outbound_transport[i].get_payload().size() != 0){ \
			bool complete = false;				\
			try{						\
				add_func(				\
					state_id,			\
					tier_state_ptr->outbound_transport[i].get_payload()); \
				complete = true;			\
			}catch(...){					\
			}						\
			if(complete){					\
				tier_state_ptr->outbound_transport.erase( \
					tier_state_ptr->outbound_transport.begin()+i); \
				i--;					\
			        continue;				\
			}						\
		}							\
	}								\
	

#endif
