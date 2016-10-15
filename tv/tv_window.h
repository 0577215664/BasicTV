#include "../main.h"
#include "tv.h"
#ifndef TV_WINDOW_H
#define TV_WINDOW_H

/*
  A channel has to exist inside of a window at all times. When moving from
  a lower resolution to a higher resolution display, the top-left channel
  is always kept and the lower-right channels are cropped off or added upon.
  The position of the window is defined in an X and Y plane
 */

/*
  tv_window_t: window class for tv_frame_t
 */

struct tv_window_t{
private:
	uint8_t x_pos = 0; // start counting at 0
	uint8_t y_pos = 0;
	uint64_t channel_id = 0;
public:
	data_id_t id;
	tv_window_t();
	~tv_window_t();
	uint8_t get_x_pos();
	void get_x_pos(uint8_t x_pos_);
	uint8_t get_y_pos();
	void get_y_pos(uint8_t y_pos_);
};

#endif
