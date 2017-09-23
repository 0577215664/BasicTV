#ifndef NET_HTTP_FILE_DRIVER_FRONTPAGE_FORMS_H
#define NET_HTTP_FILE_DRIVER_FRONTPAGE_FORMS_H

#include <string>

/*
  This is just trying to expose as much of the internals as reasonably possible.
  The direction i'm moving in is exposing as much as possible in the best way
  possible, versus oversimplifying the system and chopping off some essential
  information.
*/

extern std::string net_http_file_driver_frontpage_form_create_wallet_set();
extern std::string net_http_file_driver_frontpage_form_add_wallet_to_set();
extern std::string net_http_file_driver_frontpage_form_create_tv_channel();
extern std::string net_http_file_driver_frontpage_form_create_tv_item_plain();
extern std::string net_http_file_driver_frontpage_form_create_tv_item_upload();
extern std::string net_http_file_driver_frontpage_form_bind_tv_sink_item_window();
extern std::string net_http_file_driver_frontpage_form_unbind_tv_sink_item_window();
extern std::string net_http_file_driver_frontpage_form_control_window();

#endif
