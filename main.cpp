#include "main.h"
#include "settings.h"
#include "file.h"
#include "util.h"
#include "encrypt/encrypt.h"
#include "encrypt/encrypt_rsa.h"
#include "tv/tv.h"
#include "tv/tv_frame_standard.h"
#include "tv/tv_frame_audio.h"
#include "tv/tv_frame_video.h"
#include "tv/tv_window.h"
#include "tv/tv_dev_video.h"
#include "tv/tv_dev_audio.h"
#include "tv/tv_frame_video.h"
#include "tv/tv_frame_audio.h"
#include "tv/tv_frame_caption.h"
#include "tv/tv_dev.h"
#include "tv/tv_channel.h"
#include "input/input.h"
#include "input/input_ir.h"
#include "net/proto/net_proto.h"
#include "net/net.h" // two seperate units (right now)
#include "id/id_api.h"
#include "compress.h"
#include "convert.h"

/*
  TODO:
  Define the settings and key bindings better
  Write some simple OpenSSL wrappers for encryption
  Implement SDL_mixer in tv.cpp
  16x2 support? What would I use it for?

  BasicTV doesn't have a built-in Bitcoin wallet for security reasons, instead
  just show QR codes for payment requests and let the mobile user take care of
  it.
 */

int argc = 0;
char **argv = nullptr;
bool running = true;

#define APPEND_TYPE_TO_VECTOR(old, type)				\
	if(true){							\
		auto new_vector = id_api::cache::get(#type);		\
		old.insert(old.end(), new_vector.begin(), new_vector.end()); \
	}								\

void no_mem(){
	std::vector<id_t_> frame_ids;
	APPEND_TYPE_TO_VECTOR(frame_ids, tv_frame_video_t);
	APPEND_TYPE_TO_VECTOR(frame_ids, tv_frame_audio_t);
	APPEND_TYPE_TO_VECTOR(frame_ids, tv_frame_caption_t);
	/*
	  destroy() exports to disk if possible, so we can destroy IDs like
	  crazy if we wanted to and just reread them. Types that have been
	  exported the longest without being re-read are destroyed when we
	  run out of memory
	 */
	// TODO: try and adhere to the gen_ded_mem setting somehow
	for(uint64_t i = 0;i < frame_ids.size();i++){
		id_api::destroy(frame_ids[i]);
	}
	/*
	  tv_frame_*_t structs use up most of the space in a majority of the 
	  cases, but in edge cases, it might make sense to use a dedicated
	  no_mem function for stress and unit tests
	 */
}

#undef APPEND_TO_VECTOR

static void init(){
	std::set_new_handler(no_mem);	
	/*
	  settings_init() only reads from the file, it doesn't do anything
	  critical to setting default values
	*/
	// default port for ID networking
	settings::set_setting("network_port", "58486");
	// disable socks
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
		test_socket_->set_net_ip(ip, port, NET_IP_VER_4);
		test_socket_->connect();
		test_socket_->send({'A', 'A', 'A', 'A'});
		while(true){
			sleep_ms(1);
		}
	}
}

/*
  BasicTV works best when there are many open TCP connections at one
  time, because there isn't an overhead for connecting to a new client.
  However, consumer grade routers are terrible at this, so this is an
  automatic test to see how much the router can actually handle.

  UPDATE: Linux's internal TCP socket limit was reached, and the following
  source code doesn't actually test the router's capabilities.

  TODO: check to see if this information makes it to the router, or if
  it is just stuck locally (which makes sense, but defeats the purpose
  of the test).
 */

static void test_socket_array(std::vector<std::pair<uint64_t, uint64_t> > socket_array){
	for(uint64_t i = 0;i < socket_array.size();i++){
		net_socket_t *first =
			PTR_DATA(socket_array[i].first,
				 net_socket_t);
		net_socket_t *second =
			PTR_DATA(socket_array[i].second,
				 net_socket_t);
		if(first == nullptr || second == nullptr){
			P_V(socket_array[i].first, P_SPAM);
			P_V(socket_array[i].second, P_SPAM);
			print("SOCKETS STRUCTS ARE NULL", P_ERR);
		}
		first->send({'a','a','a','a'});
		sleep_ms(1);
		if(second->recv(4, NET_SOCKET_RECV_NO_HANG).size() == 0){
			print("SOCKET HAS CLOSED", P_ERR);
		}else{
			print("received data for socket number " + std::to_string(i), P_NOTE);
		}
	}
}

