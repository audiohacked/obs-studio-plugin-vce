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

#define ENC_WIDTH obs_encoder_get_width(obs_vce->encoder)
#define ENC_HEIGHT obs_encoder_get_height(obs_vce->encoder)

class VCEDeviceD3D11;

const char *obs_vce_amf_getname(void)
{
	return "AMD Video Codec Engine: AMF";
}

void *obs_vce_amf_create(obs_data_t *settings, obs_encoder_t *encoder)
{
	UNUSED_PARAMETER(settings);
	struct obs_amd *obs_vce = reinterpret_cast<struct obs_amd*>(bzalloc(sizeof(struct obs_amd)));
	obs_vce->encoder = encoder;
	AMF_RESULT amfReturn = AMF_OK;
	struct obs_vce_amf *ova = reinterpret_cast<struct obs_vce_amf*>(bzalloc(sizeof(struct obs_vce_amf)));

	const struct video_output_info *voi;
	voi = video_output_get_info(obs_get_video());

	//struct obs_video_info *ovi;
	//bool got_video = obs_get_video_info(ovi);
	//if (!got_video)
	//	warn("No active video");

	/* Create and Initialize context
	 * Allocate via AMFCreateContext()
	 * Set Device via
	 */
	amfReturn = AMFCreateContext(&obs_vce->context);
	if (amfReturn != AMF_OK) {
		debug("Could not create AMF Context!");
	}

	if (gs_get_device_type() == GS_DEVICE_DIRECT3D_11) {
		debug("DX11 device and AMF");
		obs_vce->context->InitDX11(NULL);
	}
	else if (gs_get_device_type() == GS_DEVICE_OPENGL) {
		debug("OpenGL device and AMF");
	}
	else {
		debug("Unknown device and AMF");
		obs_vce_amf_d3d11_init(obs_vce, ova, 0);
		obs_vce->context->InitDX11(obs_vce->dx11_device);
	}

	/* Create Component via AMFCreateComponent
	 */
	AMFCreateComponent(obs_vce->context, AMFVideoEncoderVCE_AVC, &obs_vce->vce_encoder);
	if (amfReturn != AMF_OK)
		warn("Could not create AMF VCE Component");

	/* Initialize Encoder component
	 *     Set all optional properties on component:
	 *         AMFPropertyStorage::SetProperty()
	 *     Init component:
	 *         AMFComponent::Init()
	 */
	// Static properties - can be set befor Init()
	// amf_int64(AMF_VIDEO_ENCODER_USAGE_ENUM); default = N/A; Encoder usage type. fully configures parameter set.
	amfReturn = obs_vce->vce_encoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, amf_int64(AMF_VIDEO_ENCODER_USAGE_LOW_LATENCY));
	if (amfReturn != AMF_OK)
		warn("Unable to set encoder usage type");

	// amf_int64(AMF_VIDEO_ENCODER_PROFILE_ENUM) ; default = AMF_VIDEO_ENCODER_PROFILE_MAIN;  H264 profile
	amfReturn = obs_vce->vce_encoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE, amf_int64(AMF_VIDEO_ENCODER_PROFILE_MAIN));
	if (amfReturn != AMF_OK)
		warn("Unable to set encoder profile");

	// amf_int64; default = 42; H264 profile level
	amfReturn = obs_vce->vce_encoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL, amf_int64(41));
	if (amfReturn != AMF_OK)
		warn("Unable to set encoder profile level");

	// amf_int64(AMF_VIDEO_ENCODER_QUALITY_PRESET_ENUM); default = depends on USAGE; Quality Preset
	amfReturn = obs_vce->vce_encoder->SetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, amf_int64(AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED));
	if (amfReturn != AMF_OK)
		warn("Unable to set encoder quality preset");


	// AMFSize; default = 0,0; Frame size
	// obs_vce->vce_encoder->SetProperty(amf::AMF_VIDEO_ENCODER_FRAMESIZE, );
	
	// AMFRate; default = depends on usage; Frame Rate
	// obs_vce->vce_encoder->SetProperty(amf::AMF_VIDEO_ENCODER_FRAMERATE, );
	
	// AMFInterface* - > AMFBuffer*; SPS/PPS buffer - read-only
	// obs_vce->vce_encoder->SetProperty(amf::AMF_VIDEO_ENCODER_EXTRADATA, );
	
	if (voi->format == VIDEO_FORMAT_NV12) {
		obs_vce->vce_format = amf::AMF_SURFACE_NV12;
	}

	amfReturn = obs_vce->vce_encoder->Init(obs_vce->vce_format, amf_int32(ENC_WIDTH), amf_int32(ENC_HEIGHT));
	if (amfReturn != AMF_OK)
		warn("Unable to Init Encoder");
	
	obs_vce->performance_token =
		os_request_high_performance("vce_amf encoding");

	return obs_vce;
}

