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

/* ------------------------------------------------------------------------- */

static const char *obs_vce_getname(void)
{
	return obs_vce_amf_getname();
}

static void *obs_vce_create(obs_data_t *settings, obs_encoder_t *encoder)
{
	return obs_vce_amf_create(settings, encoder);
}

static bool obs_vce_encode(void *data, struct encoder_frame *frame,
		struct encoder_packet *packet, bool *received_packet)
{
	return obs_vce_amf_encode(data, frame, packet, received_packet);
}

static void obs_vce_destroy(void *data)
{
	obs_vce_amf_destroy(data);
}

void RegisterAmfEncoder()
{
	struct obs_encoder_info obs_amd_encoder = {};

	obs_amd_encoder.id             = "obs_hw_vce_amf";
	obs_amd_encoder.type           = OBS_ENCODER_VIDEO;
	obs_amd_encoder.codec          = "h264";
	obs_amd_encoder.get_name       = obs_vce_getname;
	obs_amd_encoder.create         = obs_vce_create;
	obs_amd_encoder.destroy        = obs_vce_destroy;
	obs_amd_encoder.encode         = obs_vce_encode;
	obs_amd_encoder.update         = obs_vce_update;
	obs_amd_encoder.get_properties = obs_vce_get_properties;
	obs_amd_encoder.get_defaults   = obs_vce_get_defaults;
	obs_amd_encoder.get_extra_data = obs_vce_get_extra_data;
	obs_amd_encoder.get_sei_data   = obs_vce_get_sei;
	obs_amd_encoder.get_video_info = obs_vce_get_video_info;

	obs_register_encoder(&obs_amd_encoder);
}
