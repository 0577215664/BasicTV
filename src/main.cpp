#include "settings.h"
#include "file.h"
#include "util.h"
#include "encrypt/encrypt.h"
#include "encrypt/encrypt_rsa.h"
#include "encrypt/encrypt_aes.h"
#include "tv/tv.h"
#include "tv/tv_frame_standard.h"
#include "tv/tv_frame_audio.h"
#include "tv/tv_frame_video.h"
#include "tv/tv_window.h"
#include "tv/tv_frame_video.h"
#include "tv/tv_frame_audio.h"
#include "tv/tv_frame_caption.h"
#include "tv/tv_channel.h"
#include "input/input.h"
#include "input/input_ir.h"
#include "net/proto/net_proto.h"
#include "net/net.h"
#include "id/id_api.h"
#include "compress/compress.h"
#include "convert.h"
#include "console/console.h"
#include "system.h"
#include "escape.h"
#include "id/set/id_set.h"
#include "id/id_api.h"

#include "net/interface/net_interface.h"

#include "test/test.h" // includes benchmarking code too
#include "init.h"
#include "loop.h"
#include "close.h"

int argc = 0;
char **argv = nullptr;

bool running = false;
bool closing = false;

id_t_ production_priv_key_id = ID_BLANK_ID;

static uint64_t avg_iter_time = 0;
static uint64_t iter_count = 0;

int main(int argc_, char **argv_){
	argc = argc_;
	argv = argv_;
	init();
	if(settings::get_setting("run_tests") == "true"){
		print("tests are being ran, please do not run this in your main data_folder, change with --data_folder or --df, sleeping for 5 seconds", P_WARN);
		sleep_ms(5000);
		const uint64_t test_run_count =
			settings::get_setting_unsigned_def(
				"test_run_count", 1);
		for(uint64_t i = 0;i < test_run_count;i++){
			/*
			  Useful for random() or any situation where
			  memory errors only show up after a long time
			*/
			if(settings::get_setting(
				   "test_catch_all") == "true"){
				try{
					test_suite();
				}catch(...){}
			}else{
				test_suite();
			}
		}
	}
	try{
		running =
			settings::get_setting("init_close_only") == "false";
	}catch(...){
		running = true;
	}
	print("formally starting BasicTV, entering loop", P_NOTE);
	uint64_t start_time = get_time_microseconds();
	uint64_t working_iter_time = start_time;
	id_tier_init(); // TODO: add me to init() when i'm known to work
	while(running){
		try{
			tv_loop();
			input_loop();
			net_loop();
			console_loop();
			id_tier_loop();
		}catch(...){}

		// main loop specific stuff
		check_finite_execution_modes(
			iter_count,
			start_time);
		check_iteration_modifiers();
		check_print_modifiers(
			avg_iter_time);
		update_iteration_data(
			&avg_iter_time,
			&iter_count,
			&working_iter_time);
	}
	close();
	std::cout << "[FIN] Program formally returning zero" << std::endl;
	return 0;
}
