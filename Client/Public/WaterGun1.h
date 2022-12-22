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

class CWaterGun1 : public CGameObj
{
private:
	CWaterGun1(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CWaterGun1(const CWaterGun1& rhs);
	virtual ~CWaterGun1() = default;


public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(_float fTimeDelta);
	virtual void Late_Tick(_float fTimeDelta);
	virtual HRESULT Render();
	virtual HRESULT Render_ShadowDepth();
private:
	HRESULT SetUp_ShaderResources();
	HRESULT Ready_Components();
	void	OnBillboard();
	void	Set_Pos(_float fTimeDelta);
private:
	_float					m_fSpeed = 0.f;
	_float4					m_vPos1,m_vPos2, m_vPos3;

	_float					m_fSize = 0.f;

	_float					m_fDeadTime = 0.f;

	_float					m_fFrameTime = 0.f;
	_int					m_iFrame = 0;

	_bool					m_bSkill = false;
public:
	static CWaterGun1* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr);
	virtual void Free() override;
};

END
