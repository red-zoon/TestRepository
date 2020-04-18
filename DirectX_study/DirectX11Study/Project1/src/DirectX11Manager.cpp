#include "DirectX11Manager.h"

#include <iostream>

using namespace std;

HRESULT DirectX11Manager::Init(HINSTANCE hInstance, int cCmdShow)
{
	WNDCLASSEX wcex = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, hInstance, NULL, NULL, NULL, NULL, "DirectX11 Template", NULL };
	if (!RegisterClassEx(&wcex)) {
		return E_FAIL;
	}

	RECT rc = { 0, 0, 1270, 760 };

	hWnd = CreateWindow(wcex.lpszClassName, "DeferredRenderer", WS_DLGFRAME,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top,
		NULL, NULL, hInstance, NULL);
	if (!hWnd) {
		return E_FAIL;
	}

	ShowWindow(hWnd, cCmdShow);
	UpdateWindow(hWnd);

#pragma region HardWare Check
	IDXGIFactory* factory;
	IDXGIAdapter* adapter;
	IDXGIOutput* adapterOutput;
	unsigned int numModes = 0;
	size_t stringLength;
	DXGI_ADAPTER_DESC adapterDesc;

	//グラフィック インタフェース ファクトリを作成
	auto hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)& factory);
	if (FAILED(hr))
	{
		return hr;
	}

	int GPUNumber = 0;
	int GPUMaxMem = 0;
	//一番強いGPUアダプタを検索
	for (int i = 0; i < 100; i++)
	{
		IDXGIAdapter* add;
		hr = factory->EnumAdapters(i, &add);
		if (FAILED(hr))
			break;
		hr = add->GetDesc(&adapterDesc);

		char videoCardDescription[128];
		//ビデオカード名を取得
		int error = wcstombs_s(&stringLength, videoCardDescription, 128, adapterDesc.Description, 128);
		if (error != 0)
		{
			break;
		}
		cout << "ビデオカード名 : " << videoCardDescription << endl;

		//ビデオカードメモリを取得（MB単位）
		int videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);
		cout << "ビデオメモリー : " << videoCardMemory << endl;

		//アウトプット（モニター）に番号IDを付ける
		hr = add->EnumOutputs(0, &adapterOutput);
		if (FAILED(hr))
		{
			continue;
		}

		//DXGI_FORMAT_R8G8B8A8_UNORM の表示形式数を取得する
		hr = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
		if (FAILED(hr))
		{
			continue;
		}
		cout << "RBGA8_UNORM Count : " << numModes << endl;

		if (videoCardMemory > GPUMaxMem)
		{
			GPUMaxMem = videoCardMemory;
			GPUNumber = i;
		}
		add->Release();
		//アダプタアウトプットを解放
		adapterOutput->Release();
		adapterOutput = 0;
	}

	//グラフィック インタフェース アダプターを作成
	hr = factory->EnumAdapters(GPUNumber, &adapter);
	if (FAILED(hr))
	{
		return hr;
	}
#pragma endregion

#pragma region DirectX11Init
	UINT cdev_flags = 0;
#ifdef _DEBUG
	cdev_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	// スワップチェイン設定
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = rc.right;
	sd.BufferDesc.Height = rc.bottom;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 1000;
	sd.BufferDesc.RefreshRate.Denominator = 1;  //1/60 = 60fps
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = true;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
	};

	// DirectX11デバイスとスワップチェイン作成
	hr = D3D11CreateDeviceAndSwapChain(adapter, D3D_DRIVER_TYPE_UNKNOWN, NULL,
		cdev_flags, featureLevels, 1, D3D11_SDK_VERSION, &sd,
		&m_pSwapChain, &m_pDevice, NULL, &m_pImContext);
	if (FAILED(hr)) {
		return hr;
	}

	//アダプタを解放
	adapter->Release();
	adapter = 0;

	//ファクトリを解放
	factory->Release();
	factory = 0;

	// スワップチェインに用意されたバッファ（2Dテクスチャ）を取得
	hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&m_pRTTex));
	if (FAILED(hr)) {
		return hr;
	}

	// レンダーターゲットView作成
	hr = m_pDevice->CreateRenderTargetView(m_pRTTex.Get(), NULL, &m_pRTView);
	if (FAILED(hr)) {
		return hr;
	}

	// viewport
	m_Viewport.Width = static_cast<FLOAT>(rc.right - rc.left);
	m_Viewport.Height = static_cast<FLOAT>(rc.bottom - rc.top);
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.MaxDepth = 1.0f;
	m_Viewport.TopLeftX = 0;
	m_Viewport.TopLeftY = 0;
#pragma endregion

	return hr;
}

