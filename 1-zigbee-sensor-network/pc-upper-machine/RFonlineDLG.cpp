// RFonlineDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RFonline.h"
#include "RFonlineDlg.h"
#include <atlbase.h>//首先要包含头文件

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//#define DATA_FORM_WIFI

#define PHONE_DEFAULT_PORT    33333   //手机连接默认的port
#define WIFI_DEFAULT_PORT    8080   //wifi网关默认的port

#define BUFFER_SIZE     256

SOCKET ServerSock;
SOCKET g_ClientSock;
char szBuf[BUFFER_SIZE];
char strTxBuf[BUFFER_SIZE];
BYTE strRxBuf[BUFFER_SIZE];
BYTE ucRecvLen=0;                //报文长度
int	 RxIndex = 0;	             //接收数据下标 计数
BYTE LampAllState=0;             //默认第一次打开所有灯
CRFonlineDlg* pDlg=NULL;

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRFonlineDlg dialog


CRFonlineDlg::CRFonlineDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRFonlineDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRFonlineDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRFonlineDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRFonlineDlg)
	DDX_Control(pDX, IDC_BTN_OPEN_COM, m_btnCom);
	DDX_Control(pDX, IDC_COMBOPORT, m_ComSel);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CRFonlineDlg, CDialog)
	//{{AFX_MSG_MAP(CRFonlineDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_OPEN_COM, OnBtnOpenCom)
	ON_MESSAGE(WM_COMM_RXCHAR, OnComm)
	//}}AFX_MSG_MAP
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRFonlineDlg message handlers

BOOL CRFonlineDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here

	//获取系统中的串口个数
	int SerialCount=GetSystemPorts(pNameList,pPortList);
	//向Combo Box中添加串口个数
	for(int j=0;j<SerialCount;j++)
	{
		((CComboBox*)GetDlgItem(IDC_COMBOPORT))->AddString(pPortList[j]);
	}

  m_ComSel.SetCurSel(0);      //默认串口为Com1
  m_bComState = FALSE;        //初始化串口为关闭状态
  lastCh=0;
	

	if( !my_CFont.CreatePointFont( 100,_T("宋体"),NULL ) )
		return FALSE;
	GetDlgItem (IDC_STATIC_TEMP1)->SetFont( &my_CFont );
	GetDlgItem (IDC_STATIC_TEMP2)->SetFont( &my_CFont );
	GetDlgItem (IDC_STATIC_TEMP3)->SetFont( &my_CFont );

  pDlg=this;

  memset(&endDevInfo, 0, sizeof(EndDeviceDataInfo));
  RefreshData();      //初始化画面上所有终端的数据


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CRFonlineDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CRFonlineDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CRFonlineDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

LONG CRFonlineDlg::OnComm(WPARAM wp, LPARAM port)
{
	char buff[4]={0};
    BYTE ch=(BYTE)wp;

    if(!m_bComState)
    {
        return 0;
    }

    if(lastCh=='$' && ch=='@')
    {
    	if(RxIndex>=strRxBuf[0])
    	{
    	    BYTE len=strRxBuf[0];
            BYTE xor=0;

            //计算校验，不算第1，第2字节
            xor=CheckSum(&strRxBuf[2], len-2);

            //校验一致
            if(strRxBuf[1]==xor)
            {
                parseData(strRxBuf, RxIndex);
            }
            else
            {
                OutputDebugString("**********************parse11 error************\r\n");
            }
            
    	}
        else
        {
            OutputDebugString("**********************parse22 error************\r\n");
        }

        //复位，重新接收
        RxIndex=0;
        memset(strRxBuf, 0, BUFFER_SIZE);        
    }
    else
    {
        //存储数据
    	strRxBuf[RxIndex] = (BYTE)ch;
    	RxIndex++;

        if(RxIndex>=BUFFER_SIZE)
        {
            //复位，重新接收
            RxIndex=0;
            memset(strRxBuf, 0, BUFFER_SIZE);
            OutputDebugString("**********************parse33 error************\r\n");
        }
    }


    lastCh=ch;
    
	return 0;
}

BOOL CRFonlineDlg::parseData(BYTE* data, BYTE len)
{
    if(data==NULL)
    {
        return FALSE;
    }

    BYTE fc=data[2];		//功能码

    switch(fc)
    {

        case FUN_CODE_UPDATA_DATA://收到zigbee的数据
        {
          BYTE id=data[3];//取出ID
          if(id==1)
          {
            endDevInfo.end1_wendu=data[4];//终端1温度
            endDevInfo.end1_shidu=data[5];//终端1湿度
          }
          else if(id==2)
          {
            endDevInfo.end2_wendu=data[4];//终端1温度
            endDevInfo.end2_shidu=data[5];//终端1湿度
          }
          else if(id==3)
          {
            endDevInfo.end3_wendu=data[4];//终端1温度
            endDevInfo.end3_shidu=data[5];//终端1湿度
          }
        }
        break;
        default:
        break;
    }
    
    //刷新界面
    RefreshData();

    return TRUE;
}


