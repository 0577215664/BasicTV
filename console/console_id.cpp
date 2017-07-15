#include "console.h"

DEC_CMD(id_api_get_type_cache){
	output_table =
		console_generate_generic_id_table(
			ID_TIER_CACHE_GET(
				registers.at(0)));
}

DEC_CMD(id_api_get_all){
	output_table =
		console_generate_generic_id_table(
			id_tier::lookup::ids::from_tier(0, 0));
}
