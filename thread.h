#ifndef THREAD_H
#define THREAD_H

#include "id/id.h"

#include "thread"
#include "mutex"
#include "map"

/*
  Places in the code where settings and modifications need to deviate from
  the default, and where passing them down as parameters doesn't make much
  sense, the code refers to a thread lookup table, where default settings
  are derived from fairly easily.

  Threads can be spun up in code inline fairly easily. Although threads
  do not (yet) have access to the ID subsystem, currently this is used for
  defining behavior for ID creation.

  Thread states function similarly to locks, except they can stack on top
  of each other, and are peeled away in that order as well.

  Thread states are only to be used inside the logic code

  Thread states inherit all defaults from the previous
 */

struct thread_state_t{
public:
	/* thread_state_t( */
	/* 	data_id_transport_rules_t transport_rules_){ */
	/* 	transport_rules = transport_rules_; */
	/* } */
};

namespace threading{
	namespace state{
		void push(
			thread_state_t state);
		void pop(
			thread_state_t state); // only for asserting validity
	};
};

#endif
