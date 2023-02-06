#include <array>

#include <d3dcompiler.h>

#include "PixelShader.h"
#include "VertexShader.h"

#include "Logs.h"
#include "WindowOutputManagerHW.h"

#define OCCLUSION_STATUS_MSG WM_USER

namespace tomsksoft::libvideo {

D3D_DRIVER_TYPE DriverTypes[] = {
	D3D_DRIVER_TYPE_HARDWARE,
	D3D_DRIVER_TYPE_WARP,
	D3D_DRIVER_TYPE_REFERENCE,
};

D3D_FEATURE_LEVEL FeatureLevels[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0,
									 D3D_FEATURE_LEVEL_9_1};

struct VERTEX {
	FLOAT POS[3];
	FLOAT TexCoord[2];
};
// Be careful texture coords here flipped for normal displaying on screen
VERTEX Vertices[] = {
	{{-1.f, -1.f, 0.0f}, {0.f, 1.f}}, // left bottom
	{{-1.f, 1.f, 0.0f}, {0.f, 0.f}},  // left top
	{{1.f, -1.f, 0.0f}, {1.f, 1.f}},  // right bottom
	{{1.f, 1.f, 0.0f}, {1.f, 0.f}}	  // rigth top
};

WindowOutputManagerHW::WindowOutputManagerHW() {
	m_NumDriverTypes = ARRAYSIZE(DriverTypes);
	m_NumFeatureLevels = ARRAYSIZE(FeatureLevels);

	m_creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

	m_vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	m_vertexBufferDesc.ByteWidth = sizeof(VERTEX) * 4;
	m_vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_vertexBufferDesc.CPUAccessFlags = 0;
	m_vertexBufferDesc.MiscFlags = 0;
	m_vertexBufferDesc.StructureByteStride = 0;

	m_indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	m_indexBufferDesc.ByteWidth = sizeof(unsigned long) * 6;
	m_indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	m_indexBufferDesc.CPUAccessFlags = 0;
	m_indexBufferDesc.MiscFlags = 0;
	m_indexBufferDesc.StructureByteStride = 0;

	const unsigned long indices[] = {0, 1, 2, 2, 1, 3};
	m_indexData.pSysMem = indices;
	m_indexData.SysMemPitch = 0;
	m_indexData.SysMemSlicePitch = 0;

	m_vertexData = {};
	m_hr = S_OK;
}

WindowOutputManagerHW::~WindowOutputManagerHW() {
	for (const auto &wnd : m_wnd_list) {
		wnd.swapchain->Release();
		wnd.device->Release();
		wnd.device_ctx->Release();
		wnd.render_target_view->Release();
		wnd.sampler_linear->Release();
		wnd.factory->Release();
		wnd.vertex_shader->Release();
		wnd.pixel_shader->Release();
		wnd.input_layout->Release();
		wnd.vertex_buf->Release();
		wnd.index_buf->Release();
		wnd.texture->Release();
		wnd.luminance_shader_resource_view->Release();
		wnd.chrominance_shader_resource_view->Release();
	}
}

void WindowOutputManagerHW::initialize_window(HWND hwnd) {
	HWWindowDescription desc;
	desc.hwnd = hwnd;

	D3D_FEATURE_LEVEL FeatureLevel;
	// Create device
	for (UINT DriverTypeIndex = 0; DriverTypeIndex < m_NumDriverTypes; ++DriverTypeIndex) {
		m_hr = D3D11CreateDevice(nullptr, DriverTypes[DriverTypeIndex], nullptr, m_creationFlags, FeatureLevels,
								 m_NumFeatureLevels, D3D11_SDK_VERSION, &desc.device, &FeatureLevel, &desc.device_ctx);
		if (SUCCEEDED(m_hr)) {
			break;
		}
	}
	if (FAILED(m_hr)) {
		LibvideoLog() << "Error to create device\n";
		return;
	}

	IDXGIDevice *DxgiDevice = nullptr;
	m_hr = desc.device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void **>(&DxgiDevice));
	if (FAILED(m_hr)) {
		LibvideoLog() << "Failed to QI for DXGI Device\n";
		return;
	}

