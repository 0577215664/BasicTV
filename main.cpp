#include "main.h"
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
#include "net/net.h"
#include "id/id_api.h"
#include "compress/compress.h"
#include "convert.h"
#include "console/console.h"
#include "system.h"
#include "escape.h"
#include "id/id_set.h"
#include "id/id_disk.h"

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
/*
  TODO: should probably move into one globabl program state variable with
  bitwiser operators

  These aren't exactly opposite of each other, since the closing behavior
  is not compatiable with the initialization (running is the last thing to
  flip, since not all program functions are defined as operating at that
  point).
*/
bool running = false;
bool closing = false;

/*
  All information imported has the SHA256 hash of the public key. It follows
  the following rules:

  1. The first item to be created is the owner's encrypt_priv_key_t, this
  doesn't need a SHA256 hash (as it shouldn't be networked and doesn't add
  any security). 
  2. The second item to be created is the corresponding public key.
  3. All information created will have the encryption fingerprint included
  in the ID, as well as being encrypted with that information for transport
  (not relevant here)

  Anything can be loaded perfectly fine, but it cannot request the ID of the
  public key to link it to the private key. id_throw_exception is ONLY used
  to make this connection.

  SHA256 keys aren't needed for array lookups, and no transporting should be
  done to keep this property for long enough (entire code is only around five
  lines), so this isn't as hacky as I make myself believe it is.
*/

id_t_ production_priv_key_id = ID_BLANK_ID;
bool id_throw_exception = true;

/*
  The production private key is the private key that is associated with every
  data type created on this machine. If the information were to be imported, the
  hash would be applied to the ID, but it would be overridden by the ID
  associated with the imported data (logical, and it doesn't break the hashing)

  I made a write up on why this is needed above. In case that was deleted, then
  a tl;dr is that the public key can't reference itself because the hash needs
  to be known for get_id(), and that needs to be ran to link the private and
  public keys together (production_priv_key_id).
*/

static void bootstrap_production_priv_key_id(){
	id_api::import::load_all_of_type(
		"encrypt_pub_key_t",
		ID_API_IMPORT_FROM_DISK);
	id_api::import::load_all_of_type(
		"encrypt_priv_key_t",
		ID_API_IMPORT_FROM_DISK);
	std::vector<id_t_> all_public_keys =
		id_api::cache::get(
			"encrypt_pub_key_t");
	std::vector<id_t_> all_private_keys =
		id_api::cache::get(
			"encrypt_priv_key_t");
	encrypt_priv_key_t *priv_key = nullptr;
	encrypt_pub_key_t *pub_key = nullptr;
	if(all_private_keys.size() == 0){
		print("detected first boot, creating production id", P_NOTE);
		uint64_t bits_to_use = 4096;
		try{
			bits_to_use =
				std::stoi(
					settings::get_setting(
						"rsa_key_length"));
		}catch(...){}
		std::pair<id_t_, id_t_> key_pair =
			rsa::gen_key_pair(bits_to_use);
		priv_key =
			PTR_DATA(key_pair.first,
				 encrypt_priv_key_t);
		if(priv_key == nullptr){
			print("priv_key is a nullptr", P_ERR);
		}
		pub_key =
			PTR_DATA(key_pair.second,
				 encrypt_pub_key_t);
		if(pub_key == nullptr){
			print("pub_key is a nullptr", P_ERR);
		}
		priv_key->set_pub_key_id(key_pair.second);
		production_priv_key_id = priv_key->id.get_id();
	}else if(all_private_keys.size() == 1){
		production_priv_key_id = all_private_keys[0];
		P_V_S(convert::array::id::to_hex(all_private_keys[0]), P_SPAM);
		priv_key = PTR_DATA(all_private_keys[0], encrypt_priv_key_t);
		P_V_S(convert::array::id::to_hex(all_public_keys[0]), P_SPAM);
		pub_key = PTR_DATA(all_public_keys[0], encrypt_pub_key_t);
	}else if(all_private_keys.size() > 1){
		print("I have more than one private key, make a prompt to choose one", P_ERR);
	}
	set_id_hash(&production_priv_key_id,
		    encrypt_api::hash::sha256::gen_raw(
			    pub_key->get_encrypt_key().second));
	priv_key->id.set_id(production_priv_key_id);
	id_throw_exception = false;
	priv_key->set_pub_key_id(pub_key->id.get_id());
}

