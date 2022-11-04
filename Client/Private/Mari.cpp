#include "stdafx.h"
#include "..\Public\Mari.h"

#include "GameInstance.h"
#include "Level_GamePlay.h"
#include "Camera_Dynamic.h"
#include "TextBox.h"
#include "SoundMgr.h"

CMari::CMari(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObj(pDevice, pContext)
{
}

CMari::CMari(const CMari & rhs)
	: CGameObj(rhs)
{
}

HRESULT CMari::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMari::Initialize(void * pArg)
{

	if (FAILED(Ready_Components()))
		return E_FAIL;
	

	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);

	m_pTarget = *(&((CLevel_GamePlay::LOADFILE*)pArg)->pTarget);
	m_pCamera = *(&((CLevel_GamePlay::LOADFILE*)pArg)->pCamera);
	m_vMyBattlePos = dynamic_cast<CGameObj*>(m_pTarget)->Get_TargetBattlePos();
	XMStoreFloat4(&m_vTargetBattlePos,dynamic_cast<CGameObj*>(m_pTarget)->Get_Transfrom()->Get_State(CTransform::STATE_TRANSLATION));
	RELEASE_INSTANCE(CGameInstance);

	m_PlayerInfo.strName = L"마리";
 	m_PlayerInfo.bEvent = false;
	m_PlayerInfo.bBattle = false;

	m_pModelCom->Set_CurrentAnimIndex(0);
	m_pTransformCom->Turn2(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(90.f));
	m_pTransformCom->Set_Scale(XMLoadFloat3((&((CLevel_GamePlay::LOADFILE*)pArg)->vScale)));
	m_pTransformCom->Set_State(CTransform::STATE_TRANSLATION, XMLoadFloat4((&((CLevel_GamePlay::LOADFILE*)pArg)->vPos)));

	Ready_Script();

	return S_OK;
}

void CMari::Tick(_float fTimeDelta)
{
	if (g_Battle)
		Battle();
	else
	{
		m_fEventTime += fTimeDelta;
		if (!m_bFindPlayer)
			m_pModelCom->Set_CurrentAnimIndex(0);
		if (m_bFindPlayer && !m_PlayerInfo.bEvent)
			Move(fTimeDelta);

		if (m_bFindPlayer && m_fEventTime > 1.5f)
		{
			m_PlayerInfo.bEvent = false;
		}
		m_pAABBCom->Update(m_pTransformCom->Get_WorldMatrix());
		m_pOBBCom->Update(m_pTransformCom->Get_WorldMatrix());
	}
	m_pModelCom->Play_Animation(fTimeDelta);
}

void CMari::Late_Tick(_float fTimeDelta)
{
	if(!g_Battle)
		Check_Coll();
	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);

	if (pGameInstance->IsInFrustum(m_pTransformCom->Get_State(CTransform::STATE_TRANSLATION), m_pTransformCom->Get_Scale()))
	{
		if (!g_PokeInfo && !g_bPokeDeck && nullptr != m_pRendererCom)
			m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONALPHABLEND, this);
	}
	RELEASE_INSTANCE(CGameInstance);
}

