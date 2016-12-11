#include "main.h"
#include "settings.h"
#include "file.h"
#include "util.h"
#include "rsa.h"
#include "tv/tv.h"
#include "tv/tv_frame_standard.h"
#include "tv/tv_frame_audio.h"
#include "tv/tv_frame_video.h"
#include "tv/tv_window.h"
#include "tv/tv_patch.h"
#include "tv/tv_dev_video.h"
#include "tv/tv_dev_audio.h"
#include "tv/tv_dev.h"
#include "tv/tv_channel.h"
#include "input/input.h"
#include "input/input_ir.h"
#include "net/net_proto.h"
#include "net/net.h" // two seperate units (right now)
#include "id/id_api.h"
#include "compress.h"

/*
  TODO:
  Define the settings and key bindings better
  Write some simple OpenSSL wrappers for PGP encryption/decryption for
  the channel data
  Implement SDL_image in tv.cpp
  Implement SDL_mixer in tv.cpp
  16x2 support? What would I use it for?

  I might be able to piggyback on the RPI_BTC_ATM code for Bitcoin txs,
  but I don't want to force people to run a Bitcoin node on a restrained
  connection (look into libbitcoin, cbitcoin, etc.). I only need a dedicated
  system of transactions for a sponsorship program, so having a QR code show
  up should be enough integration for most people (at the time of writing), 
  not to mention the fact that I don't want people locking away tons of 
  money on inseucre software
 */

int argc = 0;
char **argv = nullptr;
bool running = true;

std::array<uint8_t, 4> version = {
	VERSION_MAJOR,
	VERSION_MINOR,
	VERSION_REVISION,
	NETWORK_MAJOR};

/*
  Since this is so early in development, I'm not worried about this
 */

static void init(){
	/*
	  settings_init() only reads from the file, it doesn't do anything
	  critical to setting default values
	*/
	// default port for ID networking
	settings::set_setting("network_port", "58486");
	// enable socks
	settings::set_setting("socks_enable", "false");
	// if SOCKS cannot be set up properly, then terminate
	settings::set_setting("socks_strict", "true");
	// SOCKS proxy ip address in ASCII
	settings::set_setting("socks_proxy_ip", "127.0.0.1");
	// SOCKS proxy port in ASCII
	settings::set_setting("socks_proxy_port", "9050");
	settings_init();
	
	tv_init();
	input_init();
	net_proto_init();
}

static void close(){
	tv_close();
	input_close();
	net_proto_close();
	id_api::destroy_all_data();
}

static void test_compressor(){
	std::vector<uint8_t> input_data;
	for(uint64_t i = 0;i < 1024*1024;i++){
		input_data.push_back(i&0xFF);
	}
	std::vector<uint8_t> output_data =
		compressor::from_xz(
			compressor::to_xz(
				input_data,
				0));
	if(std::equal(input_data.begin(), input_data.end(), output_data.begin())){
		print("Compressor works", P_NOTE);
	}else{
		print("input != output", P_ERR);
	}
}

static void test_socket(){
	/*
	  I cannot locally connect to this computer without using another IP
	  address (breaking 4-tuple), so just test this with laptop
	 */
	net_socket_t *test_socket_ = new net_socket_t;
	std::string ip;
	uint16_t port = 0;
	bool recv = false;
	try{
		recv = settings::get_setting(
			"test_recv") == "1";
	}catch(...){}
	if(recv){
		while(true){
			net_proto_loop();
		}
	}else{
		print("IP address to test", P_NOTE);
		std::cin >> ip;
		print("Port to test", P_NOTE);
		std::cin >> port;
		std::pair<std::string, uint16_t> laptop_conn =
			std::make_pair(ip, port);
		test_socket_->connect(laptop_conn);
		test_socket_->send({'A', 'A', 'A', 'A'});
		while(true){
			sleep_ms(1);
		}
	}
}

static void test(){}

// TODO: define some ownership, don't actually use this
// in production, but just as a leak checker

int main(int argc_, char **argv_){
	argc = argc_;
	argv = argv_;
	init();
	//test_compressor();
	//test_socket();
	while(running){
		tv_loop();
		input_loop();
		net_proto_loop();
		try{
			if(settings::get_setting("slow_iterate") == "1"){
				sleep_ms(1000);
			}
		}catch(...){}
		try{
			if(settings::get_setting("prove_iterate") == "1"){
				std::cout << "iterated" << std::endl;
			}
		}catch(...){}
	}
	close();
	return 0;
}
