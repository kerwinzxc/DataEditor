#include "stdafx.h"
#include "ToolApp.h"
#include "ToolTree.h"
#include "ToolLog.h"
#include "ACExcel.h"
#include "ExcelDB.h"

#include "ACString.h"

BEGIN_NS_AC

ToolApp* ToolApp::m_pInstance = NULL;

ToolApp::ToolApp()
: m_bIsNewing(false)
{
	m_strCurrentDB = _T("");
	m_pInstance = this;
	m_pLog = new ToolLog(this);
	m_pExcelServer = new ACExcel();
}

ToolApp::~ToolApp()
{
	FinalizeTool();

	for(std::map<CString,ExcelDB*>::iterator iter = m_mapExcelDBs.begin(); iter != m_mapExcelDBs.end(); ++iter)
	{
		_safe_delete(iter->second);
	}
	m_mapExcelDBs.clear();

	_safe_delete(m_pLog);
	_safe_delete(m_pExcelServer);
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
		_T("微软雅黑")); // Facename

	m_pLog->Create(strAppName);

	m_pMenu = new CMenu;
	m_pMenu->CreatePopupMenu();
	m_pMenu->AppendMenu(MF_STRING,ID_MENU_NEW,_T("新建"));
	m_pMenu->AppendMenu(MF_STRING,ID_MENU_SAVE,_T("保存"));
	m_pMenu->AppendMenu(MF_STRING,ID_MENU_DELETE,_T("删除"));
	m_pMenu->AppendMenu(MF_STRING,ID_MENU_COPY,_T("复制"));
	m_pMenu->AppendMenu(MF_STRING,ID_MENU_CANCEL,_T("取消"));
	SetMenu(GetMainWnd()->GetSafeHwnd(),m_pMenu->GetSafeHmenu());

	m_pMenu->EnableMenuItem(ID_MENU_NEW,FALSE);
	m_pMenu->EnableMenuItem(ID_MENU_SAVE,FALSE);
	m_pMenu->EnableMenuItem(ID_MENU_DELETE,FALSE);
	m_pMenu->EnableMenuItem(ID_MENU_COPY,FALSE);
	m_pMenu->EnableMenuItem(ID_MENU_CANCEL,TRUE);

	INFO_MSG("--------------------------------------------");
	INFO_MSG("               %s Start             ",CStringToStlString(strAppName).c_str());
	INFO_MSG("--------------------------------------------");

	return 0;
}

int ToolApp::FinalizeTool()
{
	for(std::map<int,CWnd*>::iterator iter = m_mapCheckCombos.begin(); iter != m_mapCheckCombos.end(); ++iter)
	{
		CWnd* pCheckCombo = iter->second;
		_safe_delete(pCheckCombo);
	}
	m_mapCheckCombos.clear();

	m_pMenu->DestroyMenu();
	delete m_pMenu;
	m_objFont.DeleteObject();
	return 0;
}

int ToolApp::Update()
{
	m_pLog->Update();
	return 0;
}

int ToolApp::LoadFromDB(int key)
{
	ExcelDB* pExcelDB = m_mapExcelDBs[m_strCurrentDB];
	ACCHECK(pExcelDB);

	MapCNameToValueT mapValues;
	pExcelDB->ReadDBRecord(key,mapValues);
	OnLoadFromDB(mapValues);
	return 0;
}

int ToolApp::SaveToDB(int key)
{
	ExcelDB* pExcelDB = m_mapExcelDBs[m_strCurrentDB];
	ACCHECK(pExcelDB);

	MapCNameToValueT mapValues;
	OnSaveToDB(mapValues);
	pExcelDB->WriteDBRecord(key,mapValues);
	return 0;
}

