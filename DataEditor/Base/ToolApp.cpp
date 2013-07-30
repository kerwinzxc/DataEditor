#include "stdafx.h"
#include "ToolApp.h"
#include "ACString.h"

#include "ToolConfig.h"
#include "ToolTree.h"
#include "ToolTab.h"
#include "ToolLog.h"
#include "ToolLayout.h"
#include "ToolExcel.h"

BEGIN_NS_AC

ToolApp* ToolApp::m_pInstance = NULL;

ToolApp::ToolApp()
{
	m_pInstance = this;
	m_pConfig = new ToolConfig(this);
	m_pTree = new ToolTree(this);
	m_pTab = new ToolTab(this);
	m_pLog = new ToolLog(this);
	m_pLayout = new ToolLayout(this);
	m_pExcel = new ToolExcel(this);
	m_bIsNewing = false;
}

ToolApp::~ToolApp()
{
	FinalizeTool();
	_safe_delete(m_pConfig);
	_safe_delete(m_pTree);
	_safe_delete(m_pTab);
	_safe_delete(m_pLog);
	_safe_delete(m_pLayout);
	_safe_delete(m_pExcel);
	m_pInstance = NULL;
}

int ToolApp::InitializeTool(const CString& strAppName)
{
	m_objFont.CreateFont(20, // Height
		0, // Width
		0, // Escapement
		0, // Orientation
		FW_BLACK, // Weight
		FALSE, // Italic
		FALSE, // Underline
		0, // StrikeOut
		ANSI_CHARSET, // CharSet
		OUT_DEFAULT_PRECIS, // OutPrecision
		CLIP_DEFAULT_PRECIS, // ClipPrecision
		DEFAULT_QUALITY, // Quality
		DEFAULT_PITCH | FF_SWISS, // PitchAndFamily
		_T("΢���ź�")); // Facename

	m_pLog->Create(strAppName);
	m_pTab->Create();
	m_pTree->Create();
	m_pLayout->Init();
	m_pConfig->Load(strAppName);
	m_pTab->ChangeTab(0);

	m_pMenu = new CMenu;
	m_pMenu->CreatePopupMenu();
	m_pMenu->AppendMenu(MF_STRING,ID_MENU_NEW,_T("�½�"));
	m_pMenu->AppendMenu(MF_STRING,ID_MENU_SAVE,_T("����"));
	m_pMenu->AppendMenu(MF_STRING,ID_MENU_DELETE,_T("ɾ��"));
	m_pMenu->AppendMenu(MF_STRING,ID_MENU_COPY,_T("����"));
	m_pMenu->AppendMenu(MF_STRING,ID_MENU_CANCEL,_T("ȡ��"));
	SetMenu(GetMainWnd()->GetSafeHwnd(),m_pMenu->GetSafeHmenu());
	m_pMenu->EnableMenuItem(ID_MENU_CANCEL,TRUE);

	INFO_MSG("--------------------------------------------");
	INFO_MSG("               %s Start             ",CStringToStlString(strAppName).c_str());
	INFO_MSG("--------------------------------------------");

	return 0;
}

int ToolApp::FinalizeTool()
{
	m_objFont.DeleteObject();
	m_pMenu->DestroyMenu();
	delete m_pMenu;
	return 0;
}

int ToolApp::Update()
{
	m_pLog->Update();
	return 0;
}

int ToolApp::MenuNew()
{
	if(m_bIsNewing)
		return -1;

	m_pMenu->EnableMenuItem(ID_MENU_COPY,TRUE);
	m_pMenu->EnableMenuItem(ID_MENU_NEW,TRUE);
	m_pMenu->EnableMenuItem(ID_MENU_DELETE,TRUE);
	m_pMenu->EnableMenuItem(ID_MENU_CANCEL,FALSE);

	m_bIsNewing = true;
	m_pTree->EnableWindow(FALSE);
	m_pTab->EnableKeyWindow(TRUE);
	m_pTab->LoadDefaultValues();

	CEdit* pKeyWnd = m_pTab->GetCurrentItem()->GetKeyWnd();
	ACCHECK(pKeyWnd);

	pKeyWnd->SetWindowText(_T(""));
	pKeyWnd->SetFocus();
	return 0;
}

