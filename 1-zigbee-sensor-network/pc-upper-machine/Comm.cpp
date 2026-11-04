#include "stdafx.h"
#include "Comm.h"


void HW_Printf(const char * pFormat, ...)
{
	char info[1024] = {0};
	va_list args;
	
	//增加时间
//	CTime tm=CTime::GetCurrentTime();
//	sprintf(info, "[%02d:%02d:%02d]->", tm.GetHour(),tm.GetMinute(),tm.GetSecond());
	
	va_start(args, pFormat);
	vsprintf(&info[0], (const char*)pFormat, args);
	va_end(args);

	int len=strlen((const char *)info);

	
	OutputDebugString(info);
}


CMutex m_Mutex(FALSE,_T("mutex1"));

CSerialPort::CSerialPort()
{
	bStart=FALSE;
	hComm=INVALID_HANDLE_VALUE;
	memset(m_port, 0, sizeof(m_port));
}

CSerialPort::~CSerialPort()
{
	
}

void CSerialPort::checkCom()
{
    int index=atoi(&m_port[3]);
    if(index>=10)
    {
        TCHAR buff[50]={0};
        sprintf(buff, "//.//%s", m_port);
        memset(m_port, 0, sizeof(m_port));
        memcpy(m_port, buff, strlen(buff));
    }
}

BOOL CSerialPort::InitPort(CWnd* pPortOwner,	// the owner (CWnd) of the port (receives message)
						   char*  portnr,		// portnumber (1..4)
						   UINT  baud,			// baudrate
						   char  parity,		// parity 
						   UINT  databits,		// databits 
						   UINT  stopbits,		// stopbits 
						   DWORD dwCommEvents,	// EV_RXCHAR, EV_CTS etc
						   UINT  writebuffersize)	// size to the writebuffer
{
	m_pOwner=pPortOwner;
	return Open(portnr, 115200, 0, 8, 1);
}


BOOL CSerialPort::Open(LPCSTR port, int nBaudRate, int nParity, int nByteSize, int nStopBits)
{
	BOOL re=FALSE;
	
	if(port==NULL)
	{
		return FALSE;
	}
	
	if(strlen(port)==0) return FALSE;

	memset(m_port, 0, sizeof(m_port));
	strcpy( m_port, port );	

	checkCom();

	hComm = CreateFile(m_port,	// 串口名称或设备路径
			GENERIC_READ | GENERIC_WRITE,	// 读写方式
			0,				// 共享方式：独占
			NULL,			// 默认的安全描述符
			OPEN_EXISTING,	// 创建方式
			0,				// 不需设置文件属性
			NULL);			// 不需参照模板文件
	
	if(hComm == INVALID_HANDLE_VALUE)
	{
		DWORD Error = GetLastError();
		return FALSE;		// 打开串口失败
	}

	DCB dcb;		// 串口控制块
	re=GetCommState(hComm, &dcb);		// 取DCB
	if(!re)
	{
		HW_Printf("GetCommState error GetLastError=%d", GetLastError());
		CloseHandle(hComm);
		hComm = INVALID_HANDLE_VALUE;
		return FALSE;
	}


	
	dcb.BaudRate = nBaudRate;
	dcb.ByteSize = nByteSize;
	dcb.Parity = nParity;
	switch(nStopBits)
	{
		case 1:
			dcb.StopBits = ONESTOPBIT;
			break;
		case 2:
			dcb.StopBits = TWOSTOPBITS;
			break;
		default:
			dcb.StopBits = ONESTOPBIT;
			break;
	}

	re=SetCommState(hComm, &dcb);		// 设置DCB

	if(!re)
	{
		HW_Printf("SetCommState error GetLastError=%d", GetLastError());
		CloseHandle(hComm);
		hComm = INVALID_HANDLE_VALUE;
		return FALSE;
	}

	// 设置输入输出缓冲区大小
	re=SetupComm(hComm, 1024, 1024);	
	if(!re)
	{
		HW_Printf("SetupComm error GetLastError=%d", GetLastError());
		CloseHandle(hComm);
		hComm = INVALID_HANDLE_VALUE;
		return FALSE;
	}


	// 设置超时
	//读字符有二种超时1、A的时间  2、C+B*读取的字符数
	//写字符超时  E+D*写字符数
	COMMTIMEOUTS timeouts = {	// 串口超时控制参数
		10,		//A 读字符间隔超时时间: 100 ms
		1,			//B 读操作时每字符的时间: 1 ms (n个字符总共为n ms)
		10,		//C 基本的(额外的)读超时时间: 500 ms
		1,			//D 写操作时每字符的时间: 1 ms (n个字符总共为n ms)
		10};		//E 基本的(额外的)写超时时间: 100 ms
	re=SetCommTimeouts(hComm, &timeouts);	
	if(!re)
	{
		HW_Printf("SetCommTimeouts error GetLastError=%d", GetLastError());
		CloseHandle(hComm);
		hComm = INVALID_HANDLE_VALUE;
		return FALSE;
	}
	//clear
	PurgeComm(hComm, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_TXABORT);
	
	bStart=TRUE;
	return TRUE;
}

