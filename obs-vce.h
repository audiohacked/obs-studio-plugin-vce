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
#pragma once

#include <stdio.h>
#include <util/dstr.h>
#include <util/darray.h>
#include <util/platform.h>
#include <obs-module.h>

#include <core/Buffer.h>
#include <core/Surface.h>
#include <core/Context.h>
#include <components/Component.h>
#include <components/VideoEncoderVCE.h>

#include <string>
#include <atlbase.h>
#include <d3d11.h>

#define do_log(level, format, ...) \
	blog(level, "[vce_amf encoder: '%s'] " format, \
			obs_encoder_get_name(obs_vce->encoder), ##__VA_ARGS__)

#define error(format, ...) do_log(LOG_ERROR,   "ERROR: " format, ##__VA_ARGS__)
#define warn(format, ...)  do_log(LOG_WARNING, "WARN: "  format, ##__VA_ARGS__)
#define info(format, ...)  do_log(LOG_INFO,    "INFO: "  format, ##__VA_ARGS__)
#define debug(format, ...) do_log(LOG_DEBUG,   "DEBUG: " format, ##__VA_ARGS__)

/* ------------------------------------------------------------------------- */
/* prototypes */
const char *obs_vce_amf_getname(void);
void *obs_vce_amf_create(obs_data_t *settings, obs_encoder_t *encoder);
bool obs_vce_amf_encode(void *data, struct encoder_frame *frame, struct encoder_packet *packet, bool *received_packet);
void obs_vce_amf_destroy(void *data);

bool obs_vce_update(void *data, obs_data_t *settings);
obs_properties_t *obs_vce_get_properties(void *data);
void obs_vce_get_defaults(obs_data_t *settings);
bool obs_vce_get_extra_data(void *data, uint8_t **extra_data, size_t *size);
bool obs_vce_get_sei(void *data, uint8_t **sei, size_t *size);
bool obs_vce_get_video_info(void *data, struct video_scale_info *info);

void obs_amf_result(struct obs_amd *obs_vce, AMF_RESULT amf_res);
AMF_RESULT obs_vce_amf_d3d11_terminate(struct obs_amd *obs_vce);

AMF_RESULT obs_vce_amf_d3d11_init(struct obs_amd *obs_vce, struct obs_vce_amf *ova, amf_uint32 adapterID);

void parse_packet(struct obs_amd *obs_vce, struct encoder_packet *packet, amf::AMFDataPtr pic_out);
void init_pic_data(struct obs_amd *obs_vce, struct encoder_frame *frame);

	/* ------------------------------------------------------------------------- */
/* OBS to AMF translators */

/* ------------------------------------------------------------------------- */
/* Data Structures */

struct obs_amd {
	obs_encoder_t           *encoder;

	// amf::AMFPropertyStorage vce_param;
	amf::AMFContextPtr         context;
	amf::AMFComponentPtr       vce_encoder;
	amf::AMFSurfacePtr         vce_input;
	amf::AMFBufferPtr          vce_output;

	amf::AMF_SURFACE_FORMAT    vce_format;

	DARRAY(uint8_t)            packet_data;

	uint8_t                    *extra_data;
	uint8_t                    *sei;

	size_t                     extra_data_size;
	size_t                     sei_size;

	os_performance_token_t     *performance_token;

	ATL::CComPtr<ID3D11Device> dx11_device;
};

struct obs_vce_amf {
	obs_vce_amf() {
		m_adaptersCount = 0;
		memset(m_adaptersIndexes, 0, sizeof(m_adaptersIndexes));
	}
	//~obs_vce_amf() { obs_vce_amf_d3d11_terminate(); }

	//ATL::CComPtr<ID3D11Device> GetDevice() { return m_pD3DDevice; }
	//std::wstring GetDisplayDeviceName() { return m_displayDeviceName; }
	//void Enumerate(void);
	ATL::CComPtr<ID3D11Device>      m_pD3DDevice;
	static const amf_uint32         MAXADAPTERS = 128;
	amf_uint32                      m_adaptersCount;
	amf_uint32                      m_adaptersIndexes[MAXADAPTERS];
	std::wstring                    m_displayDeviceName;

};
