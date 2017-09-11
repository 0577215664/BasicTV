#ifndef SETTINGS_H
#define SETTINGS_H
#include <vector>
#include <utility>
#include <string>
#include <mutex>
#include <sstream>
#include <cstdio>
namespace settings{
	void set_settings(std::string settings_file = "settings.cfg");
	void set_setting(std::string a, std::string b);
	std::string get_setting(std::string);
	uint64_t get_setting_unsigned_def(std::string setting, uint64_t default_val);
}
extern void settings_init();
#endif
