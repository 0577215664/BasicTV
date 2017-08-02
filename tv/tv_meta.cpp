#include "tv_meta.h"

#include "../escape.h"

tv_meta_t::tv_meta_t(){
}

tv_meta_t::~tv_meta_t(){
}

void tv_meta_t::list_virtual_data(
	data_id_t *id){
	id->add_data_one_byte_vector(&vorbis_comment, ~0);
}

std::vector<std::string> tv_meta_t::search_for_param(
	std::string param){
	std::vector<std::string> retval;
	std::vector<std::vector<uint8_t> > vorbis_searchable =
		unescape_all_vectors(
			vorbis_comment,
			TV_META_ESCAPE_CHAR).first;
	for(uint64_t i = 0;i < vorbis_searchable.size();i++){
		if(vorbis_searchable[i].size() < param.size()){
			continue;
		}
		if(memcmp(static_cast<uint8_t*>(vorbis_searchable[i].data()),
			  param.c_str(),
			  param.size()) == 0){
			retval.push_back(
				convert::string::from_bytes(
					std::vector<uint8_t>(
						std::find(
							vorbis_searchable[i].begin(),
							vorbis_searchable[i].end(),
							'='),
						vorbis_searchable[i].end())));
		}
	}
	return retval;
}

void tv_meta_t::add_param(
	std::string param,
	std::string value,
	bool replace){
	if(replace){
		std::vector<std::vector<uint8_t>> vorbis_searchable =
			unescape_all_vectors(
				vorbis_comment,
				TV_META_ESCAPE_CHAR).first;
		for(uint64_t i = 0;i < vorbis_searchable.size();i++){
			if(vorbis_searchable[i].size() < param.size()){
				continue;
			}
			if(memcmp(static_cast<uint8_t*>(vorbis_searchable[i].data()),
				  param.c_str(),
				  param.size()) == 0){
				vorbis_searchable[i].erase(
					vorbis_searchable[i].begin()+i);
				i--;
			}
		}
		vorbis_comment.clear();
		for(uint64_t i = 0;i < vorbis_searchable.size();i++){
			std::vector<uint8_t> escaped =
				escape_vector(
					vorbis_searchable[i],
					TV_META_ESCAPE_CHAR);
			vorbis_comment.insert(
				vorbis_comment.end(),
				escaped.begin(),
				escaped.end());
		}
	}
	std::vector<uint8_t> escaped =
		escape_vector(
			convert::string::to_bytes(
				param + '=' + value),
			TV_META_ESCAPE_CHAR);
	vorbis_comment.insert(
		vorbis_comment.end(),
		escaped.begin(),
		escaped.end());
}