bool obs_vce_amf_encode(void *data, struct encoder_frame *frame,
	struct encoder_packet *packet, bool *received_packet)
{
	struct obs_amd *obs_vce = reinterpret_cast<struct obs_amd*>(data);
	amf::AMFDataPtr outData;
	AMF_RESULT amfReturn = AMF_OK;

	if (!frame || !packet || !received_packet) {
		warn("Data missing");
		return false;
	}

	/* Create input data object
	 *     Allocate input surface with allocated internally surface
	 *         AMFContext::AllocSurface()
	 *     or Allocate input surface with attached
	 *         AMFContext::CreateSurfaceFrom<>
	 *     Copy input data object using native data-access functionality
	 *         AMFSurface::GetPlane(), AMFPlane::GetNative()
	 */
	if (gs_get_device_type() == GS_DEVICE_DIRECT3D_11) {
		debug("Using DX11 Device for AMF Surface");
		//obs_vce->context->CreateSurfaceFromDX11Native();
	}
	else if (gs_get_device_type() == GS_DEVICE_OPENGL) {
		debug("Using OpenGL Device for AMF Surface");
		//obs_vce->context->CreateSurfaceFromOpenGLNative();
	}
	else {
		debug("Allocating Host Surface");
		debug("AllocSurface");
		amfReturn = obs_vce->context->AllocSurface(amf::AMF_MEMORY_HOST,
				obs_vce->vce_format, ENC_WIDTH, ENC_HEIGHT,
				&obs_vce->vce_input);
		obs_amf_result(obs_vce, amfReturn);
	}
	init_pic_data(obs_vce, frame);

	// obs_vce->vce_input->SetProperty(AMF_VIDEO_ENCODER_END_OF_SEQUENCE, false);
	// obs_vce->vce_input->SetProperty(AMF_VIDEO_ENCODER_END_OF_STREAM, false);
	// obs_vce->vce_input->SetProperty(AMF_VIDEO_ENCODER_FORCE_PICTURE_TYPE, AMF_VIDEO_ENCODER_PICTURE_TYPE_IDR);
	// obs_vce->vce_input->SetProperty(AMF_VIDEO_ENCODER_INSERT_AUD, false);
	// obs_vce->vce_input->SetProperty(AMF_VIDEO_ENCODER_INSERT_SPS, false);
	// obs_vce->vce_input->SetProperty(AMF_VIDEO_ENCODER_INSERT_PPS, false);
	// obs_vce->vce_input->SetProperty(AMF_VIDEO_ENCODER_PICTURE_STRUCTURE, AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_FRAME);
	// obs_vce->vce_input->SetProperty(AMF_VIDEO_ENCODER_MARK_CURRENT_WITH_LTR_INDEX, -1);
	// obs_vce->vce_input->SetProperty(AMF_VIDEO_ENCODER_FORCE_LTR_REFERENCE_BITFIELD, 0);

	/* Submit data object to encoder
	 *     Set additional parameters on the data object
	 *     (e.g. some application ID if needed) using
	 *         AMFPropertyStorage::SetProperty()
	 *     Submits data to component by
	 *         AMFComponent::SubmitInput()
	 */
	debug("SubmitInput to VCE");
	amfReturn = obs_vce->vce_encoder->SubmitInput(obs_vce->vce_input);
	if (amfReturn == AMF_INPUT_FULL) {
		warn("VCE Encoder Input Full");
	}
	else if (amfReturn == AMF_ENCODER_NOT_PRESENT) {
		warn("VCE Hardware Not Present");
	}
	else {
		obs_amf_result(obs_vce, amfReturn);
	}

	/* Queries for results (likely in a separate thread) by
	 *     AMFComponent::QueryOutput
	 */
	debug("QueryOutput from VCE");
	amfReturn = obs_vce->vce_encoder->QueryOutput(&outData);
	if (amfReturn == AMF_OK) {
		parse_packet(obs_vce, packet, outData);
		return true;
	}

	// debug("Telling VCE to Drain");
	// amfReturn = obs_vce->vce_encoder->Drain();
	// if (amfReturn == AMF_OK)
	// 	return true;

	return false;
}

