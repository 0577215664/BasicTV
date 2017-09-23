#include "../../id.h"
#include "id_tier_benchmark.h"

void id_tier_state_benchmark_t::list_virtual_data(data_id_t *id_){
	id = id_;
	ADD_DATA_PTR(ping_micro_s, public_ruleset);
	ADD_DATA_PTR(throughput_bits_s, public_ruleset);
	
}
