#include "test.h"

#include "../cryptocurrency.h"
#include "../settings.h"

/*
  I almost panicked out of retardation since I thought I lost the private
  key to this Bitcoin wallet, but thankfully I found it on my old phone and
  I have a backup sitting safe in some undisclosed area.
 */

static uint64_t loop_number = 1;

// types are created with their default exporting rules
id_t_ test_create_generic_id(){
	wallet_set_t *wallet_set_ptr =
		new wallet_set_t;
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
	const uint64_t old_id_count =					\
		id_tier::lookup::id_mod_inc::from_tier(			\
			all_tiers).size();				\
	const uint64_t start_time_micro_s =				\
		get_time_microseconds();				\
	try{								\
		for(uint64_t i = 0;i < loop_number;i++){		\
			x();						\
		}							\
		print("test " #x " finished without a caught exception by the caller", P_NOTE); \
	}catch(...){							\
		print("TEST " #x " FAILED", P_ERR);			\
	}								\
	const uint64_t elapsed_time_micro_s =				\
		get_time_microseconds()-start_time_micro_s;		\
	if(old_id_count != id_tier::lookup::id_mod_inc::from_tier(	\
		   all_tiers).size()){					\
		P_V(id_tier::lookup::id_mod_inc::from_tier(		\
			    all_tiers).size(), P_WARN);			\
		P_V(old_id_count, P_WARN);				\
	print("test " #x " is leaking possibly invalid data, fix this", P_CRIT); \
	}								\
	P_V(elapsed_time_micro_s, P_SPAM);				\
	print("test " #x " took " + std::to_string(((long double)(elapsed_time_micro_s))/1000000) + "s", P_NOTE); \
									\
	}								\

/*
  all tests are ordered from most basic to most complex to avoid situations
  where a tests fails because a more specific test that would have failed in
  a more debugger-friendly manner hasn't ran yet
 */

void test_id_subsystem(){
	RUN_TEST(test::id_system::linked_list);
	RUN_TEST(test::id_system::id_set::proper);
	RUN_TEST(test::id_system::transport::core_functions);
	RUN_TEST(test::id_system::transport::proper);
	// RUN_TEST(test::id_system::transport::import::random);
}

void test_escape(){
	RUN_TEST(test::escape::proper);
	// RUN_TEST(test::escape::random);
}

void test_audio(){
	// RUN_TEST(test::audio::wav);
	RUN_TEST(test::audio::opus);
}

void test_math(){
	RUN_TEST(test::math::number::add);
	RUN_TEST(test::math::number::sub);
}

void test_suite(){
	loop_number =
		settings::get_setting_unsigned_def(
			"test_loop_count", 1);
	print("set test loop number to " + std::to_string(loop_number), P_NOTE);
	test_math();
	test_escape();
	test_id_subsystem();
	test_audio();
}
