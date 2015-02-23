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

void init_pic_data(struct obs_amd *obs_vce, struct encoder_frame *frame)
{
	debug("init_pic_data");
	amf::AMFPlanePtr amf_plane;

	debug("SetPts");
	obs_vce->vce_input->SetPts(frame->pts);
	obs_vce->vce_input->SetFrameType(amf::AMF_FRAME_PROGRESSIVE);
	//obs_vce->vce_input->SetDuration();

	if (obs_vce->vce_format == amf::AMF_SURFACE_NV12) {
		debug("video format NV12");
		debug("NV12 Plane Count: %d", obs_vce->vce_input->GetPlanesCount());

		debug("copy frame plane Y to VCE");
		//memcpy(obs_vce->vce_input->GetPlane(amf::AMF_PLANE_Y), frame->data[0], sizeof(frame->data[0]));
		memcpy(amf_plane, frame->data[0], frame->linesize[0]);

		info("copy frame plane UV to VCE");
		//memcpy(obs_vce->vce_input->GetPlane(amf::AMF_PLANE_UV), frame->data[1], sizeof(frame->data[1]));
		memcpy(amf_plane + frame->linesize[0], frame->data[1], frame->linesize[1]);
	}
}