int ToolApp::MenuNew()
{
	if(m_bIsNewing)
		return -1;

	ExcelDB* pExcelDB = m_mapExcelDBs[m_strCurrentDB];
	ACCHECK(pExcelDB);

	ToolTree* pTree = pExcelDB->GetTree();
	ACCHECK(pTree);

	m_bIsNewing = true;
	pTree->EnableWindow(FALSE);

	m_pMenu->EnableMenuItem(ID_MENU_NEW,TRUE);
	m_pMenu->EnableMenuItem(ID_MENU_SAVE,FALSE);
	m_pMenu->EnableMenuItem(ID_MENU_DELETE,TRUE);
	m_pMenu->EnableMenuItem(ID_MENU_COPY,TRUE);
	m_pMenu->EnableMenuItem(ID_MENU_CANCEL,FALSE);

	// 载入默认值
	MapCNameToValueT mapDefault;
	pExcelDB->GetDBDefaultValue(mapDefault);
	OnLoadFromDB(mapDefault);

	CEdit* pKeyWnd = (CEdit*)GetCurrentKeyWindow();
	ACCHECK(pKeyWnd);

	pKeyWnd->EnableWindow(TRUE);

	// 设置key默认值
	int nNewKey = pExcelDB->GetUnusedKey();
	CString strNewKey;
	strNewKey.Format(_T("%d"),nNewKey);
	pKeyWnd->SetWindowText(strNewKey);
	return 0;
}

int ToolApp::MenuSave()
{
	CEdit* pKeyWnd = (CEdit*)GetCurrentKeyWindow();
	ACCHECK(pKeyWnd);

	CString strKey;
	pKeyWnd->GetWindowText(strKey);
	int nKey = atoi(CStringToStlString(strKey).c_str());

	if(m_bIsNewing)
	{
		if(InsertByKey(nKey) != 0)
		{
			WarningMessageBox(_T("ID无效"));
			pKeyWnd->SetFocus();
			return -1;
		}

		ExcelDB* pExcelDB = m_mapExcelDBs[m_strCurrentDB];
		ACCHECK(pExcelDB);

		ToolTree* pTree = pExcelDB->GetTree();
		ACCHECK(pTree);

		m_bIsNewing = false;
		pKeyWnd->EnableWindow(FALSE);
		pTree->EnableWindow(TRUE);

		m_pMenu->EnableMenuItem(ID_MENU_NEW,FALSE);
		m_pMenu->EnableMenuItem(ID_MENU_SAVE,FALSE);
		m_pMenu->EnableMenuItem(ID_MENU_DELETE,FALSE);
		m_pMenu->EnableMenuItem(ID_MENU_COPY,FALSE);
		m_pMenu->EnableMenuItem(ID_MENU_CANCEL,TRUE);
	}
	else
		SaveToDB(nKey);

	return 0;
}

int ToolApp::MenuDelete()
{
	CEdit* pKeyWnd = (CEdit*)GetCurrentKeyWindow();
	ACCHECK(pKeyWnd);

	CString strKey;
	pKeyWnd->GetWindowText(strKey);
	int nKey = atoi(CStringToStlString(strKey).c_str());

	ExcelDB* pExcelDB = m_mapExcelDBs[m_strCurrentDB];
	ACCHECK(pExcelDB);

	return pExcelDB->DeleteByKey(nKey);
}

int ToolApp::MenuCopy()
{
	if(m_bIsNewing)
		return -1;

	ExcelDB* pExcelDB = m_mapExcelDBs[m_strCurrentDB];
	ACCHECK(pExcelDB);

	ToolTree* pTree = pExcelDB->GetTree();
	ACCHECK(pTree);

	m_bIsNewing = true;
	pTree->EnableWindow(FALSE);

	m_pMenu->EnableMenuItem(ID_MENU_NEW,TRUE);
	m_pMenu->EnableMenuItem(ID_MENU_SAVE,FALSE);
	m_pMenu->EnableMenuItem(ID_MENU_DELETE,TRUE);
	m_pMenu->EnableMenuItem(ID_MENU_COPY,TRUE);
	m_pMenu->EnableMenuItem(ID_MENU_CANCEL,FALSE);

	CEdit* pKeyWnd = (CEdit*)GetCurrentKeyWindow();
	ACCHECK(pKeyWnd);

	pKeyWnd->EnableWindow(TRUE);

	// 设置key默认值
	int nNewKey = pExcelDB->GetUnusedKey();
	CString strNewKey;
	strNewKey.Format(_T("%d"),nNewKey);
	pKeyWnd->SetWindowText(strNewKey);
	return 0;
}

