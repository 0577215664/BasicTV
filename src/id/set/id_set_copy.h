#ifndef ID_SET_COPY_H
#define ID_SET_COPY_H
extern std::vector<uint8_t> compact_id_set_copy(
	std::vector<id_t_> id_set);
extern std::vector<id_t_> expand_id_set_copy(
	std::vector<uint8_t> id_set);
#endif
