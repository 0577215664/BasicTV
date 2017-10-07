#ifndef NET_HTTP_FILE_DRIVER_HTML_TEMPLATE_H
#define NET_HTTP_FILE_DRIVER_HTML_TEMPLATE_H

#include "../../../../../id/id.h"
#include "../net_http_file_driver.h"
#include "../../../parse/net_http_parse.h"
#include "../../../net_http.h"

/*
  HTML_TEMP handles converting the HTML template placeholder into a properly
  used type, it takes no parameters, and the template string it replaces is
  out of the scope of the file.

  No parsing is done in the function by design, because i'd implement it as just
  passing the return value of one directly into the other, and forgetting
  that if somebody actually put the placeholder string inside of their data,
  the HTML would be pretty retarded.
 */

#define HTML_LOGIC(x__) std::string net_http_file_driver_html_##x__##_logic(net_http_file_driver_state_t *file_driver_state_ptr)
#define HTML_TEMP(x__) std::string net_http_file_driver_html_##x__##_temp()

#define GET_STR(name) std::string name = file_driver_state_ptr->request_payload.form_data.get_str(#name);;


#define FORM_START "<form action=\"/\" method=\"get\">"
#define FORM_END "</form>"

#define H1(var) "<b>" var "</b>"
#define FORM(hr, var, type) "<input type=" #type " name=" #var " placeholder=\'" hr "\' /><br />"

#define FORM_TEXT(hr, var) FORM(hr, var, text)
#define FORM_SUBMIT(hr, var) FORM(hr, var, submit)
#define FORM_FILE(hr, var) "<input type=file name=" #var " placeholder=\'" hr "\' enctype=\"multipart/form-data\"/><br />"
#define FORM_TEXTAREA(hr, var, row, col) "<textarea rows=" #row " col=" #col " /><br />"


extern std::string net_http_file_driver_html_template_logic(
	net_http_file_driver_state_t *file_driver_state_ptr);

extern std::string net_http_file_driver_html_template_generate(
	std::string temp);

#endif