/*
  This works up until 537 (stack smashing), and I can't find the problem. If
  you are stuck at a lower number, make sure you set the file descriptor limit
  high enough (ulimit -n 65536 works for me).
*/

static void test_max_tcp_sockets(){
	print("Local IP address:", P_NOTE);
	std::string ip;
	std::cin >> ip;
	std::vector<std::pair<uint64_t, uint64_t> > socket_pair;
	bool dropped = false;
	net_socket_t *inbound =
		new net_socket_t;
	inbound->set_net_ip("", 50000, NET_IP_VER_4);
	inbound->connect();
	while(!dropped){
		for(uint64_t i = 0;i < 128;i++){
			net_socket_t *first =
				new net_socket_t;
			first->set_net_ip(ip, 50000, NET_IP_VER_4);
			first->connect();
			net_socket_t *second =
				new net_socket_t;
			sleep_ms(1); // probably isn't needed
			TCPsocket tmp_socket =
				SDLNet_TCP_Accept(inbound->get_tcp_socket());
			if(tmp_socket != nullptr){
				second->set_tcp_socket(tmp_socket);
			}else{
				print("unable to receive connection request", P_ERR);
			}
			socket_pair.push_back(
				std::make_pair(
					first->id.get_id(),
					second->id.get_id()));
		}
		test_socket_array(socket_pair);
	}
}

static void test_id_transport_print_exp(std::vector<uint8_t> exp){
	for(uint64_t i = 0;i < exp.size();i++){
		print(std::to_string(i) + "\t" + std::to_string((int)(exp[i])) + "\t" + std::string(1, exp[i]), P_SPAM);
	}
}

static void test_id_transport(){
	// not defined behavior at all
	data_id_t *tmp =
		new data_id_t(nullptr, "TEST");
	tmp->set_next_linked_list(1);
	tmp->set_prev_linked_list(2);
	const std::vector<uint8_t> exp =
		tmp->export_data(0);
	test_id_transport_print_exp(exp);
	tmp->set_next_linked_list(0);
	tmp->set_prev_linked_list(0);
	tmp->import_data(exp);
	P_V(tmp->get_next_linked_list(), P_NOTE);
	P_V(tmp->get_prev_linked_list(), P_NOTE);
	running  = false;
}

/*
  Tests network byte order conversions. I have no doubt it works fine, but
  this is also here for benchmarking different approaches to arbitrarially
  large strings (although isn't being used for that right now)
 */

static void test_nbo_transport(){
	while(true){
		uint64_t orig_random =
			true_rand(0, ~0L);
		uint64_t random =
			orig_random;
		uint64_t mem_to_copy = true_rand(1, 7)%8;
		P_V(mem_to_copy, P_SPAM);
		convert::nbo::to(
			(uint8_t*)&random,
			mem_to_copy);
		convert::nbo::from(
			(uint8_t*)&random,
			mem_to_copy);
		if(memcmp(&orig_random, &random, mem_to_copy) == 0){
			print("IT WORKS", P_NOTE);
		}else{
			P_V_B(random, P_NOTE);
			P_V_B(orig_random, P_NOTE);
			print("IT DOESN'T WORK", P_ERR);
		}
	}
	running = false;
}

/*
  Just to see how it reacts
 */

static void test_break_id_transport(){
	while(true){
		std::vector<uint8_t> tmp;
		for(uint64_t i = 0;i < 1024;i++){
			tmp.push_back(true_rand(0, 255));
		}
		data_id_t tmp_id(nullptr, "TEST");
		tmp_id.import_data(tmp);
	}
	running = false;
}

static void test_rsa_encryption(){
	encrypt_priv_key_t *priv_key =
		new encrypt_priv_key_t;
	encrypt_pub_key_t *pub_key =
		new encrypt_pub_key_t;
}

static void test(){}

// TODO: define some ownership, don't actually use this
// in production, but just as a leak checker

int main(int argc_, char **argv_){
	argc = argc_;
	argv = argv_;
	init();
	//test_break_id_transport();
	//test_id_transport();
	//test_max_tcp_sockets();
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
