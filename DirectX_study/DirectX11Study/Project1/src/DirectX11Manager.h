//WindowsApplicationを使用するので必須
#include <windows.h>
//DirectX11のAPIが入っている
#include <d3d11.h>
//シェーダーのコンパイラ
#include <d3dcompiler.h>
//ベクトルやFLOAT3など便利な算術ライブラリ
#include <DirectXMath.h>
//デバイスの管理をしやすくするためのDirectX版のスマートポインタのようなもの
#include <wrl/client.h>

#include <string>

//DirectXAPI関係のリンク
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

//DirectXMathがDirectXのネームスペースにある
using namespace DirectX;
//ComPtr(DirectX版のスマートポインタ)
using Microsoft::WRL::ComPtr;

using namespace std;

//書きやすいようにtypedef色々
typedef ComPtr<ID3D11Buffer> ConstantBuffer, VertexBuffer, IndexBuffer, StructuredBuffer;
typedef ComPtr<ID3D11InputLayout> InputLayout;
typedef ComPtr<ID3D11VertexShader> VertexShader;
typedef ComPtr<ID3D11GeometryShader> GeometryShader;
typedef ComPtr<ID3D11PixelShader> PixelShader;
typedef ComPtr<ID3D11ComputeShader> ComputeShader;
typedef ComPtr<ID3D11Texture2D> Texture2D;
typedef ComPtr<ID3D11ShaderResourceView> ShaderTexture;
typedef ComPtr<ID3D11UnorderedAccessView> ComputeOutputView;

//管理クラス
class DirectX11Manager
{
	//Windowsのハンドル
	HWND hWnd = NULL;
public:
	//DX11のデバイス
	ComPtr<ID3D11Device>            m_pDevice = nullptr;
	//DX11の描画命令等を送るためのもの
	ComPtr<ID3D11DeviceContext>     m_pImContext = nullptr;
	//ハードウェアの情報が詰まっているもの
	ComPtr<IDXGISwapChain>          m_pSwapChain = nullptr;
	//ディスプレイのバッグバッファのテクスチャ
	Texture2D                       m_pRTTex = nullptr;
	//ディスプレイのバッグバッファのテクスチャを描画先として指定できるようにしたもの
	ComPtr<ID3D11RenderTargetView>  m_pRTView = nullptr;
	//ウィンドウのサイズの指定
	D3D11_VIEWPORT                  m_Viewport = { 0,0,0,0,0,0 };

	//初期化関数
	HRESULT Init(HINSTANCE hInstance, int cCmdShow);    

	ID3D11VertexShader* CreateVertexShader(const string& filename, const string& entrypath = "", bool erorr = true);
	ID3D11PixelShader* CreatePixelShader(const string& filename, const string& entrypath = "", bool erorr = true);

	//inputlayout作成
	ID3D11InputLayout* CreateInputLayout(D3D11_INPUT_ELEMENT_DESC* layout, UINT elem_num, const string& filename, const string& entrypath = "");

	template<class x>
	ID3D11Buffer* CreateVertexBuffer(x* VertexData, UINT VertexNum)
	{
		//頂点バッファ作成
		D3D11_BUFFER_DESC hBufferDesc;
		ZeroMemory(&hBufferDesc, sizeof(hBufferDesc));
		hBufferDesc.ByteWidth = sizeof(x) * VertexNum;
		hBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		hBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		hBufferDesc.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA hSubResourceData;
		ZeroMemory(&hSubResourceData, sizeof(hSubResourceData));
		hSubResourceData.pSysMem = VertexData;

		ID3D11Buffer* hpBuffer;
		if (FAILED(m_pDevice->CreateBuffer(&hBufferDesc, &hSubResourceData, &hpBuffer))) {
			return nullptr;
		}
		return hpBuffer;
	}
	ID3D11Buffer* CreateIndexBuffer(UINT* Index, UINT IndexNum)
	{
		//インデックスバッファ作成
		D3D11_BUFFER_DESC hBufferDesc;
		ZeroMemory(&hBufferDesc, sizeof(hBufferDesc));
		hBufferDesc.ByteWidth = sizeof(UINT) * IndexNum;
		hBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		hBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		hBufferDesc.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA hSubResourceData;
		ZeroMemory(&hSubResourceData, sizeof(hSubResourceData));
		hSubResourceData.pSysMem = Index;

		ID3D11Buffer* hpBuffer;
		if (FAILED(m_pDevice->CreateBuffer(&hBufferDesc, &hSubResourceData, &hpBuffer))) {
			return nullptr;
		}
		return hpBuffer;
	}

	//PipelineSetting
	void SetInputLayout(ID3D11InputLayout* VertexLayout);
	void SetVertexShader(ID3D11VertexShader* vs);
	void SetPixelShader(ID3D11PixelShader* ps);

	void SetVertexBuffer(ID3D11Buffer* VertexBuffer, UINT VertexSize);
	void SetIndexBuffer(ID3D11Buffer* IndexBuffer);

	void DrawBegin();
	void DrawEnd();
	void DrawIndexed(UINT VertexNum);
};