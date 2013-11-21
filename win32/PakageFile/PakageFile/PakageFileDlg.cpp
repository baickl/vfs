
// PakageFileDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "PakageFile.h"
#include "PakageFileDlg.h"
#include "RunApp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CPakageFileDlg 对话框




CPakageFileDlg::CPakageFileDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPakageFileDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPakageFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_lcList);
	DDX_Control(pDX, IDC_EDIT_DEBUG, m_editOutput);
}

BEGIN_MESSAGE_MAP(CPakageFileDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CPakageFileDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CPakageFileDlg::OnBnClickedCancel)
	ON_WM_DROPFILES()
    ON_BN_CLICKED(IDC_BUTTON_SELECT_A, &CPakageFileDlg::OnBnClickedButtonSelectA)
    ON_BN_CLICKED(IDC_BUTTON_SELECT_R, &CPakageFileDlg::OnBnClickedButtonSelectR)
    ON_BN_CLICKED(IDC_BUTTON_CLEANUP, &CPakageFileDlg::OnBnClickedButtonCleanup)
END_MESSAGE_MAP()


// CPakageFileDlg 消息处理程序

BOOL CPakageFileDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	DWORD dwStyle = m_lcList.GetExtendedStyle(); //获取当前扩展样式
	dwStyle |= LVS_EX_FULLROWSELECT; //选中某行使整行高亮（report风格时）
	dwStyle |= LVS_EX_GRIDLINES; //网格线（report风格时）
	dwStyle |= LVS_EX_CHECKBOXES; //勾选项（report风格时）
	m_lcList.SetExtendedStyle(dwStyle); //设置扩展风格

	m_lcList.InsertColumn(0, _T("文件名"), LVCFMT_LEFT, 700); //插入列
	m_lcList.InsertColumn(1, _T("状态"), LVCFMT_LEFT, 150);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

BOOL PackDir(CString strApp,CString strDir,CString& strDebug)
{

	CString szParam;
	szParam.Format(L" %s ",  strDir);

	CString strOutputFile = strDir+ L".pak";
	DeleteFile(strOutputFile);

	CRunApp rApp;
	rApp.Run(strApp, szParam, NULL,SW_SHOW);
	CString strResult = rApp;

	if( !PathFileExists(strOutputFile) )
	{
		strDebug.AppendFormat( L"PackDir Dir=\"%s\" Failed\r\n",strDir );
		if( !strResult.IsEmpty() )
		{
			strDebug += strResult;
			strDebug += L"\r\n";
		}
		return FALSE;
	}

	strDebug.AppendFormat( L"PackDir Dir=\"%s\" OK\r\n",strDir );
	if( !strResult.IsEmpty() )
	{
		strDebug += strResult;
		strDebug += L"\r\n";
	}
	return TRUE;

}

void CPakageFileDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CPakageFileDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CPakageFileDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CPakageFileDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	m_Thread.ThreadExecute.bind(this,&CPakageFileDlg::PackDirThread);
	m_Thread.Start();
}

void CPakageFileDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

void CPakageFileDlg::OnDropFiles(HDROP hDropInfo)
{
	// TODO: Add your message handler code here and/or call default

	UINT count;
	TCHAR filePath[MAX_PATH];

	count = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
	if(count)
	{
		for( UINT i = 0; i<count; ++i )
		{
			DragQueryFile(hDropInfo, i, filePath, sizeof(filePath));
			if( PathFileExists(filePath) && PathIsDirectory(filePath) )
			{

                //检查目录是否已经存在
                BOOL bExist = FALSE;
                std::vector<std::wstring>::iterator it = m_FileList.begin();
                for(;it != m_FileList.end(); ++it )
                {
                    std::wstring strDir = *it;
                    if( strDir == filePath )
                    {
                        bExist = TRUE;
                        break;
                    }
                }

                if( bExist )
                    continue;

				m_FileList.push_back(filePath);
				int nRow = m_lcList.InsertItem(m_lcList.GetItemCount(),filePath);
				m_lcList.SetItemText(nRow, 1,L"末打包");
                m_lcList.SetCheck(nRow,TRUE);
				InvalidateRect(NULL);
			}
		}
	}

	DragFinish(hDropInfo);

	CDialog::OnDropFiles(hDropInfo);
}


bool CPakageFileDlg::PackDirThread()
{
	CString strApp = L"./tools/pack_dir.exe";
	std::vector<std::wstring>::iterator it = m_FileList.begin();
	CString strDir;
	int nIndex = 0 ;

	m_editOutput.SetWindowText(L"");

	CString strDebug;
	for(;it != m_FileList.end(); ++it )
	{
		strDir = (*it).c_str();

        if( !m_lcList.GetCheck(nIndex))
            continue;

		m_lcList.SetItemText(nIndex, 1,L"正在打包");
		if( PackDir(strApp,strDir,strDebug) )
		{
			m_lcList.SetItemText(nIndex, 1,L"打包成功");
		}
		else
		{
			m_lcList.SetItemText(nIndex, 1,L"打包失败");
		}

		m_editOutput.SetWindowText(strDebug);

		int nLength=m_editOutput.SendMessage(WM_GETTEXTLENGTH);   
		m_editOutput.SetSel(nLength,   nLength);         //将光标置于最后   
		++nIndex;

	}

	return false;
}
void CPakageFileDlg::OnBnClickedButtonSelectA()
{
    // TODO: Add your control notification handler code here

    std::vector<std::wstring>::iterator it = m_FileList.begin();
    int nIndex = 0 ;
    CString strDebug;
    for(;it != m_FileList.end(); ++it )
    {
        m_lcList.SetCheck(nIndex,TRUE);
        ++nIndex;
    }

    InvalidateRect(NULL);
}

void CPakageFileDlg::OnBnClickedButtonSelectR()
{
    // TODO: Add your control notification handler code here

    std::vector<std::wstring>::iterator it = m_FileList.begin();
    int nIndex = 0 ;
    CString strDebug;
    for(;it != m_FileList.end(); ++it )
    {
        m_lcList.SetCheck(nIndex,!m_lcList.GetCheck(nIndex));
        ++nIndex;
    }

    InvalidateRect(NULL);
}

void CPakageFileDlg::OnBnClickedButtonCleanup()
{
    // TODO: Add your control notification handler code here
    m_FileList.clear();
    m_lcList.DeleteAllItems();

    m_editOutput.SetWindowText(L"");
    int nLength=m_editOutput.SendMessage(WM_GETTEXTLENGTH);   
    m_editOutput.SetSel(nLength,   nLength);         //将光标置于最后   

    InvalidateRect(NULL);
}
