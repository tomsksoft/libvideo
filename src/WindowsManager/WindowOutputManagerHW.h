#ifndef LIBVIDEO_WIN_OUT_MANAGER_HW_H
#define LIBVIDEO_WIN_OUT_MANAGER_HW_H

#include <Windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>

#include <vector>

namespace tomsksoft::libvideo {

struct WinHWData {
	void *texture;
	int texture_index;
};

struct HWWindowDescription {
	HWND hwnd = nullptr;

	IDXGISwapChain1 *swapchain = nullptr;
	ID3D11Device *device = nullptr;
	ID3D11DeviceContext *device_ctx = nullptr;
	ID3D11RenderTargetView *render_target_view = nullptr;
	ID3D11SamplerState *sampler_linear = nullptr;
	IDXGIFactory2 *factory = nullptr;

	ID3D11VertexShader *vertex_shader = nullptr;
	ID3D11PixelShader *pixel_shader = nullptr;

	ID3D11InputLayout *input_layout = nullptr;
	ID3D11Buffer *vertex_buf = nullptr;
	ID3D11Buffer *index_buf = nullptr;

	ID3D11Texture2D *texture = nullptr;

	ID3D11ShaderResourceView *luminance_shader_resource_view = nullptr;
	ID3D11ShaderResourceView *chrominance_shader_resource_view = nullptr;

	DWORD OcclusionCookie = 0;
};

class WindowOutputManagerHW {
  public:
	WindowOutputManagerHW();
	~WindowOutputManagerHW();

	void initialize_window(HWND hwnd);
	void draw(WinHWData &frameData);

  private:
	std::vector<HWWindowDescription> m_wnd_list;

	UINT m_NumDriverTypes;
	UINT m_NumFeatureLevels;
	UINT m_creationFlags;

	D3D11_BUFFER_DESC m_vertexBufferDesc;
	D3D11_SUBRESOURCE_DATA m_vertexData;

	D3D11_BUFFER_DESC m_indexBufferDesc;
	D3D11_SUBRESOURCE_DATA m_indexData;

	HRESULT m_hr;
};

} // namespace tomsksoft::libvideo

#endif // LIBVIDEO_WIN_OUT_MANAGER_HW_H