static void init(){
	// debugging information for OpenSSL's error printing
	ERR_load_crypto_strings();
	// loads OpenSSL stuff (AES only)
	OpenSSL_add_all_algorithms();
	OPENSSL_config(nullptr);
	/*
	  settings_init() only reads from the file, it doesn't do anything
	  critical to setting default values
	*/
	// default port for ID networking
	settings::set_setting("network_port", "58486");
	settings::set_setting("net_hostname", "");
	settings::set_setting("net_open_tcp_port", "false");
	// console port
	settings::set_setting("console_port", "59000");
	// disable socks
	settings::set_setting("socks_enable", "false");
	// if SOCKS cannot be set up properly, then terminate
	settings::set_setting("socks_strict", "true");
	// SOCKS proxy ip address in ASCII
	settings::set_setting("socks_proxy_ip", "127.0.0.1");
	// SOCKS proxy port in ASCII
	settings::set_setting("socks_proxy_port", "9050");
	// enable subsystems in settings
	settings::set_setting("video", "true");
	settings::set_setting("audio", "true");
	// throw level
	settings::set_setting("throw_level", std::to_string(P_CRIT));
	// shouldn't need to disable other stuff
	/*
	  Using "~" doesn't work with C++, so get the information from getenv()

	  TODO: convert the input string from settings.cfg to this

	  TODO: use getuid and that stuff when getenv doesn't work (?)
	 */
	//settings::set_setting("data_folder", ((std::string)getenv("HOME"))+"/.BasicTV/");
	id_disk_index_t *disk_index =
		new id_disk_index_t;
	disk_index->set(
		ID_DISK_MEDIUM_HDD,
		ID_DISK_TRANS_DIR,
		{ID_DISK_ENHANCE_UNDEF}, // macro to zero, here for verbosity
		((std::string)getenv("HOME"))+"/.BasicTV/");
	system_handler::mkdir(
		settings::get_setting(
			"data_folder"));
	settings_init();

	bootstrap_production_priv_key_id();

	tv_init();
	input_init();
	net_proto_init();
	console_init();
}

static void close(){
	closing = true;
	tv_close();
	input_close();
	net_proto_close();
	console_close();
	id_api::destroy_all_data();
	ENGINE_cleanup();
	EVP_cleanup();
	ERR_free_strings();
	CRYPTO_cleanup_all_ex_data();
}