HRESULT CMari::Render()
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
	{
		m_pAABBCom->Render();
		m_pOBBCom->Render();
	}
	return S_OK;
}
HRESULT CMari::Ready_Components()
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
	if (FAILED(__super::Add_Components(TEXT("Com_Model"), LEVEL_STATIC, TEXT("Prototype_Component_Model_Mari"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	CCollider::COLLIDERDESC		ColliderDesc;

	/* For.Com_AABB */
	ZeroMemory(&ColliderDesc, sizeof(CCollider::COLLIDERDESC));

	ColliderDesc.vScale = _float3(20.f, 40.f, 20.f);
	ColliderDesc.vPosition = _float3(0.f, 20.f, 0.f);
	if (FAILED(__super::Add_Components(TEXT("Com_AABB"), LEVEL_STATIC, TEXT("Prototype_Component_Collider_AABB"), (CComponent**)&m_pAABBCom, &ColliderDesc)))
		return E_FAIL;

	/* For.Com_OBB*/
	ColliderDesc.vScale = _float3(40.f, 40.f, 200.f);
	ColliderDesc.vPosition = _float3(0.f, 20.f, 80.f);
	if (FAILED(__super::Add_Components(TEXT("Com_OBB"), LEVEL_STATIC, TEXT("Prototype_Component_Collider_OBB"), (CComponent**)&m_pOBBCom, &ColliderDesc)))
		return E_FAIL;

	return S_OK;
}

void CMari::Move(_float fTimeDelta)
{
	m_pModelCom->Set_CurrentAnimIndex(2);
	_vector vTargetPos = dynamic_cast<CGameObj*>(m_pTarget)->Get_Transfrom()->Get_State(CTransform::STATE_TRANSLATION);
	_vector vLook = vTargetPos - m_pTransformCom->Get_State(CTransform::STATE_TRANSLATION);
	_vector vPos = m_pTransformCom->Get_State(CTransform::STATE_TRANSLATION);
	vPos += XMVector3Normalize(vLook) * 4.f * fTimeDelta;
	m_pTransformCom->Set_State(CTransform::STATE_TRANSLATION, vPos);
	m_pTransformCom->LookAt(vTargetPos);
}

void CMari::Ready_Script()
{
	m_vNormalScript.push_back(TEXT("거기 잠깐!!"));
	m_vNormalScript.push_back(TEXT("내 앞을 지나가려면 UI를 다 해야한다고!"));
	m_vNormalScript.push_back(TEXT("난 너가 UI를 다 했는지 안했는지 알 수 있는 방법이 있다고!"));
	m_vNormalScript.push_back(TEXT("바로! 포켓몬 승부다!"));
}

void CMari::Check_Coll()
{
	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);
	
	CCollider*	pTargetCollider = (CCollider*)pGameInstance->Get_Component(LEVEL_GAMEPLAY, TEXT("Layer_Player"), TEXT("Com_AABB"));
	CCollider*	pTargetCollider2 = (CCollider*)pGameInstance->Get_Component(LEVEL_GAMEPLAY, TEXT("Layer_Player"), TEXT("Com_OBB"));
	if (nullptr == pTargetCollider)
		return;

	if (!m_bEvent && m_pOBBCom->Collision(pTargetCollider2))
	{
		CSoundMgr::Get_Instance()->SetSoundVolume(SOUND_BGM, 0.3f);
		CSoundMgr::Get_Instance()->PlayEffect(TEXT("Battle0.wav"),1.f);
		m_bFindPlayer = true;
		m_PlayerInfo.bEvent = true;
		m_bEvent = true;
		m_fEventTime = 0.f;
		if (!dynamic_cast<CGameObj*>(m_pTarget)->Get_Event())
			dynamic_cast<CGameObj*>(m_pTarget)->OnOffEvent();
		dynamic_cast<CCamera_Dynamic*>(m_pCamera)->Set_Target(this);
		if (FAILED(pGameInstance->Add_GameObject(TEXT("Prototype_GameObject_BattleEvent"), LEVEL_GAMEPLAY, TEXT("Layer_Effect"), this)))
			return;
		if (FAILED(pGameInstance->Add_GameObject(TEXT("Prototype_GameObject_Screen"), LEVEL_GAMEPLAY, TEXT("Layer_Effect"), this)))
			return;
		CTextBox::TINFO tTInfo;

		tTInfo.iScriptSize = (_int)m_vNormalScript.size();
		tTInfo.pTarget = this;
		tTInfo.pScript = new wstring[m_vNormalScript.size()];
		for (_int i = 0; i < m_vNormalScript.size(); ++i)
			tTInfo.pScript[i] = m_vNormalScript[i];

		if (FAILED(pGameInstance->Add_GameObject(TEXT("Prototype_GameObject_TextBox"), LEVEL_GAMEPLAY, TEXT("Layer_UI"), &tTInfo)))
			return;
	}
	if (m_pAABBCom->Collision(pTargetCollider))
	{
		m_bFindPlayer = false;

	}
	RELEASE_INSTANCE(CGameInstance);
}

void CMari::Battle()
{
	m_pTransformCom->Set_State(CTransform::STATE_TRANSLATION, XMLoadFloat4(&m_vMyBattlePos));
	m_pTransformCom->LookAt(XMLoadFloat4(&m_vTargetBattlePos));
}

HRESULT CMari::SetUp_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;
	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);

	if (FAILED(m_pShaderCom->Set_RawValue("g_vCamPosition", &pGameInstance->Get_CamPosition(), sizeof(_float4))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_World4x4_TP(), sizeof(_float4x4))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_VIEW), sizeof(_float4x4))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Set_RawValue("g_ProjMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_PROJ), sizeof(_float4x4))))
		return E_FAIL;
	


	RELEASE_INSTANCE(CGameInstance);

	return S_OK;
}

CMari * CMari::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CMari*	pInstance = new CMari(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		ERR_MSG(TEXT("Failed to Created : CMari"));
		Safe_Release(pInstance);
	}

	return pInstance;
}


CGameObject * CMari::Clone(void * pArg)
{
	CGameObject*	pInstance = new CMari(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		ERR_MSG(TEXT("Failed to Cloned : CMari"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMari::Free()
{
	__super::Free();

	for (auto iter = m_vNormalScript.begin(); iter != m_vNormalScript.end();)
		iter = m_vNormalScript.erase(iter);

	m_vNormalScript.clear();


	Safe_Release(m_pAABBCom);
	Safe_Release(m_pOBBCom);
	Safe_Release(m_pModelCom);
}