int ToolApp::MenuSave()
{
	SItemTab* pTabItem = m_pTab->GetCurrentItem();
	ACCHECK(pTabItem);

	int nKey = -1;
	if(m_bIsNewing)
	{
		m_pMenu->EnableMenuItem(ID_MENU_COPY,FALSE);
		m_pMenu->EnableMenuItem(ID_MENU_NEW,FALSE);
		m_pMenu->EnableMenuItem(ID_MENU_DELETE,FALSE);
		m_pMenu->EnableMenuItem(ID_MENU_CANCEL,TRUE);

		m_bIsNewing = false;
		m_pTree->EnableWindow(TRUE);
		m_pTab->EnableKeyWindow(FALSE);

		CEdit* pCtrlKey = pTabItem->GetKeyWnd();
		ACCHECK(pCtrlKey);

		CString strKey;
		pCtrlKey->GetWindowText(strKey);
		nKey = atoi(CStringToStlString(strKey).c_str());

		MapCNameToValueT mapValues;
		pTabItem->GetAllCtrlValues(mapValues);
		if(pTabItem->GetDB()->InsertByKey(nKey,mapValues) == -1)
		{
			m_pTab->RestoreLastSelect();
			InfoMessageBox(_T("Key invalid"));
			return -1;
		}

		m_pTree->SelectKey(nKey);
	}
	else
	{
		pTabItem->CtrlToDB(m_pTree->GetSelectKey());
	}
	
	return 0;
}

int ToolApp::MenuDelete()
{
	if(m_bIsNewing)
		return -1;

	SItemTab* pItemTab = m_pTab->GetCurrentItem();
	ACCHECK(pItemTab);

	CEdit* pKeyWnd = pItemTab->GetKeyWnd();
	ACCHECK(pKeyWnd);

	CString strKey;
	pKeyWnd->GetWindowText(strKey);

	int nKey = atoi(CStringToStlString(strKey).c_str());
	pItemTab->GetDB()->DeleteByKey(nKey);

	return 0;
}

int ToolApp::MenuCopy()
{
	if(m_bIsNewing)
		return -1;

	m_pMenu->EnableMenuItem(ID_MENU_COPY,TRUE);
	m_pMenu->EnableMenuItem(ID_MENU_NEW,TRUE);
	m_pMenu->EnableMenuItem(ID_MENU_DELETE,TRUE);
	m_pMenu->EnableMenuItem(ID_MENU_CANCEL,FALSE);

	m_bIsNewing = true;
	m_pTree->EnableWindow(FALSE);
	m_pTab->EnableKeyWindow(TRUE);

	CEdit* pKeyWnd = m_pTab->GetCurrentItem()->GetKeyWnd();
	ACCHECK(pKeyWnd);

	pKeyWnd->SetWindowText(_T(""));
	pKeyWnd->SetFocus();
	return 0;
}

int ToolApp::MenuCancel()
{
	if(!m_bIsNewing)
		return -1;

	m_pMenu->EnableMenuItem(ID_MENU_COPY,FALSE);
	m_pMenu->EnableMenuItem(ID_MENU_NEW,FALSE);
	m_pMenu->EnableMenuItem(ID_MENU_DELETE,FALSE);
	m_pMenu->EnableMenuItem(ID_MENU_CANCEL,TRUE);

	m_bIsNewing = false;
	m_pTree->EnableWindow(TRUE);
	m_pTab->EnableKeyWindow(FALSE);
	m_pTab->RestoreLastSelect();

	return 0;
}

END_NS_AC