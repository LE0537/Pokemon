#include "stdafx.h"
#include "..\Public\Slowbro.h"
#include "GameInstance.h"
#include "Lv_Up.h"
#include "Data_Manager.h"	// 추가
#include "SoundMgr.h"
#include "TextBox.h"
#include "Camera_Dynamic.h"
#include "Player.h"
#include "VIBuffer_Navigation.h"

CSlowbro::CSlowbro(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObj(pDevice, pContext)
{
}

CSlowbro::CSlowbro(const CSlowbro & rhs)
	: CGameObj(rhs)
{
}

HRESULT CSlowbro::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSlowbro::Initialize(void * pArg)
{

	*(CGameObject**)pArg = this;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	Set_Stats();

	
	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);

	if (FAILED(pGameInstance->Add_GameObject(TEXT("Prototype_GameObject_Tackle"), LEVEL_STATIC, TEXT("Layer_Skill"), &m_PokemonInfo.eSkillNum1)))
		return E_FAIL;
	if (FAILED(pGameInstance->Add_GameObject(TEXT("Prototype_GameObject_NoneSkill"), LEVEL_STATIC, TEXT("Layer_Skill"), &m_PokemonInfo.eSkillNum2)))
		return E_FAIL;
	if (FAILED(pGameInstance->Add_GameObject(TEXT("Prototype_GameObject_NoneSkill"), LEVEL_STATIC, TEXT("Layer_Skill"), &m_PokemonInfo.eSkillNum3)))
		return E_FAIL;
	if (FAILED(pGameInstance->Add_GameObject(TEXT("Prototype_GameObject_NoneSkill"), LEVEL_STATIC, TEXT("Layer_Skill"), &m_PokemonInfo.eSkillNum4)))
		return E_FAIL;
	if (FAILED(pGameInstance->Add_GameObject(TEXT("Prototype_GameObject_None"), LEVEL_STATIC, TEXT("Layer_Item"), &m_PokemonInfo.eItem)))
		return E_FAIL;


	RELEASE_INSTANCE(CGameInstance);

	m_PokemonInfo.eType = WATER;
	m_PokemonInfo.eType2 = ESPER;
	m_PokemonInfo.eStatInfo = STATINFO_END;
	m_PokemonInfo.bLvUp = false;
	m_bWildPoke = false;
	m_pModelCom->Set_CurrentAnimIndex(2);
	m_pTransformCom->Set_Scale(XMVectorSet(0.05f, 0.05f, 0.05f, 0.f));
	m_pTransformCom->Set_State(CTransform::STATE_TRANSLATION, XMVectorSet(-50000.f, -50000.f, -50000.f, 1.f));
	return S_OK;
}

void CSlowbro::Tick(_float fTimeDelta)
{
	
	if (m_bOnOff)
	{
		Set_DeckPos();
		if (!m_bDeckInfo)
		{
			Key_Input(fTimeDelta);
		}
	}
	if (g_PokeInfo || g_bPokeDeck)
		m_pModelCom->Play_Animation(fTimeDelta);
	if (!m_bOnOff)
		m_bSetPos = false;

	if (m_bBattleMap)
	{
		Battle(fTimeDelta);
		if (m_PokemonInfo.bLvUp)
			LvUp();
	}
	if (m_bAnimReset)
		Reset_Battle();
	if (m_bWildPoke)
	{
		if (!m_bReadyWild)
		{
			Ready_WildBattle();
			m_bReadyWild = true;
		}
		if (g_Battle)
		{
			if (!m_bBattleStart && m_bCollCheck)
				WildBattle();
		}


		if (m_bWildPoke && !g_Battle && !dynamic_cast<CGameObj*>(m_pTarget)->Get_Event())
		{
			//	OnNavi();
			Move(fTimeDelta);
			m_pModelCom->Play_Animation(fTimeDelta);
			m_pAABBCom->Update(m_pTransformCom->Get_WorldMatrix());
		}
	}
}

