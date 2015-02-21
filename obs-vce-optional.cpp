/******************************************************************************
    Copyright (C) 2015 by Sean Nelson <audiohacked@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include "obs-vce.h"

#define TEXT_BITRATE        obs_module_text("Bitrate")
#define TEXT_KEYINT_SEC     obs_module_text("KeyframeIntervalSec")
#define TEXT_USE_CBR        obs_module_text("UseCBR")
#define TEXT_DISCARD_FILLER obs_module_text("DiscardFiller")
#define TEXT_CRF            obs_module_text("CRF")
#define TEXT_NO_INTEROP     obs_module_text("NoInterop")
#define TEXT_ADAPTER        obs_module_text("Adapter")
#define TEXT_NONE           obs_module_text("None")
#define TEXT_PROFILE        obs_module_text("Profile")

/* ------------------------------------------------------------------------- */

bool obs_vce_update(void *data, obs_data_t *settings)
{
	// struct obs_amd *obs_vce = reinterpret_cast<struct obs_amd*>(data);
	UNUSED_PARAMETER(data);
	UNUSED_PARAMETER(settings);
	// bool success = update_settings(obsx264, settings);
	// int ret;

	// if (success) {
	// 	ret = x264_encoder_reconfig(obsx264->context, &obsx264->params);
	// 	if (ret != 0)
	// 		warn("Failed to reconfigure: %d", ret);
	// 	return ret == 0;
	// }

	return false;
}
static bool use_cbr_modified(obs_properties_t *ppts, obs_property_t *p, obs_data_t *settings)
{
	bool cbr = obs_data_get_bool(settings, "cbr");
	p = obs_properties_get(ppts, "crf");
	obs_property_set_visible(p, !cbr);
	return true;
}

obs_properties_t *obs_vce_get_properties(void *data)
{
	UNUSED_PARAMETER(data);
	// struct obs_amd *obs_vce = reinterpret_cast<struct obs_amd*>(data);

	obs_properties_t *props = obs_properties_create();
	obs_property_t *list;
	obs_property_t *p;

	obs_properties_add_int(props, "bitrate", TEXT_BITRATE, 50, 10000000, 1);

	obs_properties_add_int(props, "keyint_sec", TEXT_KEYINT_SEC, 0, 20, 1);

	p = obs_properties_add_bool(props, "cbr", TEXT_USE_CBR);
	obs_properties_add_int(props, "crf", TEXT_CRF, 0, 51, 1);
	obs_property_set_modified_callback(p, use_cbr_modified);

	obs_properties_add_int(props, "adapter", TEXT_ADAPTER, 0, 4, 1);

	list = obs_properties_add_list(props, "profile", TEXT_PROFILE,
			OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	obs_property_list_add_string(list, TEXT_NONE, "");
	obs_property_list_add_string(list, "baseline", "baseline");
	obs_property_list_add_string(list, "main", "main");
	obs_property_list_add_string(list, "high", "high");

	return props;
}

void obs_vce_get_defaults(obs_data_t *settings)
{
	obs_data_set_default_int   (settings, "bitrate",     3500);
	obs_data_set_default_int   (settings, "keyint_sec",  0);
	obs_data_set_default_int   (settings, "crf",         23);
	obs_data_set_default_bool  (settings, "cbr",         false);
	obs_data_set_default_int   (settings, "adapter",     0);
}

bool obs_vce_get_extra_data(void *data, uint8_t **extra_data, size_t *size)
{
	struct obs_amd *obs_vce = reinterpret_cast<struct obs_amd*>(data);

	if (!obs_vce->context)
		return false;

	*extra_data = obs_vce->extra_data;
	*size       = obs_vce->extra_data_size;
	return true;
}

bool obs_vce_get_sei(void *data, uint8_t **sei, size_t *size)
{
	struct obs_amd *obs_vce = reinterpret_cast<struct obs_amd*>(data);

	if (!obs_vce->context)
		return false;

	*sei  = obs_vce->sei;
	*size = obs_vce->sei_size;
	return true;
}

bool obs_vce_get_video_info(void *data, struct video_scale_info *info)
{
	struct obs_amd *obs_vce = reinterpret_cast<struct obs_amd*>(data);
	video_t *video = obs_encoder_video(obs_vce->encoder);
	const struct video_output_info *vid_info = video_output_get_info(video);

	if (vid_info->format == VIDEO_FORMAT_I420 ||
	    vid_info->format == VIDEO_FORMAT_NV12)
		return false;

	info->format     = VIDEO_FORMAT_NV12;
	info->width      = vid_info->width;
	info->height     = vid_info->height;
	info->range      = vid_info->range;
	info->colorspace = vid_info->colorspace;

	return true;
}
