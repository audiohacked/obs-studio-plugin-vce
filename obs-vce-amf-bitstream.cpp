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
	debug("parse_packet");

	int frameType = -1;

	packet->type = OBS_ENCODER_VIDEO;

	debug("get data");
	packet->data = (uint8_t*)pic_out->GetNative();

	debug("get size");
	packet->size = pic_out->GetSize();

	debug("get pts");
	packet->pts = (int64_t)pic_out->GetPts();

	debug("get duration");
	packet->dts = pic_out->GetDuration();

	debug("get keyframe");
	pic_out->GetProperty(AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE, &frameType);
	packet->keyframe = (frameType == 0);

}
