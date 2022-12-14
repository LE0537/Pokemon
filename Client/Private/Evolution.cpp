#include "stdafx.h"
#include "..\Public\Evolution.h"

#include "GameInstance.h"
#include "Level_GamePlay.h"
#include "SoundMgr.h"

CEvolution::CEvolution(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObj(pDevice, pContext)
{
}

CEvolution::CEvolution(const CEvolution & rhs)
	: CGameObj(rhs)
{
}

HRESULT CEvolution::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CEvolution::Initialize(void * pArg)
{
	m_pTarget = (CGameObject*)pArg;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_fSizeX = g_iWinSizeX;
	m_fSizeY = g_iWinSizeY;
	m_fX = g_iWinSizeX >> 1;
	m_fY = g_iWinSizeY >> 1;

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixTranspose(XMMatrixIdentity()));
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixTranspose(XMMatrixOrthographicLH((_float)g_iWinSizeX, (_float)g_iWinSizeY, -500.f, 300.f)));

	_float3 vScale = { m_fSizeX,m_fSizeY,0.f };

	m_pTransformCom->Set_Scale(XMLoadFloat3(&vScale));
	m_pTransformCom->Set_State(CTransform::STATE_TRANSLATION, XMVectorSet(m_fX - g_iWinSizeX * 0.5f, -m_fY + g_iWinSizeY * 0.5f, 300.f, 1.f));

	CSoundMgr::Get_Instance()->BGM_Stop();
	CSoundMgr::Get_Instance()->PlayEffect(TEXT("Evol2.mp3"), 1.f);

	return S_OK;
}

void CEvolution::Tick(_float fTimeDelta)
{
	if (!m_bSound)
	{
		CSoundMgr::Get_Instance()->BGM_Stop();
		CSoundMgr::Get_Instance()->PlayEffect(TEXT("Evol2.mp3"), 1.f);
		m_bSound = true;
	}
	Create_Light(fTimeDelta);

	if (!g_bEvolution)
	{
		Set_Dead();
		CSoundMgr::Get_Instance()->BGM_Stop();
		CSoundMgr::Get_Instance()->PlayBGM(TEXT("hov.wav"), 0.45f);
	}
}

void CEvolution::Late_Tick(_float fTimeDelta)
{

	if (g_bEvolution && nullptr != m_pRendererCom)
		m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_UI, this);

}

HRESULT CEvolution::Render()
{

	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(SetUp_ShaderResources()))
		return E_FAIL;



	return S_OK;
}
HRESULT CEvolution::Render_ShadowDepth()
{
	return E_NOTIMPL;
}
HRESULT CEvolution::Ready_Components()
{
	/* For.Com_Renderer */
	if (FAILED(__super::Add_Components(TEXT("Com_Renderer"), LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* For.Com_Transform */
	if (FAILED(__super::Add_Components(TEXT("Com_Transform"), LEVEL_STATIC, TEXT("Prototype_Component_Transform"), (CComponent**)&m_pTransformCom)))
		return E_FAIL;

	/* For.Com_Texture */
	if (FAILED(__super::Add_Components(TEXT("Com_Texture"), LEVEL_STATIC, TEXT("Prototype_Component_Texture_Battle"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Components(TEXT("Com_Shader"), LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxTex"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Components(TEXT("Com_VIBuffer"), LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;


	return S_OK;
}

HRESULT CEvolution::Create_Light(_float fTimeDelta)
{
	m_fCreateTime += fTimeDelta;
	if (m_fCreateTime > 0.3f)
	{
		CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);
		for (_int i = 0; i < 3; ++i)
		{
			if (FAILED(pGameInstance->Add_GameObject(TEXT("Prototype_GameObject_EvolLight"), LEVEL_GAMEPLAY, TEXT("Layer_UI"))))
				return E_FAIL;
		}
		m_fCreateTime = 0.f;
		RELEASE_INSTANCE(CGameInstance);
	}

	return S_OK;
}

HRESULT CEvolution::SetUp_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;


	if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_World4x4_TP(), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", &m_ViewMatrix, sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ProjMatrix", &m_ProjMatrix, sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_ShaderResourceView("g_DiffuseTexture", m_pTextureCom->Get_SRV(21))))
		return E_FAIL;

	m_pShaderCom->Begin(0);
	m_pVIBufferCom->Render();



	return S_OK;
}

CEvolution * CEvolution::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CEvolution*	pInstance = new CEvolution(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		ERR_MSG(TEXT("Failed to Created : CEvolution"));
		Safe_Release(pInstance);
	}

	return pInstance;
}


CGameObject * CEvolution::Clone(void * pArg)
{
	CGameObject*	pInstance = new CEvolution(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		ERR_MSG(TEXT("Failed to Cloned : CEvolution"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEvolution::Free()
{
	__super::Free();


}
