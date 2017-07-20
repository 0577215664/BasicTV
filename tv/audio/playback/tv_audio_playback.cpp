#include "tv_audio_playback.h"
#incldue "tv_audio.h"

static const tv_audio_playback_t audio_playback_list[2] = {
	{tv_audio_playback_ao_init,
	 tv_audio_playback_ao_close,
	 tv_audio_playback_ao_get_decompressed,
	 tv_audio_playback_ao_play_decompressed},
	{tv_audio_playback_mixer_init,
	 tv_audio_playback_mixer_close,
	 tv_audio_playback_mixer_get_decompressed,
	 tv_audio_playback_mixer_play_decompressed}
};

static void *audio_playback_struct_ptr = nullptr;
static uint8_t audio_playback = 0;


void tv_audio_playback_ao_init(){
	tv_audio_playback_ao_struct_t *struct_ptr =
		new tv_audio_playback_ao_struct_t;
	audio_playback_struct_ptr =
		reinterpret_cast<void*>(
			struct_ptr);
	
	ao_initialize();
	int32_t ao_default_driver = ao_default_driver_id();
	CLEAR(ao_format);
	struct_ptr->ao_format.bits = output_bit_depth;
	struct_ptr->ao_format.channels = output_channel_count;
	struct_ptr->ao_format.rate = output_sampling_rate;
	struct_ptr->ao_format.byte_format = AO_FMT_NATIVE;
	struct_ptr->ao_device_ptr =
		ao_open_live(
			ao_default_driver,
			&(struct_ptr->ao_format),
			nullptr);
	ASSERT(struct_ptr->ao_device_ptr != nullptr, P_ERR);
	ASSERT(struct_ptr->ao_format.bits == output_bit_depth, P_ERR);
	ASSERT(struct_ptr->ao_format.channels == output_channel_count, P_ERR);
	ASSERT(struct_ptr->ao_format.rate == output_sampling_rate, P_ERR);
	ASSERT(struct_ptr->ao_format.byte_format = AO_FMT_NATIVE, P_ERR);
}

void tv_audio_playback_ao_close(){
	ASSERT(audio_playback_struct_ptr == nullptr, P_ERR);
	delete reinterpret_cast<tv_audio_playback_ao_struct_t*>(
		audio_playback_struct_ptr);
	audio_playback_struct_ptr = nullptr;
}


