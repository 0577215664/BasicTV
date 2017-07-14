#ifndef TEST_H
#define TEST_H

#include "../id/id.h"

/*
  Standardized testing interface, with namespaces and all that jazz, so 
  we have some organization and clarity to what is specifically being tested
  here.

  Each run is one "pass" through the test, and multiple tests can be ran to
  prove sanity and find patterns in the error output
 */

namespace test{
	namespace id_system{
		/*
		  "proper" transporting is piping export into import properly
		  and throwing on any errors (i.e. no catch and sanity checks)

		  "random" transporting is just feeding random information into
		  the import function, catching/silencing all errors, and
		  waiting for a segfault (essentially)

		  "proper" = correct implementation
		  "random" = failsafe implementation
		 */
		namespace transport{
			void proper();
			namespace import{
				void random();
			};
		};
		namespace id_set{
			void proper();
		};
	};
	namespace net{
		namespace proto_socket{
			void send_recv();
		};
		namespace socket{
			void send_recv();
		};
	};
	namespace math{ 
		namespace number{
			void add();
			void sub();
			void mul();
			void div();
		};
	};
	namespace compress{
		void zstd();
		void xz();
	};
	namespace audio{
		void opus();
	};
	namespace video{
		// void vp9(); // so close, but yet so far...
	};
	namespace util{
		// misc utils would go here
	};
	namespace escape{
		void proper();
	};
};

extern id_t_ test_create_generic_id();

extern void test_suite();

#endif
