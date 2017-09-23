#ifndef NET_HTTP_FILE_DRIVER_HTML_TEMPLATES_UNIVERSAL_H
#define NET_HTTP_FILE_DRIVER_HTML_TEMPLATES_UNIVERSAL_H

/*
  Okay, so here is where things start getting cool

  The main universal form only inputs an ID, and returns a new table
  which allows raw access to any variable with a standard getter
  and setter. When this table is returned, it sets new variables 
  with the same prepend as the first one

  Any value that is blank remains unchanged, typing DEFAULT will
  blank a string or set a sane default. The HTML_LOGIC actually
  runs as two units, and having to tell the difference between the
  first form response and the second form response, as well as
  the usual setting variables and all that fun stuff.

  The universal creator just takes an input type and returns the ID
  The universal destroyer just takes an ID and removes from 'all_tiers'
  (every tier but the networking ones, which don't recognize the delete)
 */

#include "net_http_file_driver_html_templates.h"

extern HTML_TEMP(universal_new);
extern HTML_TEMP(universal_set);
extern HTML_TEMP(unviersal_del);

extern HTML_LOGIC(universal_new);
extern HTML_LOGIC(universal_set);
extern HTML_LOGIC(universal_del);

#endif
