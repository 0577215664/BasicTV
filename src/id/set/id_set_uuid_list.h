#ifndef ID_SET_UUID_LIST_H
#define ID_SET_UUID_LIST_H
std::vector<uint8_t> compact_id_set_uuid_list(
	std::vector<id_t_> id_set);
std::vector<id_t_> expand_id_set_uuid_list(
	std::vector<uint8_t> id_set);
#endif
