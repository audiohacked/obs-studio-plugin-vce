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

void parse_packet(struct obs_amd *obs_vce,
		struct encoder_packet *packet, amf::AMFBufferPtr pic_out)
{
	info("parse_packet");
	UNUSED_PARAMETER(packet);
	//da_resize(obs_vce->packet_data, 0);
	//for (int i = 0; i < nal_count; i++) {
	//	da_push_back_array(obs_vce->packet_data, nal->p_payload,
	//			nal->i_payload);
	//}
	//packet->data = obs_vce->packet_data.array;
	//packet->size = obs_vce->packet_data.num;
	//packet->type = OBS_ENCODER_VIDEO;
	//packet->pts = (int64_t)pic_out->GetPts();
	//packet->dts = pic_out->
	//packet->keyframe = pic_out->
}

void init_pic_data(struct obs_amd *obs_vce, struct encoder_frame *frame)
{
	info("init_pic_data");
	//amf::AMFPlanePtr amf_plane[3];
	const struct video_output_info *voi;
	voi = video_output_get_info(obs_get_video());

	info("SetPts");
	obs_vce->vce_input->SetPts(frame->pts);
	obs_vce->vce_input->SetFrameType(amf::AMF_FRAME_PROGRESSIVE);
	//obs_vce->vce_input->SetDuration();

	switch (voi->format) {
	case VIDEO_FORMAT_I420:
		memcpy(obs_vce->vce_input->GetPlane(amf::AMF_PLANE_V), frame->data[2], sizeof(frame->data[2]));
		info("copy frame plane V to VCE");
		memcpy(obs_vce->vce_input->GetPlane(amf::AMF_PLANE_U), frame->data[1], sizeof(frame->data[1]));
		info("copy frame plane U to VCE");
		memcpy(obs_vce->vce_input->GetPlane(amf::AMF_PLANE_Y), frame->data[0], sizeof(frame->data[0]));
		info("copy frame plane Y to VCE");
		break;
	case VIDEO_FORMAT_NV12:
		info("video format NV12");
		info("NV12 Plane Count: %d", obs_vce->vce_input->GetPlanesCount());
		memcpy(obs_vce->vce_input->GetPlane(amf::AMF_PLANE_UV), frame->data[1], sizeof(frame->data[1]));
		info("copy frame plane UV to VCE");
		memcpy(obs_vce->vce_input->GetPlane(amf::AMF_PLANE_Y), frame->data[0], sizeof(frame->data[0]));
		info("copy frame plane Y to VCE");
		break;
	default:
		warn("Unknown Video Format");
	}
}
