#pragma once
#include "Client_Defines.h"
#include "GameObj.h"


BEGIN(Engine)

class CShader;
class CTexture;
class CRenderer;
class CTransform;
class CVIBuffer_Rect;
class CModel;
class CCollider;
class CNavigation;
END


BEGIN(Client)

class CMetapod : public CGameObj
{
private:
	CMetapod(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMetapod(const CMetapod& rhs);
	virtual ~CMetapod() = default;


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
	void	Set_DeckPos();
	void	Set_EvolPos(_float fTimeDelta);
	void	Set_EvolPos2(_float fTimeDelta); //진화
	void	Key_Input(_float fTimeDelta);
	void	Battle(_float fTimeDelta);
	void	Set_Stats();
	void	Reset_Battle();
	void	Check_Coll();
	void	LvUp();

	//야생
	void	OnNavi();
	void	Ready_WildBattle();
	void	WildBattle();
	void	Ready_Script();
	void	Move(_float fTimeDelta);

	void	Ready_EvolScript(); //진화
	void	Ready_ClearEvolScript(); //진화

	void	Race(_float fTimeDelta);
private:
	CModel*					m_pModelCom = nullptr;
	CCollider*				m_pAABBCom = nullptr;
	CNavigation*			m_pNavigationCom = nullptr;
	CShader*				m_pShaderUICom = nullptr;

	_int					m_iAnim = 0;
	_bool					m_bSetPos = false;
	_bool					m_bBrath = false;
	_bool					m_bBattle = false;
	_float					m_fStartBattle = 0.f;
	_bool					m_bAttack = false;
	_bool					m_bHit = false;
	_bool					m_bDown = false;
	_bool					m_bStopAnim = false;

	//야생
	_float					m_fDist = 0.f;
	_bool					m_bReadyWild = false;
	vector<CGameObject*>    m_vecPoke;
	vector<wstring>		    m_vNormalScript;
	_bool					m_bBattleStart = false;
	_bool					m_bCollCheck = false;
	_bool					m_bFindPlayer = false;

	_float4					m_vOriginPos;
	_int					m_iMoveIndex = 0;
	_float					m_fMoveTime = 0.f;

	_float					m_fX, m_fY, m_fSizeX, m_fSizeY;
	_float4x4				m_ViewMatrix, m_ProjMatrix;

	_float					m_EvolTime = 0.f; //진화
	_float					m_fRenderTime = 1.f; // 진화
	_bool					m_bRender = false; // 진화
	_bool					m_bEvol = false; //진화
	_bool					m_bClearEvol = false; // 진화
	CGameObject*			m_EvolPoke = nullptr; //진화

	_float					m_fSkillTime = 0.f;
public:
	void	Set_Render(_bool _bRender) { m_bRender = _bRender; }
	void	Set_CheckEvol() { m_bClearEvol = true; }
public:
	static CMetapod* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr);
	virtual void Free() override;
};

END
