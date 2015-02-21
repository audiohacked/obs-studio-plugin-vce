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

#include <SDKDDKVer.h>
#include <string>
#include <atlbase.h>
#include <d3d11.h>

#include "obs-vce.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

//struct VCEDeviceD3D11 {
//	VCEDeviceD3D11(struct obs_amd *obsvce):m_adaptersCount(0) {
//		obs_vce = obsvce;
//		memset(m_adaptersIndexes, 0, sizeof(m_adaptersIndexes));
//	}
//	~VCEDeviceD3D11() { Terminate(); }
//
//	ATL::CComPtr<ID3D11Device> GetDevice() { return m_pD3DDevice; }
//	std::wstring GetDisplayDeviceName() { return m_displayDeviceName; }
//	void Enumerate(void);
//	ATL::CComPtr<ID3D11Device>      m_pD3DDevice;
//	static const amf_uint32 MAXADAPTERS = 128;
//	amf_uint32                      m_adaptersCount;
//	amf_uint32                      m_adaptersIndexes[MAXADAPTERS];
//	std::wstring                    m_displayDeviceName;
//};

AMF_RESULT obs_vce_amf_d3d11_terminate(struct obs_amd *obs_vce)
{
	obs_vce->dx11_device.Release();
	return AMF_OK;
}

void obs_vce_amf_d3d11_enumerate(struct obs_amd *obs_vce, struct obs_vce_amf *ova)
{
    ATL::CComPtr<IDXGIFactory> pFactory;
    HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&pFactory);
    if(FAILED(hr))
    {
        warn("CreateDXGIFactory failed. Error: %x", hr);
        return;
    }

    info("DX11: List of adapters:");
    UINT count = 0;
    ova->m_adaptersCount = 0;
    while(true)
    {
		warn("DX11: EnumAdaptor");
        ATL::CComPtr<IDXGIAdapter> pAdapter;
        if(pFactory->EnumAdapters(count, &pAdapter) == DXGI_ERROR_NOT_FOUND)
        {
            break;
        }

		warn("DX11: GetDesc");
        DXGI_ADAPTER_DESC desc;
        pAdapter->GetDesc(&desc);

        if(desc.VendorId != 0x1002)
        {
            count++;
            continue;
        }
		warn("DX11: EnumOutputs");
        ATL::CComPtr<IDXGIOutput> pOutput;
        if(pAdapter->EnumOutputs(0, &pOutput) == DXGI_ERROR_NOT_FOUND)
        {
            count++;
            continue;
        }
        char strDevice[100];
        _snprintf_s(strDevice, 100, "%X", desc.DeviceId);

		info("\t%d: Device ID: %s [%s]", ova->m_adaptersCount, strDevice, desc.Description);
        ova->m_adaptersIndexes[ova->m_adaptersCount] = count;
        ova->m_adaptersCount++;
        count++;
    }
}

AMF_RESULT obs_vce_amf_d3d11_init(struct obs_amd *obs_vce, struct obs_vce_amf *ova, amf_uint32 adapterID)
{
	HRESULT hr = S_OK;
	// find adapter
	ATL::CComPtr<IDXGIAdapter> pAdapter;

	obs_vce_amf_d3d11_enumerate(obs_vce, ova);
	// CHECK_RETURN(m_adaptersCount > adapterID, AMF_INVALID_ARG, L"Invalid Adapter ID");

	//convert logical id to real index
	adapterID = ova->m_adaptersIndexes[adapterID];

	ATL::CComPtr<IDXGIFactory> pFactory;
	hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&pFactory);
	if (FAILED(hr))
	{
		warn("CreateDXGIFactory failed. Error: %x", hr);
		return AMF_FAIL;
	}

	if (pFactory->EnumAdapters(adapterID, &pAdapter) == DXGI_ERROR_NOT_FOUND)
	{
		info("AdapterID = %d not found.", adapterID);
		return AMF_FAIL;
	}

	DXGI_ADAPTER_DESC desc;
	pAdapter->GetDesc(&desc);

	char strDevice[100];
	_snprintf_s(strDevice, 100, "%X", desc.DeviceId);

	info("DX11: Choosen Device %d: Device ID: %s [%s]", adapterID, strDevice, desc.Description);

	ATL::CComPtr<IDXGIOutput> pOutput;
	if (SUCCEEDED(pAdapter->EnumOutputs(0, &pOutput)))
	{
		DXGI_OUTPUT_DESC outputDesc;
		pOutput->GetDesc(&outputDesc);
		ova->m_displayDeviceName = outputDesc.DeviceName;
	}
	ATL::CComPtr<ID3D11Device> pD3D11Device;
	ATL::CComPtr<ID3D11DeviceContext>  pD3D11Context;
	UINT createDeviceFlags = 0;

	//HMONITOR hMonitor = NULL;
	//DWORD vp = 0;

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0
	};
	D3D_FEATURE_LEVEL featureLevel;

	D3D_DRIVER_TYPE eDriverType = pAdapter != NULL ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE;
	hr = D3D11CreateDevice(pAdapter, eDriverType, NULL, createDeviceFlags, featureLevels, _countof(featureLevels),
		D3D11_SDK_VERSION, &pD3D11Device, &featureLevel, &pD3D11Context);
	if (FAILED(hr))
	{
		warn("InitDX11() failed to create HW DX11.1 device ");
	}
	else
	{
		info("InitDX11() created HW DX11 device");
	}

	obs_vce->dx11_device = pD3D11Device;

	return AMF_OK;
}