BOOL CSerialPort::Close()
{
	BOOL re = FALSE;


	m_Mutex.Lock();	
	if(hComm != INVALID_HANDLE_VALUE)
	{
		re = CloseHandle(hComm);
		hComm = INVALID_HANDLE_VALUE;
		bStart=FALSE;
	}
	
	m_Mutex.Unlock();
	return re;
}

int CSerialPort::ReadComm(void* pData, int nLength)
{
	DWORD dwNumRead=0;	// 串口收到的数据长度

	m_Mutex.Lock();
	if(hComm != INVALID_HANDLE_VALUE)
	{
		ReadFile(hComm, pData, (DWORD)nLength, &dwNumRead, NULL);
	}
	
	m_Mutex.Unlock();

	return (int)dwNumRead;
}


int CSerialPort::WriteToPort(BYTE* pData, int nLength)
{
	DWORD dwNumWrite=0;	// 串口发出的数据长度

	m_Mutex.Lock();
	
	if(hComm != INVALID_HANDLE_VALUE)
	{
		WriteFile(hComm, pData, (DWORD)nLength, &dwNumWrite, NULL);
	}

	m_Mutex.Unlock();

	return (int)dwNumWrite;
}

int CSerialPort::WriteComm(void* pData, int nLength)
{
	DWORD dwNumWrite=0;	// 串口发出的数据长度

	m_Mutex.Lock();

	if(hComm != INVALID_HANDLE_VALUE)
	{
		WriteFile(hComm, pData, (DWORD)nLength, &dwNumWrite, NULL);
	}


	m_Mutex.Unlock();

	return (int)dwNumWrite;
}

BOOL CSerialPort::isStart()
{
	BOOL re=FALSE;
	m_Mutex.Lock();
	
	if(bStart && hComm != INVALID_HANDLE_VALUE)
	{
		re = TRUE;
	}

	m_Mutex.Unlock();

	return re;
}

void CSerialPort::ClosePort()
{
	Close();
}

BOOL CSerialPort::StartMonitoring()
{
	if (!(m_Thread = AfxBeginThread(CommThread, this)))
		return FALSE;
	TRACE("Thread started\n");
	return TRUE;	
}

UINT CSerialPort::CommThread(LPVOID pParam)
{
	CSerialPort* port=(CSerialPort*)pParam;
	BYTE com_buff[50]={0};
	BYTE readCount=0;
	
	for (;;) 
	{
		if(!port->isStart())
		{
			AfxEndThread(100);
			break;
		}
		
		readCount=port->ReadComm(com_buff, 20);

		for(int i=0; i<readCount; i++)
		{
			::PostMessage((port->m_pOwner)->m_hWnd, WM_COMM_RXCHAR, (WPARAM) com_buff[i], (LPARAM) 0);
		}
	}

	return 0;
}