BOOL CRFonlineDlg::RefreshData()
{
  char buff[100]={0};

  sprintf(buff, "终端一，温度：%d，湿度：%d", endDevInfo.end1_wendu,endDevInfo.end1_shidu);
  ((CStatic*)GetDlgItem(IDC_STATIC_TEMP1))->SetWindowText(buff);
  
  sprintf(buff, "终端二，温度：%d，湿度：%d", endDevInfo.end2_wendu,endDevInfo.end2_shidu);
  ((CStatic*)GetDlgItem(IDC_STATIC_TEMP2))->SetWindowText(buff);

  sprintf(buff, "终端三，温度：%d，湿度：%d", endDevInfo.end3_wendu,endDevInfo.end3_shidu);
  ((CStatic*)GetDlgItem(IDC_STATIC_TEMP3))->SetWindowText(buff);
  return TRUE;
}

int CRFonlineDlg::GetSystemPorts(CString *pNameList, CString *pPortList)
{
	CRegKey RegKey;   
	int nCount = 0;  

	//打开注册表
	if(RegKey.Open(HKEY_LOCAL_MACHINE, "Hardware\\DeviceMap\\SerialComm") == ERROR_SUCCESS)   
	{   
		while(true)   
		{   
			char ValueName[_MAX_PATH];   
			unsigned char ValueData[_MAX_PATH];   
			DWORD nValueSize = _MAX_PATH;   
			DWORD nDataSize = _MAX_PATH;   
			DWORD nType;   
			
			//通过枚举获取，如果再无其他项则终止
			if(::RegEnumValue(HKEY(RegKey), nCount, ValueName, &nValueSize, NULL, &nType, ValueData, &nDataSize) == ERROR_NO_MORE_ITEMS)   
			{   
				break;   
			}   
						
			if(pNameList)   //名称
				pNameList[nCount] = ValueName;   
			
			if(pPortList)   //值
				pPortList[nCount] = ValueData;   
			
			nCount++;       //继续探索下一个
		}   
	}   

	//win7有权限限制，默认显示串口吧
	if(0==nCount)
	{
		char buff[10]={0};

		for(int i=0; i<MAX_COM; i++)
		{
			sprintf(buff, "COM%d", i+1);

			if(pNameList)   //名称
				pNameList[nCount] = buff;   
			
			if(pPortList)   //值
				pPortList[nCount] = buff;   
			
			nCount++;       //继续探索下一个			
		}
		
	}

	
	return nCount;
	
}

BEGIN_EVENTSINK_MAP(CRFonlineDlg, CDialog)
    //{{AFX_EVENTSINK_MAP(CRFonlineDlg)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

//int g_index=1;
void CRFonlineDlg::OnBtnOpenCom() 
{
	CString StrPort = pPortList[m_ComSel.GetCurSel()];
	
	if (!m_bComState)
	{
		if (m_SerialPort.InitPort(this, StrPort.GetBuffer(5), 115200, 'N', 8, 1, EV_RXCHAR|EV_RXFLAG, 512)) //115200   
		{
			m_bComState = TRUE;
			m_SerialPort.StartMonitoring(); //启动串口监视线程
			m_btnCom.SetWindowText("关闭串口"); 
		} 
		else
		{
			AfxMessageBox("没有发现,或被其它设备占用");
			m_bComState = FALSE;
			m_btnCom.SetWindowText("打开串口");
		}
	} 
	else
	{
		m_SerialPort.ClosePort();
		m_bComState = FALSE;        //初始化串口为关闭状态
		m_btnCom.SetWindowText("打开串口"); 
	}
}

BYTE CRFonlineDlg::XorCheckSum(BYTE * pBuf, BYTE len)
{
	BYTE i;
	BYTE byRet=0;
	
	if(len == 0)
		return byRet;
	else
		byRet = pBuf[0];
	
	for(i = 1; i < len; i ++)
		byRet = byRet ^ pBuf[i];
	
	return byRet;
}

void CRFonlineDlg::OnClose(SOCKET ClientSock)
{
	closesocket(ClientSock);
	WSACleanup();
}

BYTE CRFonlineDlg::CheckSum(BYTE *pdata, BYTE len)
{
	BYTE i;
	BYTE check_sum=0;

	for(i=0; i<len; i++)
	{
		check_sum += pdata[i];
	}
	return check_sum;
}


HBRUSH CRFonlineDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_TEMP1
   ||pWnd->GetDlgCtrlID() == IDC_STATIC_TEMP2
   ||pWnd->GetDlgCtrlID() == IDC_STATIC_TEMP3)
	{
		pDC->SetBkMode(TRANSPARENT); //设置背景为透明
		pDC->SetTextColor(RGB(0,0,255));//设置字体颜色
		//pWnd->SetFont( &my_CFont );
	}

	return hbr;
}