	IDXGIAdapter *DxgiAdapter = nullptr;
	m_hr = DxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void **>(&DxgiAdapter));
	DxgiDevice->Release();
	DxgiDevice = nullptr;
	if (FAILED(m_hr)) {
		LibvideoLog() << "Failed to get parent DXGI Adapter\n";
		return;
	}

	m_hr = DxgiAdapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void **>(&desc.factory));
	DxgiAdapter->Release();
	DxgiAdapter = nullptr;
	if (FAILED(m_hr)) {
		LibvideoLog() << "Failed to get parent DXGI Adapter\n";
		return;
	}

	m_hr = desc.factory->RegisterOcclusionStatusWindow(desc.hwnd, OCCLUSION_STATUS_MSG, &desc.OcclusionCookie);
	if (FAILED(m_hr)) {
		LibvideoLog() << "Failed to register for occlusion message\n";
		return;
	}

	RECT rect;
	GetClientRect(hwnd, &rect);
	const int cur_width = rect.right - rect.left;
	const int cur_height = rect.bottom - rect.top;

	DXGI_SWAP_CHAIN_DESC1 SwapChainDesc;
	RtlZeroMemory(&SwapChainDesc, sizeof(SwapChainDesc));
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	SwapChainDesc.BufferCount = 2;
	SwapChainDesc.Width = cur_width;
	SwapChainDesc.Height = cur_height;
	SwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SampleDesc.Quality = 0;

	m_hr = desc.factory->CreateSwapChainForHwnd(desc.device, desc.hwnd, &SwapChainDesc, nullptr, nullptr, &desc.swapchain);
	if (FAILED(m_hr)) {
		LibvideoLog() << "Failed to create window swapchain\n";
		return;
	}

	m_hr = desc.factory->MakeWindowAssociation(desc.hwnd, DXGI_MWA_NO_ALT_ENTER);
	if (FAILED(m_hr)) {
		LibvideoLog() << "Failed to make window association\n";
		return;
	}

	D3D11_TEXTURE2D_DESC const texDesc =
		CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_NV12,			  // Decoded texture format
							  cur_width,				  // Width of the video frames
							  cur_height,				  // Height of the video frames
							  1,						  // Number of textures in the array
							  1,						  // Number of miplevels in each texture
							  D3D11_BIND_SHADER_RESOURCE, // We read from this texture in the shader
							  D3D11_USAGE_DYNAMIC,		  // Because we'll be copying from CPU memory
							  D3D11_CPU_ACCESS_WRITE	  // We only need to write into the texture
		);

	m_hr = desc.device->CreateTexture2D(&texDesc, nullptr, &desc.texture);
	if (FAILED(m_hr)) {
		LibvideoLog() << "Failed to create texture\n";
		return;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC const luminancePlaneDesc =
		CD3D11_SHADER_RESOURCE_VIEW_DESC(desc.texture, D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8_UNORM);

	m_hr = desc.device->CreateShaderResourceView(desc.texture, &luminancePlaneDesc, &desc.luminance_shader_resource_view);
	if (FAILED(m_hr)) {
		LibvideoLog() << "Failed to create shader luminance resource view\n";
		return;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC const chrominancePlaneDesc =
		CD3D11_SHADER_RESOURCE_VIEW_DESC(desc.texture, D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8_UNORM);

	m_hr = desc.device->CreateShaderResourceView(desc.texture, &chrominancePlaneDesc, &desc.chrominance_shader_resource_view);
	if (FAILED(m_hr)) {
		LibvideoLog() << "Failed to create shader chrominance resource view\n";
		return;
	}

	ID3D11Texture2D *BackBuffer = nullptr;
	m_hr = desc.swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&BackBuffer));
	if (FAILED(m_hr)) {
		LibvideoLog() << "Failed to get backbuffer for making render target view\n";
		return;
	}

	m_hr = desc.device->CreateRenderTargetView(BackBuffer, nullptr, &desc.render_target_view);
	BackBuffer->Release();
	if (FAILED(m_hr)) {
		LibvideoLog() << "Failed to get backbuffer for making render target view\n";
		return;
	}

	desc.device_ctx->OMSetRenderTargets(1, &desc.render_target_view, nullptr);

	D3D11_VIEWPORT VP;
	VP.Width = static_cast<FLOAT>(cur_width);
	VP.Height = static_cast<FLOAT>(cur_height);
	VP.MinDepth = 0.0f;
	VP.MaxDepth = 1.0f;
	VP.TopLeftX = 0;
	VP.TopLeftY = 0;
	desc.device_ctx->RSSetViewports(1, &VP);

	D3D11_SAMPLER_DESC sampler_desc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());

	m_hr = desc.device->CreateSamplerState(&sampler_desc, &desc.sampler_linear);
	if (FAILED(m_hr)) {
		LibvideoLog() << "Failed to create sampler state\n";
		return;
	}

	m_hr = desc.device->CreateVertexShader(g_vs_main, sizeof(g_vs_main), nullptr, &desc.vertex_shader);
	if (FAILED(m_hr)) {
		LibvideoLog() << "Failed to create vertex shader\n";
		return;
	}

	D3D11_INPUT_ELEMENT_DESC ied[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	m_hr = desc.device->CreateInputLayout(ied, 2, g_vs_main, sizeof(g_vs_main), &desc.input_layout);
	if (FAILED(m_hr)) {
		LibvideoLog() << "Failed to create input layout\n";
		return;
	}
	desc.device_ctx->IASetInputLayout(desc.input_layout);

	m_hr = desc.device->CreatePixelShader(g_ps_main, sizeof(g_ps_main), nullptr, &desc.pixel_shader);
	if (FAILED(m_hr)) {
		LibvideoLog() << "Failed to create pixel shader\n";
		return;
	}

	m_vertexData.pSysMem = Vertices;
	m_vertexData.SysMemPitch = 0;
	m_vertexData.SysMemSlicePitch = 0;

	desc.device->CreateBuffer(&m_vertexBufferDesc, &m_vertexData, &desc.vertex_buf);
	desc.device->CreateBuffer(&m_indexBufferDesc, &m_indexData, &desc.index_buf);

	desc.device_ctx->PSSetShaderResources(0, 1, &desc.luminance_shader_resource_view);
	desc.device_ctx->PSSetShaderResources(1, 1, &desc.chrominance_shader_resource_view);
	desc.device_ctx->OMSetRenderTargets(1, &desc.render_target_view, nullptr);
	desc.device_ctx->VSSetShader(desc.vertex_shader, nullptr, 0);
	desc.device_ctx->PSSetShader(desc.pixel_shader, nullptr, 0);
	desc.device_ctx->PSSetSamplers(0, 1, &desc.sampler_linear);

	const UINT stride = sizeof(VERTEX);
	const UINT offset = 0;

	desc.device_ctx->IASetIndexBuffer(desc.index_buf, DXGI_FORMAT_R32_UINT, offset);
	desc.device_ctx->IASetVertexBuffers(0, 1, &desc.vertex_buf, &stride, &offset);

	desc.device_ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	m_wnd_list.emplace_back(desc);
}

void WindowOutputManagerHW::draw(WinHWData &frameData) {
	for (auto const &wnd : m_wnd_list) {
		wnd.device_ctx->CopySubresourceRegion(wnd.texture, 0, 0, 0, 0, reinterpret_cast<ID3D11Texture2D *>(frameData.texture),
											  frameData.texture_index, nullptr);

		wnd.device_ctx->OMSetRenderTargets(1, &wnd.render_target_view, nullptr);

		wnd.device_ctx->Draw(6, 0);

		wnd.swapchain->Present(1, 0);
	}
}

} // namespace tomsksoft::libvideo
