#ifndef STATE_H
#define STATE_H

#include "util.h"

/*
  Any state-based system should use the utilities here to get some
  standardization in medium lookups, initialization, destructors, 
  sanity checks, and the like
 */

template <typename T>
void state_sanity_check(T state){
	if(state == nullptr){
		print("state is a nullptr", P_ERR);
	}
	if(state->get_state_ptr() == nullptr){
		print("state_ptr is a nullptr", P_ERR);
	}
}

#define STD_STATE_INIT(major_type, major_name, minor_type, minor_name)	\
	major_type *major_name = new major_type;			\
	minor_type *minor_name = new minor_type;			\
	major_name->set_state_ptr((void*)minor_name);			\

#define STD_STATE_GET_PTR(major_name, minor_type, minor_name)	\
	ASSERT(major_name != nullptr, P_ERR);			\
	minor_type *minor_name =				\
		(minor_type*)(major_name->get_state_ptr());	\
	ASSERT(minor_name != nullptr, P_ERR);			\

#define STD_STATE_CLOSE(major_name, minor_type)				\
	if(major_name != nullptr){					\
		if(major_name->get_state_ptr() != nullptr){		\
			delete (minor_type*)major_name->get_state_ptr(); \
			major_name->set_state_ptr(nullptr);		\
		}else{							\
			print(#minor_type" was already destroyed (nullptr)", P_WARN); \
		}							\
		delete major_name;					\
		major_name = nullptr;					\
	}else{								\
		print(#major_name" was already destroyed (nullptr)", P_WARN); \
	}								\


struct state_t{
private:
	void *state_ptr = nullptr;
	uint8_t state_format = 0;
public:
	GET_SET_S(state_ptr, void*);
	uint8_t get_medium(){return state_format;}
	void set_medium(uint8_t state_format_){state_format = state_format_;}
	GET_SET_S(state_format, uint8_t);
};

#define STD_STATE_INIT_CLOSE(major_type, major_name)		\
	major_type (*init)() = nullptr;				\
	void (*close)(major_type* major_name) = nullptr;

#endif
