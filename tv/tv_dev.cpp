#include "tv_dev.h"

tv_dev_t::tv_dev_t(){}

tv_dev_t::~tv_dev_t(){
	close(file_descriptor);
}

void tv_dev_t::list_virtual_data(data_id_t *id){
	// no need to include it
}

void tv_dev_t::open_dev(std::string filename_){
	file_descriptor = open(filename_.c_str(),
			       O_RDWR | O_NONBLOCK,
			       0);
	if(file_descriptor == -1){
		print("unable to open file descriptor:" + std::to_string(errno), P_ERR);
	}
}

uint64_t tv_dev_t::get_last_frame_id(){
	return last_frame_id;
}

void tv_dev_t::set_last_frame_id(uint64_t last_frame_id_){
	last_frame_id = last_frame_id_;
}


uint64_t tv_dev_t::get_file_descriptor(){
	return file_descriptor;
}

int tv_dev_t::set_ioctl(int request, void *arg){
	if(file_descriptor <= 0){
		print("invalid file descriptor", P_SPAM);
	}
	int retval;
	do{
		retval = ioctl(file_descriptor, request, arg);
	}while(unlikely(retval == -1 && (errno == EINTR || errno == EAGAIN)));
	if(retval == -1){
		print("ioctl (" + convert::number::to_binary(request) + ") failed on a non-EINTR error:" + std::to_string(errno), P_ERR);
	}else{
		print("ioctl (" + convert::number::to_binary(request) + ") succeeded", P_SPAM);
	}
	return retval;
}