void obs_vce_amf_destroy(void *data)
{
	struct obs_amd *obs_vce = reinterpret_cast<struct obs_amd*>(data);

	amf::AMFDataPtr amfdata;
	AMF_RESULT amfReturn = AMF_OK;

	if (obs_vce) {
		os_end_high_performance(obs_vce->performance_token);

		/* At the end of the file execute 'drain' to force the component to 
		 * return all accumulated frames: AMFComponent::Drain()
		 */
		obs_vce->vce_encoder->Drain();

		/* Checks for EOF error returning from QueryResult() to detect end of drain
		 */
		amfReturn = obs_vce->vce_encoder->QueryOutput(reinterpret_cast<amf::AMFData**>(&obs_vce->vce_output));
		if (amfReturn == AMF_EOF)
			warn("EOF to detect end of drain");

		obs_vce->vce_input->Release();
		//obs_vce->vce_output->Release();

		/* Terminate component and context
		 * Terminate component and release all internal resources by AMFComponent::Terminate()
		 * Terminate context by AMFContext::Terminate()
		 */
		obs_vce->vce_encoder->Terminate();
		obs_vce->context->Terminate();

		/* Clean up OBS data
		*/
		bfree(obs_vce->sei);
		bfree(obs_vce->extra_data);
		obs_vce->sei = NULL;
		obs_vce->extra_data = NULL;
		da_free(obs_vce->packet_data);
		bfree(obs_vce);
	}
}

void obs_amf_result(struct obs_amd *obs_vce, AMF_RESULT amf_res)
{
	char* string = NULL;
	switch (amf_res) {
	case AMF_OK:
		string = "Ok!";
		break;
	case AMF_FAIL:
		string = "Failed!";
		break;
	case AMF_UNEXPECTED:
		string = "Unexpected!";
		break;
	case AMF_ACCESS_DENIED:
		string = "Access Denied!";
		break;
	case AMF_INVALID_ARG:
		string = "Invalid Arg";
		break;
	case AMF_OUT_OF_RANGE:
		string = "Out Of Memory";
		break;
	case AMF_OUT_OF_MEMORY:
		string = "Out Of Memory";
		break;
	case AMF_INVALID_POINTER:
		string = "Invalid Pointer";
		break;
	case AMF_NO_INTERFACE:
		string = "No Interface";
		break;
	case AMF_NOT_IMPLEMENTED:
		string = "Not Implemented";
		break;
	case AMF_NOT_SUPPORTED:
		string = "Not Supported";
		break;
	case AMF_NOT_FOUND:
		string = "Not Found";
		break;
	case AMF_ALREADY_INITIALIZED:
		string = "Already Initialized";
		break;
	case AMF_NOT_INITIALIZED:
		string = "Not Initialized";
		break;
	case AMF_INVALID_FORMAT:// invalid data format
		string = "Invalid Format";
		break;
	case AMF_WRONG_STATE:
		string = "Wrong State";
		break;
	case AMF_FILE_NOT_OPEN: // cannot open file
		string = "File Not Open";
		break;
	// device common codes
	case AMF_NO_DEVICE:
		string = "No Device";
		break;
		// device directx
	case AMF_DIRECTX_FAILED:
		string = "DirectX Failed";
		break;
		// device opencl
	case AMF_OPENCL_FAILED:
		string = "OpenCL Failed";
		break;
		// device opengl
	case AMF_GLX_FAILED: //failed to use GLX
		string = "GLX Failed";
		break;
		// device alsa
	case AMF_ALSA_FAILED: //failed to use ALSA
		string = "Alsa Failed";
		break;
	case AMF_EOF:
		string = "End of File";
		break;
	case AMF_REPEAT:
		string = "Repeat";
		break;
	case AMF_NEED_MORE_INPUT: //returned by AMFComponent::QueryOutput if more frames to be submited
		string = "Need More Input";
		break;
	case AMF_INPUT_FULL: //returned by AMFComponent::SubmitInput if input queue is full
		string = "Input Full";
		break;
	case AMF_RESOLUTION_CHANGED: //resolution changed client needs to Drain / Terminate / Init
		string = "Resolution Changed";
		break;
		//error codes
	case AMF_INVALID_DATA_TYPE:
		string = "Invalid Data Type";
		break;
	case AMF_INVALID_RESOLUTION:
		string = "Invalid Resolution";
		break;
	case AMF_CODEC_NOT_SUPPORTED:
		string = "Codec Not Supported";
		break;
	case AMF_SURFACE_FORMAT_NOT_SUPPORTED:
		string = "Surface Format Not Supported";
		break;
		//component video encoder
	case AMF_ENCODER_NOT_PRESENT:
		string = "Encoder Not Present";
		break;
	default:
		string = "Unknown";
		break;
	}
	debug("AMF_RESULT: %s", string);
}