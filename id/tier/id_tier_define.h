#ifndef ID_TIER_DEFINE_H
#define ID_TIER_DEFINE_H
// useful macro functions for all medium

#define GET_ALL_STATE_PTR(medium_)			\
	id_tier_state_t *tier_state_ptr =		\
		PTR_DATA(state_id,			\
			 id_tier_state_t);		\
	ASSERT(tier_state_ptr != nullptr, P_ERR);	\
	id_tier_##medium_##_state_t* medium_##_state_ptr =	\
		(id_tier_##medium_##_state_t*)(					\
			tier_state_ptr->get_payload());		\
	ASSERT(medium_##_state_ptr != nullptr, P_ERR);

#endif
