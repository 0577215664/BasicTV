#ifndef ID_TIER_MEMORY_H
#define ID_TIER_MEMORY_H
extern ID_TIER_INIT_STATE(mem);
extern ID_TIER_DEL_STATE(mem);
extern ID_TIER_ADD_DATA(mem);
extern ID_TIER_DEL_ID(mem);
extern ID_TIER_GET_ID(mem);
extern ID_TIER_GET_ID_MOD_INC(mem);
extern ID_TIER_GET_ID_BUFFER(mem);
extern ID_TIER_UPDATE_ID_BUFFER(mem);

extern void mem_add_id(data_id_t*);
extern void mem_del_id(data_id_t*);

/*
  We don't need this for anything, but it is here
  for the sake of completion
 */
struct id_tier_memory_state_t{
};
#endif
