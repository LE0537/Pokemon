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

class CPsychic : public CGameObj
{
private:
	CPsychic(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPsychic(const CPsychic& rhs);
	virtual ~CPsychic() = default;


public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(_float fTimeDelta);
	virtual void Late_Tick(_float fTimeDelta);
	virtual HRESULT Render();
	virtual HRESULT Render_ShadowDepth();
private:

	_float		m_fSkillTime = 0.f;
	_float		m_fDeadTime = 0.f;
	_bool		m_bSkill = false;
	_bool		m_bHitSkill = false;
private:

	void	Set_Pos(_float fTimeDelta);
public:
	static CPsychic* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr);
	virtual void Free() override;
};

END
