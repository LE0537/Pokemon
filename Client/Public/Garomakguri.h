#pragma once
#include "Client_Defines.h"
#include "GameObj.h"


BEGIN(Engine)

class CShader;
class CTexture;
class CRenderer;
class CTransform;
class CVIBuffer_Rect;

END


BEGIN(Client)

class CGaromakguri : public CGameObj
{
private:
	CGaromakguri(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGaromakguri(const CGaromakguri& rhs);
	virtual ~CGaromakguri() = default;


public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(_float fTimeDelta);
	virtual void Late_Tick(_float fTimeDelta);
	virtual HRESULT Render();

private:



public:
	static CGaromakguri* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr);
	virtual void Free() override;
};

END
