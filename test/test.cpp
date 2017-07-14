#include "test.h"

#include "../cryptocurrency.h"

/*
  I almost panicked out of retardation since I thought I lost the private
  key to this Bitcoin wallet, but thankfully I found it on my old phone and
  I have a backup sitting safe in some undisclosed area.
 */

id_t_ test_create_generic_id(){
	wallet_set_t *wallet_set_ptr =
		new wallet_set_t;
	wallet_set_ptr->id.set_lowest_global_flag_level(
		ID_DATA_RULE_UNDEF,
		ID_DATA_EXPORT_RULE_NEVER,
		ID_DATA_RULE_UNDEF);
	std::string totally_legit_bitcoin_wallet_please_give_me_money =
		"13dfmkk84rXyHoiZQmuYfTxGYykug1mDEZ";
	wallet_set_ptr->add_wallet(
		std::vector<uint8_t>({'B', 'T', 'C'}),
		std::vector<uint8_t>(
			(uint8_t*)&(totally_legit_bitcoin_wallet_please_give_me_money[0]),
			(uint8_t*)&(totally_legit_bitcoin_wallet_please_give_me_money[0])+
			totally_legit_bitcoin_wallet_please_give_me_money.size()));
	return wallet_set_ptr->id.get_id();
}

/*
  Leaking information out to the IDs is OK if we run with --init-close-only true,
  since all created data should be created with the export flags
 */

#define RUN_TEST(x)							\
	if(true){							\
		const uint64_t old_id_count =				\
			id_api::array::get_id_count();			\
		const uint64_t start_time_micro_s =			\
			get_time_microseconds();			\
		try{							\
			x();						\
			print("test " #x " finished without a caught exception by the caller", P_NOTE); \
		}catch(...){						\
			print("TEST " #x " FAILED", P_ERR);		\
		}							\
		const uint64_t elapsed_time_micro_s =			\
			get_time_microseconds()-start_time_micro_s;	\
		if(old_id_count != id_api::array::get_id_count()){	\
			P_V(id_api::array::get_id_count(), P_WARN);	\
			P_V(old_id_count, P_WARN);			\
			print("test " #x " is leaking possibly invalid data, fix this", P_CRIT); \
		}							\
		P_V(elapsed_time_micro_s, P_SPAM);			\
		print("test " #x " took " + std::to_string(((long double)(elapsed_time_micro_s))/1000000) + "s", P_NOTE); \
									\
	}								\
	
void test_id_subsystem(){
	RUN_TEST(test::id_system::transport::proper);
	RUN_TEST(test::id_system::id_set::proper);
	// RUN_TEST(test::id_system::transport::import::random);
}

void test_net(){
	RUN_TEST(test::net::proto_socket::send_recv);
	RUN_TEST(test::net::socket::send_recv);
}

void test_escape(){
	RUN_TEST(test::escape::proper);
	// RUN_TEST(test::escape::random);
}

void test_suite(){
	test_escape();
	// test_id_subsystem();
	// test_net();
}
