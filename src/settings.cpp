#include "main.h"
#include "settings.h"
#include "file.h"
#include "util.h"

static std::vector<std::pair<std::string, std::string> > settings_vector;

/*
  TODO: allow appending settings to a settings file
 */

void settings::set_setting(std::string a, std::string b){
	print("adding " + a + " == " + b, P_DEBUG);
	for(uint64_t i = 0;i < settings_vector.size();i++){
		if(unlikely(settings_vector[i].first == a)){
			settings_vector[i].second = b;
			return;
		}
	}
	settings_vector.push_back(
		std::make_pair(a, b));
}

void settings::set_settings(std::string settings_file){
	std::string cfg_file;
	try{
		cfg_file = file::read_file(settings_file);
	}catch(...){
		print("cannot load settings file", P_WARN);
		// isn't distributed in the GitHub
		return;
	}
	std::stringstream ss(cfg_file);
	std::string temp_str;
	while(std::getline(ss, temp_str)){
		if(temp_str.size() == 0){
			continue;
		}
		if(temp_str[0] == '#' || temp_str[0] == '\n'){
			continue;
		}
		std::stringstream line(temp_str);
		std::string setting, es, var;
		line >> setting >> es >> var;
		set_setting(setting, var);
	}
	for(int32_t i = 1;i < argc-1;i++){
		const uint32_t argv_len =
			strlen(argv[i]);
		if(argv_len > 2 && argv[i][0] == '-' && argv[i][1] == '-'){
			const std::string curr_setting =
				std::string(
					&argv[i][2],
					argv_len-2);
			const std::string value =
				argv[i+1];
			set_setting(curr_setting, value);
			i++; // skip over the value
		}
	}
}

// no real way to know the type, so leave that to the parent
// no way to tell a blank string from no setting, seems like good
// default behavior, but code should be secure enough to allow for
// a thrown exception, most values are stored as integers anyways

std::string settings::get_setting(std::string setting){
	std::string retval;
	for(unsigned int i = 0;i < settings_vector.size();i++){
		if(unlikely(settings_vector[i].first == setting)){
			return settings_vector[i].second;
		}
	}
	// can't call print(), it uses get_setting too
	throw std::runtime_error("setting " + setting + " not found");
	return "";
}

uint64_t settings::get_setting_unsigned_def(std::string settings, uint64_t default_val){
	try{
		return std::stoull(
			get_setting(
				settings));
	}catch(...){
		return default_val;
	}
}

void settings_init(){
	settings::set_settings("settings.cfg");
}

