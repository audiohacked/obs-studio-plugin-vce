#include <obs-module.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-vce", "en-US")

// extern struct obs_encoder_info obs_amd_encoder;
extern void RegisterAmfEncoder();

bool obs_module_load(void)
{
	// obs_register_encoder(&obs_amd_encoder);
	RegisterAmfEncoder();
	return true;
}
