// RFonlineDlg.h : header file
//
//{{AFX_INCLUDES()
//}}AFX_INCLUDES

#include <afxmt.h>


//#include "SerialPort.h"

#include "Comm.h"
#include "afxwin.h"
#define SYNC_SERIALPORT



#if !defined(AFX_RFonlineDLG_H__F83334FE_3526_411C_844F_2B9076DFDB93__INCLUDED_)
#define AFX_RFonlineDLG_H__F83334FE_3526_411C_844F_2B9076DFDB93__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define NETWORK_EVENT WM_USER+166 //定义网络事件
#define MAX_COM     100

/////////////////////////////////////////////////////////////////////////////
// CRFonlineDlg dialog

//功能码定义
#define FUN_CODE_SET_DATA				0x01	//手机/PC-->zigbee 设置数据
#define FUN_CODE_UPDATA_DATA		0x02	//zigbee-->手机/PC 上传所有传感器数据
#define FUN_CODE_UPDATA_RFID		0x03	//zigbee-->手机/PC 上传RFID


//每一个终端上的设备数据结构
typedef struct _end_device_data_info_
{
    BYTE end1_wendu;//终端1温度
    BYTE end1_shidu;//终端1湿度
    BYTE end2_wendu;//终端2温度
    BYTE end2_shidu;//终端2湿度
    BYTE end3_wendu;//终端3温度
    BYTE end3_shidu;//终端3湿度
}EndDeviceDataInfo;

class CRFonlineDlg : public CDialog
{
// Construction
public:
    BOOL RefreshData();
    BOOL parseData(BYTE* data, BYTE len);
    BYTE XorCheckSum(BYTE * pBuf, BYTE len);
    int GetSystemPorts(CString *pNameList, CString *pPortList);
    CRFonlineDlg(CWnd* pParent = NULL);	// standard constructor
    BYTE CheckSum(BYTE *pdata, BYTE len);
    void packDataAndSend(int fc, BYTE* data, int len);
    
    //网络事件处理函数	
    void OnClose(SOCKET CurSock);       //关闭Socket	

#ifdef SYNC_SERIALPORT
	CSerialPort m_SerialPort;
#else
	CSerialPort m_SerialPort;
#endif
    BOOL m_bComState;          //串口状态 打开时为TRUE 关闭时为FALSE 
    CString pNameList[MAX_COM];
    CString pPortList[MAX_COM];
    BYTE lastCh;//串口或者wifi收到的最后一个字符

	EndDeviceDataInfo  endDevInfo;

// Dialog Data
	//{{AFX_DATA(CRFonlineDlg)
	enum { IDD = IDD_WSN_DIALOG };
	CListBox	m_ListBox;
	CButton	m_btnCom;
	CComboBox	m_ComSel;
	CFont my_CFont;

	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRFonlineDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	LONG OnComm(WPARAM ch, LPARAM port);

	// Generated message map functions
	//{{AFX_MSG(CRFonlineDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBtnOpenCom();
  
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RFonlineDLG_H__F83334FE_3526_411C_844F_2B9076DFDB93__INCLUDED_)
