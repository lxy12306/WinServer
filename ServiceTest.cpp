#include <tchar.h>
#include <Windows.h>

#define SERVICE_NAME  TEXT("TestService")
#define TIMEOUT 0x1000

//�����ȫ�ֱ���
SERVICE_STATUS g_ServiceStatus;
SERVICE_STATUS_HANDLE g_hServiceStatusHandle;

//������ȫ�ֱ���
HANDLE g_hThread = NULL;
HANDLE g_hExitEvent = NULL;
bool g_bThreadExit = false;

void WINAPI Service_Ctrl(DWORD dwCtrlCode);
void WINAPI Service_Main(DWORD dwArgc, LPTSTR *plpszArgv);
bool WINAPI ServiceStart(DWORD dwArgc, LPTSTR *plpszArgv);
bool WINAPI ServiceStop();
bool WINAPI ServicePause();
bool WINAPI ServiceContinue();
bool ReportStatusToSCMgr(DWORD dwCurrentStatus, DWORD dwWin32ExitCode, DWORD dwWaitHint);
void DbgPrint(const wchar_t *wstrOutputString, ...)
{
	va_list vlargs = NULL;
	va_start(vlargs, wstrOutputString);
	size_t nLen = _vscwprintf(wstrOutputString, vlargs) + 1;

	wchar_t *wszBuffer = new wchar_t[nLen];
	_vsnwprintf_s(wszBuffer, nLen, nLen, wstrOutputString, vlargs);
	va_end(vlargs);
	OutputDebugString(wszBuffer);
	delete[] wszBuffer;	
}
void WINAPI StartTestThread();
DWORD WINAPI TestThreadFun(LPVOID pvParam);

int _tmain(int argc, _TCHAR* argv[])
{
	SERVICE_TABLE_ENTRY steDispatchTable[] = {
		{
			SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)Service_Main
		}, {NULL,NULL}
	};

	if (!StartServiceCtrlDispatcher(steDispatchTable))
	{
		DbgPrint(TEXT("StartServiceCtrlDispatcher failed ErrorCode = %d\r\n", GetLastError()));
	}
	else
	{
		DbgPrint(TEXT("StartServiceCtrlDispatcher Success!!\r\n "));
	}
	return 0;
}
void WINAPI Service_Main(DWORD dwArgc, LPTSTR *plpszArgv)
{
	g_ServiceStatus.dwServiceType = SERVICE_WIN32; //��ǰ���������
	g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING; // ��ǰ�����״̬
	g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;//ָ��������ܺ��ֿ���

	//������Ϣ
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwServiceSpecificExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;
	g_ServiceStatus.dwWaitHint = 0;

	//ע�������ƴ������
	g_hServiceStatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, Service_Ctrl);
	//���ע��ʧ��
	if (g_hServiceStatusHandle == NULL)
	{
		goto CleanUp;
	}

	//���·���״̬
	if (!ReportStatusToSCMgr(SERVICE_START_PENDING, NO_ERROR, TIMEOUT))
		goto CleanUp;
	ServiceStart(dwArgc, plpszArgv);
		return;
CleanUp:
	//�ѷ���״̬���µ� S
		if (g_hServiceStatusHandle)
			ReportStatusToSCMgr(SERVICE_STOPPED, GetLastError(), 0);
}
void WINAPI Service_Ctrl(DWORD dwCtrlCode)
{
	//��������������
	switch (dwCtrlCode)
	{
	case SERVICE_CONTROL_STOP:
	{
		ReportStatusToSCMgr(SERVICE_STOP_PENDING, NO_ERROR, TIMEOUT);
		ServiceStop();
		return;
	}break; //�ȸ��·���״̬Ϊ SERVICE_STOP_PENDING;��ֹͣ����
	case SERVICE_CONTROL_PAUSE:
	{
		ReportStatusToSCMgr(SERVICE_STOP_PENDING, NO_ERROR, TIMEOUT);
		ServicePause();
		g_ServiceStatus.dwCurrentState = SERVICE_PAUSED;
	}break;
	case SERVICE_CONTROL_CONTINUE://��ͣ����
	{
		ReportStatusToSCMgr(SERVICE_CONTINUE_PENDING, NO_ERROR, TIMEOUT);
		ServiceContinue();	
		g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	}break;
	case SERVICE_CONTROL_INTERROGATE://���·���״̬
	{

	}break;
	default:
		break;
	}
	ReportStatusToSCMgr(g_ServiceStatus.dwCurrentState, NO_ERROR, 0);
}
bool WINAPI ServiceStart(DWORD dwArgc, LPTSTR *plpszArgv)
{
	DbgPrint(TEXT("���Է���ʼ\r\n"));

	StartTestThread();

	if (!ReportStatusToSCMgr(SERVICE_RUNNING, NO_ERROR, TIMEOUT))
		return false;
	return true;
}
bool WINAPI ServiceStop()
{
	DbgPrint(TEXT("����ֹͣ\r\n"));

	g_bThreadExit = true;

	DWORD dwRet = WaitForSingleObject(g_hExitEvent, INFINITE);
	if (dwRet == WAIT_OBJECT_0)
	{
		DbgPrint(TEXT("�߳��˳��ɹ�\r\n"));
	}
	else
	{
		DbgPrint(TEXT("�߳��˳�ʧ�� Error Code =%d\r\n"),GetLastError());
	}

	CloseHandle(g_hExitEvent);
	g_hExitEvent = NULL;
	CloseHandle(g_hThread);
	g_hThread = NULL;

	return ReportStatusToSCMgr(SERVICE_STOPPED, NO_ERROR, TIMEOUT);
		
}

bool WINAPI ServicePause()
{
	DbgPrint(TEXT("���Է�����ͣ\r\n"));
	SuspendThread(g_hThread);
	return ReportStatusToSCMgr(SERVICE_PAUSED, NO_ERROR, TIMEOUT);
}
bool WINAPI ServiceContinue()
{
	DbgPrint(TEXT("���Է����������\r\n"));
	ResumeThread(g_hThread);
	return ReportStatusToSCMgr(SERVICE_RUNNING, NO_ERROR, TIMEOUT);
}

bool ReportStatusToSCMgr(DWORD dwCurrentStatus, DWORD dwWin32ExitCode, DWORD dwWaitHint)
{
	g_ServiceStatus.dwCurrentState = dwCurrentStatus;
	g_ServiceStatus.dwCheckPoint = 0;
	g_ServiceStatus.dwWin32ExitCode = dwWin32ExitCode;
	g_ServiceStatus.dwWaitHint = dwWaitHint;

	if (!SetServiceStatus(g_hServiceStatusHandle, &g_ServiceStatus))
	{
		DbgPrint("SetServiceStatus Error\r\n");
		return false;
	}
	return true;
}
void WINAPI StartTestThread()
{
	g_hThread = CreateThread(NULL, 0, TestThreadFun, NULL, 0, NULL);
}
DWORD WINAPI TestThreadFun(LPVOID pvParam)
{
	int i = 0;
	g_hExitEvent = CreateEvent(NULL, FALSE, FALSE, nullptr);

	while (1)
	{
		DbgPrint(TEXT("Test Count = %d\r\n"), i);

		if (g_bThreadExit)
			break;
		++i;
		Sleep(TIMEOUT);
	}
	SetEvent(g_hExitEvent);

	return 0;
}