int ToolApp::MenuCancel()
{
	if(!IsNewing())
		return -1;

	ExcelDB* pExcelDB = m_mapExcelDBs[m_strCurrentDB];
	ACCHECK(pExcelDB);

	ToolTree* pTree = pExcelDB->GetTree();
	ACCHECK(pTree);
	
	LoadFromDB(pTree->GetSelectedKey());

	CEdit* pKeyWnd = (CEdit*)GetCurrentKeyWindow();
	ACCHECK(pKeyWnd);

	m_bIsNewing = false;
	pKeyWnd->EnableWindow(FALSE);
	pTree->EnableWindow(TRUE);

	m_pMenu->EnableMenuItem(ID_MENU_NEW,FALSE);
	m_pMenu->EnableMenuItem(ID_MENU_SAVE,FALSE);
	m_pMenu->EnableMenuItem(ID_MENU_DELETE,FALSE);
	m_pMenu->EnableMenuItem(ID_MENU_COPY,FALSE);
	m_pMenu->EnableMenuItem(ID_MENU_CANCEL,TRUE);
	return 0;
}

int ToolApp::InsertByKey(int key)
{
	ExcelDB* pExcelDB = m_mapExcelDBs[m_strCurrentDB];
	ACCHECK(pExcelDB);

	MapCNameToValueT mapDefault;
	pExcelDB->GetDBDefaultValue(mapDefault);

	MapCNameToValueT mapCurrent;
	OnSaveToDB(mapCurrent);

	for(MapCNameToValueT::iterator iter = mapCurrent.begin(); iter != mapCurrent.end(); ++iter)
	{
		CString strCName = iter->first;
		CString strValue = iter->second;

		MapCNameToValueT::iterator iterDefault = mapDefault.find(strCName);
		if(iterDefault != mapDefault.end())
			iterDefault->second = strValue;
	}

	if(pExcelDB->InsertByKey(key,mapDefault) > 0)
		return 0;
	else
		return -1;
}

void ToolApp::InsertCheckCombo(int nDlgID, CWnd* pCheckCombo)
{
	m_mapCheckCombos.insert(std::make_pair(nDlgID,pCheckCombo));
}

CWnd* ToolApp::FindCheckCombo(int nDlgID)
{
	std::map<int,CWnd*>::iterator iter = m_mapCheckCombos.find(nDlgID);
	if(iter != m_mapCheckCombos.end())
		return iter->second;

	return NULL;
}

ExcelDB* ToolApp::OpenExcelDB(const SExcelConfig& rConfig)
{
	ExcelDB* pExcelDB = new ExcelDB();
	ACCHECK(pExcelDB);

	if(pExcelDB->OpenDB(rConfig) != 0)
	{
		_safe_delete(pExcelDB);

		CString strErr;
		strErr.Format(_T("OpenExcelDB failed:%s"),rConfig.m_strExcelPath);
		ErrorMessageBox(strErr);
		ExitProcess(-1);
	}

	m_mapExcelDBs.insert(std::make_pair(rConfig.m_strExcelCName,pExcelDB));
	return pExcelDB;
}

void ToolApp::SetCurrentDB(const CString& strDBName)
{
	if(m_strCurrentDB == strDBName)
		return;

	if(!m_strCurrentDB.IsEmpty())
	{
		ExcelDB* pOldDB = m_mapExcelDBs[m_strCurrentDB];
		ACCHECK(pOldDB);

		ToolTree* pOldTree = pOldDB->GetTree();
		ACCHECK(pOldTree);

		pOldTree->ShowWindow(SW_HIDE);
	}

	m_strCurrentDB = strDBName;

	if(!m_strCurrentDB.IsEmpty())
	{
		ExcelDB* pNewDB = m_mapExcelDBs[m_strCurrentDB];
		ACCHECK(pNewDB);

		ToolTree* pNewTree = pNewDB->GetTree();
		ACCHECK(pNewTree);

		pNewTree->ShowWindow(SW_SHOW);
	}
}

END_NS_AC