#include "stdafx.h"
#include <Windows.h>
#include <winsvc.h>

#define ST TEXT("ServiceTest")

/************************************************
ȫ�ֱ���
************************************************/
WCHAR g_tszCurrentPath[MAX_PATH+1] = { 0 };

/************************************************
��ȡ��ǰ·��
************************************************/
bool GetCurrentPath()
{
	WCHAR tszTempPathName[MAX_PATH + 1] = { 0 };

	//��ȡ��ǰ·��

	if (GetModuleFileName(NULL, tszTempPathName, MAX_PATH) == 0)
		return false;

	PWCHAR ptszPtr = wcsrchr(tszTempPathName, TEXT('\\'));
	if (ptszPtr == NULL) return false;
	*ptszPtr = 0;
	wcscpy_s(g_tszCurrentPath, MAX_PATH, tszTempPathName);

	return true;
}

//��װ��������
int InstallService()
{
	int nRet = -1;
	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;
	SERVICE_STATUS InstallServiceStatus = { 0 };
	WCHAR tszServiceExePath[MAX_PATH+1] = { 0 };

	swprintf_s(tszServiceExePath, MAX_PATH, TEXT("%s\\ServiceTest.exe"), g_tszCurrentPath);

	//�򿪷���������ݿ�
	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (schSCManager == NULL)
	{
		goto Exit0;
	}
	//��������
	schService = CreateService(schSCManager)

Exit0:
}