static void test_compressor(){
	std::vector<uint8_t> input_data;
	for(uint64_t i = 0;i < 1024*1024;i++){
		input_data.push_back(i&0xFF);
	}
	std::vector<uint8_t> output_data =
		compressor::xz::from(
			compressor::xz::to(
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
		test_socket_->set_net_ip(ip, port);
		test_socket_->connect();
		test_socket_->send("AAAA");
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

static void test_socket_array(std::vector<std::pair<id_t_, id_t_> > socket_array){
	for(uint64_t i = 0;i < socket_array.size();i++){
		net_socket_t *first =
			PTR_DATA(socket_array[i].first,
				 net_socket_t);
		net_socket_t *second =
			PTR_DATA(socket_array[i].second,
				 net_socket_t);
		if(first == nullptr || second == nullptr){
			P_V_S(convert::array::id::to_hex(socket_array[i].first), P_SPAM);
			P_V_S(convert::array::id::to_hex(socket_array[i].second), P_SPAM);
			print("SOCKETS STRUCTS ARE NULL", P_ERR);
		}
		first->send("aaaa");
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
	std::vector<std::pair<id_t_, id_t_> > socket_pair;
	bool dropped = false;
	net_socket_t *inbound =
		new net_socket_t;
	inbound->set_net_ip("", 50000);
	inbound->connect();
	while(!dropped){
		for(uint64_t i = 0;i < 128;i++){
			net_socket_t *first =
				new net_socket_t;
			first->set_net_ip(ip, 50000);
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
	P_V(exp.size(), P_SPAM);
	for(uint64_t i = 0;i < exp.size();i++){
		print(std::to_string(i) + "\t" + std::to_string((int)(exp[i])) + "\t" + std::string(1, exp[i]), P_SPAM);
	}
}

/*
  TODO: rework this to use different types
 */

static void test_id_transport(){
	// not defined behavior at all
	settings::set_setting("export_data", "true");
	net_proto_peer_t *tmp =
		new net_proto_peer_t;
	tmp->set_net_ip("127.0.0.1", 58486);
	const std::vector<uint8_t> exp =
		tmp->id.export_data(ID_DATA_NOEXP | ID_DATA_NONET);
	//test_id_transport_print_exp(exp);
	net_proto_peer_t *tmp_2 =
		new net_proto_peer_t;
	tmp_2->id.import_data(exp);
	P_V_S(convert::array::id::to_hex(tmp->id.get_id()), P_NOTE);
	P_V_S(convert::array::id::to_hex(tmp_2->id.get_id()), P_NOTE);
	P_V(tmp_2->get_net_port(), P_NOTE);
	P_V_S(tmp_2->get_net_ip_str(), P_NOTE);
	running = false;
}

/*
  Tests network byte order conversions. I have no doubt it works fine, but
  this is also here for benchmarking different approaches to arbitrarially
  large strings (although isn't being used for that right now)
 */

static void test_nbo_transport(){
	// 2, 4, and 8 are optimized, 7 isn't
	std::vector<uint8_t> test =
		{'T', 'E', 'S', 'T', 'I', 'N', 'G'};
	std::vector<uint8_t> test_2 = test;
	test_2 =
		convert::nbo::from(
			convert::nbo::to(
				test_2));
	if(test == test_2){
		print("it works", P_NOTE);
	}else{
		for(uint64_t i = 0;i < test.size();i++){
			P_V_C(test[i], P_NOTE);
			P_V_C(test_2[i], P_NOTE);
		}
		print("it doesn't work", P_ERR);
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
		data_id_t tmp_id(nullptr, 255);
		tmp_id.import_data(tmp);
	}
	running = false;
}

// currently only does key generation

static void test_rsa_key_gen(){
	std::pair<id_t_, id_t_> rsa_key_pair =
		rsa::gen_key_pair(4096);
	encrypt_priv_key_t *priv =
		PTR_DATA(rsa_key_pair.first,
			 encrypt_priv_key_t);
	if(priv == nullptr){
		print("priv key is a nullptr", P_ERR);
	}
	/*
	  First is a macro for the encryption type
	  Second is the DER formatted vector
	 */
	P_V(priv->get_encrypt_key().second.size(), P_NOTE);
	encrypt_pub_key_t *pub =
		PTR_DATA(rsa_key_pair.second,
			 encrypt_pub_key_t);
	if(pub == nullptr){
		print("pub key is a nullptr", P_ERR);
	}
	P_V(pub->get_encrypt_key().second.size(), P_NOTE);
}

static void test_rsa_encryption(){
	print("Bit length of RSA key:", P_NOTE);
	uint64_t key_len = 4096;
	std::cin >> key_len;
	std::pair<id_t_, id_t_> rsa_key_pair =
		rsa::gen_key_pair(key_len);
	std::vector<uint8_t> test_data;
	for(uint64_t i = 0;i < 65536;i++){
		for(uint64_t x = 0;x < 1024;x++){
			test_data.push_back(
				(uint8_t)true_rand(0, 255));
		}
		std::vector<uint8_t> test_data_output =
			encrypt_api::decrypt(
				encrypt_api::encrypt(
					test_data,
					rsa_key_pair.first),
				rsa_key_pair.second);
		if(test_data == test_data_output){
			print("it worked with " + std::to_string((long double)test_data.size()/(1024.0*1024.0)) + " MB", P_NOTE);
		}else{
			print("FAILED", P_ERR);
		}
	}
}

static void test_aes(){
	std::vector<uint8_t> key = {'f','f','f','f','f','f','f','f','f','f','f','f','f','f','f','f','f','f','f','f','f','f','f','f'};
	std::vector<uint8_t> data = {'T', 'E', 'S', 'T', 'I', 'N', 'G'};
	if(aes::decrypt(
		   aes::encrypt(
			   data, key),
		   key) == data){
		print("it works", P_CRIT);
	}else{
		print("it does not work", P_CRIT);
	}
		
}

static void test(){}

/*
  TODO: make these tests so they can be more easily ran (all of them)
 */

static std::pair<uint64_t, std::vector<uint8_t> > benchmark_timed_encryption(std::vector<uint8_t> data, id_t_ key){
	std::pair<uint64_t, std::vector<uint8_t> > retval;
	const uint64_t start_time_micro_s =
		get_time_microseconds();
	retval.second = 
		encrypt_api::encrypt(data, key);
	retval.first =
		get_time_microseconds()-start_time_micro_s;
	return retval;
}

static uint64_t benchmark_timed_decryption(std::vector<uint8_t> data, id_t_ key){
	const uint64_t start_time_micro_s =
		get_time_microseconds();
	encrypt_api::decrypt(data, key);
	return get_time_microseconds()-start_time_micro_s;
}

static std::pair<uint64_t, uint64_t> encryption_benchmark_datum(std::vector<uint8_t> data,
								std::pair<id_t_, id_t_> keys,
								std::string scheme){
	std::pair<uint64_t, uint64_t> retval;
	const uint64_t start_time_micro_s =
		get_time_microseconds();
	std::vector<uint8_t> encrypt_data;
	if(scheme == "rsa"){
		print("forcing RSA for benchmark datum", P_NOTE);
		encrypt_data =
			encrypt_api::encrypt(
				data, keys.first, ENCRYPT_RSA);
	}else if(scheme == "aes"){
		print("forcing AES-192 for benchmark datum", P_NOTE);
		encrypt_data =
			encrypt_api::encrypt(
				data, keys.first, ENCRYPT_AES192_SHA256);
	}else{
		print("letting encryption API choose scheme for benchmark datum", P_NOTE);
		encrypt_data =
			encrypt_api::encrypt(data, keys.first);
	}
	retval.first = get_time_microseconds()-start_time_micro_s;
	encrypt_api::decrypt(encrypt_data, keys.second);
	retval.second = get_time_microseconds()-(retval.first+start_time_micro_s);
	return retval;
}

static void benchmark_encryption(std::string method){
	// payload of data, encryption time, decryptiont ime
	std::vector<uint32_t> benchmark_data;
	for(uint64_t i = 1;i <=1024;i++){
		benchmark_data.push_back(i);
	}
	std::pair<id_t_, id_t_> rsa_key_pair =
		rsa::gen_key_pair(4096); // TODO: modify encrypt API to not assume this
	data_id_t *priv_key = PTR_ID(rsa_key_pair.first, );
	if(priv_key != nullptr){
		priv_key->noexp_all_data();
	}
	data_id_t *pub_key = PTR_ID(rsa_key_pair.second, );
	if(pub_key != nullptr){
		pub_key->noexp_all_data();
	}
	std::ofstream out(method + ".bench");
	if(out.is_open() == false){
		print("can't open benchmark output file", P_ERR);
	}
	std::vector<uint8_t> payload;
	for(uint64_t i = 0;i < benchmark_data.size();i++){
		const uint64_t size_bytes = (benchmark_data[i]*1024*1024);
		if(payload.size() > size_bytes){
			payload.erase(
				payload.begin()+size_bytes,
				payload.end());
		}else if(payload.size() < size_bytes){
			payload.insert(
				payload.end(),
				size_bytes-payload.size(),
				0b10101010);
		}
		std::pair<uint64_t, uint64_t> datum =
			encryption_benchmark_datum(
				payload,
				rsa_key_pair,
				method);
		P_V(size_bytes/(1024*1024), P_NOTE);
		P_V_S(get_readable_time(datum.first), P_NOTE);
		P_V_S(get_readable_time(datum.second), P_NOTE);
		out << size_bytes << " " << datum.first << " " << datum.second << std::endl;
		print(std::to_string(benchmark_data.size()-i-1) + " left to go", P_NOTE);
	}
	out.close();
	print("benchmark completed", P_NOTE);
}

static void test_escape_string(){
	const char escape_char = 0xFF;
	std::vector<uint8_t> all_escaped_stuff;
	for(uint64_t i = 0;i < 512;i++){
		std::vector<uint8_t> tmp =
			escape_vector(
				true_rand_byte_vector(
					8192),
				escape_char);
		all_escaped_stuff.insert(
			all_escaped_stuff.end(),
			tmp.begin(),
			tmp.end());
	}
	P_V(all_escaped_stuff.size(), P_NOTE);
	std::pair<std::vector<std::vector<uint8_t> >, std::vector<uint8_t> > deconstructed =
		unescape_all_vectors(
			all_escaped_stuff,
			escape_char);
	P_V(deconstructed.first.size(), P_NOTE);
	P_V(deconstructed.second.size(), P_NOTE);
}

static void test_id_set_compression(){
	std::vector<id_t_> id_set;
	std::array<uint8_t, 32> hash;
	for(uint64_t i = 0;i < 8192;i++){
		// i is the UUID
		if(true_rand(0, 30) == 0){
			print("computing new has at iteration " + std::to_string(i), P_NOTE);
			hash = encrypt_api::hash::sha256::gen_raw(
				true_rand_byte_vector(64));
		}
		id_t_ tmp_id;
		set_id_uuid(&tmp_id, i);
		set_id_hash(&tmp_id, hash);
		id_set.push_back(tmp_id);
	}
	std::vector<uint8_t> id_set_compact =
		compact_id_set(id_set);
	std::vector<id_t_> id_set_new =
		expand_id_set(id_set_compact);
	if(id_set_new == id_set){
		print("successfully decompressed, checks out", P_NOTE);
		const long double compression_ratio =
			(id_set.size()*sizeof(id_t_))/(id_set_compact.size());
		P_V(compression_ratio, P_NOTE);
	}else{
		print("ERROR, COULDN'T PROPERLY DECOMPRESS", P_NOTE);
		P_V(id_set.size(), P_NOTE);
		P_V(id_set_new.size(), P_NOTE);
		// for(uint64_t i = 0;i < (id_set.size() > id_set_new.size()) ? id_set.size() : id_set_new.size();i++){
		// 	if(id_set.size() > i){
		// 		P_V_S(convert::array::id::to_hex(id_set[i]), P_NOTE);
		// 	}
		// 	if(id_set_new.size() > i){
		// 		P_V_S(convert::array::id::to_hex(id_set_new[i]), P_NOTE);
		// 	}
		// }
	}
	running = false;
}

static void benchmark_encryption(){
	try{
		if(settings::get_setting("benchmark_encryption") == "rsa"){
			benchmark_encryption("rsa");
		}else if(settings::get_setting("benchmark_encryption") == "aes"){
			benchmark_encryption("aes");
		}else if(settings::get_setting("benchmark_encryption") != ""){
			benchmark_encryption("");
		}
	}catch(...){} // don't expect it to be set unless it is true
}

int main(int argc_, char **argv_){
	argc = argc_;
	argv = argv_;
	init();
	running = true;
	test_id_set_compression();
	//test_escape_string();
	//test_aes();
	//test_id_hex();
	//test_rsa_encryption(); // includes AES too now
	//test_break_id_transport();
	//test_id_transport();
	//test_max_tcp_sockets();
	//test_compressor();
	//test_socket();
	//test_nbo_transport();
	//test_id_transport();
	//running = false;
	while(running){
		tv_loop();
		input_loop();
		net_proto_loop();
		console_loop();
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
