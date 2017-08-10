#ifndef TV_META_H
#define TV_META_H

#include "../id/id.h"

#define TV_META_ESCAPE_CHAR 0x0F

#define VORBIS_COMMENT_PARAM_TITLE "TITLE"
#define VORBIS_COMMENT_PARAM_CONTACT "CONTACT"
#define VORBIS_COMMENT_PARAM_DESCRIPTION "DESCRIPTION"

struct tv_meta_t{
private:
	// escpaed
	std::vector<uint8_t> vorbis_comment;
public:
	tv_meta_t();
	~tv_meta_t();

	void list_virtual_data(data_id_t *id);
	std::vector<std::string> search_for_param(
		std::string param);
	void add_param(
		std::string param,
		std::string value,
		bool replace);
};

#endif
