#include "thread.h"
#include "main.h"
#include "lock.h"

static std::map<std::thread::id, std::vector<thread_state_t> > thread_state_map;

// void threading::state::push(
// 	thread_state_t state){
// }

// void threading::state::pop(
// 	thread_state_t state){
// }
