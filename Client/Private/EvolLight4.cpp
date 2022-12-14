#include "stdafx.h"
#include "..\Public\EvolLight4.h"

#include "GameInstance.h"

CEvolLight4::CEvolLight4(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObj(pDevice, pContext)
{
}

CEvolLight4::CEvolLight4(const CEvolLight4 & rhs)
	: CGameObj(rhs)
{
}

HRESULT CEvolLight4::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CEvolLight4::Initialize(void * pArg)
{
	if (FAILED(Ready_Components()))
		return E_FAIL;


	m_fSizeX = 30.f;
	m_fSizeY = 30.f;

	m_fX = _float(rand() % 600 + 340.f);
	m_fY = _float(rand() % 600);

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixTranspose(XMMatrixIdentity()));
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixTranspose(XMMatrixOrthographicLH((_float)g_iWinSizeX, (_float)g_iWinSizeY, -200.f, 100.f)));

	_float3 vScale = { m_fSizeX,m_fSizeY,0.f };

	m_pTransformCom->Set_Scale(XMLoadFloat3(&vScale));
	m_pTransformCom->Set_State(CTransform::STATE_TRANSLATION, XMVectorSet(m_fX - g_iWinSizeX * 0.5f, -m_fY + g_iWinSizeY * 0.5f, 0.f, 1.f));

	return S_OK;
}

void CEvolLight4::Tick(_float fTimeDelta)
{
	Set_Pos(fTimeDelta);

	if (!g_bEvolution)
		Set_Dead();
}

void CEvolLight4::Late_Tick(_float fTimeDelta)
{
	if (g_bEvolution && nullptr != m_pRendererCom)
		m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CEvolLight4::Render()
{
	if (nullptr == m_pShaderCom || nullptr == m_pVIBufferCom)
		return E_FAIL;

	if (FAILED(SetUp_ShaderResources()))
		return E_FAIL;

	return S_OK;
}

HRESULT CEvolLight4::Render_ShadowDepth()
{
	return E_NOTIMPL;
}

HRESULT CEvolLight4::Ready_Components()
{
	/* For.Com_Renderer */
	if (FAILED(__super::Add_Components(TEXT("Com_Renderer"), LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* For.Com_Transform */
	if (FAILED(__super::Add_Components(TEXT("Com_Transform"), LEVEL_STATIC, TEXT("Prototype_Component_Transform"), (CComponent**)&m_pTransformCom)))
		return E_FAIL;


	/* For.Com_Shader */
	if (FAILED(__super::Add_Components(TEXT("Com_Shader"), LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxTex"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;


	/* For.Com_Texture */
	if (FAILED(__super::Add_Components(TEXT("Com_Texture"), LEVEL_STATIC, TEXT("Prototype_Component_Texture_BallEffect"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Components(TEXT("Com_VIBuffer"), LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;


	return S_OK;
}

HRESULT CEvolLight4::SetUp_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_World4x4_TP(), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", &m_ViewMatrix, sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ProjMatrix", &m_ProjMatrix, sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_ShaderResourceView("g_DiffuseTexture", m_pTextureCom->Get_SRV(3))))
		return E_FAIL;

	m_pShaderCom->Begin(11);
	m_pVIBufferCom->Render();


	return S_OK;
}

void CEvolLight4::Set_Pos(_float fTimeDelta)
{
	m_fSize = _float(rand() % 100 + 20);
	_vector vScale = { m_fSize ,m_fSize ,1.f ,0.f };
	m_pTransformCom->Set_Scale(vScale);

	m_fDeadTime += fTimeDelta;
	if (m_fDeadTime > 0.5f)
		Set_Dead();
}




CEvolLight4 * CEvolLight4::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CEvolLight4*	pInstance = new CEvolLight4(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		ERR_MSG(TEXT("Failed to Created : CEvolLight4"));
		Safe_Release(pInstance);
	}

	return pInstance;
}


CGameObject * CEvolLight4::Clone(void * pArg)
{
	CGameObject*	pInstance = new CEvolLight4(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		ERR_MSG(TEXT("Failed to Cloned : CEvolLight4"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEvolLight4::Free()
{
	__super::Free();

}