ID3D11VertexShader* DirectX11Manager::CreateVertexShader(const string & filename, const string & entrypath, bool erorr)
{
	ID3D11VertexShader* Shader;

#if defined(_DEBUG)
	// グラフィックデバッグツールによるシェーダーのデバッグを有効にする
	UINT    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT    compileFlags = 0;
#endif
	ComPtr<ID3DBlob> blob;
	wchar_t ws[512];

	setlocale(LC_CTYPE, "jpn");
	mbstowcs(ws, filename.c_str(), 512);
	ComPtr<ID3DBlob> pErrorBlob = NULL;
	HRESULT hr = D3DCompileFromFile(ws, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entrypath.c_str(), "vs_5_0", compileFlags, 0, &blob, &pErrorBlob);

	// エラーチェック.
	if (FAILED(hr))
	{
		if (erorr)
		{
			// エラーメッセージを出力.
			if (pErrorBlob != NULL)
			{
				MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "", 0);
				return nullptr;
			}
		}
		else
		{
			string er = (char*)pErrorBlob->GetBufferPointer();
			if (er.find("entrypoint not found") == string::npos)
				MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "", 0);;
			cout << filename << "(" << entrypath << ") is notfound" << endl;
			return nullptr;
		}
	}

	hr = m_pDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), NULL, &Shader);
	assert(SUCCEEDED(hr));

	return Shader;
}

ID3D11PixelShader* DirectX11Manager::CreatePixelShader(const string & filename, const string & entrypath, bool erorr)
{
	ID3D11PixelShader* Shader;

#if defined(_DEBUG)
	// グラフィックデバッグツールによるシェーダーのデバッグを有効にする
	UINT    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT    compileFlags = 0;
#endif
	ComPtr<ID3DBlob> blob;
	wchar_t ws[512];

	setlocale(LC_CTYPE, "jpn");
	mbstowcs(ws, filename.c_str(), 512);
	ComPtr<ID3DBlob> pErrorBlob = NULL;
	HRESULT hr = D3DCompileFromFile(ws, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entrypath.c_str(), "ps_5_0", compileFlags, 0, &blob, &pErrorBlob);
	// エラーチェック.
	if (FAILED(hr))
	{
		if (erorr) {
			// エラーメッセージを出力.
			if (pErrorBlob != NULL && erorr)
			{
				MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "", 0);
				return nullptr;
			}
		}
		else
		{
			string er = (char*)pErrorBlob->GetBufferPointer();
			if (er.find("entrypoint not found") == string::npos)
				MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "", 0);;
			cout << filename << "(" << entrypath << ") is notfound" << endl;
			return nullptr;
		}
	}

	hr = m_pDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), NULL, &Shader);
	assert(SUCCEEDED(hr));

	return Shader;
}

ID3D11InputLayout* DirectX11Manager::CreateInputLayout(D3D11_INPUT_ELEMENT_DESC * layout, UINT elem_num, const string & filename, const string & entrypath)
{
	ID3D11InputLayout* pVertexLayout;

#if defined(_DEBUG)
	// グラフィックデバッグツールによるシェーダーのデバッグを有効にする
	UINT    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT    compileFlags = 0;
#endif
	ComPtr<ID3DBlob> blob;
	wchar_t ws[512];

	setlocale(LC_CTYPE, "jpn");
	mbstowcs(ws, filename.c_str(), 512);
	ComPtr<ID3DBlob> pErrorBlob = NULL;
	HRESULT hr = D3DCompileFromFile(ws, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entrypath.c_str(), "vs_5_0", compileFlags, 0, &blob, &pErrorBlob);
	// エラーチェック.
	if (FAILED(hr))
	{
		// エラーメッセージを出力.
		if (pErrorBlob != NULL)
		{
			MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "", 0);
		}
	}
	hr = m_pDevice->CreateInputLayout(layout, elem_num, blob->GetBufferPointer(),
		blob->GetBufferSize(), &pVertexLayout);
	assert(SUCCEEDED(hr));

	return pVertexLayout;
}

void DirectX11Manager::SetInputLayout(ID3D11InputLayout* VertexLayout)
{
	m_pImContext->IASetInputLayout(VertexLayout);
}
void DirectX11Manager::SetVertexShader(ID3D11VertexShader* vs)
{
	m_pImContext->VSSetShader(vs, nullptr, 0);
}
void DirectX11Manager::SetPixelShader(ID3D11PixelShader* ps)
{
	m_pImContext->PSSetShader(ps, nullptr, 0);
}
void DirectX11Manager::SetVertexBuffer(ID3D11Buffer* VertexBuffer, UINT VertexSize)
{
	UINT hOffsets = 0;
	m_pImContext->IASetVertexBuffers(0, 1, &VertexBuffer, &VertexSize, &hOffsets);
}
void DirectX11Manager::SetIndexBuffer(ID3D11Buffer* IndexBuffer)
{
	m_pImContext->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
}

//void DirectX11Manager::SetTexture2D(UINT RegisterNo, ID3D11ShaderResourceView* Texture)
//{
//	m_pImContext->PSSetShaderResources(RegisterNo, 1, &Texture);
//}

void DirectX11Manager::DrawBegin()
{
	//ポリゴンの生成方法の指定
	m_pImContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 指定色で画面クリア
	float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; //red,green,blue,alpha
	m_pImContext->ClearRenderTargetView(m_pRTView.Get(), ClearColor);
	m_pImContext->RSSetViewports(1, &m_Viewport);

	//RenderTargetをバックバッファ
	ID3D11RenderTargetView* rtv[1] = { m_pRTView.Get() };
	m_pImContext->OMSetRenderTargets(1, rtv, nullptr);
}

void DirectX11Manager::DrawEnd()
{
	m_pSwapChain->Present(0, 0);
}

void DirectX11Manager::DrawIndexed(UINT VertexNum)
{
	m_pImContext->DrawIndexed(VertexNum, 0, 0);
}

