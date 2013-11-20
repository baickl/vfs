
// PakageFileDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include <string>
#include <vector>
#include "afxwin.h"
#include "MyThread.h"


// CPakageFileDlg 对话框
class CPakageFileDlg : public CDialog
{
// 构造
public:
	CPakageFileDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_PAKAGEFILE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CListCtrl m_lcList;
	afx_msg void OnDropFiles(HDROP hDropInfo);
	std::vector<std::wstring> m_FileList;
	CEdit m_editOutput;

	bool PackDirThread();

	mylib::CThread m_Thread;
};
