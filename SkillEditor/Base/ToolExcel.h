#ifndef _TOOL_EXCEL_H_
#define _TOOL_EXCEL_H_

#include "ToolDef.h"
#include "ToolApp.h"

#include "ToolExcelDef.h"

#include "stdafx.h"
#include "CApplication.h"
#include "CWorkbooks.h"
#include "CWorkbook.h"
#include "CWorksheets.h"
#include "CWorksheet.h"
#include "CRange.h"

BEGIN_NS_AC

// 将行号列好转成Cell的名字，如(1000,52) = "AZ1000"
CString MakeCellName(int nRow,int nCol);

class ExcelWorkbook
{
public:
	ExcelWorkbook(CString strPath,LPDISPATCH workbook);
	virtual ~ExcelWorkbook();
public:
	int GetSheetCount();
	int GetUsedRowCount(int sheetidx);
	int GetUsedColumnCount(int sheetidx);
public:
	CString GetCellText(int sheetidx,int row,int col);
	void SetCellText(int sheetidx,int row,int col,CString val);
public:
	void SortAllSheetByColumn(int sortByCol,int startRow);
public:
	void DeleteRow(int sheetidx,int row);
	void GetRowText(int sheetidx,int row,std::vector<CString>& vtStr);
	void InsertRow(int sheetidx,int row,std::vector<CString>& vtStr);
	void InsertEmptyRow(int sheetidx,int row);
	void AppendEmptyRow(int sheetidx);
public:
	void Save();
	void Close();
private:
	CString m_strPath;
	CWorkbook m_objWorkbook;
	CWorksheets m_objWorkSheets;
};

class ToolExcel : public Module<ToolApp>
{
public:
	ToolExcel(ToolApp* app);
	virtual ~ToolExcel();
public:
	int CreateExcelServer();
	int DestroyExcelServer();
public:
	ExcelWorkbook* OpenWorkbook(CString strPath);
protected:
	typedef std::map<CString,ExcelWorkbook*> MapNameToWorkbookT;
private:
	CApplication m_objApplication;
	CWorkbooks m_objWorkbooks;
	MapNameToWorkbookT m_mapWorkbooks;
};

END_NS_AC

#endif//_TOOL_EXCEL_H_