void CSlowbro::Late_Tick(_float fTimeDelta)
{
	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);

	if (!g_Battle && m_bWildPoke)
		Check_Coll();

	if (pGameInstance->IsInFrustum(m_pTransformCom->Get_State(CTransform::STATE_TRANSLATION), 10.f))
	{
		if (m_bWildPoke && !m_bBattleMap && !g_Battle && nullptr != m_pRendererCom)
			m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONALPHABLEND, this);
	}
	if ((g_PokeInfo || g_bPokeDeck) && m_bOnOff && nullptr != m_pRendererCom)
		m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_UIPOKE, this);
	else if (m_bBattleMap && g_Battle && nullptr != m_pRendererCom)
		m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONALPHABLEND, this);
	RELEASE_INSTANCE(CGameInstance);
}

HRESULT CSlowbro::Render()
{

	if (nullptr == m_pShaderCom ||
		nullptr == m_pModelCom)
		return E_FAIL;

	if (FAILED(SetUp_ShaderResources()))
		return E_FAIL;

	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);

	_uint		iNumMeshes = m_pModelCom->Get_NumMeshContainers();

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(m_pModelCom->SetUp_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(m_pShaderCom, i, 0)))
			return E_FAIL;

	}

	RELEASE_INSTANCE(CGameInstance);

	if (g_CollBox)
		m_pAABBCom->Render();
	return S_OK;
}
HRESULT CSlowbro::Ready_Components()
{
	/* For.Com_Renderer */
	if (FAILED(__super::Add_Components(TEXT("Com_Renderer"), LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* For.Com_Transform */
	if (FAILED(__super::Add_Components(TEXT("Com_Transform"), LEVEL_STATIC, TEXT("Prototype_Component_Transform"), (CComponent**)&m_pTransformCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Components(TEXT("Com_Shader"), LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Model*/
	if (FAILED(__super::Add_Components(TEXT("Com_Model"), LEVEL_STATIC, TEXT("Slowbro"), (CComponent**)&m_pModelCom)))
		return E_FAIL;
	CCollider::COLLIDERDESC		ColliderDesc;

	/* For.Com_AABB */
	ZeroMemory(&ColliderDesc, sizeof(CCollider::COLLIDERDESC));

	ColliderDesc.vScale = _float3(20.f, 40.f, 20.f);
	ColliderDesc.vPosition = _float3(0.f, 20.f, 0.f);
	if (FAILED(__super::Add_Components(TEXT("Com_AABB"), LEVEL_STATIC, TEXT("Prototype_Component_Collider_AABB"), (CComponent**)&m_pAABBCom, &ColliderDesc)))
		return E_FAIL;

	CNavigation::NAVIDESC			NaviDesc;
	ZeroMemory(&NaviDesc, sizeof NaviDesc);

	NaviDesc.iCurrentCellIndex = 0;

	if (FAILED(__super::Add_Components(TEXT("Com_Navigation"), LEVEL_STATIC, TEXT("Prototype_Component_Navigation"), (CComponent**)&m_pNavigationCom, &NaviDesc)))
		return E_FAIL;
	return S_OK;
}
void CSlowbro::Reset_Battle()
{
	m_iAnim = 0;
	m_bSetPos = false;
	m_bBrath = false;
	m_bBattle = false;
	m_fStartBattle = 0.f;
	m_bAttack = false;
	m_bHit = false;
	m_bDown = false;
	m_bStopAnim = false;
	m_bAnimReset = false;
	m_bBattleMap = false;
}
void CSlowbro::Check_Coll()
{
	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);

	CCollider*	pTargetCollider = (CCollider*)pGameInstance->Get_Component(LEVEL_GAMEPLAY, TEXT("Layer_Player"), TEXT("Com_AABB"));

	if (nullptr == pTargetCollider)
		return;


	if (m_pAABBCom->Collision(pTargetCollider))
	{
		if (!m_bCollCheck)
		{
			dynamic_cast<CPlayer*>(m_pTarget)->Set_TargetPoke(&m_vecPoke);
			dynamic_cast<CPlayer*>(m_pTarget)->Set_BattleTarget(this, BATTLE_WILD);
			if (!dynamic_cast<CGameObj*>(m_pTarget)->Get_Event())
				dynamic_cast<CGameObj*>(m_pTarget)->OnOffEvent();
			dynamic_cast<CCamera_Dynamic*>(m_pCamera)->Set_Target(this);

			if (FAILED(pGameInstance->Add_GameObject(TEXT("Prototype_GameObject_BattleIntro"), LEVEL_GAMEPLAY, TEXT("Layer_Effect"))))
				return;
			CSoundMgr::Get_Instance()->BGM_Stop();
			CSoundMgr::Get_Instance()->PlayEffect(TEXT("Battle1.wav"), 0.75f);
			m_bCollCheck = true;
		}
	}

	RELEASE_INSTANCE(CGameInstance);
}
void CSlowbro::Ready_WildBattle()
{
	m_vecPoke.reserve(6);;
	m_vecPoke.push_back(this);
	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);
	CGameObject* tInfo;
	if (FAILED(pGameInstance->Add_GameObject(TEXT("Prototype_GameObject_NonePoke"), LEVEL_STATIC, TEXT("Layer_Pokemon"), &tInfo)))
		return;
	m_vecPoke.push_back(tInfo);
	if (FAILED(pGameInstance->Add_GameObject(TEXT("Prototype_GameObject_NonePoke"), LEVEL_STATIC, TEXT("Layer_Pokemon"), &tInfo)))
		return;
	m_vecPoke.push_back(tInfo);
	if (FAILED(pGameInstance->Add_GameObject(TEXT("Prototype_GameObject_NonePoke"), LEVEL_STATIC, TEXT("Layer_Pokemon"), &tInfo)))
		return;
	m_vecPoke.push_back(tInfo);
	if (FAILED(pGameInstance->Add_GameObject(TEXT("Prototype_GameObject_NonePoke"), LEVEL_STATIC, TEXT("Layer_Pokemon"), &tInfo)))
		return;
	m_vecPoke.push_back(tInfo);
	if (FAILED(pGameInstance->Add_GameObject(TEXT("Prototype_GameObject_NonePoke"), LEVEL_STATIC, TEXT("Layer_Pokemon"), &tInfo)))
		return;
	m_vecPoke.push_back(tInfo);
	RELEASE_INSTANCE(CGameInstance);


}
void CSlowbro::WildBattle()
{
	//	_vector vLook = XMLoadFloat4(&dynamic_cast<CGameObj*>(m_pTarget)->Get_MyBattlePos()) - XMLoadFloat4(&m_vMyBattlePos);
	//	XMVector3Normalize(vLook);
	_vector vPos = XMLoadFloat4(&m_vMyBattlePos);
	//	_vector vTargetPos = XMLoadFloat4(&m_vMyBattlePos) + vLook * 200.f;
	m_pTransformCom->Set_State(CTransform::STATE_TRANSLATION, vPos);
	m_pTransformCom->LookAt(XMLoadFloat4(&dynamic_cast<CGameObj*>(m_pTarget)->Get_MyBattlePos()));
	Set_BattleMap(true, 0.f);
	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);
	Ready_Script();
	CTextBox::TINFO tTInfo;

	tTInfo.iScriptSize = (_int)m_vNormalScript.size();
	tTInfo.pTarget = this;
	tTInfo.pScript = new wstring[m_vNormalScript.size()];
	tTInfo.iType = 1;
	for (_int i = 0; i < m_vNormalScript.size(); ++i)
		tTInfo.pScript[i] = m_vNormalScript[i];

	if (FAILED(pGameInstance->Add_GameObject(TEXT("Prototype_GameObject_TextBox"), LEVEL_GAMEPLAY, TEXT("Layer_UI"), &tTInfo)))
		return;

	RELEASE_INSTANCE(CGameInstance);

	m_bBattleStart = true;
	m_PlayerInfo.bEvent = false;

}
void CSlowbro::Ready_Script()
{
	wstring szScriptBegin = TEXT("앗!! 야생의 '");
	wstring szScriptEnd = TEXT("'(이)가   나타났다!");
	szScriptBegin += m_PokemonInfo.strName;
	szScriptBegin += szScriptEnd;
	m_vNormalScript.push_back(szScriptBegin);
}
void CSlowbro::Move(_float fTimeDelta)
{
	_vector vTargetPos = dynamic_cast<CGameObj*>(m_pTarget)->Get_Transfrom()->Get_State(CTransform::STATE_TRANSLATION);
	_vector vLook = vTargetPos - m_pTransformCom->Get_State(CTransform::STATE_TRANSLATION);
	_vector vPos = m_pTransformCom->Get_State(CTransform::STATE_TRANSLATION);

	m_fDist = XMVectorGetX(XMVector3Length(vLook));
	if (m_fDist < 10.f)
	{
		if (!m_bFindPlayer)
		{
			m_PlayerInfo.bEvent = true;
			CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);
			if (FAILED(pGameInstance->Add_GameObject(TEXT("Prototype_GameObject_BattleEvent"), LEVEL_GAMEPLAY, TEXT("Layer_Effect"), this)))
				return;
			RELEASE_INSTANCE(CGameInstance);
			m_bFindPlayer = true;
		}
		m_pModelCom->Set_CurrentAnimIndex(8);
		vPos += XMVector3Normalize(vLook) * 4.f * fTimeDelta;
		m_pTransformCom->Set_State(CTransform::STATE_TRANSLATION, vPos);
		m_pTransformCom->LookAt(vTargetPos);
	}
	else
	{
		m_PlayerInfo.bEvent = false;
		m_pModelCom->Set_CurrentAnimIndex(2);
		m_bFindPlayer = false;
	}
}
void CSlowbro::Set_DeckPos()
{
	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);

	if (m_bDeckInfo)
	{
		if (!m_bSetPos)
		{
			_vector		vLook = { -0.3f,0.f,-1.f,0.f };

			_vector		vAxisY = XMVectorSet(0.f, 1.f, 0.f, 0.f);

			_vector		vRight = XMVector3Cross(vAxisY, vLook);

			_vector		vUp = XMVector3Cross(vLook, vRight);

			m_pTransformCom->Set_State(CTransform::STATE_RIGHT, XMVector3Normalize(vRight));
			m_pTransformCom->Set_State(CTransform::STATE_UP, XMVector3Normalize(vUp));
			m_pTransformCom->Set_State(CTransform::STATE_LOOK, XMVector3Normalize(vLook));
			m_bSetPos = true;
		}

		m_fSizeX = 7.f;
		m_fSizeY = 7.f;
		m_fX = 1000;
		m_fY = 550;

		XMStoreFloat4x4(&m_ViewMatrix, XMMatrixTranspose(XMMatrixIdentity()));
		XMStoreFloat4x4(&m_ProjMatrix, XMMatrixTranspose(XMMatrixOrthographicLH((_float)g_iWinSizeX, (_float)g_iWinSizeY, -500.f, 100.f)));
		_float3 vScale = { m_fSizeX,m_fSizeY,7.f };
		m_pTransformCom->Set_Scale(XMLoadFloat3(&vScale));
		m_pTransformCom->Set_State(CTransform::STATE_TRANSLATION, XMVectorSet(m_fX - g_iWinSizeX * 0.5f, -m_fY + g_iWinSizeY * 0.5f, -200.f, 1.f));

	}
	else if (!m_bDeckInfo)
	{
		if (!m_bSetPos)
		{
			_vector		vLook = { 0.f,0.f,-1.f,0.f };

			_vector		vAxisY = XMVectorSet(0.f, 1.f, 0.f, 0.f);

			_vector		vRight = XMVector3Cross(vAxisY, vLook);

			_vector		vUp = XMVector3Cross(vLook, vRight);

			m_pTransformCom->Set_State(CTransform::STATE_RIGHT, XMVector3Normalize(vRight));
			m_pTransformCom->Set_State(CTransform::STATE_UP, XMVector3Normalize(vUp));
			m_pTransformCom->Set_State(CTransform::STATE_LOOK, XMVector3Normalize(vLook));
			m_bSetPos = true;
		}

		m_fSizeX = 7.f;
		m_fSizeY = 7.f;
		m_fX = 280;
		m_fY = 550;

		XMStoreFloat4x4(&m_ViewMatrix, XMMatrixTranspose(XMMatrixIdentity()));
		XMStoreFloat4x4(&m_ProjMatrix, XMMatrixTranspose(XMMatrixOrthographicLH((_float)g_iWinSizeX, (_float)g_iWinSizeY, -800.f, 500.f)));
		_float3 vScale = { m_fSizeX,m_fSizeY,7.f };
		m_pTransformCom->Set_Scale(XMLoadFloat3(&vScale));
		m_pTransformCom->Set_State(CTransform::STATE_TRANSLATION, XMVectorSet(m_fX - g_iWinSizeX * 0.5f, -m_fY + g_iWinSizeY * 0.5f, -200.f, 1.f));

	}

	RELEASE_INSTANCE(CGameInstance);
}
void CSlowbro::Key_Input(_float fTimeDelta)
{
	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);

	if (pGameInstance->Key_Down(DIK_W))
	{
		++m_iAnim;
		if (m_iAnim > 9)
			m_iAnim = 9;

		m_pModelCom->Set_CurrentAnimIndex(m_iAnim);
	}
	else if (pGameInstance->Key_Down(DIK_S))
	{
		--m_iAnim;
		if (m_iAnim < 0)
			m_iAnim = 0;

		m_pModelCom->Set_CurrentAnimIndex(m_iAnim);
	}
	else if (pGameInstance->Key_Pressing(DIK_A))
	{
		m_pTransformCom->Turn2(m_pTransformCom->Get_State(CTransform::STATE_UP), XMConvertToRadians(3.f));
	}
	else if (pGameInstance->Key_Pressing(DIK_D))
	{
		m_pTransformCom->Turn2(m_pTransformCom->Get_State(CTransform::STATE_UP), XMConvertToRadians(-3.f));
	}

	RELEASE_INSTANCE(CGameInstance);
}
void CSlowbro::Battle(_float fTimeDelta)
{
	if (!m_bBattle)
	{
		m_pTransformCom->Set_Scale(XMVectorSet(0.05f, 0.05f, 0.05f, 0.f));
		m_fStartBattle += fTimeDelta;
		if (m_iAnimIndex == 0)
		{
			m_pModelCom->Set_Loop(m_iAnimIndex);
			m_pModelCom->Set_CurrentAnimIndex(m_iAnimIndex);
		}
		if (!m_bBrath && (m_fStartBattle > m_fBattleMapTime) && m_pModelCom->Get_End(m_iAnimIndex))
		{
			m_pModelCom->Set_End(m_iAnimIndex);
			m_iAnimIndex = 1;
			m_pModelCom->Set_Loop(m_iAnimIndex);
			m_pModelCom->Set_CurrentAnimIndex(m_iAnimIndex);
			m_bBrath = true;
		}
		if (m_bBattleMap)
		{
			if (m_bBrath && m_pModelCom->Get_End(m_iAnimIndex))
			{
				m_pModelCom->Set_End(m_iAnimIndex);
				m_iAnimIndex = 2;
				m_pModelCom->Set_CurrentAnimIndex(m_iAnimIndex);
				m_bBattle = true;
				if (m_bWildPoke)
				{
					m_bDelay = true;
					m_fDelayTime = 0.f;
				}
			}

		}
	}
	if (m_bDelay)
	{
		m_fDelayTime += fTimeDelta;
		if (m_fDelayTime > 1.f)
		{
			dynamic_cast<CPlayer*>(m_pTarget)->Set_BattleStart();
			m_bDelay = false;
		}
	}
	if (m_bAttack && m_pModelCom->Get_End(m_iAnimIndex))
	{
		m_pModelCom->Set_End(m_iAnimIndex);
		m_iAnimIndex = 2;
		m_pModelCom->Set_CurrentAnimIndex(m_iAnimIndex);
		m_bAttack = false;
	}
	if (m_iAnimIndex == 3 || m_iAnimIndex == 4)
	{
		m_pModelCom->Set_Loop(m_iAnimIndex);
		m_pModelCom->Set_CurrentAnimIndex(m_iAnimIndex);
		m_bAttack = true;
	}
	if (m_bHit && m_pModelCom->Get_End(m_iAnimIndex))
	{
		m_pModelCom->Set_End(m_iAnimIndex);
		m_iAnimIndex = 2;
		m_pModelCom->Set_CurrentAnimIndex(m_iAnimIndex);
		m_bHit = false;
	}
	if (m_iAnimIndex == 5)
	{
		m_pModelCom->Set_Loop(m_iAnimIndex);
		m_pModelCom->Set_CurrentAnimIndex(m_iAnimIndex);
		m_bHit = true;
	}
	if (m_bDown && m_pModelCom->Get_End(m_iAnimIndex))
	{
		m_pModelCom->Set_End(m_iAnimIndex);
		m_bStopAnim = true;
	}
	if (!m_bDown && m_iAnimIndex == 6)
	{
		m_pModelCom->Set_Loop(m_iAnimIndex);
		m_pModelCom->Set_CurrentAnimIndex(m_iAnimIndex);
		m_bDown = true;
	}

	if (!m_bStopAnim)
		m_pModelCom->Play_Animation(fTimeDelta * 1.1f);
}
void CSlowbro::Set_Stats()
{
	_float fHp = 95.f;
	_float fDmg = 75.f;
	_float fDef = 110.f;
	_float fSDmg = 100.f;
	_float fSDef = 80.f;
	_float fSpeed = 30.f;

	m_PlayerInfo.strName = TEXT("야도란");
	m_PlayerInfo.bEvent = false;

	m_PokemonInfo.strName = TEXT("야도란");
	m_PokemonInfo.strInfo = TEXT("준석도란. \n밴티 찾고 모나뽑고 원신 안하기??\n밴티 왜 찾음?...");
	m_PokemonInfo.strChar = TEXT("웹마스터");
	m_PokemonInfo.iPokeNum = 80;
	m_PokemonInfo.iLv = 15;
	m_PokemonInfo.iMaxHp = _int(((fHp * 2.f) + 31.f + 100) * (m_PokemonInfo.iLv / 100.f) + 10.f);
	m_PokemonInfo.iHp = m_PokemonInfo.iMaxHp;
	m_PokemonInfo.iDmg = _int(((fDmg * 2.f) + 31.f) * (m_PokemonInfo.iLv / 100.f) + 5.f);
	m_PokemonInfo.iSDmg = _int(((fSDmg * 2.f) + 31.f) * (m_PokemonInfo.iLv / 100.f) + 5.f);
	m_PokemonInfo.iDef = _int(((fDef * 2.f) + 31.f) * (m_PokemonInfo.iLv / 100.f) + 5.f);
	m_PokemonInfo.iSDef = _int(((fSDef * 2.f) + 31.f) * (m_PokemonInfo.iLv / 100.f) + 5.f);
	m_PokemonInfo.iSpeed = _int(((fSpeed * 2.f) + 31.f) * (m_PokemonInfo.iLv / 100.f) + 5.f);
	m_PokemonInfo.iMaxExp = m_PokemonInfo.iLv * 2;
	m_PokemonInfo.iExp = 0;
	m_PokemonInfo.iSex = rand() % 2;
	m_PokemonInfo.iBallNum = 0;
	m_PokemonInfo.bRide = false;
	m_PokemonInfo.bEvolution = false;
}
void CSlowbro::LvUp()
{
	_float fHp = 95.f;
	_float fDmg = 75.f;
	_float fDef = 110.f;
	_float fSDmg = 100.f;
	_float fSDef = 80.f;
	_float fSpeed = 30.f;

	_int iPrevMaxHp = m_PokemonInfo.iMaxHp;
	_int iPrevHp = m_PokemonInfo.iHp;
	_int iPrevDmg = m_PokemonInfo.iDmg;
	_int iPrevDef = m_PokemonInfo.iDef;
	_int iPrevSDmg = m_PokemonInfo.iSDmg;
	_int iPrevSDef = m_PokemonInfo.iSDef;
	_int iPrevSpeed = m_PokemonInfo.iSpeed;

	m_PokemonInfo.iMaxHp = _int(((fHp * 2.f) + 31.f + 100) * (m_PokemonInfo.iLv / 100.f) + 10.f);
	m_PokemonInfo.iHp = m_PokemonInfo.iMaxHp - iPrevMaxHp + iPrevHp;
	m_PokemonInfo.iDmg = _int(((fDmg * 2.f) + 31.f) * (m_PokemonInfo.iLv / 100.f) + 5.f);
	m_PokemonInfo.iSDmg = _int(((fSDmg * 2.f) + 31.f) * (m_PokemonInfo.iLv / 100.f) + 5.f);
	m_PokemonInfo.iDef = _int(((fDef * 2.f) + 31.f) * (m_PokemonInfo.iLv / 100.f) + 5.f);
	m_PokemonInfo.iSDef = _int(((fSDef * 2.f) + 31.f) * (m_PokemonInfo.iLv / 100.f) + 5.f);
	m_PokemonInfo.iSpeed = _int(((fSpeed * 2.f) + 31.f) * (m_PokemonInfo.iLv / 100.f) + 5.f);

	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);

	CLv_Up::LVUPINFO tInfo;
	tInfo.iPrevMaxHp = iPrevMaxHp;
	tInfo.iPrevDmg = iPrevDmg;
	tInfo.iPrevDef = iPrevDef;
	tInfo.iPrevSDmg = iPrevSDmg;
	tInfo.iPrevSDef = iPrevSDef;
	tInfo.iPrevSpeed = iPrevSpeed;

	tInfo.iMaxHp = m_PokemonInfo.iMaxHp;
	tInfo.iDmg = m_PokemonInfo.iDmg;
	tInfo.iDef = m_PokemonInfo.iDef;
	tInfo.iSDmg = m_PokemonInfo.iSDmg;
	tInfo.iSDef = m_PokemonInfo.iSDef;
	tInfo.iSpeed = m_PokemonInfo.iSpeed;


	if (FAILED(pGameInstance->Add_GameObject(TEXT("Prototype_GameObject_Lv_Up"), LEVEL_GAMEPLAY, TEXT("Layer_UI"), &tInfo)))
		return;

	RELEASE_INSTANCE(CGameInstance);


	m_PokemonInfo.bLvUp = false;
}
HRESULT CSlowbro::SetUp_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;
	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);

	if (FAILED(m_pShaderCom->Set_RawValue("g_vCamPosition", &pGameInstance->Get_CamPosition(), sizeof(_float4))))
		return E_FAIL;

	if (!m_bOnOff)
	{
		if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_World4x4_TP(), sizeof(_float4x4))))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_VIEW), sizeof(_float4x4))))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Set_RawValue("g_ProjMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_PROJ), sizeof(_float4x4))))
			return E_FAIL;
	}
	else if (m_bOnOff)
	{

		if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_World4x4_TP(), sizeof(_float4x4))))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", &m_ViewMatrix, sizeof(_float4x4))))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Set_RawValue("g_ProjMatrix", &m_ProjMatrix, sizeof(_float4x4))))
			return E_FAIL;

	}

	RELEASE_INSTANCE(CGameInstance);

	return S_OK;
}

CSlowbro * CSlowbro::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CSlowbro*	pInstance = new CSlowbro(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		ERR_MSG(TEXT("Failed to Created : CSlowbro"));
		Safe_Release(pInstance);
	}

	return pInstance;
}


CGameObject * CSlowbro::Clone(void * pArg)
{
	CGameObject*	pInstance = new CSlowbro(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		ERR_MSG(TEXT("Failed to Cloned : CSlowbro"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSlowbro::Free()
{
	__super::Free();

	if (m_bWildPoke)
		m_vecPoke.clear();

	for (auto iter = m_vNormalScript.begin(); iter != m_vNormalScript.end();)
		iter = m_vNormalScript.erase(iter);

	m_vNormalScript.clear();

	Safe_Release(m_pNavigationCom);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pAABBCom);
}
