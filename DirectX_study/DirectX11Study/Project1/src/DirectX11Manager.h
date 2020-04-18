//WindowsApplication���g�p����̂ŕK�{
#include <windows.h>
//DirectX11��API�������Ă���
#include <d3d11.h>
//�V�F�[�_�[�̃R���p�C��
#include <d3dcompiler.h>
//�x�N�g����FLOAT3�ȂǕ֗��ȎZ�p���C�u����
#include <DirectXMath.h>
//�f�o�C�X�̊Ǘ������₷�����邽�߂�DirectX�ł̃X�}�[�g�|�C���^�̂悤�Ȃ���
#include <wrl/client.h>

#include <string>

//DirectXAPI�֌W�̃����N
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

//DirectXMath��DirectX�̃l�[���X�y�[�X�ɂ���
using namespace DirectX;
//ComPtr(DirectX�ł̃X�}�[�g�|�C���^)
using Microsoft::WRL::ComPtr;

using namespace std;

//�����₷���悤��typedef�F�X
typedef ComPtr<ID3D11Buffer> ConstantBuffer, VertexBuffer, IndexBuffer, StructuredBuffer;
typedef ComPtr<ID3D11InputLayout> InputLayout;
typedef ComPtr<ID3D11VertexShader> VertexShader;
typedef ComPtr<ID3D11GeometryShader> GeometryShader;
typedef ComPtr<ID3D11PixelShader> PixelShader;
typedef ComPtr<ID3D11ComputeShader> ComputeShader;
typedef ComPtr<ID3D11Texture2D> Texture2D;
typedef ComPtr<ID3D11ShaderResourceView> ShaderTexture;
typedef ComPtr<ID3D11UnorderedAccessView> ComputeOutputView;

//�Ǘ��N���X
class DirectX11Manager
{
	//Windows�̃n���h��
	HWND hWnd = NULL;
public:
	//DX11�̃f�o�C�X
	ComPtr<ID3D11Device>            m_pDevice = nullptr;
	//DX11�̕`�施�ߓ��𑗂邽�߂̂���
	ComPtr<ID3D11DeviceContext>     m_pImContext = nullptr;
	//�n�[�h�E�F�A�̏�񂪋l�܂��Ă������
	ComPtr<IDXGISwapChain>          m_pSwapChain = nullptr;
	//�f�B�X�v���C�̃o�b�O�o�b�t�@�̃e�N�X�`��
	Texture2D                       m_pRTTex = nullptr;
	//�f�B�X�v���C�̃o�b�O�o�b�t�@�̃e�N�X�`����`���Ƃ��Ďw��ł���悤�ɂ�������
	ComPtr<ID3D11RenderTargetView>  m_pRTView = nullptr;
	//�E�B���h�E�̃T�C�Y�̎w��
	D3D11_VIEWPORT                  m_Viewport = { 0,0,0,0,0,0 };

	//�������֐�
	HRESULT Init(HINSTANCE hInstance, int cCmdShow);    

	ID3D11VertexShader* CreateVertexShader(const string& filename, const string& entrypath = "", bool erorr = true);
	ID3D11PixelShader* CreatePixelShader(const string& filename, const string& entrypath = "", bool erorr = true);

	//inputlayout�쐬
	ID3D11InputLayout* CreateInputLayout(D3D11_INPUT_ELEMENT_DESC* layout, UINT elem_num, const string& filename, const string& entrypath = "");

	template<class x>
	ID3D11Buffer* CreateVertexBuffer(x* VertexData, UINT VertexNum)
	{
		//���_�o�b�t�@�쐬
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
		//�C���f�b�N�X�o�b�t�@�쐬
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