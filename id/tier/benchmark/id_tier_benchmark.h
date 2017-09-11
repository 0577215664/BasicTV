#ifndef ID_TIER_BENCHMARK_H
#define ID_TIER_BENCHMARK_H

// ping just means delays in accessing, seek times/ping/etc
struct id_tier_state_benchmark_t{
private:
	uint64_t ping_micro_s = 0;
	uint64_t throughput_bits_s = 0;
	data_id_t *id = nullptr;
public:
	void list_virtual_data(data_id_t *id_);
	GET_SET_V(ping_micro_s, uint64_t);
	GET_SET_V(throughput_bits_s, uint64_t);
};

#endif
