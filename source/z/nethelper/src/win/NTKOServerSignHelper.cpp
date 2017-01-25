#include "ntkomemstr.h"
#include "NTKOServerSignHelper.h"
#include "NTKOUtil.h"
#include "Res/resource.h"
#include "NTKOMUIStrs.h"
#include "ntkocmndlg.h"
#include "ntkoprgmacro.h"
#include "keygen.h"
#include "MD5Class.h"
#include "base64.h"

#ifdef _WIN64
#include "ntkoproxyhelper.h"
#endif

//EXTERN CNtkoMuiHelper * v_pNtkoMUIHelper;
EXTERN HANDLE g_hModule;
EXTERN BOOL   v_fUnicodeAPI ;
EXTERN BOOL   v_fWindows2KPlus;
EXTERN BOOL   v_fWindowsXPPlus;
EXTERN HICON g_signListDlgICON;

CNTKOServerSignHelper::CNTKOServerSignHelper(void)
{
	ZeroMemory(&m_secsignSettings,sizeof(NtkoSecsignSettings));
	ZeroMemory(&m_serverRightSettings,sizeof(NtkoServerSecsignRightSettings));
	ZeroMemory(&m_publishRightSettings, sizeof(NtkoPublishRightSettings));

	m_IsLogin= FALSE;
	ZeroMemory(m_LoginUserName,sizeof(m_LoginUserName));
	ZeroMemory(m_LoginPassword,sizeof(m_LoginPassword));
	ZeroMemory(m_LoginRealName,sizeof(m_LoginRealName));
	ZeroMemory(m_rootSvrUrl,sizeof(m_rootSvrUrl));
	m_pNTkoSecstrPublishInfo = NULL;
	m_pNtkoSignEkey = NULL;
	m_pNtkoUIDelayDlg = NULL;
	m_PublishInfo_AUTOUseEkeyLogin = FALSE;
	m_HwndHiddenForMsg = NULL;
	m_hwndParent= NULL;
	ZeroMemory(m_EKEYPassword,sizeof(m_EKEYPassword));
	m_bstrEKEYSN = NULL;
	m_appType = NTKOAPPType_Other;
	m_pXMLDoc = NULL;
	// m_iServerRequestType = NTKOServerRequestType_DosignLog;
	m_bstrServerVersion = NULL;
	m_bstrServerInfoVersion = NULL;
	m_bstrServerInfoName = NULL;
	m_bstrServerInfoLicenseUserName = NULL;
	m_bstrServerInfoLicenseCount = NULL;
	m_bstrThisServerInfoName = NULL;
	m_iThisServerInfoServerId = 0;
	m_iServerErrCode = 0;
	m_bstrFailReason = NULL;
	m_bstrServerTime = NULL;
	ZeroMemory(&m_NtkoUserSigns,sizeof(m_NtkoUserSigns));
	m_iUserSignsCount = 0 ;
// 	ZeroMemory(&m_docSigns,sizeof(m_docSigns));
// 	m_iDocSignsCount = 0;
	ZeroMemory(&m_NtkoCheckSignServers,sizeof(m_NtkoCheckSignServers));
	m_iCheckSignServersCount = 0;
	ZeroMemory(&m_NtkoServerSigns,sizeof(m_NtkoServerSigns));
	m_iServerSignsCount = 0;
	m_iCurServerId = 0;
	m_hwndCheckSignDlg = NULL;
	m_pNtkoMuiHelper =NULL;
	
	m_dwReceiveDataTimeOut = 0;
	m_dwConnectServerTimeOut= 0;
	m_hLargeSelSign = NULL;
	m_hSmallSelSign = NULL;
	m_selectedSignIndex = -1;	//盖章时服务器印章选择
	m_iSelectedSignIndex = -1; //检查服务器所有印章对话框选择
	m_hModulePublishInfo = NULL;
	m_IsLogin = FALSE;
	m_hbmpError = NULL;
	m_hbmpOk = NULL;

	m_pwszSignFilter = NULL;

	m_dwHiddenCheckItems = 0;

	m_iSignSvrLicType = ntkoSignSvrLicTypeAll;
	ZeroMemory(&m_Custom_jiangshuCA_CompanyInfo, sizeof(NTKO_CUSTOM_jiangshuCA_CompanyInfo));

	m_bstr_custom_ChuanDaZhiSheng_UserCertSN = NULL;
	InitNtkoCmnLib((HINSTANCE)g_hModule,936,NTKO_DEFAULT_THEME,NTKO_LANGID_AUTO);
}

BOOL CNTKOServerSignHelper::DoLogin(LPWSTR username, LPWSTR password,BOOL bUseEkey)
{
	if (m_IsLogin)
	{
		return TRUE;
	}

	BOOL isOk = FALSE;
	BOOL bUseEkeyLogin = bUseEkey;

	//登录时需要清除证书,否则无法重新选择证书了。
	ReleaseCurCert();
	ReleaseCurStore();

	m_iThisServerInfoServerId = 0;

#if (!defined(NTKO_CUSTOM_SHANGHAI_WEITAI)) && (!defined(NTKO_CUSTOM_JIANGSU_CA)) && (!defined(NTKO_CUSTOM_DOLOGIN_USECERT))

	INT userNameLen = lstrlenW(username);
	INT passwordLen = lstrlenW(password);

	if (userNameLen>0 && userNameLen<= NTKO_MAX_LOGINUSERNAME)
	{
		ZeroMemory(m_LoginUserName,sizeof(m_LoginUserName));
		CopyMemory(m_LoginUserName, username, userNameLen*sizeof(WCHAR));
	}
	
	if (passwordLen>0 && passwordLen<= NTKO_MAX_LOGINUSERNAME)
	{
		ZeroMemory(m_LoginPassword,sizeof(m_LoginPassword));
		CopyMemory(m_LoginPassword, password, passwordLen*sizeof(WCHAR));
	}

#if defined(NTKO_LICENSE_PANGANG)	
	//bUseEkey = TRUE;
#endif
	if(m_PublishInfo_AUTOUseEkeyLogin || bUseEkey)
	{
		//m_secsignSettings.bUseEkeyLogin = TRUE;
		bUseEkeyLogin = TRUE;
	}
	else
	{
		if (0==userNameLen || userNameLen> NTKO_MAX_LOGINUSERNAME || 
			0==passwordLen || passwordLen> NTKO_MAX_LOGINUSERNAME)
		{
			HRESULT ret = NtkoShowMiddleOkCancelFrameDlg(m_hwndParent,CNTKOServerSignHelper::LoginDlgFrameHandler,this);
			if (IDOK != ret)
			{
				return FALSE;
			}
			bUseEkeyLogin = m_secsignSettings.bUseEkeyLogin;
		}
	}

	if ( bUseEkeyLogin )
	{
		NtkoCmnDlgParam dlgParam;
		ZeroMemory(&dlgParam,sizeof(dlgParam));
#if defined(NTKO_LICENSE_PANGANG)
		dlgParam.dwFlags = NTKO_CDPF_VERIFYEKEY | NTKO_CDPF_GETEKEYSN;
#else
		dlgParam.dwFlags = NTKO_CDPF_VERIFYEKEY | NTKO_CDPF_GETEKEYSN | NTKO_CDPF_GETEKEYLOGININFO;
#endif
		dlgParam.EkeyType = m_secsignSettings.ekeyType;
		dlgParam.EkeyVersion = NTKO_EKEYVER_AUTO;
		dlgParam.EkeyIndex = -1;
		
		if (m_PublishInfo_AUTOUseEkeyLogin)
		{
			dlgParam.bIsUseExistSignEkeyObj = TRUE;
			dlgParam.pExistNtkoSignEkey = m_pNtkoSignEkey;
		}

		int dlgRet = IDCANCEL;
		//提示选择EKEY
		LPWSTR pwszTitle = LoadMUIStringW(IDSMUI_SELEKEYDLGTITLE);
		dlgRet = NtkoShowGetEkeyAndPassDlg(m_hwndParent,m_EKEYPassword,NTKO_MINEKEY_PASSWORD,NTKO_MAXEKEY_PASSWORD,pwszTitle,&dlgParam);
		SAFE_FREESTRING(pwszTitle);
		if ( IDOK != dlgRet)
		{	
			SAFE_FREEBSTRING(dlgParam.bstrEkeySN);		
			SAFE_FREEBSTRING(dlgParam.bstrEkeyLoginUserName);	
			SAFE_FREEBSTRING(dlgParam.bstrEkeyLoginPassword);	
			return FALSE;
		}

		if (NTKO_EKEYUSAGE_EKEY_ALL!=dlgParam.dwEkeyUsage && 
			NTKO_EKEYUSAGE_EKEY_LOGIN!=(dlgParam.dwEkeyUsage&NTKO_EKEYUSAGE_EKEY_LOGIN) )
		{
			ShowMsgBoxMUIStrW(IDSMUI_EKEYMES_ERR_USAGE,IDSMUI_MSGBOXTITLE_TIP,MB_OK | MB_ICONINFORMATION);
			SAFE_FREEBSTRING(dlgParam.bstrEkeySN);		
			SAFE_FREEBSTRING(dlgParam.bstrEkeyLoginUserName);	
			SAFE_FREEBSTRING(dlgParam.bstrEkeyLoginPassword);
			return FALSE;
		}

		NtkoCopyStringToDst(m_LoginUserName,dlgParam.bstrEkeyLoginUserName,sizeof(m_LoginUserName));
		NtkoCopyStringToDst(m_LoginPassword,dlgParam.bstrEkeyLoginPassword,sizeof(m_LoginPassword));
		m_bstrEKEYSN = ::SysAllocString(dlgParam.bstrEkeySN);
				
		SAFE_FREEBSTRING(dlgParam.bstrEkeySN);			
		SAFE_FREEBSTRING(dlgParam.bstrEkeyLoginUserName);	
		SAFE_FREEBSTRING(dlgParam.bstrEkeyLoginPassword);	
	}

#elif defined(NTKO_CUSTOM_SHANGHAI_WEITAI)
	m_secsignSettings.bUseEkeyLogin = FALSE;
	LPWSTR pwszCertName = NULL;
	LPWSTR pwszCertSN = NULL;
	isOk = GetCertificateLogonInfo(m_hwndParent,&pwszCertName,&pwszCertSN);
	if(!isOk)
	{
		SAFE_FREESTRING(pwszCertName);	
		SAFE_FREESTRING(pwszCertSN);
		return FALSE;
	}

	WCHAR wszWinUser[1024];
	ZeroMemory(wszWinUser,sizeof(wszWinUser));
	DWORD dwBufSize = sizeof(wszWinUser)/sizeof(wszWinUser[0]);
	isOk = GetUserNameW(wszWinUser,&dwBufSize);
	if(!isOk)
	{
		SAFE_FREESTRING(pwszCertName);	
		SAFE_FREESTRING(pwszCertSN);
		return FALSE;
	}

#if defined(_DEBUG)
	MessageBoxW(m_hwndParent,pwszCertName,pwszCertSN,MB_OK);
	MessageBoxW(m_hwndParent,wszWinUser,L"wszWinUser=",MB_OK);
#endif

#if 0 //不比较是否相同。只是用操作系统登录
	isOk = NtkoAreStringsEqualW(pwszCertName,-1,wszWinUser,-1);
	if(!isOk)
	{
		SAFE_FREESTRING(pwszCertName);	
		SAFE_FREESTRING(pwszCertSN);
		MyMessageBoxUseMUIStrW(hwndDlgParent,IDSMUI_LOGINSERVER_FAIL_USERNAMEINVALID,
						IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		return FALSE;
	}
	lstrcpyW(m_LoginUserName,pwszCertName);
#else
	lstrcpyW(m_LoginUserName,wszWinUser);
#endif
	lstrcpyW(m_LoginPassword,pwszCertSN);

	SAFE_FREESTRING(pwszCertName);	
	SAFE_FREESTRING(pwszCertSN);


#elif ( defined(NTKO_CUSTOM_JIANGSU_CA) || defined(NTKO_CUSTOM_DOLOGIN_USECERT) ) //江苏CA双向登录，不需要传递用户名和口令，用证书验证.但是后台限制，因此传入windows用户为用户名
	WCHAR wszWinUser[1024];
	ZeroMemory(wszWinUser,sizeof(wszWinUser));
	DWORD dwBufSize = sizeof(wszWinUser)/sizeof(wszWinUser[0]);
	isOk = GetUserNameW(wszWinUser,&dwBufSize);
	if(0 == lstrlenW(wszWinUser))
	{
		return FALSE;
	}
	lstrcpyW(m_LoginUserName,wszWinUser);
	lstrcpyW(m_LoginPassword,L"11111111");

#endif

	BOOL isGetRootServerOk = TRUE;

	m_serverReqestType = NTKOServerRequestType_GetRootServerInfo; //1:获取根服务器信息。
	ShowDelayDlgMUIStrW(IDSMUI_GETTINGSERVER_INFO);
	BSTR bstrRetXML = NULL;
	isOk = DoWebGet(m_secsignSettings.serverURL,NTKO_HC_GETROOTSERVER_URL,&bstrRetXML);
	CloseDelayDlg();

	if(isOk && bstrRetXML) //下载成功且有返回值,需要分析看是否正确的返回值
	{
		m_serverReqestType = NTKOServerRequestType_GetRootServerInfo; //1:获取根服务器信息。
		isOk = ParseXML(bstrRetXML);
		if(isOk) //解析成功，但是服务器有可能有错误
		{ 
			if( 0 == m_iServerErrCode)
			{
				if(NtkoAreStringsEqualW(m_bstrServerVersion,-1,NTKO_ServerSecsignControlProtocalVersion,-1))
				{
					isOk = TRUE;
				}
				else
				{
					ShowMsgBoxMUIStrW(IDSMUI_LOGINSERVER_FAIL_INVALIDSERVER_VERSION,IDSMUI_MSGBOXTITLE_TIP,MB_OK|MB_ICONINFORMATION);
					isOk = FALSE;
					isGetRootServerOk = FALSE;
				}
			}
			else //服务器返回有错误
			{
				WCHAR wszBuf[1024];
				LPWSTR pwszFmt = LoadMUIStringW(IDSMUI_LOGINSERVER_FAIL_SERVERFAILWITHCODE);
				NtkoFwsprintf(wszBuf,pwszFmt,m_iServerErrCode,m_bstrFailReason);
				SAFE_FREESTRING(pwszFmt);

				LPWSTR pwszMUITitle = LoadMUIStringW(IDSMUI_MSGBOXTITLE_TIP);
				NtkoShowMsgBox(m_hwndParent,wszBuf,pwszMUITitle ,MB_OK|MB_ICONINFORMATION);
				SAFE_FREESTRING(pwszMUITitle);
				isOk = FALSE;
				isGetRootServerOk = FALSE;
			}
		}
		else //解析错误,可能返回的是错误页面。因此可能老版本服务器,不改变登录服务器
		{

		}
	}
	else //下载失败,老版本服务器,不改变登录服务器
	{

	}

	if(isOk)
	{
		isOk = ParseRootServerInfo();
		if(!isOk)
		{
			ShowMsgBoxMUIStrW(IDSMUI_LOGINSERVER_FAIL_PARSERESPONSEERROR,
				IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
			isGetRootServerOk = FALSE;
		}

		if(isOk)
		{
			isOk = ParseThisServerInfo();
			if(!isOk)
			{
				ShowMsgBoxMUIStrW(IDSMUI_LOGINSERVER_FAIL_PARSERESPONSEERROR,
					IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
				isGetRootServerOk = FALSE;
			}
		}
	}	
	SAFE_FREEBSTRING(bstrRetXML);

	if(!isGetRootServerOk)
	{
		return FALSE;
	}


#if defined(NTKO_CUSTOM_JIANGSU_CA)
	WCHAR wszCPARA[1024*8];
	#if defined(NTKO_CUSTOM_JIANGSU_CA_NO_HTTPS)
		if(!SelectLogonCertNoHTTPS(TRUE) || (!m_pCurSignCert))
		{
			return FALSE;
		}
	#endif
	if(m_pCurSignCert)
	{		
		LPWSTR pwszCertName = NtkoCertGetNameStringW(m_pCurSignCert,CERT_NAME_SIMPLE_DISPLAY_TYPE,0);
		NTKO_Custom_SetComName(pwszCertName);
		lstrcpyW(m_LoginUserName,pwszCertName);
		SAFE_FREESTRING(pwszCertName);			
	}
#elif defined(NTKO_CUSTOM_DOLOGIN_USECERT)
	WCHAR wszCPARA[1024*8];
	if(!SelectLogonCertNoHTTPS(FALSE) || (!m_pCurSignCert))
	{
		return FALSE;
	}
	if(m_pCurSignCert)
	{		
		LPWSTR pwszCertName = NtkoCertGetNameStringW(m_pCurSignCert,CERT_NAME_SIMPLE_DISPLAY_TYPE,0);
		lstrcpyW(m_LoginUserName,pwszCertName);
		SAFE_FREESTRING(pwszCertName);	
		LPWSTR pwszCertSN = NtkoCertGetSNStringW(m_pCurSignCert,NULL);
		lstrcpyW(m_LoginPassword,pwszCertSN);
		SAFE_FREESTRING(pwszCertSN);		
	}
#else
	WCHAR wszCPARA[1024];
#endif
	//构造参数
	ZeroMemory(&wszCPARA[0],sizeof(wszCPARA));
	lstrcatW(wszCPARA,L"username=");
	lstrcatW(wszCPARA,m_LoginUserName);
	lstrcatW(wszCPARA,L"&password=");

#if defined(NTKO_CUSTOM_JIANGSU_CA)
	LPWSTR pwszTmpSignedMsg = GenTempSignedPKCS7Msg();
	lstrcatW(wszCPARA,pwszTmpSignedMsg);
	SAFE_FREESTRING(pwszTmpSignedMsg);

	#if defined(NTKO_CUSTOM_JIANGSU_CA_NO_HTTPS)
		if(m_pCurSignCert)
		{		
			lstrcatW(wszCPARA,L"&certsn=");
			LPWSTR pwszCertSN = NtkoCertGetSNStringW(m_pCurSignCert,NULL);
			lstrcatW(wszCPARA,pwszCertSN);
			SAFE_FREESTRING(pwszCertSN);
		}
	#endif
#else
	lstrcatW(wszCPARA,m_LoginPassword);
#endif

	lstrcatW(wszCPARA,L"&clientver=");
	lstrcatW(wszCPARA,NTKO_ServerSecsignControlProtocalVersion);


	if(m_secsignSettings.bUseEkeyLogin)
	{
		lstrcatW(wszCPARA,L"&ekeysn=");
		lstrcatW(wszCPARA,(LPWSTR)m_bstrEKEYSN);
	}

	WCHAR wszLicType[1024];
	ZeroMemory(&wszLicType[0],sizeof(wszLicType));
	NtkoFwsprintf(wszLicType,L"&lictype=%d",m_iSignSvrLicType);
	lstrcatW(wszCPARA,wszLicType);


	//向服务器提交请求
	ShowDelayDlgMUIStrW(IDSMUI_LOGINGINSERVER);

	isOk = DoWebExecute2(m_rootSvrUrl,NTKO_HC_LOGIN_URL,wszCPARA,&bstrRetXML);	
	
	if(isOk && bstrRetXML)
	{			
		ShowDelayDlgMUIStrW(IDSMUI_PREPARE_USERENV);	

		m_serverReqestType = NTKOServerRequestType_Login; //3:用户登录
		m_serverRightSettings.bUseHandsign = FALSE;
		m_serverRightSettings.bUseKeyboardsign = FALSE;
		m_serverRightSettings.bUseNormalsign = FALSE;
		m_serverRightSettings.bUseEkeySign = FALSE;
		isOk = ParseXML(bstrRetXML);

		CloseDelayDlg();

		//MyMessageBoxW(hwndDlgParent, bstrRetXML,pwszMUITitle ,MB_OK|MB_ICONINFORMATION);
		if(isOk) //解析成功，但是服务器有可能有错误
		{ 
			if( 0 == m_iServerErrCode) 
			{
				if(NtkoAreStringsEqualW(m_bstrServerVersion,-1,NTKO_ServerSecsignControlProtocalVersion,-1))
				{
					isOk = TRUE;
				}
				else
				{
					ShowMsgBoxMUIStrW(IDSMUI_LOGINSERVER_FAIL_INVALIDSERVER_VERSION,
						IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
					isOk = FALSE;
				}
			}
			else //服务器返回有错误
			{
				WCHAR wszBuf[1024];
				LPWSTR pwszFmt = LoadMUIStringW(IDSMUI_LOGINSERVER_FAIL_SERVERFAILWITHCODE);
				NtkoFwsprintf(wszBuf,pwszFmt,m_iServerErrCode,m_bstrFailReason);
				SAFE_FREESTRING(pwszFmt);

				LPWSTR pwszMUITitle = LoadMUIStringW(IDSMUI_MSGBOXTITLE_TIP);
				NtkoMessageBoxW(m_hwndParent,wszBuf,pwszMUITitle ,MB_OK|MB_ICONINFORMATION);
				SAFE_FREESTRING(pwszMUITitle);
				isOk = FALSE;
			}
		}
		else //解析错误
		{
			ShowMsgBoxMUIStrW(IDSMUI_LOGINSERVER_FAIL_PARSERESPONSEERROR,
				IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
			isOk = FALSE;
		}
	}
	else
	{			
		CloseDelayDlg();
		ShowMsgBoxMUIStrW( IDSMUI_LOGINSERVER_FAIL,IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		isOk = FALSE;
	}

	if(isOk) 
	{
		isOk = ParseThisServerInfo();
		if(!isOk)
		{
			ShowMsgBoxMUIStrW(IDSMUI_LOGINSERVER_FAIL_PARSERESPONSEERROR,
				IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		}
		else
		{
			ParseThisUserInfo();
		}
	}

#if defined(NTKO_CUSTOM_CHUANDA_ZHISHENG)
	if (isOk)
	{
		isOk = NTKO_Custom_ParseCertSN(); //
		if(!isOk)
		{
			ShowMsgBoxMUIStrW(IDSMUI_LOGINSERVER_FAIL_PARSERESPONSEERROR,
				IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		}
	}
#endif


#if defined(NTKO_CUSTOM_JIANGSU_CA)

#if 0 //初次版本的江苏ca请求。现在的版本客户端不需要直接和seal服务器打交道，禁用，保留代码备查
	if (isOk) 
	{	
		isOk = NTKO_Custom_ParseThisSealServerInfo();
		if (!isOk) 
		{
			ShowMsgBoxMUIStrW(IDSMUI_LOGINSERVER_FAIL_PARSERESPONSEERROR,
				IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		}
	}
#endif

	if (isOk)
	{
		isOk = ParseSignOptions();
		if(!isOk)
		{
			ShowMsgBoxMUIStrW(IDSMUI_LOGINSERVER_FAIL_PARSERESPONSEERROR,
				IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		}
	}
#endif

	SAFE_FREEBSTRING(bstrRetXML);

	m_IsLogin = isOk;
	return isOk;
}


INT_PTR CNTKOServerSignHelper::LoginDlgFrameHandler(HWND hDlg,HWND hwndTab, 
				UINT Msg,WPARAM wParam,LPARAM lParam,LPVOID pUserParam)
{
	CNTKOServerSignHelper* pHelper = (CNTKOServerSignHelper*)pUserParam;
	switch (Msg) 
	{ 
	case WM_INITDIALOG:
		{
			if(pHelper)
			{
				LPWSTR pwszTitle = pHelper->LoadMUIStringW(IDSMUI_LOGINDLG_TITLE);	
				NtkoSetWindowTextW(hDlg,pwszTitle);
				SAFE_FREESTRING(pwszTitle);
				pwszTitle = pHelper->LoadMUIStringW(IDSMUI_CMDCAP_LOGONSVR);
				NtkoAddDlgTabPage((HINSTANCE)g_hModule,hDlg,hwndTab,IDD_DIALOGLOGIN,
					(DLGPROC)CNTKOServerSignHelper::LoginDlgTabProc,pwszTitle,(LPARAM)pHelper);
				SAFE_FREESTRING(pwszTitle);
				
				NtkoShowDlgTabPage(hwndTab,0);
				
				return FALSE;
			}
			else
			{
				return TRUE;
			}
		}
		return TRUE;
	} 
	return FALSE;
}



INT_PTR CNTKOServerSignHelper::LoginDlgTabProc(HWND hDlg, UINT Msg, WPARAM wParam,LPARAM lParam)
{
#ifdef _WIN64	// zhuguanglin add 20150313
	CNTKOServerSignHelper* pHelper = (CNTKOServerSignHelper*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
#else
	CNTKOServerSignHelper* pHelper = (CNTKOServerSignHelper*)GetWindowLong(hDlg, GWL_USERDATA);
#endif
	HWND hDlgParent = GetParent(hDlg);
	switch (Msg) 
	{ 
		case WM_INITDIALOG:
			{
				BOOL retValue = TRUE;
				if (NULL != lParam)
				{				
					pHelper = (CNTKOServerSignHelper*)lParam;
#ifdef _WIN64	// zhuguanglin add 20150313
					SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)pHelper);
#else
					SetWindowLong(hDlg, GWL_USERDATA, (LONG)pHelper);
#endif
					if(pHelper)
					{
						//初始化控件标题
						pHelper->SetMUIItemStrW(hDlg,IDC_STATICUSERNAME,IDCMUI_STATICUSERNAME);
						pHelper->SetMUIItemStrW(hDlg,IDC_STATICPASSWORD,IDCMUI_STATICPASSWORD);
						pHelper->SetMUIItemStrW(hDlg,IDC_CHECKREMEMBERUSER,IDCMUI_CHECKREMEMBERUSER);
						pHelper->SetMUIItemStrW(hDlg,IDC_CHECKUSEEKEY,IDCMUI_CHECKUSEEKEY);
						pHelper->SetMUIItemStrW(hDlg,IDC_STATICINFO_INSERTEKEY,IDCMUI_STATICINFO_INSERTEKEY);

						pHelper->SetMUIItemStrW(hDlgParent,IDOK,IDCMUI_BUTTON_LOGIN);

						CheckDlgButton(hDlg,IDC_CHECKUSEEKEY,pHelper->m_secsignSettings.bUseEkeyLogin);
						if(pHelper->m_secsignSettings.bUseEkeyLogin)
						{
							NtkoSetDlgItemTextW(hDlg,IDC_EDITUSERNAME,L"");
							CheckDlgButton(hDlg,IDC_CHECKREMEMBERUSER,FALSE);
							pHelper->m_secsignSettings.bRememberUserName = FALSE;
						}
						else
						{
							CheckDlgButton(hDlg,IDC_CHECKREMEMBERUSER,pHelper->m_secsignSettings.bRememberUserName);
						}

						if(pHelper->m_secsignSettings.bRememberUserName)
						{
							LPWSTR pwszUserName =NtkoANSIToLPWSTR(pHelper->m_secsignSettings.rememberedUserName);
							NtkoSetDlgItemTextW(hDlg,IDC_EDITUSERNAME,pwszUserName);
							if( lstrlenW(pwszUserName) )
							{
								SetFocus(GetDlgItem(hDlg,IDC_EDITPASSWORD));
								retValue = FALSE;
							}
							SAFE_FREESTRING(pwszUserName);
						}
						SendDlgItemMessage(hDlg,IDC_EDITUSERNAME,EM_LIMITTEXT,NTKO_MAX_LOGINUSERNAME,0);
						SendDlgItemMessage(hDlg,IDC_EDITPASSWORD,EM_LIMITTEXT,NTKO_MAX_PASSWORD,0);
						//刷新界面按钮可用状态		
						SendMessage(hDlg,WM_COMMAND,MAKEWPARAM(IDC_CHECKUSEEKEY,BN_CLICKED),0);				
					}
				}
				return retValue;
			}
			break;

		case WM_CTLCOLORDLG:
			{
				if(pHelper)
				{
					HDC hdcControl = (HDC)wParam;
					SetBkMode(hdcControl,TRANSPARENT) ;
					return (int)GetStockObject(NTKO_TABEDDLG_BKCOLOR);
				}
			}
			break;
		case WM_CTLCOLORSTATIC:
			{
				int ictlID = GetDlgCtrlID((HWND)lParam);
				HDC hdc = (HDC)wParam;
				switch(ictlID)
				{
				case IDC_STATICINFO_INSERTEKEY:
					{
						SetTextColor(hdc,NTKO_HILIGHT_TEXTCOLOR);	
						SetBkMode(hdc, TRANSPARENT);
						return (int)GetStockObject(NTKO_TABEDDLG_BKCOLOR);
					}
					break;		
				default:
					SetBkMode(hdc, TRANSPARENT);
					return (int)GetStockObject(NTKO_TABEDDLG_BKCOLOR);
				}
			}
			break;
		case WM_COMMAND: 
			switch (LOWORD(wParam)) 
			{ 
				case IDOK: 		
					{
						pHelper->m_secsignSettings.bRememberUserName = IsDlgButtonChecked(hDlg,IDC_CHECKREMEMBERUSER);

						NtkoGetDlgItemTextW(hDlg,IDC_EDITUSERNAME,pHelper->m_LoginUserName,NTKO_MAX_LOGINUSERNAME+1);
						NtkoGetDlgItemTextW(hDlg,IDC_EDITPASSWORD,pHelper->m_LoginPassword,NTKO_MAX_PASSWORD+1);

						if(pHelper->m_secsignSettings.bRememberUserName)
						{
							LPSTR tempStr = NtkoLPWSTRToANSI(pHelper->m_LoginUserName);
							lstrcpyn(pHelper->m_secsignSettings.rememberedUserName,tempStr,sizeof(pHelper->m_secsignSettings.rememberedUserName));
							SAFE_FREESTRING(tempStr);
							
							WriteRegisterInfo(HKEY_CURRENT_USER, "CLSID\\"NTKOSecsignConfigHk, 
								NTKO_HC_SETTING_RememberedUserName,pHelper->m_secsignSettings.rememberedUserName);
						}
						else
						{
							ZeroMemory(pHelper->m_secsignSettings.rememberedUserName, sizeof(pHelper->m_secsignSettings.rememberedUserName));
							WriteRegisterInfo(HKEY_CURRENT_USER, "CLSID\\"NTKOSecsignConfigHk, 
								NTKO_HC_SETTING_RememberedUserName,"");
						}
						//保存使用ekey登录信息
						pHelper->m_secsignSettings.bUseEkeyLogin= IsDlgButtonChecked(hDlg,IDC_CHECKUSEEKEY);	
						WriteRegisterLong(HKEY_CURRENT_USER, "CLSID\\"NTKOSecsignConfigHk, 
							NTKO_HC_SETTING_IsRememberUserName,pHelper->m_secsignSettings.bRememberUserName);
						WriteRegisterLong(HKEY_CURRENT_USER, "CLSID\\"NTKOSecsignConfigHk, 
							NTKO_HC_SETTING_IsUseEkeyLogin,	pHelper->m_secsignSettings.bUseEkeyLogin);
						break;				
					}
				case IDCANCEL:
					{
						//EndDialog(hDlg, IDCANCEL);
						break;
					}
				case IDC_CHECKUSEEKEY:	
					{
						if(BN_CLICKED == (HIWORD(wParam))  )
						{
							BOOL useEkey = IsDlgButtonChecked(hDlg,IDC_CHECKUSEEKEY);
							if(useEkey)
							{
								NtkoSetDlgItemTextW(hDlg,IDC_EDITUSERNAME,L"");
								CheckDlgButton(hDlg,IDC_CHECKREMEMBERUSER,FALSE);
							}
							EnableWindow(GetDlgItem(hDlg,IDC_EDITUSERNAME),(!useEkey));
							EnableWindow(GetDlgItem(hDlg,IDC_EDITPASSWORD),(!useEkey));
							EnableWindow(GetDlgItem(hDlg,IDC_CHECKREMEMBERUSER),(!useEkey));
							ShowWindow(GetDlgItem(hDlg,IDC_STATICUSERNAME),useEkey?SW_HIDE:SW_SHOW); 
							ShowWindow(GetDlgItem(hDlg,IDC_STATICPASSWORD),useEkey?SW_HIDE:SW_SHOW); 
							ShowWindow(GetDlgItem(hDlg,IDC_EDITUSERNAME),useEkey?SW_HIDE:SW_SHOW); 
							ShowWindow(GetDlgItem(hDlg,IDC_EDITPASSWORD),useEkey?SW_HIDE:SW_SHOW); 
							ShowWindow(GetDlgItem(hDlg,IDC_STATICINFO_INSERTEKEY),useEkey?SW_SHOW:SW_HIDE);
							SendMessage(hDlg,WM_COMMAND,MAKEWPARAM(IDC_EDITUSERNAME,EN_CHANGE),0);
						}
						break;				
					}
				case IDC_EDITUSERNAME:
				case IDC_EDITPASSWORD:
					{
						if(EN_CHANGE == HIWORD(wParam))
						{
							char buf[NTKO_MAX_LOGINUSERNAME*2+1];
							if( IsDlgButtonChecked(hDlg,IDC_CHECKUSEEKEY) ||
								(
								GetDlgItemText(hDlg,IDC_EDITUSERNAME,buf,NTKO_MAX_LOGINUSERNAME*2) &&
								GetDlgItemText(hDlg,IDC_EDITPASSWORD,buf,NTKO_MAX_LOGINUSERNAME*2) 
								)
								)
							{
								EnableWindow(GetDlgItem(hDlgParent,IDOK),TRUE);
								SendMessage(hDlgParent, DM_SETDEFID, (WPARAM) IDOK, (LPARAM) 0); 
								SendMessage(GetDlgItem(hDlgParent,IDCANCEL),BM_SETSTYLE,(WPARAM) BS_PUSHBUTTON,(LPARAM)TRUE);
							}
							else
							{
								EnableWindow(GetDlgItem(hDlgParent,IDOK),FALSE);
							}
							InvalidateRect(GetDlgItem(hDlgParent,IDOK),NULL,TRUE);
							break;
						}
					}
					break;
				default:					
					break;
			}  //end switch
			break;
	} 
	return FALSE;
}

LPWSTR CNTKOServerSignHelper::LoadMUIStringW(DWORD dwKey)
{
	LPWSTR pwszRet = NtkoLoadMUIStringW(m_pNtkoMuiHelper,dwKey);
	if(!pwszRet)
	{
		pwszRet = NtkoCopyStringW(NTKO_DEFAULT_NOFOUND_TEXT);
	}
	return pwszRet;
}

VOID CNTKOServerSignHelper::SetMUIItemStrW(HWND hDlg,UINT idItem,DWORD dwKey)
{
	LPWSTR pwszText = LoadMUIStringW(dwKey);
	if(pwszText)
	{
		NtkoSetDlgItemTextW(hDlg,idItem,pwszText);
		SAFE_FREESTRING(pwszText);
	}
	else
	{
		NtkoSetDlgItemTextW(hDlg,idItem,NTKO_DEFAULT_NOFOUND_TEXT);
	}
}
void  CNTKOServerSignHelper::SetSignSvrLicType(NtkoSignSvrLicType iSignSvrLicType)
{
	m_iSignSvrLicType = iSignSvrLicType;
}
BOOL CNTKOServerSignHelper::InitNew(HWND hwndParent,HMODULE hModuleIncludePublishData)
{
//	if (!hModuleIncludePublishData || !hwndParent || !IsWindow(hwndParent))
	if (!hwndParent || !IsWindow(hwndParent))
	{
		return FALSE;
	}

	if(m_hwndParent!=hwndParent)
	{
		ODS_NTKOLIB("CNTKOServerSignHelper::InitNew already inited!\n");
		m_hwndParent = hwndParent;
	}
	
	//hModuleIncludePublishData == NULL 时正常情况不应进入该，要修改初始化配置项
	//暂时让其进入，以便不单独进行初始化（针对永中版本）
	if (hModuleIncludePublishData!=m_hModulePublishInfo || hModuleIncludePublishData == NULL)
	{
		m_hModulePublishInfo = hModuleIncludePublishData;
		NTKO_SAFEFreePoint(m_pNTkoSecstrPublishInfo);
		m_pNTkoSecstrPublishInfo = new CNtkoSecStrs();

		if (!m_pNTkoSecstrPublishInfo)	return FALSE;

		m_pNTkoSecstrPublishInfo->LoadFromModule(hModuleIncludePublishData);

		m_PublishInfo_AUTOUseEkeyLogin = NtkoSecChk_SecSignControl_AutoEkeyLogon(m_pNTkoSecstrPublishInfo,FALSE);
		m_secsignSettings.ekeyType = NtkoSecChk_General_GetDefautEkeyType(m_pNTkoSecstrPublishInfo,NTKO_DEFAULT_EKEY_TYPE) ;

		if (m_PublishInfo_AUTOUseEkeyLogin)
		{
			NTKO_SAFEFreePoint(m_pNtkoSignEkey);

			m_pNtkoSignEkey = new CNtkoSignEkey(m_secsignSettings.ekeyType);

			if(m_pNtkoSignEkey)
			{
				if (CreateHiddenWindowForMsg())
				{
					m_pNtkoSignEkey->GetEkeyList();
					m_pNtkoSignEkey->BeginListenSlotsEvent(Notify_SlotChanged,this);
				}
			}
		}
		
		// 读取发布时,确定的印章服务器url
		LPWSTR pwszDefServerURL = NtkoSecChk_General_GetSecString(m_pNTkoSecstrPublishInfo,
			NTKO_CONSTKEY_SECSIGN_DEF_SERVERURL,
			NTKO_DEFAULT_NOLOAD_SERVERURL);

		LPSTR pszDefServerURL = NtkoLPWSTRToANSI(pwszDefServerURL);
		m_pNTkoSecstrPublishInfo->MySafeFreeString(pwszDefServerURL);

		lstrcpy(m_secsignSettings.serverURL,pszDefServerURL);
		SAFE_FREESTRING(pszDefServerURL);

		NTKO_SAFEFreePoint(m_pNtkoUIDelayDlg);
		m_pNtkoUIDelayDlg = new CNtkoUIDelayDlg;

		m_hwndParent = hwndParent;
		ReadNtkoSecsignSettings(&m_secsignSettings);
		lstrcpyn(m_EKEYPassword,NTKO_SIGN_INITUSERPIN,sizeof(m_EKEYPassword));

		lstrcpyn(m_rootSvrUrl,m_secsignSettings.serverURL,sizeof(m_rootSvrUrl));

		m_secsignSettings.CurLangId = NtkoSecChk_General_GetDefaultLangId(m_pNTkoSecstrPublishInfo,m_secsignSettings.CurLangId);

		m_dwReceiveDataTimeOut = 180;
		m_dwConnectServerTimeOut = 180;

		//FreeNtkoCmnLib();
		SetNtkoCmnLibLangId(m_secsignSettings.CurLangId);		

		NTKO_SAFEFreePoint(m_pNtkoMuiHelper);
		m_pNtkoMuiHelper = new CNtkoMuiHelper;

		if (m_pNtkoMuiHelper)
		{
			m_pNtkoMuiHelper->InitFromResource((HINSTANCE)g_hModule, IDR_MUISTRS);
			m_pNtkoMuiHelper->SetResStrLocale(m_secsignSettings.CurLangId);
		}


		m_hbmpError = (HBITMAP)LoadImage((HINSTANCE)g_hModule, MAKEINTRESOURCE(IDB_BITMAPERROR), IMAGE_BITMAP, 0, 0,LR_LOADTRANSPARENT|LR_LOADMAP3DCOLORS);
		m_hbmpOk = (HBITMAP)LoadImage((HINSTANCE)g_hModule, MAKEINTRESOURCE(IDB_BITMAPOK), IMAGE_BITMAP, 0, 0,LR_LOADTRANSPARENT|LR_LOADMAP3DCOLORS);

		//load support functions
		LPWSTR pwszResConfig = NULL;
		pwszResConfig = NtkoSecChk_General_GetSecString(m_pNTkoSecstrPublishInfo,NTKO_CONSTKEY_SECSIGN_SPTFUNCTIONS,NTKO_DEFAULT_NOLOAD_SPTFUNCTIONS);

		m_publishRightSettings.bUseHandsign			= NtkoHasSubStringW(pwszResConfig,NTKO_SECSIGN_SPTFUNC_HANDSIGN);
		m_publishRightSettings.bUseKeyboardsign		= NtkoHasSubStringW(pwszResConfig,NTKO_SECSIGN_SPTFUNC_KEYBOARDSIGN);
		m_publishRightSettings.bUseNormalsign		= NtkoHasSubStringW(pwszResConfig,NTKO_SECSIGN_SPTFUNC_NORMALSIGN);
		m_publishRightSettings.bUseEkeySign			= NtkoHasSubStringW(pwszResConfig,NTKO_SECSIGN_SPTFUNC_EKEYSIGN);
		m_publishRightSettings.bUseMutisign			= NtkoHasSubStringW(pwszResConfig,NTKO_SECSIGN_SPTFUNC_MULTISIGN);
		m_publishRightSettings.bUseMutisignEkey		= NtkoHasSubStringW(pwszResConfig,NTKO_SECSIGN_SPTFUNC_MULTIEKEYSIGN);
		m_publishRightSettings.bUseDbCrossPagesign	= NtkoHasSubStringW(pwszResConfig,NTKO_SECSIGN_SPTFUNC_QIFENG_2PAGE);
		m_publishRightSettings.bUseMutiCrossPageSign= NtkoHasSubStringW(pwszResConfig,NTKO_SECSIGN_SPTFUNC_QIFENG_NPAGE);
		//m_publishRightSettings.bUseMutisignEkey		= NtkoHasSubStringW(pwszResConfig,NTKO_SECSIGN_SPTFUNC_QIFENG_EKEY);
		m_publishRightSettings.bUseBarcode			= NtkoHasSubStringW(pwszResConfig,NTKO_SECSIGN_SPTFUNC_BARCODE);
		m_publishRightSettings.bUseFingerPrint		= NtkoHasSubStringW(pwszResConfig,NTKO_SECSIGN_SPTFUNC_FINGERPRINT);
		m_publishRightSettings.bUseMuticrosssignEkey= NtkoHasSubStringW(pwszResConfig,NTKO_SECSIGN_SPTFUNC_QIFENG_EKEY);
		//m_bUseExportNtkf = NtkoHasSubStringW(pwszResConfig,NTKO_SECSIGN_SPTFUNC_EXPORTNTKF);
		//m_bUseSignConfig = NtkoHasSubStringW(pwszResConfig,NTKO_SECSIGN_SPTFUNC_SIGNCONFIG);
		m_pNTkoSecstrPublishInfo->MySafeFreeString(pwszResConfig);
	}

	m_hCertStore = NULL;        
	m_pCurSignCert = NULL;
	m_IsJSCAUseSealSvr = TRUE;

	return TRUE;
}

DWORD WINAPI CNTKOServerSignHelper::Notify_SlotChanged(BOOL bIsOk,LPVOID pParam,BOOL bIsInserted)
{
	if(!bIsOk)
	{
		return 1; //停止监听事件
	};

	CNTKOServerSignHelper *pHelper = NULL;
	pHelper = (CNTKOServerSignHelper *)pParam;
	if(pHelper)
	{
		HWND hHiddenForMsg = pHelper->m_HwndHiddenForMsg;
		if(hHiddenForMsg)
		{
			PostMessage(hHiddenForMsg,NTKO_WM_USER_EKEYINERT,bIsInserted?1:0,0);
		}
	}
	return 0; // 返回0标识继续监听,返回非0标识退出
}

BOOL CNTKOServerSignHelper::CreateHiddenWindowForMsg()
{
	if (m_HwndHiddenForMsg) return TRUE;
	
	UINT  clsStyle = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
	DWORD winStyle = WS_OVERLAPPEDWINDOW|WS_CLIPSIBLINGS|WS_CLIPCHILDREN;
	DWORD winStyleEx = WS_EX_WINDOWEDGE|WS_EX_APPWINDOW|WS_EX_OVERLAPPEDWINDOW;
	int	winWidth  = 100;
	int winHeight = 100;
	
	m_HwndHiddenForMsg = NtkoCreateWindowExW((HINSTANCE)g_hModule,NULL,TRUE,L"_NtkoHiddenSvrHelperMsgWinClass",
		NULL,clsStyle,winStyle,winStyleEx,
		0,0,winWidth,winHeight,
		NULL,
		LoadCursor(NULL, IDC_ARROW),(HBRUSH)GetStockObject(WHITE_BRUSH),
		NULL,
		this,CNTKOServerSignHelper::HiddenWindowForMsgProcess);

	return m_HwndHiddenForMsg?TRUE:FALSE;
}

LRESULT CALLBACK CNTKOServerSignHelper::HiddenWindowForMsgProcess(HWND hwnd, UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	CNTKOServerSignHelper* pHelper = (CNTKOServerSignHelper*)GetProp(hwnd,NTKO_HIDDEN_MSGWIN_PROP);
	
	if( ( WM_NCCREATE != uMsg) && (!pHelper) )
	{
		return ( DefWindowProcW( hwnd, uMsg, wParam, lParam ));
	}
	
	switch( uMsg ) 
	{
	case WM_NCCREATE:
		{
			if (NULL != lParam)
			{
				LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
				if(!pHelper)
				{
					pHelper = (CNTKOServerSignHelper*)pcs->lpCreateParams;	
					SetProp(hwnd, NTKO_HIDDEN_MSGWIN_PROP, (HANDLE) pHelper);
				}
				if(pHelper)
				{
					return TRUE;
				}
				else
				{
					return FALSE;
				}
			}
			else
			{
				return FALSE;
			}
			return 0;
		}
		break;
	case NTKO_WM_USER_EKEYINERT://需要激活OnHTEkeyInserted || OnHTEkeyRemoved事件
		if(pHelper) 
		{
			BOOL bLoggedOn = pHelper->m_IsLogin;
			BOOL bIsInserted = (1 == wParam);
			
			pHelper->m_pNtkoSignEkey->GetEkeyList();
			
			if(  
				( (bIsInserted) && (!bLoggedOn)) ||
				( (!bIsInserted) && (bLoggedOn)) 
				)
			{
				pHelper->DoLogin(NULL,NULL,TRUE);
			}	
		}
		break;
	case WM_NCDESTROY:
		{
			RemoveProp(hwnd, NTKO_HIDDEN_MSGWIN_PROP);
		}		
		break;
	default:
		return( DefWindowProcW( hwnd, uMsg, wParam, lParam ));
	}
	return DefWindowProcW( hwnd, uMsg, wParam, lParam );
}

INT CNTKOServerSignHelper::ShowMsgBoxMUIStrW(DWORD idText,DWORD idCaption,UINT type)
{
	LPWSTR pwszCaption = LoadMUIStringW(idCaption);
	LPWSTR pwszTxt = LoadMUIStringW(idText);
	INT iRet = NtkoShowMsgBox(m_hwndParent,pwszTxt,pwszCaption,type);
	SAFE_FREESTRING(pwszTxt);
	SAFE_FREESTRING(pwszCaption);
	return iRet;
}

INT CNTKOServerSignHelper::ShowMsgBoxMUIStrW(HWND hwndParent,DWORD idText,DWORD idCaption,UINT type)
{
	LPWSTR pwszCaption = LoadMUIStringW(idCaption);
	LPWSTR pwszTxt = LoadMUIStringW(idText);
	INT iRet = NtkoShowMsgBox(hwndParent,pwszTxt,pwszCaption,type);
	SAFE_FREESTRING(pwszTxt);
	SAFE_FREESTRING(pwszCaption);
	return iRet;
}

BOOL CNTKOServerSignHelper::ShowDelayDlg(LPWSTR pwszMes)
{
	HWND hwndDlgParent = m_hwndParent;

	if(m_pNtkoUIDelayDlg)
	{
		m_pNtkoUIDelayDlg->Show(hwndDlgParent,pwszMes);
	}
	return TRUE;
}

VOID CNTKOServerSignHelper::ShowDelayDlgMUIStrW(UINT idText)
{
	LPWSTR pwszTxt = LoadMUIStringW(idText);
	ShowDelayDlg(pwszTxt);
	SAFE_FREESTRING(pwszTxt);
}


BOOL CNTKOServerSignHelper::DoWebGet(LPSTR pszServerApp,LPWSTR pwszURL,BSTR* pbstr)
{
	HRESULT		hr = E_FAIL;
	ILockBytes*	pILockBytes = NULL;		//pointer to ILockBytes


	BYTE*		rgbBuf = NULL;
	int			bufSize = 0;
	ULONG		cbRead = 0;
	BOOL		retValue = FALSE;

	if (!pbstr) return FALSE;
	*pbstr = NULL;

	// We must have a string (URL) or an object to open from...
	if (!(pwszURL) || (*pwszURL == L'\0'))
	{
		return FALSE;
	}
	hr = CreateILockBytesOnHGlobal(NULL, TRUE, &pILockBytes);
	if ( (hr != S_OK) || (pILockBytes == NULL) )
	{
		return FALSE;
	}


	retValue = DownLoadURLToILockBytes(pszServerApp,pwszURL,pILockBytes);

	if (retValue)
	{
		STATSTG stststg;
		pILockBytes->Stat(&stststg,STATFLAG_NONAME);
		
		bufSize = (unsigned long)(stststg.cbSize.QuadPart);

		bufSize = (bufSize<(ULONGLONG)0x7FFFFFFF)?(int)(bufSize):0x7FFFFFFE;

		rgbBuf = new BYTE[bufSize+1]; //a buffer for return string
		
		if (rgbBuf)
		{
			ZeroMemory((void*)rgbBuf,bufSize+1);
			ULARGE_INTEGER ullOffset;
			ullOffset.QuadPart = (ULONGLONG)0;
			hr = pILockBytes->ReadAt(ullOffset,rgbBuf, bufSize, &cbRead);
			*pbstr = NtkoUTF8ToBSTR((LPSTR)rgbBuf);
			delete [] rgbBuf;
			rgbBuf = NULL;
			retValue = TRUE;
		}
		else
		{
			retValue = FALSE;
		}
	}
	SAFE_RELEASE_INTERFACE(pILockBytes);
	return retValue;
}
BOOL CNTKOServerSignHelper::GetHTTPReturnData(HINTERNET hReq,BSTR* pbstr)
{
	// 获取返回值,通过download到ILockBytes,取消了10K的限制.
	BYTE*	rgbBuf = NULL;
	ILockBytes*	pDownloadILockBytes = NULL;
	pDownloadILockBytes = NULL;		//pointer to ILockBytes
	BOOL	retbValue = TRUE;
	ULONG   cbRead = 0, cbWritten=0;
	DWORD dwInfoLevel = HTTP_QUERY_RAW_HEADERS_CRLF;
	DWORD dwInfoBufferLength = NTKO_DEF_NETREADBUFFERSIZE;

	HRESULT     hr     = E_UNEXPECTED;

	ULARGE_INTEGER ullOffset;
	ullOffset.QuadPart = (ULONGLONG)0;

	if (!pbstr) return FALSE;
	*pbstr = NULL;


	hr = CreateILockBytesOnHGlobal(NULL, TRUE, &pDownloadILockBytes);
	if ( (hr != S_OK) || (pDownloadILockBytes == NULL) )
	{
		retbValue = FALSE;
		goto errClr;
	}

	rgbBuf = new BYTE[dwInfoBufferLength+1];

	if (!rgbBuf) 
	{
		retbValue = FALSE;
		goto errClr;
	}

	while (!HttpQueryInfo(hReq, dwInfoLevel, rgbBuf, &dwInfoBufferLength, NULL))
	{
		DWORD dwError = GetLastError();
		if (dwError == ERROR_INSUFFICIENT_BUFFER)
		{
			delete [] rgbBuf;
			rgbBuf = new BYTE[dwInfoBufferLength+1];
			if (!rgbBuf) 
			{
				retbValue = FALSE;
				goto errClr;
			}
		}
		else
		{		
			break;
		}
	}

	rgbBuf[dwInfoBufferLength] = '\0';
	delete [] rgbBuf;
	rgbBuf = NULL;

	DWORD dwBytesAvailable;
	while (InternetQueryDataAvailable(hReq, &dwBytesAvailable, 0, 0))
	{
		rgbBuf = new BYTE[dwBytesAvailable+1];
		if (!rgbBuf) 
		{
			retbValue = FALSE;
			goto errClr;
		}
		if (!InternetReadFile (hReq, (void*)rgbBuf, dwBytesAvailable , &cbRead))
		{
			DWORD dwError = GetLastError();
			hr = HRESULT_FROM_WIN32(dwError);
			retbValue = FALSE;
			goto errClr;
		}
		
		if	(cbRead == 0) break;
		
		hr = pDownloadILockBytes->WriteAt(ullOffset, rgbBuf, cbRead, &cbWritten);

		delete [] rgbBuf;
		rgbBuf = NULL;

		if (FAILED(hr) )
		{
			retbValue = FALSE;
			goto errClr;
		}
		ullOffset.QuadPart += cbWritten;
		hr = S_OK;
	} 		
	if(rgbBuf)
	{
		delete [] rgbBuf;
		rgbBuf = NULL;
	}

	int bufSize;
	bufSize = (ullOffset.QuadPart<(ULONGLONG)0x7FFFFFFF)?(int)(ullOffset.QuadPart):0x7FFFFFFE;
	rgbBuf = new BYTE[bufSize+1]; //a buffer for return string
		
	if (!rgbBuf)
	{
		retbValue = FALSE;
		goto errClr;
	};	

	ZeroMemory((void*)rgbBuf,bufSize+1);

	ullOffset.QuadPart = (ULONGLONG)0;
	hr = pDownloadILockBytes->ReadAt(ullOffset,rgbBuf, bufSize, &cbRead);


	*pbstr = NtkoUTF8ToBSTR((LPSTR)rgbBuf);

		
	SAFE_RELEASE_INTERFACE(pDownloadILockBytes)
	delete [] rgbBuf;
	rgbBuf = NULL;
	
	DWORD httpStatusCode;
	httpStatusCode = 0;
	DWORD dwSize ;
	dwSize = sizeof(DWORD);

	if(
		HttpQueryInfo(hReq,HTTP_QUERY_STATUS_CODE|HTTP_QUERY_FLAG_NUMBER,(void*)&httpStatusCode,&dwSize,NULL) &&
		(HTTP_STATUS_OK == httpStatusCode) //OK
		)
	{
		retbValue = TRUE;
	}
	else
	{
		WCHAR errInfo[255];

		LPWSTR pwszFmt = LoadMUIStringW(IDSMUI_HTTPERRORCODE);
		NtkoFwsprintf(errInfo,pwszFmt,httpStatusCode);
		SAFE_FREESTRING(pwszFmt);
		
		BSTR temp = *pbstr;
		LPWSTR pwszTemp =  NtkoCopyStringCatW((LPWSTR)temp,errInfo);

		SAFE_FREEBSTRING(*pbstr);

		*pbstr = ::SysAllocString(pwszTemp);
		SAFE_FREESTRING(pwszTemp);
		SAFE_FREEBSTRING(temp);
		retbValue = FALSE;
	};
	
	return retbValue;

errClr:
	if(rgbBuf)
	{
		delete [] rgbBuf;
		rgbBuf = NULL;
	}
	SAFE_RELEASE_INTERFACE(pDownloadILockBytes);
	return retbValue;
}

BOOL CNTKOServerSignHelper::DownLoadURLToILockBytes(LPSTR pszServerApp,LPWSTR pwszURL,ILockBytes* pILockBytes)
{
	LPSTR	pstrUA = NULL;			// User Agent
	LPSTR	pstrBaseURL = NULL;
	LPSTR	pstrURL = NULL;


	HRESULT     hr     = E_UNEXPECTED;
	BYTE       *rgbBuf = NULL;
	ULONG       cbRead, cbWritten;
	HINTERNET	hOpen = NULL, hConnect = NULL, hReq = NULL;
	URL_COMPONENTS temURLStructure;
	DWORD		dw;


	if ( !(pwszURL) || (*pwszURL == L'\0') )
	{
		return FALSE;
	}

	if (!(pILockBytes))
	{
		return FALSE;
	}

	pstrBaseURL = pszServerApp;
	pstrUA = NTKO_HC_AGENT;
	pstrURL = NtkoLPWSTRToANSI(pwszURL); //need free


	char lpszURL[1024];
	char Scheme[63];
	char HostName[1024];
	char Username[256];
	char Password[256];
	char URlPath[1024];
	char ExtraInfo[1024];

	dw = sizeof lpszURL;
	InternetCombineUrl(pstrBaseURL,pstrURL,lpszURL,&dw, ICU_BROWSER_MODE);

	SAFE_FREESTRING(pstrURL);

	temURLStructure.dwStructSize = sizeof(temURLStructure);
	temURLStructure.lpszScheme = Scheme;
	temURLStructure.dwSchemeLength = sizeof Scheme;
	temURLStructure.lpszHostName = HostName;
	temURLStructure.dwHostNameLength = sizeof HostName;
	temURLStructure.lpszUserName = Username;
	temURLStructure.dwUserNameLength =sizeof Username;
	temURLStructure.lpszPassword = Password;
	temURLStructure.dwPasswordLength = sizeof Password;
	temURLStructure.lpszUrlPath = URlPath;
	temURLStructure.dwUrlPathLength = sizeof URlPath;
	temURLStructure.lpszExtraInfo = ExtraInfo;
	temURLStructure.dwExtraInfoLength = sizeof ExtraInfo;
	
	InternetCrackUrl(lpszURL,dw,0,/*ICU_DECODE,*/&temURLStructure);	

	
	if (
		( INTERNET_SCHEME_HTTP != temURLStructure.nScheme )
		&& ( INTERNET_SCHEME_HTTPS != temURLStructure.nScheme )
		)
	{
		return FALSE;
	}
	
	//这儿需要转换URLPath为UTF-8编码
	ConvertANSIToUTF8Hex(URlPath);

	lstrcatA(URlPath,ExtraInfo);
	
	hOpen = InternetOpen  //open internet session
         (
            pstrUA,                     // user agent
            INTERNET_OPEN_TYPE_PRECONFIG,    // open type: direct
            NULL,                        // proxy server name
            NULL,                                         // by passed proxy
            0
         );
	if ( NULL == hOpen ) 
	{
		return FALSE;
	}
	
	hConnect = InternetConnect
         (
            hOpen,                               // session handle
            HostName,                   // server name
            temURLStructure.nPort ,                             // service port no
            NTKO_HTTP_USER,                                  // username
            NTKO_HTTP_PASS,                                  // password
            INTERNET_SERVICE_HTTP,               // service required
            0 ,
            0
         );

	if ( NULL == hConnect )
	{
		if ( hOpen ) InternetCloseHandle(hOpen);
		return FALSE;
	}



	BOOL bIsUseHTTP11 = IsInternetSettingEnableHTTP11();
	
	if (v_fUnicodeAPI)
	{
   		WCHAR *waccept[2];
		waccept[0] = L"*/*";
		waccept[1] = NULL;
		LPWSTR pwszURLPath = NtkoANSIToLPWSTR(URlPath);
		hReq = HttpOpenRequestW                        // new HTTP request handle
			 (
				hConnect,                            // connect handle
				L"GET",                               // method
				pwszURLPath,                  // object
				(bIsUseHTTP11)?L"HTTP/1.1":L"HTTP/1.0",                // HTTP Vertion
				NULL,                                  // referer
				(const WCHAR**)waccept,                                  // accept types
				INTERNET_FLAG_RELOAD |              // Internet flag values
					INTERNET_FLAG_NO_CACHE_WRITE |
					 INTERNET_FLAG_KEEP_CONNECTION |
					 (INTERNET_SCHEME_HTTPS == 
					 temURLStructure.nScheme ? INTERNET_FLAG_SECURE:0) ,
				0                                    // applicaion define value
			 );
		SAFE_FREESTRING(pwszURLPath);
	}
	else
	{
   		char *accept[2];

		accept[0] = "*/*";
		accept[1] = NULL;

		hReq = HttpOpenRequestA                        // new HTTP request handle
			 (
				hConnect,                            // connect handle
				"GET",                               // method
				URlPath,                  // object
				(bIsUseHTTP11)?"HTTP/1.1":"HTTP/1.0",                // HTTP Vertion
				NULL,                                  // referer
				(const char**)accept,                                  // accept types
				INTERNET_FLAG_RELOAD |              // Internet flag values
					INTERNET_FLAG_NO_CACHE_WRITE |
					 INTERNET_FLAG_KEEP_CONNECTION |
					 (INTERNET_SCHEME_HTTPS == 
					 temURLStructure.nScheme ? INTERNET_FLAG_SECURE:0) ,
				0                                    // applicaion define value
			 );
	}

	if ( NULL == hReq ) 
	{
		if ( hConnect ) InternetCloseHandle(hConnect);
		if ( hOpen ) InternetCloseHandle(hOpen);
		return FALSE;
	}

	dw = m_dwConnectServerTimeOut * 1000;
	if (!InternetSetOption(hReq, INTERNET_OPTION_CONNECT_TIMEOUT,
						  &dw, sizeof(dw)) )
	{
		if ( hReq )InternetCloseHandle(hReq);
		if ( hConnect ) InternetCloseHandle(hConnect);
		if ( hOpen ) InternetCloseHandle(hOpen);
		return FALSE;
	};

	dw = m_dwReceiveDataTimeOut * 1000;

	if (!InternetSetOption(hReq, INTERNET_OPTION_RECEIVE_TIMEOUT,
		&dw, sizeof(dw)) )
	{
		if ( hReq )InternetCloseHandle(hReq);
		if ( hConnect ) InternetCloseHandle(hConnect);
		if ( hOpen ) InternetCloseHandle(hOpen);
		return FALSE;
	};

	if(INTERNET_SCHEME_HTTPS ==	 temURLStructure.nScheme)
	{
		dw = 0;
		DWORD dwLen = sizeof(dw);
		if(InternetQueryOption(hReq, INTERNET_OPTION_SECURITY_FLAGS,&dw,&dwLen) )
		{
			dw |= SECURITY_FLAG_IGNORE_UNKNOWN_CA |SECURITY_FLAG_IGNORE_REVOCATION |SECURITY_FLAG_IGNORE_WRONG_USAGE;
			InternetSetOption (hReq, INTERNET_OPTION_SECURITY_FLAGS,&dw, sizeof(dw));
		}

		if(m_pCurSignCert)
		{
			if (!InternetSetOption(hReq, INTERNET_OPTION_CLIENT_CERT_CONTEXT, 
				(void *)m_pCurSignCert, sizeof(CERT_CONTEXT)) )
			{
				if ( hReq )InternetCloseHandle(hReq);
				if ( hConnect ) InternetCloseHandle(hConnect);
				if ( hOpen ) InternetCloseHandle(hOpen);
				return FALSE;
			}
		}
	}


	char CookieHeader[NTKO_MAX_HTTPHEADER];
	char tBuffer[512];

	BOOL isOk = FALSE;
	LPSTR	pstrCookieHeader = NULL;

	memset(CookieHeader,0,sizeof(CookieHeader));
	memset(tBuffer,0,sizeof(tBuffer));

	isOk = ReadRegisterInfo(HKEY_CURRENT_USER, 
		"Software\\Microsoft\\Internet Explorer\\International","AcceptLanguage",
					tBuffer,sizeof(tBuffer));
	if(isOk)
	{
		lstrcpyA(CookieHeader,"Accept-Language: ");
		lstrcatA(CookieHeader,tBuffer);
		lstrcatA(CookieHeader,"\r\n");
		pstrCookieHeader = CookieHeader;
	}

	dw = pstrCookieHeader?strlen(pstrCookieHeader):0;
	while (!HttpSendRequest
		(
		hReq,                                // request handle
		pstrCookieHeader,                          // header
		dw,                 // header length
		NULL,                                 // optional data buffer pointer
		0                                    // optional buffer length
		))
	{
		DWORD dwError = GetLastError();
		if ( dwError == ERROR_INTERNET_CLIENT_AUTH_CERT_NEEDED )
		{				
			if(SelectLogonCert(hReq))
			{
				continue;
			}
		}
		if ( hReq )InternetCloseHandle(hReq);
		if ( hConnect ) InternetCloseHandle(hConnect);
		if ( hOpen ) InternetCloseHandle(hOpen);
		return FALSE;
	};
	
	
	ULARGE_INTEGER ulOffset;
	ulOffset.QuadPart = (ULONGLONG)0;
	
	DWORD dwInfoLevel = HTTP_QUERY_RAW_HEADERS_CRLF;
	DWORD dwInfoBufferLength = NTKO_DEF_NETREADBUFFERSIZE;
	rgbBuf = new BYTE[dwInfoBufferLength+1];

	if (!rgbBuf) 
	{
		if ( hReq )InternetCloseHandle(hReq);
		if ( hConnect ) InternetCloseHandle(hConnect);
		if ( hOpen ) InternetCloseHandle(hOpen);
		return FALSE;
	}

	while (!HttpQueryInfo(hReq, dwInfoLevel, rgbBuf, &dwInfoBufferLength, NULL))
	{
		DWORD dwError = GetLastError();
		if (dwError == ERROR_INSUFFICIENT_BUFFER)
		{
			delete [] rgbBuf;
			rgbBuf = new BYTE[dwInfoBufferLength+1];
			if (!rgbBuf) 
			{
				if ( hReq )InternetCloseHandle(hReq);
				if ( hConnect ) InternetCloseHandle(hConnect);
				if ( hOpen ) InternetCloseHandle(hOpen);
				return FALSE;
			}
		}
		else
		{
			break;
		}
	}

	rgbBuf[dwInfoBufferLength] = '\0';
	delete [] rgbBuf;
	rgbBuf = NULL;




	DWORD FileSize;
	FileSize = 0;
	DWORD dwFileSize;
	dwFileSize = sizeof(DWORD);
	if(!HttpQueryInfo(hReq,HTTP_QUERY_CONTENT_LENGTH |HTTP_QUERY_FLAG_NUMBER,(void*)&FileSize,&dwFileSize,NULL))
	{
	}
	else
	{
	}

	DWORD dwDownloadSize;
	dwDownloadSize = 0;

	DWORD dwBytesToReceive;
	dwBytesToReceive = NTKO_INIT_NETBUFFERSIZE;

	DWORD dwLastBytesToReceive;
	dwLastBytesToReceive = 0;
	//while (InternetQueryDataAvailable(hReq, &dwBytesToReceive, 0, 0))
	while( (dwDownloadSize < FileSize) || (0 == FileSize) )
	{
		rgbBuf = new BYTE[dwBytesToReceive+1];
		if (!rgbBuf) 
		{
			if ( hReq )InternetCloseHandle(hReq);
			if ( hConnect ) InternetCloseHandle(hConnect);
			if ( hOpen ) InternetCloseHandle(hOpen);
			return FALSE;
		}

		DWORD dwStart = GetTickCount();

		if (!InternetReadFile (hReq, (void*)rgbBuf, dwBytesToReceive , &cbRead))
		{
			DWORD dwError = GetLastError();
			hr = HRESULT_FROM_WIN32(dwError);			
			break;
		}

		DWORD dwEnd = GetTickCount();

		if	(cbRead == 0) 
		{
			break;
		}

		hr = pILockBytes->WriteAt(ulOffset, rgbBuf, cbRead, &cbWritten);
		
		if (FAILED(hr) )
		{			
			hr = HRESULT_FROM_WIN32(GetLastError());
			break;
		}

		delete [] rgbBuf;
		rgbBuf = NULL;

		ulOffset.QuadPart += cbWritten;
		dwDownloadSize += cbRead;

		//调整每次读取的缓存
		if(dwEnd >= dwStart)
		{
			DWORD dwUsedTime = dwEnd - dwStart;
			if(dwUsedTime == 0)
			{
				dwUsedTime = 1;
			}
			
			DWORD speed = cbRead*1000 /dwUsedTime;

			if(speed > dwLastBytesToReceive)
			{				
				dwBytesToReceive = speed<<1;
			}
			else
			{
				dwBytesToReceive = speed;
			}
			dwLastBytesToReceive = speed;
			/*
			if(dwBytesToReceive > dwLastBytesToReceive)
			{
				dwLastBytesToReceive = cbRead*100 /dwUsedTime;
				dwBytesToReceive = dwLastBytesToReceive<<2;
			}
			*/
			if(dwBytesToReceive > NTKO_MAX_NETBUFFERSIZE)
			{
				dwBytesToReceive = NTKO_MAX_NETBUFFERSIZE;
			}
		}

		hr = S_OK;
	} 			
	
	if(rgbBuf)
	{
		delete [] rgbBuf;
	}


	DWORD httpStatusCode = 0;
	DWORD dwSize = sizeof(DWORD);

	if(
		HttpQueryInfo(hReq,HTTP_QUERY_STATUS_CODE|HTTP_QUERY_FLAG_NUMBER,(void*)&httpStatusCode,&dwSize,NULL) &&
		(HTTP_STATUS_OK == httpStatusCode) //OK
		)
	{

	}
	else
	{
		hr = E_FAIL;
	};

	if ( hReq )InternetCloseHandle(hReq);
	if ( hConnect ) InternetCloseHandle(hConnect);
	if ( hOpen ) InternetCloseHandle(hOpen);
	
	return FAILED(hr)?FALSE:TRUE;
}

BOOL CNTKOServerSignHelper::CloseDelayDlg()
{
	if(m_pNtkoUIDelayDlg)
	{
		m_pNtkoUIDelayDlg->Hide();
	}
	return TRUE;	
}


void CNTKOServerSignHelper::HideUIDelayDlg()
{
	if(m_pNtkoUIDelayDlg)
	{
		m_pNtkoUIDelayDlg->HideForRestore();
	}
}
void CNTKOServerSignHelper::ShowUIDelayDlgAgain()
{
	if(m_pNtkoUIDelayDlg)
	{
		m_pNtkoUIDelayDlg->ShowAgain();
	}
}


BOOL CNTKOServerSignHelper::ParseXML(BSTR bstrXML)
{
	HRESULT hr = S_OK;
	BOOL isOk = FALSE;
	
	if(!m_pXMLDoc)
	{
		hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void**)&m_pXMLDoc);
	}
	
	if( FAILED(hr) || (!m_pXMLDoc) )
	{
		return FALSE;
	}
	
	VARIANT_BOOL vbOk = VARIANT_FALSE;
	hr = m_pXMLDoc->loadXML(bstrXML,&vbOk);
	if(FAILED(hr) ||(VARIANT_FALSE == vbOk))
	{
		return FALSE;
	}
	
	isOk = ParseResponseStatus();	
	
	if( (0 != m_iServerErrCode) || (!isOk) || (NTKOServerRequestType_Logout==m_serverReqestType) ) //如果解析出错，或者服务器返回了错误，或者是用户注销请求
	{
		return isOk;
	}

	//如果不是10 获取服务器信息请求,或者不是11获取服务器时间请求，或者不是 12检查印章 需要解析用户权限 13 获取服务器变更(过期)信息
	if( (NTKOServerRequestType_GetRootServerInfo!=m_serverReqestType)  && 
		(NTKOServerRequestType_DosignLog!=m_serverReqestType)  && 
		(NTKOServerRequestType_TestConnetServer != m_serverReqestType) && 
		(NTKOServerRequestType_GetserverTime!=m_serverReqestType) && 
		(NTKOServerRequestType_CheckSign!=m_serverReqestType) && 
		(NTKOServerRequestType_GetServerChangeInfo!=m_serverReqestType) && 
		(NTKOServerRequestType_GetServerComments!=m_serverReqestType) && 
		(NTKOServerRequestType_SetServerComments!=m_serverReqestType) )
	{
#if defined(NTKO_CUSTOM_JIANGSU_CA)

	#if 0 //初次版本的江苏ca请求。现在的版本客户端不需要直接和seal服务器打交道，禁用，保留代码备查
		if (NTKOServerRequestType_Login==m_serverReqestType)
	#else
		if ((NTKOServerRequestType_Login==m_serverReqestType) || (NTKOServerRequestType_GetSignList==m_serverReqestType) )
		{
			isOk = ParseUserRights();
		}		
	#endif
#else

		isOk = ParseUserRights();
#endif
	}
	return isOk;
}

BOOL CNTKOServerSignHelper::ParseRootServerInfo()
{
	HRESULT hr = S_OK;
	BOOL isOk = FALSE;
	IXMLDOMNode *pIDOMNode = NULL;
	
	hr = m_pXMLDoc->selectSingleNode( OLESTR("//ResponseInfo"), &pIDOMNode);
	if(FAILED(hr) || (!pIDOMNode))
	{
		return FALSE;
	}
	
	IXMLDOMNode *pIDOMNode2 = NULL;
	
	hr = pIDOMNode->selectSingleNode( OLESTR("./RootServerInfo"), &pIDOMNode2);
	if(SUCCEEDED(hr) && pIDOMNode2)
	{
		IXMLDOMNamedNodeMap *pAttributeMap = NULL;
		hr = pIDOMNode2->get_attributes(&pAttributeMap);
		if(SUCCEEDED(hr) && pAttributeMap)
		{
			IXMLDOMNode *pIDOMNode3 = NULL;
			//RserverAppURL
			hr = pAttributeMap->getNamedItem(L"RserverAppURL",&pIDOMNode3);
			if(SUCCEEDED(hr) && pIDOMNode3)
			{
				BSTR bstrTemp = NULL;
				pIDOMNode3->get_text(&bstrTemp);
				if(bstrTemp && lstrlenW(bstrTemp))
				{
					LPSTR pszLoginServerName = NtkoLPWSTRToANSI(bstrTemp);
					lstrcpyn(m_rootSvrUrl,pszLoginServerName,sizeof(m_rootSvrUrl));
					SAFE_FREESTRING(pszLoginServerName);						
				}
				isOk = TRUE;
				SAFE_FREEBSTRING(bstrTemp);								
				RELEASE_INTERFACE(pIDOMNode3);
			}			
			RELEASE_INTERFACE(pAttributeMap);
		}
		RELEASE_INTERFACE(pIDOMNode2);
	}
	RELEASE_INTERFACE(pIDOMNode);
	return isOk;
}

BOOL CNTKOServerSignHelper::ParseThisServerInfo()
{
	if(0 != m_iThisServerInfoServerId)
	{
		return TRUE;
	}
	
	HRESULT hr = S_OK;
	BOOL isOk = FALSE;
	IXMLDOMNode *pIDOMNode = NULL;
	
	hr = m_pXMLDoc->selectSingleNode( OLESTR("//ResponseInfo"), &pIDOMNode);
	if(FAILED(hr) || (!pIDOMNode))
	{
		return FALSE;
	}
	
	IXMLDOMNode *pIDOMNode2 = NULL;
	
	hr = pIDOMNode->selectSingleNode( OLESTR("./ThisServerInfo"), &pIDOMNode2);
	if(SUCCEEDED(hr) && pIDOMNode2)
	{
		IXMLDOMNamedNodeMap *pAttributeMap = NULL;
		hr = pIDOMNode2->get_attributes(&pAttributeMap);
		if(SUCCEEDED(hr) && pAttributeMap)
		{
			IXMLDOMNode *pIDOMNode3 = NULL;
			//version
			hr = pAttributeMap->getNamedItem(L"serverId",&pIDOMNode3);
			if(SUCCEEDED(hr) && pIDOMNode3)
			{
				BSTR bstrTemp = NULL;
				pIDOMNode3->get_text(&bstrTemp);
				if(bstrTemp)
				{
					if(NtkoStrToIntExW(bstrTemp,STIF_DEFAULT,&m_iThisServerInfoServerId) && (m_iThisServerInfoServerId>0))
					{
						isOk = TRUE;
					}
					else
					{
						m_iThisServerInfoServerId = 0;
						isOk = FALSE;
					}
				}
				SAFE_FREEBSTRING(bstrTemp);								
				RELEASE_INTERFACE(pIDOMNode3);
			}
			//serverName
			if(isOk)
			{
				isOk = FALSE;
				hr = pAttributeMap->getNamedItem(L"serverName",&pIDOMNode3);
				if(SUCCEEDED(hr) && pIDOMNode3)
				{
					SAFE_FREEBSTRING(m_bstrThisServerInfoName);
					pIDOMNode3->get_text(&m_bstrThisServerInfoName);
					if(::SysStringByteLen(m_bstrThisServerInfoName))	
					{				
						isOk = TRUE;
					}
					else
					{
						isOk = FALSE;
					}							
					RELEASE_INTERFACE(pIDOMNode3);
				}
			}
			RELEASE_INTERFACE(pAttributeMap);
		}
		RELEASE_INTERFACE(pIDOMNode2);
	}
	RELEASE_INTERFACE(pIDOMNode);
	return isOk;
}


BOOL CNTKOServerSignHelper::ParseThisUserInfo()
{
	HRESULT hr = S_OK;
	BOOL isOk = FALSE;
	IXMLDOMNode *pIDOMNode = NULL;

	hr = m_pXMLDoc->selectSingleNode( OLESTR("//ResponseInfo"), &pIDOMNode);
	if(FAILED(hr) || (!pIDOMNode))
	{
		return FALSE;
	}
		
	IXMLDOMNode *pIDOMNode2 = NULL;
	
	hr = pIDOMNode->selectSingleNode( OLESTR("./ThisUserInfo"), &pIDOMNode2);
	if(SUCCEEDED(hr) && pIDOMNode2)
	{
		IXMLDOMNamedNodeMap *pAttributeMap = NULL;
		hr = pIDOMNode2->get_attributes(&pAttributeMap);
		if(SUCCEEDED(hr) && pAttributeMap)
		{
			IXMLDOMNode *pIDOMNode3 = NULL;
			//RserverAppURL
			hr = pAttributeMap->getNamedItem(L"RealName",&pIDOMNode3);
			if(SUCCEEDED(hr) && pIDOMNode3)
			{
				BSTR bstrTemp = NULL;
				pIDOMNode3->get_text(&bstrTemp);
				if(bstrTemp && lstrlenW(bstrTemp))
				{
					NtkoCopyStringToDst(m_LoginRealName,bstrTemp,sizeof(m_LoginRealName));	
				}
				isOk = TRUE;
				SAFE_FREEBSTRING(bstrTemp);								
				RELEASE_INTERFACE(pIDOMNode3);
			}			

#if defined(NTKO_CUSTOM_JIANGSU_CA)  || defined(NTKO_CUSTOM_SHANXI_CA) || defined(NTKO_CUSTOM_DOLOGIN_USECERT) //证书登录需要根据返回证书设定用户名
			hr = pAttributeMap->getNamedItem(L"UserName",&pIDOMNode3);
			if(SUCCEEDED(hr) && pIDOMNode3)
			{
				BSTR bstrTemp = NULL;
				pIDOMNode3->get_text(&bstrTemp);
				if(bstrTemp && lstrlenW(bstrTemp))
				{
					NtkoCopyStringToDst(m_LoginUserName,bstrTemp,sizeof(m_LoginUserName));	
				}
				isOk = TRUE;
				SAFE_FREEBSTRING(bstrTemp);								
				RELEASE_INTERFACE(pIDOMNode3);
			}	
#endif

			RELEASE_INTERFACE(pAttributeMap);
		}
		RELEASE_INTERFACE(pIDOMNode2);
	}
	RELEASE_INTERFACE(pIDOMNode);
	return isOk;
}

BOOL CNTKOServerSignHelper::NTKO_Custom_ParseThisSealServerInfo()
{
#if !defined(NTKO_CUSTOM_JIANGSU_CA)
	return FALSE;
#endif
	HRESULT hr = S_OK;
	BOOL isOk = FALSE;
	IXMLDOMNode *pIDOMNode = NULL;
	
	hr = m_pXMLDoc->selectSingleNode( OLESTR("//ResponseInfo"), &pIDOMNode);
	if(FAILED(hr) || (!pIDOMNode))
	{
		return FALSE;
	}
	
	IXMLDOMNode *pIDOMNode2 = NULL;
	
	hr = pIDOMNode->selectSingleNode( OLESTR("./ThisSealServerInfo"), &pIDOMNode2);
	if(SUCCEEDED(hr) && pIDOMNode2)
	{
		IXMLDOMNamedNodeMap *pAttributeMap = NULL;
		hr = pIDOMNode2->get_attributes(&pAttributeMap);
		if(SUCCEEDED(hr) && pAttributeMap)
		{
			IXMLDOMNode *pIDOMNode3 = NULL;
			//version
			hr = pAttributeMap->getNamedItem(L"sealserverIp",&pIDOMNode3);
			if(SUCCEEDED(hr) && pIDOMNode3)
			{
				BSTR bstrTemp = NULL;
				pIDOMNode3->get_text(&bstrTemp);
				if(bstrTemp && lstrlenW(bstrTemp))
				{
					LPSTR signListUrl = NtkoLPWSTRToANSI(bstrTemp);
					lstrcpyn(m_CustomSignServerUrl,signListUrl,sizeof(m_CustomSignServerUrl));
					SAFE_FREESTRING(signListUrl);
					isOk = TRUE;
				}
				SAFE_FREEBSTRING(bstrTemp);								
				RELEASE_INTERFACE(pIDOMNode3);
			}
			RELEASE_INTERFACE(pAttributeMap);
		}
		RELEASE_INTERFACE(pIDOMNode2);
	}
	RELEASE_INTERFACE(pIDOMNode);
	return isOk;
}


BOOL CNTKOServerSignHelper::NTKO_Custom_ParseCertSN()
{
#if !defined(NTKO_CUSTOM_CHUANDA_ZHISHENG)
	return TRUE;
#endif
	HRESULT hr = S_OK;
	BOOL isOk = FALSE;
	IXMLDOMNode *pIDOMNode = NULL;
	
	hr = m_pXMLDoc->selectSingleNode( OLESTR("//ResponseInfo"), &pIDOMNode);
	if(FAILED(hr) || (!pIDOMNode))
	{
		return FALSE;
	}
	
	IXMLDOMNode *pIDOMNode2 = NULL;
	
	hr = pIDOMNode->selectSingleNode( OLESTR("./CertSNInfo"), &pIDOMNode2);
	if(SUCCEEDED(hr) && pIDOMNode2)
	{
		IXMLDOMNamedNodeMap *pAttributeMap = NULL;
		hr = pIDOMNode2->get_attributes(&pAttributeMap);
		if(SUCCEEDED(hr) && pAttributeMap)
		{
			IXMLDOMNode *pIDOMNode3 = NULL;
			//version
			hr = pAttributeMap->getNamedItem(L"certsn",&pIDOMNode3);
			if(SUCCEEDED(hr) && pIDOMNode3)
			{
				SAFE_FREEBSTRING(m_bstr_custom_ChuanDaZhiSheng_UserCertSN);
				pIDOMNode3->get_text(&m_bstr_custom_ChuanDaZhiSheng_UserCertSN);
				if(m_bstr_custom_ChuanDaZhiSheng_UserCertSN && ::SysStringByteLen(m_bstr_custom_ChuanDaZhiSheng_UserCertSN))
				{
					isOk = TRUE;
				}
				else
				{
					isOk = FALSE;
				}
							
				RELEASE_INTERFACE(pIDOMNode3);
			}
			RELEASE_INTERFACE(pAttributeMap);
		}
		RELEASE_INTERFACE(pIDOMNode2);
	}
	RELEASE_INTERFACE(pIDOMNode);
	return isOk;
}

BOOL CNTKOServerSignHelper::DoWebExecute2(LPSTR pszServerApp,LPWSTR pwszURL,LPWSTR pwszCPARA,BSTR* pbstr)
{
	int			bufSize = 0;
	ULONG		cbRead = 0;
	ULONG		cbWritten = 0;

	LPSTR	pstrUA = NULL;			// User Agent
	LPSTR	pstrBaseURL = NULL;
	LPSTR	pstrURL = NULL;

	LPSTR	pstrCPARA=NULL;	//Para transformed to pstr
	LPSTR	pstrHead = NULL;		//Head buffer
	DWORD	dwCalMem = 0; // total memory will be alloced
	
	HRESULT     hr     = E_UNEXPECTED;
	HINTERNET	hOpen = NULL, hConnect = NULL, hReq = NULL;
	URL_COMPONENTS temURLStructure;
	DWORD		dw = 0,sum = 0;

	if (!pbstr) return FALSE;
	*pbstr = NULL;

	if ( !(pwszURL) || (*pwszURL == L'\0') )
	{
		return FALSE;
	}

	if( (!pwszCPARA) ||(*pwszCPARA == L'\0') )
	{
		return FALSE;
	}


	pstrBaseURL = pszServerApp;
	pstrUA = NTKO_HC_AGENT;
	pstrURL = NtkoLPWSTRToANSI(pwszURL); //need free

	char lpszURL[1024];
	char Scheme[63];
	char HostName[1024];
	char Username[256];
	char Password[256];
	char URlPath[1024];
	char ExtraInfo[1024];


	dw = sizeof lpszURL;

	InternetCombineUrl(pstrBaseURL,pstrURL,lpszURL,&dw, ICU_BROWSER_MODE);
	SAFE_FREESTRING(pstrURL);

	temURLStructure.dwStructSize = sizeof(temURLStructure);
	temURLStructure.lpszScheme = Scheme;
	temURLStructure.dwSchemeLength = sizeof Scheme;
	temURLStructure.lpszHostName = HostName;
	temURLStructure.dwHostNameLength = sizeof HostName;
	temURLStructure.lpszUserName = Username;
	temURLStructure.dwUserNameLength =sizeof Username;
	temURLStructure.lpszPassword = Password;
	temURLStructure.dwPasswordLength = sizeof Password;
	temURLStructure.lpszUrlPath = URlPath;
	temURLStructure.dwUrlPathLength = sizeof URlPath;
	temURLStructure.lpszExtraInfo = ExtraInfo;
	temURLStructure.dwExtraInfoLength = sizeof ExtraInfo;
	
	InternetCrackUrl(lpszURL,dw,0,/*ICU_DECODE,*/&temURLStructure);

	if (
		( INTERNET_SCHEME_HTTP != temURLStructure.nScheme )
		&& ( INTERNET_SCHEME_HTTPS != temURLStructure.nScheme )
		)
	{
		return FALSE;
	}
	
	//这儿需要转换URLPath为UTF-8编码

	ConvertANSIToUTF8Hex(URlPath);

	lstrcatA(URlPath,ExtraInfo);
	

	hOpen = InternetOpen  //open internet session
         (
            pstrUA,                     // user agent
            INTERNET_OPEN_TYPE_PRECONFIG,    // open type: direct
            NULL,                        // proxy server name
            NULL,                                         // by passed proxy
            0
         );
	if ( NULL == hOpen ) 
	{
		return FALSE; 
	}
	
	hConnect = InternetConnect
         (
            hOpen,                               // session handle
            HostName,                   // server name
            temURLStructure.nPort ,                             // service port no
            NTKO_HTTP_USER,                                  // username
            NTKO_HTTP_PASS,                                  // password
            INTERNET_SERVICE_HTTP,               // service required
            0 ,
            0
         );

	if ( NULL == hConnect )
	{
		if ( hOpen ) InternetCloseHandle(hOpen);
		return FALSE; 
	}


	BOOL bIsUseHTTP11 = IsInternetSettingEnableHTTP11();
	
	if (v_fUnicodeAPI)
	{
   		WCHAR *waccept[2];
		waccept[0] = L"*/*";
		waccept[1] = NULL;
		LPWSTR pwszURLPath = NtkoANSIToLPWSTR(URlPath);
		hReq = HttpOpenRequestW                        // new HTTP request handle
			 (
				hConnect,                            // connect handle
				L"POST",                               // method
				pwszURLPath,                  // object
				(bIsUseHTTP11)?L"HTTP/1.1":L"HTTP/1.0",                // HTTP Vertion
				NULL,                                  // referer
				(const WCHAR**)waccept,                                  // accept types
				INTERNET_FLAG_RELOAD |              // Internet flag values
					INTERNET_FLAG_NO_CACHE_WRITE |
					 INTERNET_FLAG_KEEP_CONNECTION |
					 //INTERNET_FLAG_NO_AUTH |
					 (INTERNET_SCHEME_HTTPS == 
					 temURLStructure.nScheme ? INTERNET_FLAG_SECURE:0) ,
				0                                    // applicaion define value
			 );
		SAFE_FREESTRING(pwszURLPath);
	}
	else
	{
   		char *accept[2];

		accept[0] = "*/*";
		accept[1] = NULL;

		hReq = HttpOpenRequestA                        // new HTTP request handle
			 (
				hConnect,                            // connect handle
				"POST",                               // method
				URlPath,                  // object
				(bIsUseHTTP11)?"HTTP/1.1":"HTTP/1.0",                // HTTP Vertion
				NULL,                                  // referer
				(const char**)accept,                                  // accept types
				INTERNET_FLAG_RELOAD |              // Internet flag values
					INTERNET_FLAG_NO_CACHE_WRITE |
					 INTERNET_FLAG_KEEP_CONNECTION |
					 //INTERNET_FLAG_NO_AUTH |
					 (INTERNET_SCHEME_HTTPS == 
					 temURLStructure.nScheme ? INTERNET_FLAG_SECURE:0) ,
				0                                    // applicaion define value
			 );
	}


	if ( NULL == hReq ) 
	{
		if ( hConnect ) InternetCloseHandle(hConnect);
		if ( hOpen ) InternetCloseHandle(hOpen);
		return FALSE; 
	}
	
	dw = m_dwConnectServerTimeOut * 1000;
	if (!InternetSetOption(hReq, INTERNET_OPTION_CONNECT_TIMEOUT,
						  &dw, sizeof(dw)) )
	{
		if ( hReq )InternetCloseHandle(hReq);
		if ( hConnect ) InternetCloseHandle(hConnect);
		if ( hOpen ) InternetCloseHandle(hOpen);
		return FALSE; 
	};

	if(INTERNET_SCHEME_HTTPS ==	 temURLStructure.nScheme)
	{
		dw = 0;
		DWORD dwLen = sizeof(dw);
		if(InternetQueryOption(hReq, INTERNET_OPTION_SECURITY_FLAGS,&dw,&dwLen) )
		{
			dw |= SECURITY_FLAG_IGNORE_UNKNOWN_CA |SECURITY_FLAG_IGNORE_REVOCATION |SECURITY_FLAG_IGNORE_WRONG_USAGE;
			InternetSetOption (hReq, INTERNET_OPTION_SECURITY_FLAGS,&dw, sizeof(dw));
		}

		if(m_pCurSignCert)
		{
			if (!InternetSetOption(hReq, INTERNET_OPTION_CLIENT_CERT_CONTEXT, 
				(void *)m_pCurSignCert, sizeof(CERT_CONTEXT)) )
			{
				if ( hReq )InternetCloseHandle(hReq);
				if ( hConnect ) InternetCloseHandle(hConnect);
				if ( hOpen ) InternetCloseHandle(hOpen);
				return FALSE;
			}
		}
	}

#ifdef	CUSTOMHTTPUSERPASSOLD
#pragma message("!!!!!!!!!注意!!!!!!!!! CUSTOMHTTPUSERPASS 被定义!" __FILE__)
	HttpSendRequest (hReq, NULL, 0, NULL, 0); // at this point normal authentication logic can be used. If
	// credentials are supplied in InternetConnect, then Wininet will
	// resubmit credentials itself.  See HttpDump Internet Client SDK sample
	// for more information.  // Read all returned data with InternetReadFile () 
	
	BYTE       *rgbBuf;
	rgbBuf = NULL;
	rgbBuf = new BYTE[NTKO_DEF_NETREADBUFFERSIZE ]; //a 10-k buffer for reading
	if (!rgbBuf) 
	{
		if ( hReq )InternetCloseHandle(hReq);
		if ( hConnect ) InternetCloseHandle(hConnect);
		if ( hOpen ) InternetCloseHandle(hOpen);
		return FALSE; 
	}
	do
	{
		InternetReadFile (hReq,(void*)rgbBuf, NTKO_DEF_NETREADBUFFERSIZE , &cbRead);
	}
	while ( cbRead != 0); // Now send real request that will be send with HttpSendRequestEx. By
	if(rgbBuf)
	{
		delete [] rgbBuf;
		rgbBuf = NULL;
	}
	// this time all authentication is done // Note that we are using the same handle as HttpSendRequest<BR/>
#endif	

	dw = m_dwReceiveDataTimeOut * 1000;
	if (!InternetSetOption(hReq, INTERNET_OPTION_SEND_TIMEOUT,
		&dw, sizeof(dw)) )
	{
		if ( hReq )InternetCloseHandle(hReq);
		if ( hConnect ) InternetCloseHandle(hConnect);
		if ( hOpen ) InternetCloseHandle(hOpen);
		return FALSE; 
	};
	
	INTERNET_BUFFERS BufferIn;
	BufferIn.dwStructSize = sizeof( INTERNET_BUFFERS ); // Must be set or error will occur
    BufferIn.Next = NULL; 
    BufferIn.lpcszHeader = NULL;
    BufferIn.dwHeadersLength = 0;
    BufferIn.dwHeadersTotal = 0;
    BufferIn.lpvBuffer = NULL;                
    BufferIn.dwBufferLength = 0;
    BufferIn.dwBufferTotal = 0; 
    BufferIn.dwOffsetLow = 0;
    BufferIn.dwOffsetHigh = 0;

	char CookieHeader[NTKO_MAX_HTTPHEADER];
	char tBuffer[512];

	BOOL isOk = FALSE;
	LPSTR	pstrCookieHeader = NULL;

	memset(CookieHeader,0,sizeof(CookieHeader));
	memset(tBuffer,0,sizeof(tBuffer));

	isOk = ReadRegisterInfo(HKEY_CURRENT_USER, 
		"Software\\Microsoft\\Internet Explorer\\International","AcceptLanguage",
					tBuffer,sizeof(tBuffer));
	if(isOk)
	{
		lstrcpyA(CookieHeader,"Accept-Language: ");
		lstrcatA(CookieHeader,tBuffer);
		lstrcatA(CookieHeader,"\r\n");
		pstrCookieHeader = CookieHeader;
	}

	dw = pstrCookieHeader?strlen(pstrCookieHeader):0;
	if(dw > 0)
	{
		// Add multipart/form-data header;
		if	( !HttpAddRequestHeaders
				(
				hReq,
				pstrCookieHeader,
				dw,
				HTTP_ADDREQ_FLAG_ADD || HTTP_ADDREQ_FLAG_REPLACE
				) 
			)
		{
			if ( hReq )InternetCloseHandle(hReq);
			if ( hConnect ) InternetCloseHandle(hConnect);
			if ( hOpen ) InternetCloseHandle(hOpen);
			return FALSE; 
		}
	}

	char HTML_HEADER_CONTENTTYPE[MAX_HEADER_LEN];
	
	lstrcpyn(HTML_HEADER_CONTENTTYPE,"Content-Type: application/x-www-form-urlencoded" HTML_HEADER_CR,MAX_HEADER_LEN);

	// Add Content-Type application/x-www-form-urlencoded header;
	if	( !HttpAddRequestHeaders
		(
		hReq,
		HTML_HEADER_CONTENTTYPE,
		lstrlenA(HTML_HEADER_CONTENTTYPE),
		HTTP_ADDREQ_FLAG_ADD || 
		HTTP_ADDREQ_FLAG_REPLACE
		) 
		)
	{
		if ( hReq )InternetCloseHandle(hReq);
		if ( hConnect ) InternetCloseHandle(hConnect);
		if ( hOpen ) InternetCloseHandle(hOpen);
		return FALSE; 
	}
	
	//generate Multipart/form-data Other Header data
	//pstrCPARA = ConvertToANSI(pwszCPARA); //For example: pa=v1&p2=v2&p3=v3...need free

	pstrCPARA = NtkoLPWSTRToUTF8(pwszCPARA); //need free

	LPSTR  pstrCPARAEncode = NtkoANSIToFormEncode(pstrCPARA);
	SAFE_FREESTRING(pstrCPARA);
	pstrCPARA = pstrCPARAEncode;

	pstrHead = pstrCPARA;

	dwCalMem = lstrlenA(pstrHead);

	BufferIn.dwBufferTotal += dwCalMem; //增加计算的需要发送的数据量 ＋自定义参数所有长度


#ifdef CUSTOMHTTPUSERPASS
resend:
#endif
		if (!HttpSendRequestEx
		(
		hReq,                                // request handle
		&BufferIn,                          // header
		NULL,                 // header length
		HSR_INITIATE,                      // optional data buffer pointer
		0                                    // optional buffer length
		) )
	{
		DWORD dwError = GetLastError();
		if ( dwError == ERROR_INTERNET_CLIENT_AUTH_CERT_NEEDED )
		{			
			if(SelectLogonCert(hReq))
			{
				goto resend;
			}
		}
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto errClr;
	};
	
	sum = 0;
	
	//写入cpara参数标识的参数头部和数据
	if(pstrHead)
	{
		if (!InternetWriteFile (hReq, (void*)pstrHead, dwCalMem, &cbWritten))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			goto errClr;
		}
		sum += cbWritten;
	}
	

	hr = (BufferIn.dwBufferTotal != sum) ? E_UNEXPECTED: S_OK;
	if(FAILED(hr))
	{
		goto errClr;
	}

	if(!HttpEndRequest(hReq, NULL, 0, 0))
	{
		//ODS("!!!!!!--CNtkoWoControl::UpLoadILockBytesToURL:HttpEndRequest ERROR!\n");		
		//OnStatusChange(NTKO_OFFICECONTROL_NETERROR,2);
		hr = HRESULT_FROM_WIN32(GetLastError());
#ifdef CUSTOMHTTPUSERPASS
		DWORD	dwError;
		dwError = GetLastError();
		if (dwError == ERROR_INTERNET_FORCE_RETRY)
		{
			goto resend;
		}
#endif
    }

	// 获取返回值.

	if(!GetHTTPReturnData(hReq,pbstr))
	{		
		hr = E_FAIL;
	}

errClr:
	SAFE_FREESTRING(pstrHead);
	if ( hReq )InternetCloseHandle(hReq);
	if ( hConnect ) InternetCloseHandle(hConnect);
	if ( hOpen ) InternetCloseHandle(hOpen);
	return FAILED(hr)?FALSE:TRUE;
}

BOOL CNTKOServerSignHelper::ParseUserRights()
{
	HRESULT hr = S_OK;
	BOOL isOk = FALSE;
	IXMLDOMNode *pIDOMNode = NULL;

	hr = m_pXMLDoc->selectSingleNode( OLESTR("//ResponseInfo"), &pIDOMNode);
	if(FAILED(hr) || (!pIDOMNode))
	{
		return FALSE;
	}
		
	IXMLDOMNode *pIDOMNode2 = NULL;
	
	//除了用户注销请求之外，其它都需要解析用户权限
	hr = pIDOMNode->selectSingleNode( OLESTR("./UserRights"), &pIDOMNode2);
	if(SUCCEEDED(hr) && pIDOMNode2)
	{
		IXMLDOMNamedNodeMap *pAttributeMap = NULL;
		hr = pIDOMNode2->get_attributes(&pAttributeMap);
		if(SUCCEEDED(hr) && pAttributeMap)
		{
			IXMLDOMNode *pIDOMNode3 = NULL;
			//获取canDoHandSign
			hr = pAttributeMap->getNamedItem(L"canDoHandSign",&pIDOMNode3);
			if(SUCCEEDED(hr) && pIDOMNode3)
			{
				BSTR bstrTemp = NULL;
				pIDOMNode3->get_text(&bstrTemp);
				if(bstrTemp)
				{
					if( NtkoAreStringsEqualW(bstrTemp,-1,L"true",-1) )
					{
						m_serverRightSettings.bUseHandsign = TRUE;
						isOk = TRUE;
					}
					else if( NtkoAreStringsEqualW(bstrTemp,-1,L"false",-1) )
					{
						m_serverRightSettings.bUseHandsign = FALSE;
						isOk = TRUE;
					}
					else
					{
						isOk = FALSE;
					}
				}
				SAFE_FREEBSTRING(bstrTemp);								
				RELEASE_INTERFACE(pIDOMNode3);
			}

			//canDoBarcode
			hr = pAttributeMap->getNamedItem(L"canDoBarcode",&pIDOMNode3);
			if(SUCCEEDED(hr) && pIDOMNode3)
			{
				BSTR bstrTemp = NULL;
				pIDOMNode3->get_text(&bstrTemp);
				if(bstrTemp)
				{
					if( NtkoAreStringsEqualW(bstrTemp,-1,L"true",-1) )
					{
						m_serverRightSettings.bUseBarcode = TRUE;
						isOk = TRUE;
					}
					else if( NtkoAreStringsEqualW(bstrTemp,-1,L"false",-1) )
					{
						m_serverRightSettings.bUseBarcode = FALSE;
						isOk = TRUE;
					}
					else
					{
						isOk = FALSE;
					}
				}
				SAFE_FREEBSTRING(bstrTemp);								
				RELEASE_INTERFACE(pIDOMNode3);
			}
			//canDoKeyBoardComment
			if(isOk)
			{
				isOk = FALSE;
				hr = pAttributeMap->getNamedItem(L"canDoKeyBoardComment",&pIDOMNode3);
				if(SUCCEEDED(hr) && pIDOMNode3)
				{
					BSTR bstrTemp = NULL;
					pIDOMNode3->get_text(&bstrTemp);
					if(bstrTemp)
					{
						if( NtkoAreStringsEqualW(bstrTemp,-1,L"true",-1) )
						{
							m_serverRightSettings.bUseKeyboardsign = TRUE;
							isOk = TRUE;
						}
						else if( NtkoAreStringsEqualW(bstrTemp,-1,L"false",-1) )
						{
							m_serverRightSettings.bUseKeyboardsign = FALSE;
							isOk = TRUE;
						}
						else
						{
							isOk = FALSE;
						}
					}
					SAFE_FREEBSTRING(bstrTemp);								
					RELEASE_INTERFACE(pIDOMNode3);
				}
			}

			//canDoSign
			if(isOk)
			{
				isOk = FALSE;
				hr = pAttributeMap->getNamedItem(L"canDoSign",&pIDOMNode3);
				if(SUCCEEDED(hr) && pIDOMNode3)
				{
					BSTR bstrTemp = NULL;
					pIDOMNode3->get_text(&bstrTemp);
					if(bstrTemp)
					{
						if( NtkoAreStringsEqualW(bstrTemp,-1,L"true",-1) )
						{
							m_serverRightSettings.bUseNormalsign = TRUE;
							isOk = TRUE;
						}
						else if( NtkoAreStringsEqualW(bstrTemp,-1,L"false",-1) )
						{
							m_serverRightSettings.bUseNormalsign = FALSE;
							isOk = TRUE;
						}
						else
						{
							isOk = FALSE;
						}
					}
					SAFE_FREEBSTRING(bstrTemp);								
					RELEASE_INTERFACE(pIDOMNode3);
				}
			}

			//canDoEkeySign
			if(isOk)
			{
				isOk = FALSE;
				hr = pAttributeMap->getNamedItem(L"canDoEkeySign",&pIDOMNode3);
				if(SUCCEEDED(hr) && pIDOMNode3)
				{
					BSTR bstrTemp = NULL;
					pIDOMNode3->get_text(&bstrTemp);
					if(bstrTemp)
					{
						if( NtkoAreStringsEqualW(bstrTemp,-1,L"true",-1) )
						{
							m_serverRightSettings.bUseEkeySign = TRUE;
							isOk = TRUE;
						}
						else if( NtkoAreStringsEqualW(bstrTemp,-1,L"false",-1) )
						{
							m_serverRightSettings.bUseEkeySign = FALSE;
							isOk = TRUE;
						}
						else
						{
							isOk = FALSE;
						}
					}
					SAFE_FREEBSTRING(bstrTemp);								
					RELEASE_INTERFACE(pIDOMNode3);
				}
			}
			RELEASE_INTERFACE(pAttributeMap);
		}
		RELEASE_INTERFACE(pIDOMNode2);
	}
	RELEASE_INTERFACE(pIDOMNode);


#pragma message(NTKOPRGWARNNING"═════待修改 @TODO ══════ ★★★★★★★★★★★★★★★ 服务器端没有配置多页骑缝盖章,因此和ekey权限一致!☆☆☆☆☆☆☆☆☆☆" )
	m_serverRightSettings.bUseDbCrossPagesign = m_serverRightSettings.bUseEkeySign;
	m_serverRightSettings.bUseMutiCrossPageSign = m_serverRightSettings.bUseEkeySign;
	m_serverRightSettings.bUseMutisign = m_serverRightSettings.bUseEkeySign;
	m_serverRightSettings.bUseMutisignEkey = m_serverRightSettings.bUseEkeySign;

	return isOk;
}

BOOL CNTKOServerSignHelper::ParseResponseStatus()
{
	HRESULT hr = S_OK;
	BOOL isOk = FALSE;
	IXMLDOMNode *pIDOMNode = NULL;	
	
	m_iServerErrCode = -1;

	if(!m_pXMLDoc)
	{
		return FALSE;
	}

	//验证根节点是否是NtkoSecsignResponse
	hr = m_pXMLDoc->selectSingleNode( OLESTR("/*"), &pIDOMNode);
	if(FAILED(hr) || (!pIDOMNode))
	{	
		return FALSE;
	}
	else
	{
		BSTR bstrRootNode = NULL;
		pIDOMNode->get_nodeName(&bstrRootNode);
		if(NtkoAreStringsEqualW(bstrRootNode,-1,L"NtkoSecsignResponse",-1))
		{
			isOk = TRUE;
		}
		else
		{
			isOk = FALSE;
		}
		SAFE_FREEBSTRING(bstrRootNode);	
		RELEASE_INTERFACE(pIDOMNode);
	}

	if(!isOk)
	{
		return FALSE;
	}

	isOk = FALSE;
	
	//解析ResponseStatus
	hr = m_pXMLDoc->selectSingleNode( OLESTR("//ResponseStatus"), &pIDOMNode);
	if(FAILED(hr) || (!pIDOMNode))
	{
		return FALSE;
	}
		
	IXMLDOMNode *pIDOMNode2 = NULL;
	//获取服务器版本号
	hr = pIDOMNode->selectSingleNode( OLESTR("./ServerVersion"), &pIDOMNode2);
	if(SUCCEEDED(hr) && pIDOMNode2)
	{
		IXMLDOMNamedNodeMap *pAttributeMap = NULL;
		hr = pIDOMNode2->get_attributes(&pAttributeMap);
		if(SUCCEEDED(hr) && pAttributeMap)
		{
			IXMLDOMNode *pIDOMNode3 = NULL;
			hr = pAttributeMap->getNamedItem(L"version",&pIDOMNode3);
			if(SUCCEEDED(hr) && pIDOMNode3)
			{
				SAFE_FREEBSTRING(m_bstrServerVersion);
				pIDOMNode3->get_text(&m_bstrServerVersion);
				RELEASE_INTERFACE(pIDOMNode3);
				if(::SysStringByteLen(m_bstrServerVersion))				
				{
					isOk = TRUE;
				}
				else
				{
					isOk = FALSE;
				}
			}
			RELEASE_INTERFACE(pAttributeMap);
		}
		RELEASE_INTERFACE(pIDOMNode2);
	}

	if(!isOk)
	{
		RELEASE_INTERFACE(pIDOMNode);
		return FALSE;
	}

	isOk = FALSE;

	//获取服务器返回状态
	hr = pIDOMNode->selectSingleNode( OLESTR("./Status"), &pIDOMNode2);
	if(SUCCEEDED(hr) && pIDOMNode2)
	{
		IXMLDOMNamedNodeMap *pAttributeMap = NULL;
		hr = pIDOMNode2->get_attributes(&pAttributeMap);
		if(SUCCEEDED(hr) && pAttributeMap)
		{
			IXMLDOMNode *pIDOMNode3 = NULL;
			//获取状态码
			hr = pAttributeMap->getNamedItem(L"errCode",&pIDOMNode3);
			if(SUCCEEDED(hr) && pIDOMNode3)
			{
				BSTR bstrErrCode = NULL;
				pIDOMNode3->get_text(&bstrErrCode);
				if(bstrErrCode)
				{
					if(!NtkoStrToIntExW(bstrErrCode,STIF_DEFAULT,&m_iServerErrCode))
					{
						m_iServerErrCode = -1;
						isOk = FALSE;
					}
					else
					{
						isOk = TRUE;
					}
				}
				SAFE_FREEBSTRING(bstrErrCode);								
				RELEASE_INTERFACE(pIDOMNode3);
			}
			//获取失败原因
			if(isOk)
			{
				isOk = FALSE;
				hr = pAttributeMap->getNamedItem(L"failReason",&pIDOMNode3);
				if(SUCCEEDED(hr) && pIDOMNode3)
				{
					SAFE_FREEBSTRING(m_bstrFailReason);
					pIDOMNode3->get_text(&m_bstrFailReason);							
					RELEASE_INTERFACE(pIDOMNode3);
					isOk = TRUE;
				}
			}
			RELEASE_INTERFACE(pAttributeMap);
		}
		RELEASE_INTERFACE(pIDOMNode2);
	}
	RELEASE_INTERFACE(pIDOMNode);
	return isOk;
}

BOOL CNTKOServerSignHelper::IsInternetSettingEnableHTTP11()
{
	HKEY    hk;
	DWORD   dwret;
	DWORD	dwValue = 1;
	DWORD	cSize = sizeof(dwValue);
	/*
	http://support.microsoft.com/kb/258425/EN-US/
	For typical connections: 
	HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Internet Settings
	"EnableHttp1_1"=dword:00000001 (default for HTTP/1.1 enabled) 
	For proxy connections: 
	HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Internet Settings
	"ProxyHttp1.1"=dword:00000000 (default for HTTP/1.1 disabled through proxy) 
	*/
	CHAR  subkey[256];
	ZeroMemory(subkey,256);
	
	lstrcpy(subkey,"Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings");
	
	if ((dwret = RegOpenKeyEx(HKEY_CURRENT_USER,subkey, 0, KEY_READ, &hk)) == ERROR_SUCCESS)
	{
		dwret = RegQueryValueEx(hk, "EnableHttp1_1", NULL, NULL, (BYTE*)&dwValue, &cSize);
		RegCloseKey(hk);
	}	
	return (1 == dwValue)?TRUE:FALSE;
}

BOOL CNTKOServerSignHelper::DoLogout()
{
	if(!m_IsLogin) return TRUE;

	BOOL isOk = FALSE;
	BSTR bstrRetXML = NULL;
	m_serverReqestType = NTKOServerRequestType_Logout;
	ShowDelayDlgMUIStrW(IDSMUI_LOGINGOFFSERVER);
	isOk = DoWebGet(m_rootSvrUrl,NTKO_HC_LOGOFF_URL,&bstrRetXML);		
	CloseDelayDlg();

	ReleaseCurCert();
	ReleaseCurStore();
	
#if 0
	if(isOk && bstrRetXML)
	{
		isOk = ParseXML(bstrRetXML);
		if(isOk) //解析成功，但是服务器有可能有错误
		{ 
			if( 0 == m_iServerErrCode) 
			{
				if(NtkoAreStringsEqualW(m_bstrServerVersion,-1,NTKO_ServerSecsignControlProtocalVersion,-1))
				{
					isOk = TRUE;
				}
				else
				{
					ShowMsgBoxMUIStrW(IDSMUI_LOGINSERVER_FAIL_INVALIDSERVER_VERSION,
						IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
					isOk = FALSE;
				}
			}
			else //服务器返回有错误
			{
				WCHAR wszBuf[1024];
				LPWSTR pwszFmt = LoadMUIStringW(IDSMUI_LOGINSERVER_FAIL_SERVERFAILWITHCODE);
				NtkoFwsprintf(wszBuf,pwszFmt,m_iServerErrCode,m_bstrFailReason);
				SAFE_FREESTRING(pwszFmt);
				
				LPWSTR pwszMUITitle = LoadMUIStringW(IDSMUI_MSGBOXTITLE_TIP);
				NtkoShowMsgBox(m_hwndParent,wszBuf,pwszMUITitle ,MB_OK|MB_ICONINFORMATION);
				SAFE_FREESTRING(pwszMUITitle);
				
				isOk = FALSE;
			}
		}
		else //解析错误
		{
			ShowMsgBoxMUIStrW(IDSMUI_LOGINSERVER_FAIL_PARSERESPONSEERROR,
				IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
			isOk = FALSE;
		}
	}
	else
	{			
		ShowMsgBoxMUIStrW(IDSMUI_LOGINSERVER_FAIL,IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		isOk = FALSE;
	}
#endif
	SAFE_FREEBSTRING(bstrRetXML);
	m_IsLogin = isOk?FALSE:TRUE;
	ZeroMemory(m_LoginUserName,sizeof(m_LoginUserName) );
	ZeroMemory(m_LoginPassword,sizeof(m_LoginPassword) );
	return isOk;
}

BOOL CNTKOServerSignHelper::CheckEKEYUsageWithMes(DWORD ekeyUsage,DWORD usage)
{
	BOOL isOk = FALSE;
	
	if(NTKO_EKEYUSAGE_EKEY_ALL == ekeyUsage) //OFFICE印章,兼容老版本
	{
		return TRUE;
	}
	
	isOk = (usage==(ekeyUsage&usage))?TRUE:FALSE;
	
	if(!isOk)
	{
		ShowMsgBoxMUIStrW(IDSMUI_EKEYMES_ERR_USAGE,IDSMUI_MSGBOXTITLE_TIP,MB_OK | MB_ICONINFORMATION);
	}
	return isOk;
}

BOOL CNTKOServerSignHelper::CheckOpRight(NTKOServerRequestType opType)
{
	BOOL isOk = FALSE;

	m_serverReqestType = opType;

	switch(opType)
	{
	case NTKOServerRequestType_GetSignList: 
		{
			isOk = CheckServerBeforeNomalSign();
		}
		break;
	case NTKOServerRequestType_GetEkeyUseableState:
		{
			isOk = CheckServerBeforeEkeySign();
		}
		break;
	case NTKOServerRequestType_CheckRightHandsign:
		{
			isOk = CheckServerBeforeHandSign();
		}
		break;
	case NTKOServerRequestType_CheckRightKeyBoradComment:
		{
			isOk = CheckServerBeforeKeyboard();
		}
		break;
	case NTKOServerRequestType_CheckRightBarcode:
		{
#pragma message(NTKOPRGWARNNING "═════待修改 @TODO ══════ ★★★★★★★★★★★★★★★ 由于协议未修改，暂时先写成检查手写签名的权限。最终应修改为 CheckServerBeforeBarcode ☆☆☆☆☆☆☆☆☆☆")
			// 由于协议未修改，暂时先写成检查手写签名的权限。
			isOk = CheckServerBeforeHandSign();
		}
		break;
	default:
		{
			return FALSE;
		}
		break;
	}
	return isOk;
}

BOOL CNTKOServerSignHelper::CheckServerBeforeHandSign()
{
	BOOL isOk = FALSE;
	BSTR bstrRetXML = NULL;
	
	ShowDelayDlgMUIStrW(IDSMUI_CHECKING_USERRIGHTS);
	isOk = DoWebGet(m_rootSvrUrl,NTKO_HC_GETRIGHTS_URL,&bstrRetXML);	
	CloseDelayDlg();
	if(isOk && bstrRetXML)
	{
		isOk = ParseXML(bstrRetXML);
		if(isOk) //解析成功，但是服务器有可能有错误
		{ 
			if( 0 == m_iServerErrCode) 
			{
				if(NtkoAreStringsEqualW(m_bstrServerVersion,-1,NTKO_ServerSecsignControlProtocalVersion,-1))
				{
					isOk = TRUE;
				}
				else
				{
					ShowMsgBoxMUIStrW(IDSMUI_CHECKRIGHT_FAIL_INVALIDSERVER_VERSION,
						IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
					isOk = FALSE;
				}
			}
			else //服务器返回有错误
			{
				WCHAR wszBuf[1024];
				LPWSTR pwszFmt = LoadMUIStringW(IDSMUI_CHECKRIGHT_FAIL_SERVERFAILWITHCODE);
				NtkoFwsprintf(wszBuf,pwszFmt,m_iServerErrCode,m_bstrFailReason);
				SAFE_FREESTRING(pwszFmt);
				
				LPWSTR pwszMUITitle = LoadMUIStringW(IDSMUI_MSGBOXTITLE_TIP);
				NtkoMessageBoxW(m_hwndParent,wszBuf,pwszMUITitle ,MB_OK|MB_ICONINFORMATION);
				SAFE_FREESTRING(pwszMUITitle);
				
				if(1 == m_iServerErrCode)
				{
					m_IsLogin = FALSE;
				}
				isOk = FALSE;
			}
		}
		else //解析错误
		{
			ShowMsgBoxMUIStrW(IDSMUI_CHECKRIGHT_FAIL_PARSERESPONSEERROR,
				IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
			isOk = FALSE;
		}
	}
	else
	{			
		ShowMsgBoxMUIStrW(IDSMUI_CHECKRIGHT_FAIL,IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		isOk = FALSE;
	}
	
	if(isOk)
	{
		isOk = m_serverRightSettings.bUseHandsign;
		
		if(!isOk)
		{
			ShowMsgBoxMUIStrW(IDSMUI_CHECKRIGHT_USER_NORIGHT,IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		}
		else
		{
			isOk = ParseSignOptions();
			if(!isOk)
			{
				ShowMsgBoxMUIStrW(IDSMUI_GETSIGNOPTIONS_FAIL_HANDSIGNERROR_PARSEERROR,
					IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
			}
		}
	}
	SAFE_FREEBSTRING(bstrRetXML);
	return isOk;
}
BOOL CNTKOServerSignHelper::CheckServerBeforeBarcode()
{
	BOOL isOk = FALSE;
	BSTR bstrRetXML = NULL;
	
	ShowDelayDlgMUIStrW(IDSMUI_CHECKING_USERRIGHTS);
	isOk = DoWebGet(m_rootSvrUrl,NTKO_HC_GETRIGHTS_URL,&bstrRetXML);	
	CloseDelayDlg();
	if(isOk && bstrRetXML)
	{
		isOk = ParseXML(bstrRetXML);
		if(isOk) //解析成功，但是服务器有可能有错误
		{ 
			if( 0 == m_iServerErrCode) 
			{
				if(NtkoAreStringsEqualW(m_bstrServerVersion,-1,NTKO_ServerSecsignControlProtocalVersion,-1))
				{
					isOk = TRUE;
				}
				else
				{
					ShowMsgBoxMUIStrW(IDSMUI_CHECKRIGHT_FAIL_INVALIDSERVER_VERSION,
						IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
					isOk = FALSE;
				}
			}
			else //服务器返回有错误
			{
				WCHAR wszBuf[1024];
				LPWSTR pwszFmt = LoadMUIStringW(IDSMUI_CHECKRIGHT_FAIL_SERVERFAILWITHCODE);
				NtkoFwsprintf(wszBuf,pwszFmt,m_iServerErrCode,m_bstrFailReason);
				SAFE_FREESTRING(pwszFmt);
				
				LPWSTR pwszMUITitle = LoadMUIStringW(IDSMUI_MSGBOXTITLE_TIP);
				NtkoMessageBoxW(m_hwndParent,wszBuf,pwszMUITitle ,MB_OK|MB_ICONINFORMATION);
				SAFE_FREESTRING(pwszMUITitle);
				
				if(1 == m_iServerErrCode)
				{
					m_IsLogin = FALSE;
				}
				isOk = FALSE;
			}
		}
		else //解析错误
		{
			ShowMsgBoxMUIStrW(IDSMUI_CHECKRIGHT_FAIL_PARSERESPONSEERROR,
				IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
			isOk = FALSE;
		}
	}
	else
	{			
		ShowMsgBoxMUIStrW(IDSMUI_CHECKRIGHT_FAIL,IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		isOk = FALSE;
	}
	
	if(isOk)
	{
		isOk = m_serverRightSettings.bUseBarcode;
		
		if(!isOk)
		{
			ShowMsgBoxMUIStrW(IDSMUI_CHECKRIGHT_USER_NORIGHT,IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		}
		else
		{
			isOk = ParseSignOptions();
			if(!isOk)
			{
				ShowMsgBoxMUIStrW(IDSMUI_GETSIGNOPTIONS_FAIL_HANDSIGNERROR_PARSEERROR,
					IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
			}
		}
	}
	SAFE_FREEBSTRING(bstrRetXML);
	return isOk;
}
BOOL CNTKOServerSignHelper::CheckServerBeforeKeyboard()
{
	BOOL isOk = FALSE;
	BSTR bstrRetXML = NULL;
	
	ShowDelayDlgMUIStrW(IDSMUI_CHECKING_USERRIGHTS);
	isOk = DoWebGet(m_rootSvrUrl,NTKO_HC_GETRIGHTS_URL,&bstrRetXML);	
	CloseDelayDlg();
	if(isOk && bstrRetXML)
	{
		isOk = ParseXML(bstrRetXML);
		if(isOk) //解析成功，但是服务器有可能有错误
		{ 
			if( 0 == m_iServerErrCode) 
			{
				if(NtkoAreStringsEqualW(m_bstrServerVersion,-1,NTKO_ServerSecsignControlProtocalVersion,-1))
				{
					isOk = TRUE;
				}
				else
				{
					ShowMsgBoxMUIStrW(IDSMUI_CHECKRIGHT_FAIL_INVALIDSERVER_VERSION,
						IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
					isOk = FALSE;
				}
			}
			else //服务器返回有错误
			{
				WCHAR wszBuf[1024];
				LPWSTR pwszFmt = LoadMUIStringW(IDSMUI_CHECKRIGHT_FAIL_SERVERFAILWITHCODE);
				NtkoFwsprintf(wszBuf,pwszFmt,m_iServerErrCode,m_bstrFailReason);
				SAFE_FREESTRING(pwszFmt);
				
				LPWSTR pwszMUITitle = LoadMUIStringW(IDSMUI_MSGBOXTITLE_TIP);
				NtkoMessageBoxW(m_hwndParent,wszBuf,pwszMUITitle ,MB_OK|MB_ICONINFORMATION);
				SAFE_FREESTRING(pwszMUITitle);
				
				if(1 == m_iServerErrCode)
				{
					m_IsLogin = FALSE;
				}
				isOk = FALSE;
			}
		}
		else //解析错误
		{
			ShowMsgBoxMUIStrW(IDSMUI_CHECKRIGHT_FAIL_PARSERESPONSEERROR,
				IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
			isOk = FALSE;
		}
	}
	else
	{			
		ShowMsgBoxMUIStrW(IDSMUI_CHECKRIGHT_FAIL,IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		isOk = FALSE;
	}
	
	if(isOk)
	{
		isOk = m_serverRightSettings.bUseKeyboardsign;
		
		if(!isOk)
		{
			ShowMsgBoxMUIStrW(IDSMUI_CHECKRIGHT_USER_NORIGHT,IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		}
		else
		{
			isOk = ParseSignOptions();
			if(!isOk)
			{
				ShowMsgBoxMUIStrW(IDSMUI_GETSIGNOPTIONS_FAIL_HANDSIGNERROR_PARSEERROR,
					IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
			}
		}
	}
	SAFE_FREEBSTRING(bstrRetXML);
	return isOk;
}

BOOL CNTKOServerSignHelper::CheckServerBeforeEkeySign()
{
	BOOL isOk = FALSE;
	BSTR bstrRetXML = NULL;
	WCHAR wszCPARA[1024];
	
	if(!m_bstrEKEYSN)
	{
		NtkoCmnDlgParam dlgParam;
		ZeroMemory(&dlgParam,sizeof(dlgParam));
		
		dlgParam.dwFlags = NTKO_CDPF_VERIFYEKEY | NTKO_CDPF_GETEKEYSN;
		dlgParam.EkeyType = m_secsignSettings.ekeyType;
		dlgParam.EkeyVersion = NTKO_EKEYVER_AUTO;
		dlgParam.EkeyIndex = -1;
		
		int dlgRet = IDCANCEL;
		//提示选择EKEY
		LPWSTR pwszTitle = LoadMUIStringW(IDSMUI_SELEKEYDLGTITLE);
		dlgRet = NtkoShowGetEkeyAndPassDlg(m_hwndParent,m_EKEYPassword,NTKO_MINEKEY_PASSWORD,NTKO_MAXEKEY_PASSWORD,pwszTitle,&dlgParam);
		SAFE_FREESTRING(pwszTitle);
		if ((IDOK != dlgRet) || (!CheckEKEYUsageWithMes(dlgParam.dwEkeyUsage)) )
		{			
			SAFE_FREEBSTRING(dlgParam.bstrEkeySN);			
			return FALSE;
		}
		m_bstrEKEYSN = ::SysAllocString(dlgParam.bstrEkeySN);
		SAFE_FREEBSTRING(dlgParam.bstrEkeySN);			
		
	}
	
	//构造参数
	ZeroMemory(&wszCPARA[0],sizeof(wszCPARA));
	lstrcatW(wszCPARA,L"ekeysn=");
	lstrcatW(wszCPARA,(LPWSTR)m_bstrEKEYSN);
	
	//向服务器提交请求
	ShowDelayDlgMUIStrW(IDSMUI_CHECKING_USERRIGHTS);
	isOk = DoWebExecute2(m_rootSvrUrl,NTKO_HC_CHECKEKEY_URL,wszCPARA,&bstrRetXML);		
	CloseDelayDlg();
	if(isOk && bstrRetXML)
	{
		isOk = ParseXML(bstrRetXML);
		if(isOk) //解析成功，但是服务器有可能有错误
		{ 
			if( 0 == m_iServerErrCode) 
			{
				if(NtkoAreStringsEqualW(m_bstrServerVersion,-1,NTKO_ServerSecsignControlProtocalVersion,-1))
				{
					isOk = TRUE;
				}
				else
				{
					ShowMsgBoxMUIStrW(IDSMUI_CHECKRIGHT_FAIL_INVALIDSERVER_VERSION,
						IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
					isOk = FALSE;
				}
			}
			else //服务器返回有错误
			{
				WCHAR wszBuf[1024];
				LPWSTR pwszFmt = LoadMUIStringW(IDSMUI_CHECKRIGHT_FAIL_SERVERFAILWITHCODE);
				NtkoFwsprintf(wszBuf,pwszFmt,m_iServerErrCode,m_bstrFailReason);
				SAFE_FREESTRING(pwszFmt);
				
				LPWSTR pwszMUITitle = LoadMUIStringW(IDSMUI_MSGBOXTITLE_TIP);
				NtkoMessageBoxW(m_hwndParent,wszBuf,pwszMUITitle ,MB_OK|MB_ICONINFORMATION);
				SAFE_FREESTRING(pwszMUITitle);
				
				if(1 == m_iServerErrCode)
				{
					m_IsLogin = FALSE;
				}
				isOk = FALSE;
			}
		}
		else //解析错误
		{
			ShowMsgBoxMUIStrW(IDSMUI_CHECKRIGHT_FAIL_PARSERESPONSEERROR,
				IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
			isOk = FALSE;
		}
	}
	else
	{			
		ShowMsgBoxMUIStrW( IDSMUI_CHECKRIGHT_FAIL,IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		isOk = FALSE;
	}
	if(isOk)
	{
		isOk = m_serverRightSettings.bUseEkeySign;
		if(!isOk)
		{
			ShowMsgBoxMUIStrW(IDSMUI_CHECKRIGHT_USER_NORIGHT,IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
			goto out;
		}
		isOk = ParseSignOptions();
		if(!isOk)
		{
			ShowMsgBoxMUIStrW(IDSMUI_GETSIGNOPTIONS_FAIL_EKEYSIGNERROR_PARSEERROR,
				IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		}
		else
		{
			isOk = ParseNtkoCheckEkey();
			if(!isOk)
			{
				ShowMsgBoxMUIStrW(IDSMUI_CHECKRIGHT_FAIL_EKEYSIGNERROR_PARSEERROR,
					IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
			}
			else
			{
				isOk = m_CanUseThisEkey;
				if(!isOk)
				{
					ShowMsgBoxMUIStrW(IDSMUI_CHECKRIGHT_USER_NORIGHT_EKEY,IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
					goto out;
				}
			}
		}
	}
out:
	SAFE_FREEBSTRING(bstrRetXML);
	return isOk;
}

BOOL CNTKOServerSignHelper::CheckServerBeforeNomalSign()
{
	BOOL isOk = FALSE;
	BSTR bstrRetXML = NULL;
	
	ShowDelayDlgMUIStrW(IDSMUI_CHECKING_USERRIGHTS_ANDREADSIGNLIST);
#if !defined(NTKO_CUSTOM_JIANGSU_CA)
	isOk = DoWebGet(m_rootSvrUrl,NTKO_HC_LISTSIGNS_URL,&bstrRetXML);	
#else
	if(m_IsJSCAUseSealSvr)
	{
	#if 0 //初次版本的江苏ca请求。现在的版本客户端不需要直接和seal服务器打交道，禁用，保留代码备查
		WCHAR doWebGetParams[512] = {0};
		LPWSTR timeStamp = NULL;
		LPWSTR key = NULL;
		if ( !NTKO_Custom_CreateKey(&key,&timeStamp) )
		{
			return FALSE;
		}

		LPWSTR pwszUTF8Name = ConvertLPWSTRToUTF8HexW(m_Custom_jiangshuCA_CompanyInfo.companyName);
		NtkoFwsprintfSafe(doWebGetParams, sizeof(doWebGetParams),
			L"%s?ComName=%s&TimeStamp=%s&Key=%s",
			NTKO_HC_LISTSIGNS_URL_JSCA, 
			pwszUTF8Name,//m_Custom_jiangshuCA_CompanyInfo.companyName,
			timeStamp, key);

		SAFE_FREESTRING(pwszUTF8Name);

		SAFE_FREESTRING(timeStamp);
		SAFE_FREESTRING(key);

		isOk = DoWebGet(m_CustomSignServerUrl,doWebGetParams,&bstrRetXML);
	#else
		WCHAR doWebGetParams[512] = {0};
		LPWSTR timeStamp = NULL;
		LPWSTR key = NULL;
		if ( !NTKO_Custom_CreateKey(&key,&timeStamp) )
		{
			return FALSE;
		}

		LPWSTR pwszUTF8Name = ConvertLPWSTRToUTF8HexW(m_Custom_jiangshuCA_CompanyInfo.companyName);
		NtkoFwsprintfSafe(doWebGetParams, sizeof(doWebGetParams),
			L"%s?ComName=%s&TimeStamp=%s&Key=%s",
			NTKO_HC_LISTSIGNS_URL_JSCA, 
			pwszUTF8Name,//m_Custom_jiangshuCA_CompanyInfo.companyName,
			timeStamp, key);

		SAFE_FREESTRING(pwszUTF8Name);

		SAFE_FREESTRING(timeStamp);
		SAFE_FREESTRING(key);

		isOk = DoWebGet(m_rootSvrUrl,doWebGetParams,&bstrRetXML);
	#endif
	}
	else
	{
		isOk = DoWebGet(m_rootSvrUrl,NTKO_HC_LISTSIGNS_URL,&bstrRetXML);
	}
#endif
	CloseDelayDlg();
	if(isOk && bstrRetXML)
	{
		isOk = ParseXML(bstrRetXML);
		if(isOk) //解析成功，但是服务器有可能有错误
		{ 
			if( 0 == m_iServerErrCode) 
			{
				if(NtkoAreStringsEqualW(m_bstrServerVersion,-1,NTKO_ServerSecsignControlProtocalVersion,-1))
				{
					isOk = TRUE;
				}
				else
				{
					ShowMsgBoxMUIStrW(IDSMUI_CHECKRIGHT_FAIL_INVALIDSERVER_VERSION,
						IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
					isOk = FALSE;
				}
			}
			else //服务器返回有错误
			{
				WCHAR wszBuf[1024];
				LPWSTR pwszFmt = LoadMUIStringW(IDSMUI_CHECKRIGHT_FAIL_SERVERFAILWITHCODE);
				NtkoFwsprintf(wszBuf,pwszFmt,m_iServerErrCode,m_bstrFailReason);
				SAFE_FREESTRING(pwszFmt);
				
				LPWSTR pwszMUITitle = LoadMUIStringW(IDSMUI_MSGBOXTITLE_TIP);
				NtkoMessageBoxW(m_hwndParent,wszBuf,pwszMUITitle ,MB_OK|MB_ICONINFORMATION);
				SAFE_FREESTRING(pwszMUITitle);
				
				if(1 == m_iServerErrCode)
				{
					m_IsLogin = FALSE;
				}
				isOk = FALSE;
			}
		}
		else //解析错误
		{
			ShowMsgBoxMUIStrW(IDSMUI_CHECKRIGHT_FAIL_PARSERESPONSEERROR,
				IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
			isOk = FALSE;
		}
	}
	else
	{			
		ShowMsgBoxMUIStrW(IDSMUI_CHECKRIGHT_FAIL,IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		isOk = FALSE;
	}
	if(isOk)
	{		
		isOk = m_serverRightSettings.bUseNormalsign;
		if(!isOk)
		{
			ShowMsgBoxMUIStrW(IDSMUI_CHECKRIGHT_USER_NORIGHT,IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		}
		else
		{
			isOk = ParseNtkoUserSigns();
			if(!isOk)
			{
				ShowMsgBoxMUIStrW(IDSMUI_CHECKRIGHT_FAIL_SIGNERROR_PARSEERROR,
					IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
			}
			else if( m_iUserSignsCount<=0 )
			{
				ShowMsgBoxMUIStrW(IDSMUI_CHECKRIGHT_FAIL_SIGNERROR_NOAVAILSIGNS,
					IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
				isOk = FALSE;
			}

//初次版本的江苏ca请求。现在的版本客户端不需要直接和seal服务器打交道，禁用，保留代码备查
			// 江苏CA定制版本不能再这里解析,
			// 是在登录时解析签章选项
//#if !defined(NTKO_CUSTOM_JIANGSU_CA) 
			if ( isOk)
			{
				isOk = ParseSignOptions();
				if(!isOk)
				{
					ShowMsgBoxMUIStrW(IDSMUI_GETSIGNOPTIONS_FAIL_SIGNERROR_PARSEERROR,
						IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
				}
			}
//#endif
		}
	}
	SAFE_FREEBSTRING(bstrRetXML);
	return isOk;
}

BOOL CNTKOServerSignHelper::ParseSignOptions()
{
	HRESULT hr = S_OK;
	BOOL isOk = FALSE;
	IXMLDOMNode *pIDOMNode = NULL;
	
	hr = m_pXMLDoc->selectSingleNode( OLESTR("//ResponseInfo"), &pIDOMNode);
	if(FAILED(hr) || (!pIDOMNode))
	{
		return FALSE;
	}
	
	IXMLDOMNode *pIDOMNode2 = NULL;
	
	hr = pIDOMNode->selectSingleNode( OLESTR("./SignOptions"), &pIDOMNode2);
	if(SUCCEEDED(hr) && pIDOMNode2)
	{
		IXMLDOMNamedNodeMap *pAttributeMap = NULL;
		hr = pIDOMNode2->get_attributes(&pAttributeMap);
		if(SUCCEEDED(hr) && pAttributeMap)
		{
			IXMLDOMNode *pIDOMNode3 = NULL;
			//获取isCheckDocChange
			hr = pAttributeMap->getNamedItem(L"isCheckDocChange",&pIDOMNode3);
			if(SUCCEEDED(hr) && pIDOMNode3)
			{
				BSTR bstrTemp = NULL;
				pIDOMNode3->get_text(&bstrTemp);
				if(bstrTemp)
				{
					if( NtkoAreStringsEqualW(bstrTemp,-1,L"true",-1) )
					{
						m_secsignSettings.bCheckDocChange = TRUE;
						isOk = TRUE;
					}
					else if( NtkoAreStringsEqualW(bstrTemp,-1,L"false",-1) )
					{
						m_secsignSettings.bCheckDocChange = FALSE;
						isOk = TRUE;
					}
					else
					{
						isOk = FALSE;
					}
				}
				SAFE_FREEBSTRING(bstrTemp);								
				RELEASE_INTERFACE(pIDOMNode3);
			}
			//isAddSignComment
			if(isOk)
			{
				isOk = FALSE;
				hr = pAttributeMap->getNamedItem(L"isAddSignComment",&pIDOMNode3);
				if(SUCCEEDED(hr) && pIDOMNode3)
				{
					BSTR bstrTemp = NULL;
					pIDOMNode3->get_text(&bstrTemp);
					if(bstrTemp)
					{
						if( NtkoAreStringsEqualW(bstrTemp,-1,L"true",-1) )
						{
							m_secsignSettings.bComment = TRUE;
							isOk = TRUE;
						}
						else if( NtkoAreStringsEqualW(bstrTemp,-1,L"false",-1) )
						{
							m_secsignSettings.bComment = FALSE;
							isOk = TRUE;
						}
						else
						{
							isOk = FALSE;
						}
					}
					SAFE_FREEBSTRING(bstrTemp);								
					RELEASE_INTERFACE(pIDOMNode3);
				}
			}
			//isUseCertificate
			if(isOk)
			{
				isOk = FALSE;
				hr = pAttributeMap->getNamedItem(L"isUseCertificate",&pIDOMNode3);
				if(SUCCEEDED(hr) && pIDOMNode3)
				{
					BSTR bstrTemp = NULL;
					pIDOMNode3->get_text(&bstrTemp);
					if(bstrTemp)
					{
						if( NtkoAreStringsEqualW(bstrTemp,-1,L"true",-1) )
						{
							m_secsignSettings.bUseCert = TRUE;
							isOk = TRUE;
						}
						else if( NtkoAreStringsEqualW(bstrTemp,-1,L"false",-1) )
						{
							m_secsignSettings.bUseCert = FALSE;
							isOk = TRUE;
						}
						else
						{
							isOk = FALSE;
						}
					}
					SAFE_FREEBSTRING(bstrTemp);								
					RELEASE_INTERFACE(pIDOMNode3);
				}
			}
			//PrintMode
			if(isOk)
			{
				isOk = FALSE;
				hr = pAttributeMap->getNamedItem(L"PrintMode",&pIDOMNode3);
				if(SUCCEEDED(hr) && pIDOMNode3)
				{
					BSTR bstrTemp = NULL;
					pIDOMNode3->get_text(&bstrTemp);
					if(bstrTemp)
					{
						if( NtkoAreStringsEqualW(bstrTemp,-1,L"0",-1) )
						{
							m_secsignSettings.PrintMode = NTKOSecsignPrintMode_NOPrint;
							isOk = TRUE;
						}
						else if( NtkoAreStringsEqualW(bstrTemp,-1,L"1",-1) )
						{
							m_secsignSettings.PrintMode = NTKOSecsignPrintMode_Gray;
							isOk = TRUE;
						}
						else if( NtkoAreStringsEqualW(bstrTemp,-1,L"2",-1) )
						{
							m_secsignSettings.PrintMode = NTKOSecsignPrintMode_Origin;
							isOk = TRUE;
						}
						else
						{
							isOk = FALSE;
						}
					}
					SAFE_FREEBSTRING(bstrTemp);								
					RELEASE_INTERFACE(pIDOMNode3);
				}
			}
			//isShowUI
			if(isOk)
			{
				isOk = FALSE;
				hr = pAttributeMap->getNamedItem(L"isShowUI",&pIDOMNode3);
				if(SUCCEEDED(hr) && pIDOMNode3)
				{
					BSTR bstrTemp = NULL;
					pIDOMNode3->get_text(&bstrTemp);
					if(bstrTemp)
					{
						if( NtkoAreStringsEqualW(bstrTemp,-1,L"true",-1) )
						{
							m_secsignSettings.bShowUI = TRUE;
							isOk = TRUE;
						}
						else if( NtkoAreStringsEqualW(bstrTemp,-1,L"false",-1) )
						{
							m_secsignSettings.bShowUI = FALSE;
							isOk = TRUE;
						}
						else
						{
							isOk = FALSE;
						}
					}
					SAFE_FREEBSTRING(bstrTemp);								
					RELEASE_INTERFACE(pIDOMNode3);
				}
			}
			//isLocked,为兼容之前版本，可以不存在
			if(isOk)
			{				
				hr = pAttributeMap->getNamedItem(L"isLocked",&pIDOMNode3);
				if(SUCCEEDED(hr) && pIDOMNode3)
				{
					BSTR bstrTemp = NULL;
					pIDOMNode3->get_text(&bstrTemp);
					if(bstrTemp)
					{
						if( NtkoAreStringsEqualW(bstrTemp,-1,L"true",-1) )
						{
							m_secsignSettings.bLockSign = TRUE;
							isOk = TRUE;
						}
						else if( NtkoAreStringsEqualW(bstrTemp,-1,L"false",-1) )
						{
							m_secsignSettings.bLockSign = FALSE;
							isOk = TRUE;
						}
						else
						{
							isOk = FALSE;
						}
					}
					SAFE_FREEBSTRING(bstrTemp);								
					RELEASE_INTERFACE(pIDOMNode3);
				}
			}
			//isSaved,为兼容之前版本，可以不存在
			if(isOk)
			{				
				hr = pAttributeMap->getNamedItem(L"isSaved",&pIDOMNode3);
				if(SUCCEEDED(hr) && pIDOMNode3)
				{
					BSTR bstrTemp = NULL;
					pIDOMNode3->get_text(&bstrTemp);
					if(bstrTemp)
					{
						if( NtkoAreStringsEqualW(bstrTemp,-1,L"true",-1) )
						{
							m_secsignSettings.bLockDoc = TRUE;
							isOk = TRUE;
						}
						else if( NtkoAreStringsEqualW(bstrTemp,-1,L"false",-1) )
						{
							m_secsignSettings.bLockDoc = FALSE;
							isOk = TRUE;
						}
						else
						{
							isOk = FALSE;
						}
					}
					SAFE_FREEBSTRING(bstrTemp);								
					RELEASE_INTERFACE(pIDOMNode3);
				}
			}
			//isCheckFont,为兼容之前版本，可以不存在
			if(isOk)
			{				
				hr = pAttributeMap->getNamedItem(L"isCheckFont",&pIDOMNode3);
				if(SUCCEEDED(hr) && pIDOMNode3)
				{
					BSTR bstrTemp = NULL;
					pIDOMNode3->get_text(&bstrTemp);
					if(bstrTemp)
					{
						if( NtkoAreStringsEqualW(bstrTemp,-1,L"true",-1) )
						{
							m_secsignSettings.bCheckFont = TRUE;
							isOk = TRUE;
						}
						else if( NtkoAreStringsEqualW(bstrTemp,-1,L"false",-1) )
						{
							m_secsignSettings.bCheckFont = FALSE;
							isOk = TRUE;
						}
						else
						{
							isOk = FALSE;
						}
					}
					SAFE_FREEBSTRING(bstrTemp);								
					RELEASE_INTERFACE(pIDOMNode3);
				}
			}
			//isAddSignHand,为兼容之前版本，可以不存在
			if(isOk)
			{				
				hr = pAttributeMap->getNamedItem(L"isAddSignHand",&pIDOMNode3);
				if(SUCCEEDED(hr) && pIDOMNode3)
				{
					BSTR bstrTemp = NULL;
					pIDOMNode3->get_text(&bstrTemp);
					if(bstrTemp)
					{
						if( NtkoAreStringsEqualW(bstrTemp,-1,L"true",-1) )
						{
							m_secsignSettings.bHandsign = TRUE;
							isOk = TRUE;
						}
						else if( NtkoAreStringsEqualW(bstrTemp,-1,L"false",-1) )
						{
							m_secsignSettings.bHandsign = FALSE;
							isOk = TRUE;
						}
						else
						{
							isOk = FALSE;
						}
					}
					SAFE_FREEBSTRING(bstrTemp);								
					RELEASE_INTERFACE(pIDOMNode3);
				}
			}
			//isAddSignDate,为兼容之前版本，可以不存在
			if(isOk)
			{				
				hr = pAttributeMap->getNamedItem(L"isAddSignDate",&pIDOMNode3);
				if(SUCCEEDED(hr) && pIDOMNode3)
				{
					BSTR bstrTemp = NULL;
					pIDOMNode3->get_text(&bstrTemp);
					if(bstrTemp)
					{
						if( NtkoAreStringsEqualW(bstrTemp,-1,L"true",-1) )
						{
							m_secsignSettings.bAddTime = TRUE;
							isOk = TRUE;
						}
						else if( NtkoAreStringsEqualW(bstrTemp,-1,L"false",-1) )
						{
							m_secsignSettings.bAddTime = FALSE;
							isOk = TRUE;
						}
						else
						{
							isOk = FALSE;
						}
					}
					SAFE_FREEBSTRING(bstrTemp);								
					RELEASE_INTERFACE(pIDOMNode3);
				}
			}
			
			//CSPIssuerName.可以不存在
			if(isOk)
			{
				hr = pAttributeMap->getNamedItem(L"CSPIssuerName",&pIDOMNode3);
				if(SUCCEEDED(hr) && pIDOMNode3)
				{
					BSTR bstrTemp = NULL;
					pIDOMNode3->get_text(&bstrTemp);
					if(bstrTemp)
					{
						LPSTR pszCSPIssuerName = NtkoLPWSTRToANSI(bstrTemp);
						if(pszCSPIssuerName)
						{
							ZeroMemory(m_secsignSettings.CertIssuerName,sizeof(m_secsignSettings.CertIssuerName));
							lstrcpyn(m_secsignSettings.CertIssuerName,pszCSPIssuerName,sizeof(m_secsignSettings.CertIssuerName));
							SAFE_FREESTRING(pszCSPIssuerName);	
							isOk = TRUE;
						}
						else
						{
							isOk = FALSE;
						}
					}
					SAFE_FREEBSTRING(bstrTemp);								
					RELEASE_INTERFACE(pIDOMNode3);
				}
			}
			RELEASE_INTERFACE(pAttributeMap);
		}
		RELEASE_INTERFACE(pIDOMNode2);
	}
	RELEASE_INTERFACE(pIDOMNode);
	return isOk;
}

BOOL CNTKOServerSignHelper::ParseNtkoCheckEkey()
{
	HRESULT hr = S_OK;
	BOOL isOk = FALSE;
	IXMLDOMNode *pIDOMNode = NULL;
	
	m_CanUseThisEkey = FALSE;
	
	hr = m_pXMLDoc->selectSingleNode( OLESTR("//ResponseInfo"), &pIDOMNode);
	if(FAILED(hr) || (!pIDOMNode))
	{
		return FALSE;
	}
	
	IXMLDOMNode *pIDOMNode2 = NULL;
	
	//除了用户注销请求之外，其它都需要解析用户权限
	hr = pIDOMNode->selectSingleNode( OLESTR("./NtkoCheckEkey"), &pIDOMNode2);
	if(SUCCEEDED(hr) && pIDOMNode2)
	{
		IXMLDOMNamedNodeMap *pAttributeMap = NULL;
		hr = pIDOMNode2->get_attributes(&pAttributeMap);
		if(SUCCEEDED(hr) && pAttributeMap)
		{
			IXMLDOMNode *pIDOMNode3 = NULL;
			//获取canDoHandSign
			hr = pAttributeMap->getNamedItem(L"canUseThisEkey",&pIDOMNode3);
			if(SUCCEEDED(hr) && pIDOMNode3)
			{
				BSTR bstrTemp = NULL;
				pIDOMNode3->get_text(&bstrTemp);
				if(bstrTemp)
				{
					if( NtkoAreStringsEqualW(bstrTemp,-1,L"true",-1) )
					{
						m_CanUseThisEkey = TRUE;
						isOk = TRUE;
					}
					else if( NtkoAreStringsEqualW(bstrTemp,-1,L"false",-1) )
					{
						m_CanUseThisEkey = FALSE;
						isOk = TRUE;
					}
					else
					{
						isOk = FALSE;
					}
				}
				SAFE_FREEBSTRING(bstrTemp);								
				RELEASE_INTERFACE(pIDOMNode3);
			}
			RELEASE_INTERFACE(pAttributeMap);
		}
		RELEASE_INTERFACE(pIDOMNode2);
	}
	RELEASE_INTERFACE(pIDOMNode);
	return isOk;
}

BOOL CNTKOServerSignHelper::ParseServerTime()
{
	HRESULT hr = S_OK;
	BOOL isOk = FALSE;
	IXMLDOMNode *pIDOMNode = NULL;
	
	hr = m_pXMLDoc->selectSingleNode( OLESTR("//ResponseInfo"), &pIDOMNode);
	if(FAILED(hr) || (!pIDOMNode))
	{
		return FALSE;
	}
	
	IXMLDOMNode *pIDOMNode2 = NULL;
	
	//除了用户注销请求之外，其它都需要解析用户权限
	hr = pIDOMNode->selectSingleNode( OLESTR("./ServerTime"), &pIDOMNode2);
	if(SUCCEEDED(hr) && pIDOMNode2)
	{
		IXMLDOMNamedNodeMap *pAttributeMap = NULL;
		hr = pIDOMNode2->get_attributes(&pAttributeMap);
		if(SUCCEEDED(hr) && pAttributeMap)
		{
			IXMLDOMNode *pIDOMNode3 = NULL;
			//获取canDoHandSign
			hr = pAttributeMap->getNamedItem(L"timeStamp",&pIDOMNode3);
			if(SUCCEEDED(hr) && pIDOMNode3)
			{
				SAFE_FREEBSTRING(m_bstrServerTime);
				pIDOMNode3->get_text(&m_bstrServerTime);
				if(m_bstrServerTime)
				{
					FILETIME	tTime;
					if(NtkoLPWSTRToFileTime(m_bstrServerTime,&tTime) )
					{
						isOk = TRUE;
					}
					else
					{
						isOk = FALSE;
					}
				}							
				RELEASE_INTERFACE(pIDOMNode3);
			}
			RELEASE_INTERFACE(pAttributeMap);
		}
		RELEASE_INTERFACE(pIDOMNode2);
	}
	RELEASE_INTERFACE(pIDOMNode);
	return isOk;
}

BOOL CNTKOServerSignHelper::ParseServerInfo()
{
	HRESULT hr = S_OK;
	BOOL isOk = FALSE;
	IXMLDOMNode *pIDOMNode = NULL;
	
	hr = m_pXMLDoc->selectSingleNode( OLESTR("//ResponseInfo"), &pIDOMNode);
	if(FAILED(hr) || (!pIDOMNode))
	{
		return FALSE;
	}
	
	IXMLDOMNode *pIDOMNode2 = NULL;
	
	hr = pIDOMNode->selectSingleNode( OLESTR("./ServerInfo"), &pIDOMNode2);
	if(SUCCEEDED(hr) && pIDOMNode2)
	{
		IXMLDOMNamedNodeMap *pAttributeMap = NULL;
		hr = pIDOMNode2->get_attributes(&pAttributeMap);
		if(SUCCEEDED(hr) && pAttributeMap)
		{
			IXMLDOMNode *pIDOMNode3 = NULL;
			//version
			hr = pAttributeMap->getNamedItem(L"version",&pIDOMNode3);
			if(SUCCEEDED(hr) && pIDOMNode3)
			{
				SAFE_FREEBSTRING(m_bstrServerInfoVersion);
				pIDOMNode3->get_text(&m_bstrServerInfoVersion);
				if(::SysStringByteLen(m_bstrServerInfoVersion))	
				{				
					isOk = TRUE;
				}
				else
				{
					isOk = FALSE;
				}							
				RELEASE_INTERFACE(pIDOMNode3);
			}
			//serverName
			if(isOk)
			{
				isOk = FALSE;
				hr = pAttributeMap->getNamedItem(L"serverName",&pIDOMNode3);
				if(SUCCEEDED(hr) && pIDOMNode3)
				{
					SAFE_FREEBSTRING(m_bstrServerInfoName);
					pIDOMNode3->get_text(&m_bstrServerInfoName);
					if(::SysStringByteLen(m_bstrServerInfoName))	
					{				
						isOk = TRUE;
					}
					else
					{
						isOk = FALSE;
					}							
					RELEASE_INTERFACE(pIDOMNode3);
				}
			}
			
			//licenseUserName
			if(isOk)
			{
				isOk = FALSE;
				hr = pAttributeMap->getNamedItem(L"licenseUserName",&pIDOMNode3);
				if(SUCCEEDED(hr) && pIDOMNode3)
				{
					SAFE_FREEBSTRING(m_bstrServerInfoLicenseUserName);
					pIDOMNode3->get_text(&m_bstrServerInfoLicenseUserName);
					if(::SysStringByteLen(m_bstrServerInfoLicenseUserName))	
					{				
						isOk = TRUE;
					}
					else
					{
						isOk = FALSE;
					}							
					RELEASE_INTERFACE(pIDOMNode3);
				}
			}
			
			//licenseCount
			if(isOk)
			{
				isOk = FALSE;
				hr = pAttributeMap->getNamedItem(L"licenseCount",&pIDOMNode3);
				if(SUCCEEDED(hr) && pIDOMNode3)
				{
					SAFE_FREEBSTRING(m_bstrServerInfoLicenseCount);
					pIDOMNode3->get_text(&m_bstrServerInfoLicenseCount);
					if(::SysStringByteLen(m_bstrServerInfoLicenseCount))	
					{				
						isOk = TRUE;
					}
					else
					{
						isOk = FALSE;
					}							
					RELEASE_INTERFACE(pIDOMNode3);
				}
			}
			RELEASE_INTERFACE(pAttributeMap);
		}
		RELEASE_INTERFACE(pIDOMNode2);
	}
	RELEASE_INTERFACE(pIDOMNode);
	return isOk;
}

BOOL CNTKOServerSignHelper::ParseNtkoUserSigns()
{
	HRESULT hr = S_OK;
	BOOL isOk = FALSE;
	IXMLDOMNode *pIDOMNode = NULL;
	
	ClearUserSigns();

#if defined(NTKO_CUSTOM_JIANGSU_CA)
	
//初次版本的江苏ca请求。现在的版本客户端不需要直接和seal服务器打交道，禁用，保留代码备查
	//return NTKO_Custom_ParseSigns();

	if(m_IsJSCAUseSealSvr)
	{
		return NTKO_Custom_ParseSigns_New();
	}	
#endif

	
	hr = m_pXMLDoc->selectSingleNode( OLESTR("//ResponseInfo"), &pIDOMNode);
	if(FAILED(hr) || (!pIDOMNode))
	{
		return FALSE;
	}
	
	long value = 0;
	IXMLDOMNodeList *pIDOMNodeList = NULL;
	
	hr = pIDOMNode->selectNodes( OLESTR("./NtkoUserSign"), &pIDOMNodeList);
	if(SUCCEEDED(hr) && pIDOMNodeList)
	{
		hr = pIDOMNodeList->get_length(&value);
		if(SUCCEEDED(hr))
		{
			isOk = TRUE;
			pIDOMNodeList->reset();
			for(int ii = 0; (ii < value) && (ii < NTKO_MAX_SIGNS); ii++)
			{
				IXMLDOMNode *pIDOMNode2 = NULL;
				hr = pIDOMNodeList->get_item(ii, &pIDOMNode2);
				if(pIDOMNode2)
				{
					IXMLDOMNamedNodeMap *pAttributeMap = NULL;
					hr = pIDOMNode2->get_attributes(&pAttributeMap);
					if(SUCCEEDED(hr) && pAttributeMap)
					{
						IXMLDOMNode *pIDOMNode3 = NULL;
						//获取signName
						hr = pAttributeMap->getNamedItem(L"signName",&pIDOMNode3);
						if(SUCCEEDED(hr) && pIDOMNode3)
						{
							pIDOMNode3->get_text(&m_NtkoUserSigns[m_iUserSignsCount].bstrSignName);			
							RELEASE_INTERFACE(pIDOMNode3);
							if(::SysStringByteLen(m_NtkoUserSigns[m_iUserSignsCount].bstrSignName))
							{
								isOk = TRUE;
							}
							else
							{
								isOk = FALSE;
							}
						}
						
						if(isOk)
						{
							isOk = FALSE;
							hr = pAttributeMap->getNamedItem(L"signUser",&pIDOMNode3);
							if(SUCCEEDED(hr) && pIDOMNode3)
							{
								pIDOMNode3->get_text(&m_NtkoUserSigns[m_iUserSignsCount].bstrSignUser);			
								RELEASE_INTERFACE(pIDOMNode3);
								if(::SysStringByteLen(m_NtkoUserSigns[m_iUserSignsCount].bstrSignUser))
								{
									isOk = TRUE;
								}
								else
								{
									isOk = FALSE;
								}
							}
						}
						
						if(isOk)
						{
							isOk = FALSE;
							hr = pAttributeMap->getNamedItem(L"signURL",&pIDOMNode3);
							if(SUCCEEDED(hr) && pIDOMNode3)
							{
								pIDOMNode3->get_text(&m_NtkoUserSigns[m_iUserSignsCount].bstrSignURL);			
								RELEASE_INTERFACE(pIDOMNode3);
								if(::SysStringByteLen(m_NtkoUserSigns[m_iUserSignsCount].bstrSignURL))
								{
									isOk = TRUE;
								}
								else
								{
									isOk = FALSE;
								}
							}
						}
						
						if(isOk)
						{
							m_iUserSignsCount++;
						}
						
						RELEASE_INTERFACE(pAttributeMap);
					}
					RELEASE_INTERFACE(pIDOMNode2);
				}
			}// end for
		}
		RELEASE_INTERFACE(pIDOMNodeList);
	}
	RELEASE_INTERFACE(pIDOMNode);
	return isOk;
}

BOOL CNTKOServerSignHelper::NTKO_Custom_ParseSigns_New()
{
#if !defined(NTKO_CUSTOM_JIANGSU_CA)
	return FALSE;
#endif

	HRESULT hr = S_OK;
	BOOL isOk = FALSE;
	IXMLDOMNode *pIDOMNode = NULL;

	ClearUserSigns();

	hr = m_pXMLDoc->selectSingleNode( OLESTR("//ResponseInfo"), &pIDOMNode);
	if(FAILED(hr) || (!pIDOMNode))
	{
		return FALSE;
	}
	
	long value = 0;
	IXMLDOMNodeList *pIDOMNodeList = NULL;
	
	hr = pIDOMNode->selectNodes( OLESTR("./NtkoSeal"), &pIDOMNodeList);
	if(SUCCEEDED(hr) && pIDOMNodeList)
	{
		hr = pIDOMNodeList->get_length(&value);
		if(SUCCEEDED(hr))
		{
			isOk = TRUE;
			pIDOMNodeList->reset();
			for(int ii = 0; (ii < value) && (ii < NTKO_MAX_SIGNS); ii++)
			{
				IXMLDOMNode *pIDOMNode2 = NULL;
				hr = pIDOMNodeList->get_item(ii, &pIDOMNode2);
				if(pIDOMNode2)
				{
					IXMLDOMNamedNodeMap *pAttributeMap = NULL;
					hr = pIDOMNode2->get_attributes(&pAttributeMap);
					if(SUCCEEDED(hr) && pAttributeMap)
					{
						IXMLDOMNode *pIDOMNode3 = NULL;

						//获取isEsp
						hr = pAttributeMap->getNamedItem(L"isEsp",&pIDOMNode3);
						if(SUCCEEDED(hr) && pIDOMNode3)
						{
							BSTR bstrIsEsp = NULL;
							pIDOMNode3->get_text(&bstrIsEsp);			
							RELEASE_INTERFACE(pIDOMNode3);
							m_NtkoUserSigns[m_iUserSignsCount].bIsESPSign = ((NtkoAreStringsEqualW(bstrIsEsp,-1, L"Y",-1))?TRUE:FALSE);
							isOk = TRUE;
						}

						//获取signName
						if(isOk)
						{
							isOk = FALSE;
							hr = pAttributeMap->getNamedItem(L"sealId",&pIDOMNode3);
							if(SUCCEEDED(hr) && pIDOMNode3)
							{
								pIDOMNode3->get_text(&m_NtkoUserSigns[m_iUserSignsCount].bstrSignURL);			
								RELEASE_INTERFACE(pIDOMNode3);
								if(::SysStringByteLen(m_NtkoUserSigns[m_iUserSignsCount].bstrSignURL))
								{
									isOk = TRUE;
								}
								else
								{
									isOk = FALSE;
								}
							}
						}
						
						if(isOk)
						{
							isOk = FALSE;
							hr = pAttributeMap->getNamedItem(L"companyName",&pIDOMNode3);
							if(SUCCEEDED(hr) && pIDOMNode3)
							{
								pIDOMNode3->get_text(&m_NtkoUserSigns[m_iUserSignsCount].bstrSignUser);			
								RELEASE_INTERFACE(pIDOMNode3);
								if(::SysStringByteLen(m_NtkoUserSigns[m_iUserSignsCount].bstrSignUser))
								{
									isOk = TRUE;
								}
								else
								{
									isOk = FALSE;
								}
							}
						}
						
						if(isOk)
						{
							isOk = FALSE;
							hr = pAttributeMap->getNamedItem(L"sealName",&pIDOMNode3);
							if(SUCCEEDED(hr) && pIDOMNode3)
							{
								pIDOMNode3->get_text(&m_NtkoUserSigns[m_iUserSignsCount].bstrSignName);			
								RELEASE_INTERFACE(pIDOMNode3);
								if(::SysStringByteLen(m_NtkoUserSigns[m_iUserSignsCount].bstrSignName))
								{
									isOk = TRUE;
								}
								else
								{
									isOk = FALSE;
								}
							}

							if(!isOk)
							{
								hr = pAttributeMap->getNamedItem(L"sealType",&pIDOMNode3);
								if(SUCCEEDED(hr) && pIDOMNode3)
								{
									pIDOMNode3->get_text(&m_NtkoUserSigns[m_iUserSignsCount].bstrSignName);			
									RELEASE_INTERFACE(pIDOMNode3);
									if(::SysStringByteLen(m_NtkoUserSigns[m_iUserSignsCount].bstrSignName))
									{
										isOk = TRUE;
									}
									else
									{
										isOk = FALSE;
									}
								}
							}

						}
						
						if(isOk)
						{
							m_iUserSignsCount++;
						}
						
						RELEASE_INTERFACE(pAttributeMap);
					}
					RELEASE_INTERFACE(pIDOMNode2);
				}
			}// end for
		}
		RELEASE_INTERFACE(pIDOMNodeList);
	}
	RELEASE_INTERFACE(pIDOMNode);
	return isOk;
}

BOOL CNTKOServerSignHelper::NTKO_Custom_ParseSigns()
{
#if !defined(NTKO_CUSTOM_JIANGSU_CA)
	return FALSE;
#endif

	HRESULT hr = S_OK;
	BOOL isOk = FALSE;
	IXMLDOMNode *pIDOMNode = NULL;
	
	ClearUserSigns();
	
	hr = m_pXMLDoc->selectSingleNode( OLESTR("//ResponseInfo"), &pIDOMNode);
	if(FAILED(hr) || (!pIDOMNode))
	{
		return FALSE;
	}
	
	IXMLDOMNode * pCompanyNode = NULL;

	hr = pIDOMNode->selectSingleNode( OLESTR("./company"), &pCompanyNode);
	if (SUCCEEDED(hr) && pCompanyNode)
	{
		IXMLDOMNamedNodeMap *pAttributeMap = NULL;
		hr = pCompanyNode->get_attributes(&pAttributeMap);
		if(SUCCEEDED(hr) && pAttributeMap)
		{
			IXMLDOMNode *pDomNodeTemp = NULL;
			BSTR bstrTemp = NULL;

			hr = pAttributeMap->getNamedItem(L"id",&pDomNodeTemp);

			if(SUCCEEDED(hr) && pDomNodeTemp)
			{
				pDomNodeTemp->get_text(&bstrTemp);
				m_Custom_jiangshuCA_CompanyInfo.companyId = NtkoCopyStringW(bstrTemp);
				SAFE_FREEBSTRING(bstrTemp);
				SAFE_RELEASE_INTERFACE(pDomNodeTemp);
			}

			hr = pAttributeMap->getNamedItem(L"name",&pDomNodeTemp);
			
			if(SUCCEEDED(hr) && pDomNodeTemp)
			{
				pDomNodeTemp->get_text(&bstrTemp);
				//m_Custom_jiangshuCA_CompanyInfo.companyName = NtkoCopyStringW(bstrTemp);
				SAFE_FREEBSTRING(bstrTemp);
				SAFE_RELEASE_INTERFACE(pDomNodeTemp);
			}

			hr = pAttributeMap->getNamedItem(L"code",&pDomNodeTemp);
			
			if(SUCCEEDED(hr) && pDomNodeTemp)
			{
				pDomNodeTemp->get_text(&bstrTemp);
				m_Custom_jiangshuCA_CompanyInfo.companyCode = NtkoCopyStringW(bstrTemp);
				SAFE_FREEBSTRING(bstrTemp);
				SAFE_RELEASE_INTERFACE(pDomNodeTemp);
			}



			long value = 0;
			IXMLDOMNodeList *pSignListNode = NULL;
			
			// 	<seal id="印章编号" name="印章名称" type="公章"><seal/>
			hr = pCompanyNode->selectNodes( OLESTR("./seal"), &pSignListNode);
			if(SUCCEEDED(hr) && pSignListNode)
			{
				hr = pSignListNode->get_length(&value);
				if( SUCCEEDED(hr) )
				{
					isOk = TRUE;
					pSignListNode->reset();
					for(int ii = 0; (ii < value) && (ii < NTKO_MAX_SIGNS); ii++)
					{
						IXMLDOMNode *pSignNode = NULL;
						hr = pSignListNode->get_item(ii, &pSignNode);
						if(pSignNode)
						{
							IXMLDOMNamedNodeMap *pAttributeMap = NULL;
							hr = pSignNode->get_attributes(&pAttributeMap);
							if(SUCCEEDED(hr) && pAttributeMap)
							{
								IXMLDOMNode *pSignAttributeNode = NULL;
								// 获取签章状态,判断是否可用
								hr = pAttributeMap->getNamedItem(L"status",&pSignAttributeNode);
								if (SUCCEEDED(hr) && pSignAttributeNode)
								{
									BSTR status;
									pSignAttributeNode->get_text(&status);
									RELEASE_INTERFACE(pSignAttributeNode);
									
									isOk = 0!=StrCmpIW(status, L"Y")?FALSE:TRUE;
								}

								if (isOk)
								{	
									//获取signName
									hr = pAttributeMap->getNamedItem(L"name",&pSignAttributeNode);
									if(SUCCEEDED(hr) && pSignAttributeNode)
									{
										pSignAttributeNode->get_text(&m_NtkoUserSigns[m_iUserSignsCount].bstrSignName);			
										RELEASE_INTERFACE(pSignAttributeNode);
										if(::SysStringByteLen(m_NtkoUserSigns[m_iUserSignsCount].bstrSignName))
										{
											isOk = TRUE;
										}
										else
										{
											isOk = FALSE;
										}
									}
								}

								if(isOk)
								{
									isOk = FALSE;
									hr = pAttributeMap->getNamedItem(L"id",&pSignAttributeNode);
									if(SUCCEEDED(hr) && pSignAttributeNode)
									{
										pSignAttributeNode->get_text(&m_NtkoUserSigns[m_iUserSignsCount].bstrSignURL);			
										if(::SysStringByteLen(m_NtkoUserSigns[m_iUserSignsCount].bstrSignURL))
										{
											isOk = TRUE;
										}
										else
										{
											isOk = FALSE;
										}
									}
								}
								
								if(isOk)
								{
									isOk = FALSE;
									hr = pAttributeMap->getNamedItem(L"type",&pSignAttributeNode);
									if(SUCCEEDED(hr) && pSignAttributeNode)
									{
										pSignAttributeNode->get_text(&m_NtkoUserSigns[m_iUserSignsCount].bstrSignUser);			
										RELEASE_INTERFACE(pSignAttributeNode);
										if(::SysStringByteLen(m_NtkoUserSigns[m_iUserSignsCount].bstrSignUser))
										{
											isOk = TRUE;
										}
										else
										{
											isOk = FALSE;
										}
									}
								}
								
								if(isOk)
								{
									m_iUserSignsCount++;
								}
								
								RELEASE_INTERFACE(pAttributeMap);
							}
							RELEASE_INTERFACE(pSignNode);
						}
					}// end for
				}
				RELEASE_INTERFACE(pSignListNode);
			}

		}
		SAFE_RELEASE_INTERFACE(pCompanyNode);
	}



	
	RELEASE_INTERFACE(pIDOMNode);
	return isOk;
}


VOID CNTKOServerSignHelper::ClearUserSigns()
{
	for(int i=0;i<NTKO_MAX_SIGNS;i++)
	{
		SAFE_FREEBSTRING(m_NtkoUserSigns[i].bstrSignName);
		SAFE_FREEBSTRING(m_NtkoUserSigns[i].bstrSignUser);
		SAFE_FREEBSTRING(m_NtkoUserSigns[i].bstrSignURL);
	}
	ZeroMemory(&m_NtkoUserSigns[0],sizeof(m_NtkoUserSigns));
	m_iUserSignsCount = 0;
}

BOOL CNTKOServerSignHelper::DoServerSignLog(NtkoSecuritySignInfo  * pSignInfo,NtkoSignOpType opType,LPWSTR fileName,LPWSTR fileInfo)
{
	if (!pSignInfo) return FALSE;
	
	m_serverReqestType = NTKOServerRequestType_DosignLog;
	HRESULT hr = S_OK;
	BOOL isOk = FALSE;
	WCHAR wszCPARA[2048];
	BSTR bstrRetXML = NULL;
	WCHAR wszTemp[128];
	LPWSTR pwszTemp = NULL;
	
	//构造参数
	ZeroMemory(&wszCPARA[0],sizeof(wszCPARA));
	
	//username==xxx&password=xxx
	lstrcatW(wszCPARA,L"username=");
	lstrcatW(wszCPARA,m_LoginUserName);
	
	lstrcatW(wszCPARA,L"&password=");
	lstrcatW(wszCPARA,m_LoginPassword);
	
	lstrcatW(wszCPARA,L"&signname=");
	lstrcatW(wszCPARA,pSignInfo->signName);
	
	lstrcatW(wszCPARA,L"&signuser=");
	lstrcatW(wszCPARA,pSignInfo->signUserName);
	
	lstrcatW(wszCPARA,L"&signsn=");
	lstrcatW(wszCPARA,pSignInfo->signSN);

	lstrcatW(wszCPARA,L"&signunid=");
	lstrcatW(wszCPARA,pSignInfo->signUNID);
	
	lstrcatW(wszCPARA,L"&ekeysn=");
	lstrcatW(wszCPARA,pSignInfo->ekeySN);

	lstrcatW(wszCPARA,L"&servertime=");
	lstrcatW(wszCPARA,pSignInfo->signTime);		
	
	lstrcatW(wszCPARA,L"&appname=");
	lstrcatW(wszCPARA,pSignInfo->appName);
	
	lstrcatW(wszCPARA,L"&cspreleasename=");
	lstrcatW(wszCPARA,pSignInfo->certInfo.pwszIssuerName);		
	
	lstrcatW(wszCPARA,L"&cspusename=");
	lstrcatW(wszCPARA,pSignInfo->certInfo.pwszSubjectName);		


	LPWSTR pwszInfo = NULL;
	DWORD	iConvertType = 0;

	switch(pSignInfo->signOrigin)
	{
	case NtkoSecuritySignFile:
	case NtkoSecuritySignUrl:
		{
			pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNTYPE0);
			iConvertType = 0;
		}
		break;
	case NtkoSecuritySignHandSign:
		{
			pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNTYPE1);
			iConvertType = 1;
		}
		break;
	case NtkoSecuritySignEkey:
		{
			pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNTYPE2);
			iConvertType = 2;
		}
		break;
	case NtkoSecuritySignKeyBoard:
		{
			pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNTYPE3);
			iConvertType = 3;
		}
		break;
	case NtkoSecuritySignBarcode:
		{
			pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNTYPE4);
			iConvertType = 5;
		}
		break;
	default:
		{
			pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNTYPEOTHER);
		}
		break;
	}


	ZeroMemory(wszTemp,sizeof(wszTemp));
	NtkoFwsprintf(wszTemp,L"&signtype=%d", iConvertType);
	lstrcatW(wszCPARA, wszTemp);	


	lstrcatW(wszCPARA,L"&signop=");	
	ZeroMemory(wszTemp,sizeof(wszTemp));
	LPWSTR pwszFmt = NULL;
	
	switch(opType)
	{
	case NtkoSignOpType_Add:
		{
			pwszFmt = LoadMUIStringW(IDSMUI_HC_LOG_SIGNOP0);
			NtkoFwsprintf(wszTemp,pwszFmt,pwszInfo);
			lstrcatW(wszCPARA,wszTemp);	
		}
		break;
	case NtkoSignOpType_Lock:
		{
			pwszFmt = LoadMUIStringW(IDSMUI_HC_LOG_SIGNOP1);
			NtkoFwsprintf(wszTemp,pwszFmt,pwszInfo);
			lstrcatW(wszCPARA,wszTemp);
		}	
		break;
	case NtkoSignOpType_UnLock:
		{
			pwszFmt = LoadMUIStringW(IDSMUI_HC_LOG_SIGNOP2);
			NtkoFwsprintf(wszTemp,pwszFmt,pwszInfo);
			lstrcatW(wszCPARA,wszTemp);
		}
		break;
	case NtkoSignOpType_Protect:
		{
			SAFE_FREESTRING(pwszInfo);
			pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNOP3);
			lstrcatW(wszCPARA,pwszInfo);
		}
		break;
	case NtkoSignOpType_Unprotect:
		{		
			SAFE_FREESTRING(pwszInfo);
			pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNOP4);
			lstrcatW(wszCPARA,pwszInfo);
		}
		break;
	case NtkoSignOpType_EnableCheckDocChange:
		{
			SAFE_FREESTRING(pwszInfo);
			pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNOP5);
			lstrcatW(wszCPARA,pwszInfo);
		}
		break;
	case NtkoSignOpType_DisableCheckDocChange:
		{
			SAFE_FREESTRING(pwszInfo);
			pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNOP6);
			lstrcatW(wszCPARA,pwszInfo);
		}
		break;
	case NtkoSignOpType_Delete:
		{
			pwszFmt = LoadMUIStringW(IDSMUI_HC_LOG_SIGNOP7);
			NtkoFwsprintf(wszTemp,pwszFmt,pwszInfo);
			lstrcatW(wszCPARA,wszTemp);	
		}
		break;
	case NtkoSignOpType_Uncrypt:
		{
			SAFE_FREESTRING(pwszInfo);
			pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNOP8);
			lstrcatW(wszCPARA,pwszInfo);
		}
		break;
	}
	SAFE_FREESTRING(pwszInfo);
	SAFE_FREESTRING(pwszFmt);


	lstrcatW(wszCPARA,L"&docfile=");
	lstrcatW(wszCPARA,fileName);		

	lstrcatW(wszCPARA,L"&docinfo=");
	lstrcatW(wszCPARA,fileInfo);		

	if (NtkoSignType_single==pSignInfo->type)
	{
		ZeroMemory(wszTemp,sizeof(wszTemp));
		NtkoFwsprintf(wszTemp,L"第%d页",pSignInfo->pageNo+1 );
		lstrcatW(wszCPARA,L"&signpos=");
		lstrcatW(wszCPARA,wszTemp);
	}
	
	//向服务器提交请求
	ShowDelayDlgMUIStrW(IDSMUI_DOINGLOGSIGN);
	isOk = DoWebExecute2(m_secsignSettings.serverURL,NTKO_HC_DOLOGSIGN_URL,wszCPARA,&bstrRetXML);		
	CloseDelayDlg();
	if(isOk && bstrRetXML)
	{			
		isOk = ParseXML(bstrRetXML);
		//MyMessageBoxW(hwndDlgParent, bstrRetXML,L"bstrRetXML" ,MB_OK|MB_ICONINFORMATION);
		if(isOk) //解析成功，但是服务器有可能有错误
		{ 
			if( 0 == m_iServerErrCode) 
			{
				if( NtkoAreStringsEqualW(m_bstrServerVersion,-1,NTKO_ServerSecsignControlProtocalVersion,-1) )
				{
					isOk = TRUE;
				}
				else
				{
					ShowMsgBoxMUIStrW(IDSMUI_DOLOGSIGN_FAIL_INVALIDSERVER_VERSION,
						IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
					isOk = FALSE;
				}
			}
			else //服务器返回有错误
			{
				WCHAR wszBuf[1024];
				
				LPWSTR pwszFmt = LoadMUIStringW(IDSMUI_DOLOGSIGN_FAIL_SERVERFAILWITHCODE);
				NtkoFwsprintf(wszBuf,pwszFmt,m_iServerErrCode,m_bstrFailReason);
				SAFE_FREESTRING(pwszFmt);
				
				LPWSTR pwszMUITitle = LoadMUIStringW(IDSMUI_MSGBOXTITLE_TIP);
				NtkoMessageBoxW(m_hwndParent,wszBuf,pwszMUITitle ,MB_OK|MB_ICONINFORMATION);
				SAFE_FREESTRING(pwszMUITitle);
				
				if(1 == m_iServerErrCode)
				{
					m_IsLogin = FALSE;
				}
				isOk = FALSE;
			}
		}
		else //解析错误
		{
			ShowMsgBoxMUIStrW(IDSMUI_DOLOGSIGN_FAIL_PARSERESPONSEERROR,
				IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
			isOk = FALSE;
		}
	}
	else
	{			
		// 		LPWSTR pwszMUITitle = LoadMUIStringW(IDSMUI_MSGBOXTITLE_TIP);
		// 		NtkoMessageBoxW(hwndDlgParent,bstrRetXML,pwszMUITitle ,MB_OK|MB_ICONINFORMATION);
		// 		SAFE_FREESTRING(pwszMUITitle);
		
		ShowMsgBoxMUIStrW(IDSMUI_DOLOGSIGN_FAIL_CONNECT,IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		isOk = FALSE;
	}
	
	SAFE_FREEBSTRING(bstrRetXML);
	return isOk;
}

CONST LPWSTR CNTKOServerSignHelper::GetServerTime()
{
	m_serverReqestType = NTKOServerRequestType_GetserverTime; //13 获取签章验证服务器信息
	BSTR bstrRetXML = NULL;

	ShowDelayDlgMUIStrW(IDSMUI_BEGIN_GETSERVERTIME);
	BOOL isOk = DoWebGet(m_rootSvrUrl,NTKO_HC_GETSERVERTIME_URL,&bstrRetXML);	
	CloseDelayDlg();
	if(isOk && bstrRetXML)
	{			
		isOk = ParseXML(bstrRetXML);

		if(isOk) //解析成功，但是服务器有可能有错误
		{
			if(0 == m_iServerErrCode)
			{
				if(NtkoAreStringsEqualW(m_bstrServerVersion,-1,NTKO_ServerSecsignControlProtocalVersion,-1))
				{
					isOk = TRUE;
				}
				else
				{
					ShowMsgBoxMUIStrW(IDSMUI_CHECKSIGNS_FAIL_INVALIDSERVER_VERSION,
						IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
					isOk = FALSE;
				}
			}
			else //服务器返回有错误
			{
				WCHAR wszBuf[1024];
				
				LPWSTR pwszFmt = LoadMUIStringW(IDSMUI_CHECKSIGNS_FAIL_SERVERFAILWITHCODE);
				NtkoFwsprintf(wszBuf,pwszFmt,m_iServerErrCode,m_bstrFailReason);
				SAFE_FREESTRING(pwszFmt);
				
				LPWSTR pwszMUITitle = LoadMUIStringW(IDSMUI_MSGBOXTITLE_TIP);
				NtkoMessageBoxW(m_hwndParent,wszBuf,pwszMUITitle ,MB_OK|MB_ICONINFORMATION);
				SAFE_FREESTRING(pwszMUITitle);
				
				isOk = FALSE;
			}
		}
		else //解析错误
		{
			ShowMsgBoxMUIStrW(IDSMUI_CHECKSIGNS_FAIL_PARSERESPONSEERROR,
				IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
			isOk = FALSE;
		}
	}
	else
	{			
		ShowMsgBoxMUIStrW(IDSMUI_CHECKSIGNS_FAIL_CONNECT,IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		isOk = FALSE;
	}
	SAFE_FREEBSTRING(bstrRetXML);
	ParseServerTime();
	return m_bstrServerTime;
}

CNTKOServerSignHelper::~CNTKOServerSignHelper()
{
	NTKO_SAFEFreePoint(m_pNTkoSecstrPublishInfo);
	NTKO_SAFEFreePoint(m_pNtkoSignEkey);
	NTKO_SAFEFreePoint(m_pNtkoUIDelayDlg);
	SAFE_FREEBSTRING(m_bstrServerVersion);

	SAFE_FREEBSTRING(m_bstrServerInfoVersion);
	SAFE_FREEBSTRING(m_bstrServerInfoName);
	SAFE_FREEBSTRING(m_bstrServerInfoLicenseUserName);
	SAFE_FREEBSTRING(m_bstrServerInfoLicenseCount);
	SAFE_FREEBSTRING(m_bstrThisServerInfoName);
	SAFE_FREEBSTRING(m_bstrFailReason);
	SAFE_FREEBSTRING(m_bstrServerTime);
	SAFE_FREEBSTRING(m_bstrServerVersion);
	SAFE_FREEBSTRING(m_bstrServerVersion);
	SAFE_FREEBSTRING(m_bstrServerVersion);
	SAFE_FREEBSTRING(m_bstrServerVersion);
	FreeNtkoCmnLib();
	DeleteObject(m_hbmpError);
	DeleteObject(m_hbmpOk);

	SAFE_FREESTRING(m_pwszSignFilter);

	NTKO_Custom_ClearCompanyInfo();

	ReleaseCurCert();
	ReleaseCurStore();

	SAFE_FREEBSTRING(m_bstr_custom_ChuanDaZhiSheng_UserCertSN);
}

BOOL CNTKOServerSignHelper::DownLoadURLToFile(LPSTR pszServerApp,LPWSTR pwszURL, LPWSTR pwszFile)
{
	LPSTR	pstrUA = NULL;			// User Agent
	LPSTR	pstrBaseURL = NULL;
	LPSTR	pstrURL = NULL;
	
	HRESULT     hr     = E_UNEXPECTED;
	BYTE       *rgbBuf = NULL;
	HANDLE      hFile;
	ULONG       cbRead, cbWritten;
	HINTERNET	hOpen = NULL, hConnect = NULL, hReq = NULL;
	URL_COMPONENTS temURLStructure;
	DWORD		dw;
	
	if ( !(pwszURL) || (*pwszURL == L'\0') )
	{
		return FALSE;
	}
	
	if (!(pwszFile))
	{
		return FALSE;
	}
	
	pstrBaseURL = pszServerApp;
	pstrUA = NTKO_HC_AGENT;
	pstrURL = NtkoLPWSTRToANSI(pwszURL); //need free
	
	char lpszURL[1024];
	char Scheme[63];
	char HostName[1024];
	char Username[256];
	char Password[256];
	char URlPath[1024];
	char ExtraInfo[1024];
	
	dw = sizeof lpszURL;
	
	InternetCombineUrl(pstrBaseURL,pstrURL,lpszURL,&dw, ICU_BROWSER_MODE);
	SAFE_FREESTRING(pstrURL);
	
	temURLStructure.dwStructSize = sizeof(temURLStructure);
	temURLStructure.lpszScheme = Scheme;
	temURLStructure.dwSchemeLength = sizeof Scheme;
	temURLStructure.lpszHostName = HostName;
	temURLStructure.dwHostNameLength = sizeof HostName;
	temURLStructure.lpszUserName = Username;
	temURLStructure.dwUserNameLength =sizeof Username;
	temURLStructure.lpszPassword = Password;
	temURLStructure.dwPasswordLength = sizeof Password;
	temURLStructure.lpszUrlPath = URlPath;
	temURLStructure.dwUrlPathLength = sizeof URlPath;
	temURLStructure.lpszExtraInfo = ExtraInfo;
	temURLStructure.dwExtraInfoLength = sizeof ExtraInfo;
	
	InternetCrackUrl(lpszURL,dw,0,/*ICU_DECODE,*/&temURLStructure);	
	
	if (
		( INTERNET_SCHEME_HTTP != temURLStructure.nScheme )
		&& ( INTERNET_SCHEME_HTTPS != temURLStructure.nScheme )
		)
	{
		return FALSE;
	}
	
	
	//这儿需要转换URLPath为UTF-8编码
	ConvertANSIToUTF8Hex(URlPath);
	
	lstrcatA(URlPath,ExtraInfo);
	
	hOpen = InternetOpen  //open internet session
		(
		pstrUA,                     // user agent
		INTERNET_OPEN_TYPE_PRECONFIG,  
		NULL,                        // proxy server name
		NULL,                                         // by passed proxy
		0
		);
	if ( NULL == hOpen ) 
	{
		return FALSE;
	}
	
	hConnect = InternetConnect
		(
		hOpen,                               // session handle
		HostName,                   // server name
		temURLStructure.nPort ,                             // service port no
		NTKO_HTTP_USER,                                  // username
		NTKO_HTTP_PASS,                                  // password
		INTERNET_SERVICE_HTTP,               // service required
		0 ,
		0
		);
	
	if ( NULL == hConnect )
	{
		if ( hOpen ) InternetCloseHandle(hOpen);
		return FALSE;
	}
	
	BOOL bIsUseHTTP11 = IsInternetSettingEnableHTTP11();
	if (v_fUnicodeAPI)
	{
		WCHAR *waccept[2];
		waccept[0] = L"*/*";
		waccept[1] = NULL;
		LPWSTR pwszURLPath = NtkoANSIToLPWSTR(URlPath);
		hReq = HttpOpenRequestW                        // new HTTP request handle
			(
			hConnect,                            // connect handle
			L"GET",                               // method
			pwszURLPath,                  // object
			(bIsUseHTTP11)?L"HTTP/1.1":L"HTTP/1.0",                // HTTP Vertion
			NULL,                                  // referer
			(const WCHAR**)waccept,                                  // accept types
			INTERNET_FLAG_RELOAD |              // Internet flag values
			INTERNET_FLAG_NO_CACHE_WRITE |
			INTERNET_FLAG_KEEP_CONNECTION |
			(INTERNET_SCHEME_HTTPS == 
			temURLStructure.nScheme ? INTERNET_FLAG_SECURE:0) ,
			0                                    // applicaion define value
			);
		SAFE_FREESTRING(pwszURLPath);
	}
	else
	{
		char *accept[2];
		
		accept[0] = "*/*";
		accept[1] = NULL;
		
		hReq = HttpOpenRequestA                        // new HTTP request handle
			(
			hConnect,                            // connect handle
			"GET",                               // method
			URlPath,                  // object
			(bIsUseHTTP11)?"HTTP/1.1":"HTTP/1.0",                // HTTP Vertion
			NULL,                                  // referer
			(const char**)accept,                                  // accept types
			INTERNET_FLAG_RELOAD |              // Internet flag values
			INTERNET_FLAG_NO_CACHE_WRITE |
			INTERNET_FLAG_KEEP_CONNECTION |
			(INTERNET_SCHEME_HTTPS == 
			temURLStructure.nScheme ? INTERNET_FLAG_SECURE:0) ,
			0                                    // applicaion define value
			);
	}
	if ( NULL == hReq ) 
	{
		if ( hConnect ) InternetCloseHandle(hConnect);
		if ( hOpen ) InternetCloseHandle(hOpen);
		return FALSE;
	}
	
	dw = m_dwConnectServerTimeOut * 1000;
	if (!InternetSetOption(hReq, INTERNET_OPTION_CONNECT_TIMEOUT,
						  &dw, sizeof(dw)) )
	{
		if ( hReq )InternetCloseHandle(hReq);
		if ( hConnect ) InternetCloseHandle(hConnect);
		if ( hOpen ) InternetCloseHandle(hOpen);
		return FALSE;
	};
	
	dw = m_dwReceiveDataTimeOut * 1000;
	if (!InternetSetOption(hReq, INTERNET_OPTION_RECEIVE_TIMEOUT,
		&dw, sizeof(dw)) )
	{
		if ( hReq )InternetCloseHandle(hReq);
		if ( hConnect ) InternetCloseHandle(hConnect);
		if ( hOpen ) InternetCloseHandle(hOpen);
		return FALSE;
	};
	

	if(INTERNET_SCHEME_HTTPS ==	 temURLStructure.nScheme)
	{
		dw = 0;
		DWORD dwLen = sizeof(dw);
		if(InternetQueryOption(hReq, INTERNET_OPTION_SECURITY_FLAGS,&dw,&dwLen) )
		{
			dw |= SECURITY_FLAG_IGNORE_UNKNOWN_CA |SECURITY_FLAG_IGNORE_REVOCATION |SECURITY_FLAG_IGNORE_WRONG_USAGE;
			InternetSetOption (hReq, INTERNET_OPTION_SECURITY_FLAGS,&dw, sizeof(dw));
		}

		if(m_pCurSignCert)
		{
			if (!InternetSetOption(hReq, INTERNET_OPTION_CLIENT_CERT_CONTEXT, 
				(void *)m_pCurSignCert, sizeof(CERT_CONTEXT)) )
			{
				if ( hReq )InternetCloseHandle(hReq);
				if ( hConnect ) InternetCloseHandle(hConnect);
				if ( hOpen ) InternetCloseHandle(hOpen);
				return FALSE;
			}
		}
	}

	char CookieHeader[NTKO_MAX_HTTPHEADER];
	char tBuffer[512];
	
	BOOL isOk = FALSE;
	LPSTR	pstrCookieHeader = NULL;
	
	memset(CookieHeader,0,sizeof(CookieHeader));
	memset(tBuffer,0,sizeof(tBuffer));
	
	isOk = ReadRegisterInfo(HKEY_CURRENT_USER, 
		"Software\\Microsoft\\Internet Explorer\\International","AcceptLanguage",
		tBuffer,sizeof(tBuffer));
	if(isOk)
	{
		lstrcpyA(CookieHeader,"Accept-Language: ");
		lstrcatA(CookieHeader,tBuffer);
		lstrcatA(CookieHeader,"\r\n");
		pstrCookieHeader = CookieHeader;
	}
	
	dw = pstrCookieHeader?lstrlen(pstrCookieHeader):0;
	while (!HttpSendRequest
		(
		hReq,                                // request handle
		pstrCookieHeader,                          // header
		dw,                 // header length
		NULL,                                 // optional data buffer pointer
		0                                    // optional buffer length
		))
	{
		DWORD dwError = GetLastError();
		if ( dwError == ERROR_INTERNET_CLIENT_AUTH_CERT_NEEDED )
		{			
			if(SelectLogonCert(hReq))
			{
				continue;
			}
		}
		if ( hReq )InternetCloseHandle(hReq);
		if ( hConnect ) InternetCloseHandle(hConnect);
		if ( hOpen ) InternetCloseHandle(hOpen);
		return FALSE;
	};
	
	
	DWORD dwInfoLevel = HTTP_QUERY_RAW_HEADERS_CRLF;
	DWORD dwInfoBufferLength = NTKO_DEF_NETREADBUFFERSIZE;
	rgbBuf = new BYTE[dwInfoBufferLength+1];
	
	if (!rgbBuf) 
	{
		if ( hReq )InternetCloseHandle(hReq);
		if ( hConnect ) InternetCloseHandle(hConnect);
		if ( hOpen ) InternetCloseHandle(hOpen);
		return FALSE;
	}
	
	while (!HttpQueryInfo(hReq, dwInfoLevel, rgbBuf, &dwInfoBufferLength, NULL))
	{
		DWORD dwError = GetLastError();
		if (dwError == ERROR_INSUFFICIENT_BUFFER)
		{
			delete [] rgbBuf;
			rgbBuf = new BYTE[dwInfoBufferLength+1];
			if (!rgbBuf) 
			{
				if ( hReq )InternetCloseHandle(hReq);
				if ( hConnect ) InternetCloseHandle(hConnect);
				if ( hOpen ) InternetCloseHandle(hOpen);
				return FALSE;
			}
		}
		else
		{
			break;
		}
	}
	
	rgbBuf[dwInfoBufferLength] = '\0';
	delete [] rgbBuf;
	rgbBuf = NULL;

#ifdef _WIN64	// tanger add 20160316
	if (Proxy_OpenLocalFile(pwszFile, GENERIC_WRITE, 0, CREATE_ALWAYS, &hFile))
#else	
	if (NtkoOpenLocalFileW(pwszFile, GENERIC_WRITE, 0, CREATE_ALWAYS, &hFile))
#endif
	{
		DWORD dwBytesAvailable;
		while (InternetQueryDataAvailable(hReq, &dwBytesAvailable, 0, 0))
		{
			rgbBuf = new BYTE[dwBytesAvailable+1];
			if (!rgbBuf) 
			{
				if ( hReq )InternetCloseHandle(hReq);
				if ( hConnect ) InternetCloseHandle(hConnect);
				if ( hOpen ) InternetCloseHandle(hOpen);
				return FALSE;
			}
			if (!InternetReadFile (hReq, (void*)rgbBuf, dwBytesAvailable , &cbRead))
			{
				DWORD dwError = GetLastError();
				hr = HRESULT_FROM_WIN32(dwError);		
				break;
			}
			
			if	(cbRead == 0) 
			{
				break;
			}
			
#ifdef _WIN64	// tanger add 20160316
			if (FALSE == Proxy_WriteLocalFile(hFile, rgbBuf, cbRead, &cbWritten, NULL))
#else			
			if (FALSE == WriteFile(hFile, rgbBuf, cbRead, &cbWritten, NULL))
#endif
			{
				hr = HRESULT_FROM_WIN32(GetLastError());
				break;
			}
			delete [] rgbBuf;
			rgbBuf = NULL;
			hr = S_OK;
		} 		
#ifdef _WIN64	// // tanger add 20160316
		Proxy_CloseLocalFile(hFile);
#else
		CloseHandle(hFile);	
#endif		
	}
	else
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
	}
	
	if(rgbBuf)
	{
		delete [] rgbBuf;
	}
	
	
	DWORD httpStatusCode = 0;
	DWORD dwSize = sizeof(DWORD);
	
	if(SUCCEEDED(hr))
	{
		if(
			HttpQueryInfo(hReq,HTTP_QUERY_STATUS_CODE|HTTP_QUERY_FLAG_NUMBER,(void*)&httpStatusCode,&dwSize,NULL) &&
			(HTTP_STATUS_OK == httpStatusCode) //OK
			)
		{
			hr = S_OK;
		}
		else
		{
			hr = E_FAIL;
		};
	}

	if ( hReq )InternetCloseHandle(hReq);
	if ( hConnect ) InternetCloseHandle(hConnect);
	if ( hOpen ) InternetCloseHandle(hOpen);
	return FAILED(hr)?FALSE:TRUE;
}

LPWSTR CNTKOServerSignHelper::GetLoadedToLocalsignPath(NtkoSignType signtype)
{
	//if( !CheckServerBeforeNomalSign()) return NULL;

	if(!CheckOpRight(NTKOServerRequestType_GetSignList))  return NULL;
	
	if (	NtkoSignType_single==signtype || 
			NtkoSignType_multi==signtype || 
			NtkoSignType_dubleCrossPage==signtype || 
			NtkoSignType_multiCrossPage==signtype )
// 		 (NtkoSignType_multi==signtype&&m_publishRightSettings.bUseMutisignEkey) ||
// 		 ((NtkoSignType_dubleCrossPage==signtype||NtkoSignType_multiCrossPage==signtype)&&m_publishRightSettings.bUseMuticrosssignEkey) )
	{
		//m_iSelectedSignIndex = -1;
		
		if(m_iUserSignsCount>1)
		{
			int dlgRet = IDCANCEL;
			dlgRet = NtkoShowLargeOkCancelFrameDlg(m_hwndParent,SelSignIndexDlgFrameHandler,this);

			if( (IDOK != dlgRet) )
			{
				m_selectedSignIndex = -1;
				return NULL;
			}
		}
		else if( 1 == m_iUserSignsCount)
		{
			m_selectedSignIndex = 0;
		}
		else
		{
			ShowMsgBoxMUIStrW(IDSMUI_SERVER_NO_AVAIL_SIGN,IDSMUI_MSGBOXTITLE_TIP,MB_OK|MB_ICONINFORMATION);
			return NULL;
		}


		if((m_selectedSignIndex<0)||(m_selectedSignIndex>=NTKO_MAX_SIGNS))
		{
			ShowMsgBoxMUIStrW(IDSMUI_SIGNSLISTDLG_PLZSELECTSIGN,IDSMUI_MSGBOXTITLE_TIP,MB_OK|MB_ICONINFORMATION);
			return NULL;
		}
		
		LPWSTR pwszTempFile = NULL;
		if(!GetTempNewFilePath(&pwszTempFile,"~tmpnssf",".esp"))
		{
			return NULL;
		}
		
		ShowDelayDlgMUIStrW(IDSMUI_DOWNLOADSIGN_DATA);

#if !defined(NTKO_CUSTOM_JIANGSU_CA)
		BOOL isContinue = DownLoadURLToFile(m_rootSvrUrl,m_NtkoUserSigns[m_selectedSignIndex].bstrSignURL,pwszTempFile);
#else
		BOOL isContinue = FALSE;
		if(m_IsJSCAUseSealSvr)
		{
		#if 0 //初次版本的江苏ca请求。现在的版本客户端不需要直接和seal服务器打交道，禁用，保留代码备查
			WCHAR doWebGetParams[512] = {0};
			LPWSTR timeStamp = NULL;
			LPWSTR key = NULL;
			if ( NTKO_Custom_CreateKey(&key,&timeStamp) )
			{
				NtkoFwsprintfSafe(doWebGetParams, sizeof(doWebGetParams),
					L"%s?SealID=%s&TimeStamp=%s&Key=%s",
					NTKO_HC_DOWNLOADSIGN_URL_JSCA, 
					m_NtkoUserSigns[m_selectedSignIndex].bstrSignURL,
					timeStamp, key);
				
				SAFE_FREESTRING(timeStamp);
				SAFE_FREESTRING(key);
				isContinue = DownLoadURLToFile(m_CustomSignServerUrl,doWebGetParams,pwszTempFile);
			}
		#else
			WCHAR doWebGetParams[512] = {0};
			LPWSTR timeStamp = NULL;
			LPWSTR key = NULL;
			if ( NTKO_Custom_CreateKey(&key,&timeStamp) )
			{
				NtkoFwsprintfSafe(doWebGetParams, sizeof(doWebGetParams),
					L"%s?sealId=%s&TimeStamp=%s&Key=%s",
					NTKO_HC_DOWNLOADSIGN_URL_JSCA, 
					m_NtkoUserSigns[m_selectedSignIndex].bstrSignURL,
					timeStamp, key);
				
				SAFE_FREESTRING(timeStamp);
				SAFE_FREESTRING(key);
				if(!m_NtkoUserSigns[m_selectedSignIndex].bIsESPSign)
				{
					isContinue = DownLoadURLToFile(m_rootSvrUrl,doWebGetParams,pwszTempFile);
				}
				else
				{
					isContinue = DownLoadURLToFile(m_rootSvrUrl,m_NtkoUserSigns[m_selectedSignIndex].bstrSignURL,pwszTempFile);
				}
			}
		#endif
		}
		else
		{
			isContinue = DownLoadURLToFile(m_rootSvrUrl,m_NtkoUserSigns[m_selectedSignIndex].bstrSignURL,pwszTempFile);
		}
#endif
		CloseDelayDlg();

		if(!isContinue)
		{
			SAFE_FREESTRING(pwszTempFile);
			ShowMsgBoxMUIStrW(IDSMUI_DOWNLOADSIGN_FAIL,IDSMUI_MSGBOXTITLE_TIP,MB_OK | MB_ICONINFORMATION);
			return NULL;
		}
		return pwszTempFile;
	}

	return NULL;
}


INT_PTR CNTKOServerSignHelper::SelSignIndexDlgFrameHandler(HWND hDlg,HWND hwndTab,UINT Msg,WPARAM wParam,LPARAM lParam,LPVOID pUserParam)
{
	CNTKOServerSignHelper * pHelper = (CNTKOServerSignHelper*)pUserParam;
	
	switch (Msg) 
	{ 
	case WM_INITDIALOG:
		{
			if(pHelper)
			{
				LPWSTR pwszTitle = pHelper->LoadMUIStringW(IDSMUI_SELECTSIGN_DLG_TITLE);	
				NtkoSetWindowTextW(hDlg,pwszTitle);
				SAFE_FREESTRING(pwszTitle);
				pwszTitle = pHelper->LoadMUIStringW(IDCMUI_SELECTSIGN_DLG_TABT);
				NtkoAddDlgTabPage((HINSTANCE)g_hModule,hDlg,hwndTab,IDD_DIALOGSELECTSIGNINDEX,
					(DLGPROC)CNTKOServerSignHelper::SelSignIndexDlgTabProc,pwszTitle,(LPARAM)pHelper);
				SAFE_FREESTRING(pwszTitle);
				
				NtkoShowDlgTabPage(hwndTab,0);
				
				return FALSE;
			}
			else
			{
				return TRUE;
			}
		}
		return TRUE;
	} 
	return FALSE;
}

INT_PTR CNTKOServerSignHelper::SelSignIndexDlgTabProc(HWND hDlg, UINT Msg, WPARAM wParam,LPARAM lParam)
{
#ifdef _WIN64	// zhuguanglin add 20150313
	CNTKOServerSignHelper* pHelper = (CNTKOServerSignHelper*)GetWindowLongPtr(hDlg, GWLP_USERDATA);	
#else
	CNTKOServerSignHelper* pHelper = (CNTKOServerSignHelper*)GetWindowLong(hDlg, GWL_USERDATA);	
#endif
	HWND hwndParent = GetParent(hDlg);
	switch (Msg) 
	{ 
		case WM_INITDIALOG:			
			if (NULL != lParam)
			{
				pHelper = (CNTKOServerSignHelper*)lParam;
#ifdef _WIN64	// zhuguanglin add 20150313
				SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)pHelper);	
#else
				SetWindowLong(hDlg, GWL_USERDATA, (LONG)pHelper);	
#endif
				EnableWindow(GetDlgItem(hwndParent,IDOK),FALSE);
				if(pHelper)
				{
					//初始化控件标题
					//初始化控件标题
					LPWSTR pwszText = pHelper->LoadMUIStringW(IDCMUI_BUTTON_CONTINUE);
					NtkoSetDlgItemTextW(hwndParent,IDCMUI_BUTTON_CONTINUE,pwszText);
					SAFE_FREESTRING(pwszText);

					pwszText = pHelper->LoadMUIStringW(IDCMUI_SELECTSIGN_DLG_INPUTINFO);
					NtkoSetDlgItemTextW(hDlg,IDC_STATIC_INPUTINFO,pwszText);
					SAFE_FREESTRING(pwszText);

					SetDlgItemTextW(hDlg,IDC_EDIT_SIGNFILTER,L"");

					HWND hWndListView = GetDlgItem(hDlg,IDC_LISTSIGNS);	

					ListView_SetExtendedListViewStyleEx(hWndListView,
						LVS_EX_FULLROWSELECT|LVS_EX_REGIONAL,
						LVS_EX_FULLROWSELECT|LVS_EX_REGIONAL);
					 // Assign the image lists to the list-view control. 
					pHelper->SetDlgListImageList(hWndListView,0);
					if(pHelper->InitSelSignListViewColumns(hWndListView))
					{
						pHelper->FillSelSignListWithSignsInfo(hWndListView,FALSE,NULL);					
					}
					return FALSE; // we have set focus,so return false
				}
			}
			return TRUE;
		case WM_CTLCOLORDLG:
			{
				if(pHelper)
				{
					HDC hdcControl = (HDC)wParam;
					SetBkMode(hdcControl,TRANSPARENT) ;
					return (int)GetStockObject(NTKO_TABEDDLG_BKCOLOR);
				}
			}
			break;
		case WM_CTLCOLORSTATIC:
			{
				int ictlID = GetDlgCtrlID((HWND)lParam);
				HDC hdc = (HDC)wParam;
				SetBkMode(hdc, TRANSPARENT);
				return (int)GetStockObject(NTKO_TABEDDLG_BKCOLOR);
			}
			break;
		case WM_NOTIFY:
			if(IDC_LISTSIGNS == (int)wParam)
			{
				LPNMITEMACTIVATE lpnmitem;
				lpnmitem = (LPNMITEMACTIVATE) lParam;
				UINT  iNotifyCode = lpnmitem->hdr.code;
				switch(iNotifyCode)
				{
				case NM_CLICK:
				case NM_DBLCLK:
					{
						BOOL isOk = !(BOOL)(lpnmitem->iItem<0);
						EnableWindow(GetDlgItem(hwndParent,IDOK),isOk);
						InvalidateRect(GetDlgItem(hwndParent,IDOK),NULL,TRUE);
						HWND hWndListView = GetDlgItem(hDlg,IDC_LISTSIGNS);	
						if(pHelper)
						{
							pHelper->m_selectedSignIndex = NtkoGetListViewItemParamW(hWndListView,lpnmitem->iItem);
						}
					
						if( (NM_DBLCLK == iNotifyCode) && isOk )
						{
							SendMessage(hwndParent,WM_COMMAND,(WPARAM)IDOK,0);
						}
					}
					break;
				}
			}
			break;
		case WM_DESTROY:
			{				
				if(pHelper)
				{
					pHelper->DestroyDlgListImageList();		
				}
				break;
			}
		case WM_COMMAND: 
			switch (LOWORD(wParam)) 
			{ 
			case IDC_EDIT_SIGNFILTER:
				{
					if( EN_CHANGE == HIWORD(wParam))
					{
						WCHAR wszFilter[1024];
						ZeroMemory(wszFilter,sizeof(wszFilter));
						int countChar = NtkoGetDlgItemTextW(hDlg,IDC_EDIT_SIGNFILTER,&wszFilter[0],
							sizeof(wszFilter)/sizeof(wszFilter[0]));
						HWND hWndListView = GetDlgItem(hDlg,IDC_LISTSIGNS);	
						pHelper->FillSelSignListWithSignsInfo(hWndListView,FALSE,wszFilter);	
					}
				}
				break;
			case IDOK: 	
				{
					int i = (pHelper)?pHelper->m_selectedSignIndex:-1;
					if(i<0)
					{
						pHelper->ShowMsgBoxMUIStrW(IDSMUI_SIGNSLISTDLG_PLZSELECTSIGN,
							IDSMUI_MSGBOXTITLE_TIP,MB_OK | MB_ICONINFORMATION);

#ifdef _WIN64	// zhuguanglin add 20150312
						SetWindowLongPtr(hDlg,DWLP_MSGRESULT,TRUE);//保持对话框
#else
						SetWindowLong(hDlg,DWL_MSGRESULT,TRUE);//保持对话框
#endif
						return TRUE;
					}
					else
					{
						//EndDialog(hDlg, (INT_PTR)(wParam));
					}						
				}
				break;
			case IDCANCEL:
				pHelper->m_selectedSignIndex = -1;
				//EndDialog(hDlg, (INT_PTR)(wParam));
				break;
			} 
			break;
	} 
	return FALSE;
}

void CNTKOServerSignHelper::SetDlgListImageList(HWND hWndListView,int iDlgType)
{
	// ODS("CNTKOHandSignControl::SetDlgListImageList()\n");
	HICON hiconItem;     // icon for list-view items 
	
	// Create the full-sized icon image lists. 
	m_hLargeSelSign = ImageList_Create(GetSystemMetrics(SM_CXICON), 
		GetSystemMetrics(SM_CYICON), ILC_MASK, 1, 1); 
	m_hSmallSelSign = ImageList_Create(GetSystemMetrics(SM_CXSMICON), 
		GetSystemMetrics(SM_CYSMICON), ILC_MASK, 1, 1); 
	
	if(0 == iDlgType)
	{
		// Add an icon to each image list.  
		hiconItem = LoadIcon((HINSTANCE)g_hModule, MAKEINTRESOURCE(IDI_ICONLOGO)); 
		ImageList_AddIcon(m_hLargeSelSign, hiconItem); 
		ImageList_AddIcon(m_hSmallSelSign, hiconItem); 
		DestroyIcon(hiconItem); 
	}
	else if(1 == iDlgType)
	{
		hiconItem = LoadIcon((HINSTANCE)g_hModule, MAKEINTRESOURCE(IDI_ICONLOGO)); 
		ImageList_AddIcon(m_hLargeSelSign, hiconItem); 
		ImageList_AddIcon(m_hSmallSelSign, hiconItem); 
		DestroyIcon(hiconItem); 
		hiconItem = LoadIcon((HINSTANCE)g_hModule, MAKEINTRESOURCE(IDI_ICONERROR)); 
		ImageList_AddIcon(m_hLargeSelSign, hiconItem); 
		ImageList_AddIcon(m_hSmallSelSign, hiconItem); 
		DestroyIcon(hiconItem); 
	}
	// Assign the image lists to the list-view control. 
	ListView_SetImageList(hWndListView, m_hLargeSelSign, LVSIL_NORMAL); 
	ListView_SetImageList(hWndListView, m_hSmallSelSign, LVSIL_SMALL);
}

BOOL CNTKOServerSignHelper::InitSelSignListViewColumns(HWND hWndListView)
{
	//ODS("CNTKOHandSignControl::InitSelSignListViewColumns()\n");
	LPWSTR pwszText = NULL;
	int colWidth = 100;
	int iCol; 
	
	// Add the columns. 
	for (iCol = 0; iCol < 3; iCol++) 
	{
		pwszText = LoadMUIStringW(IDSMUI_LVSELSIGNSFIRSTCOLUMN0+ iCol);        
		NtkoInsertListViewColumnW(hWndListView,LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM,
			iCol,LVCFMT_LEFT, pwszText,colWidth);
		SAFE_FREESTRING(pwszText);
	} 
	return TRUE;
}

BOOL CNTKOServerSignHelper::FillSelSignListWithSignsInfo(HWND hWndListView,BOOL isIncludeBlankSigns,LPWSTR pwszFilter) 
{
	ListView_DeleteAllItems(hWndListView);
	BOOL bInserted = FALSE;
	for(int ic=0;((ic<m_iUserSignsCount)&&(ic<NTKO_MAX_SIGNS));ic++)
	{
		WCHAR wszCol1[255];
#if !defined(NTKO_CUSTOM_JIANGSU_CA)
		NtkoFwsprintf(wszCol1,L"%d",ic+1);
#else
		if(m_IsJSCAUseSealSvr)
		{
			if(!m_NtkoUserSigns[ic].bIsESPSign)
			{
				NtkoFwsprintf(wszCol1,L"%s",m_NtkoUserSigns[ic].bstrSignURL);
			}			
			else
			{
				NtkoFwsprintf(wszCol1,L"RH%04d",ic+1);
			}
		}	
		else
		{
			NtkoFwsprintf(wszCol1,L"RH%04d",ic+1);
		}
#endif
		
		if( (m_NtkoUserSigns[ic].bstrSignName) && 
			( 
				(!m_pwszSignFilter) || 
				(0 == lstrlenW(m_pwszSignFilter)) || 
				NtkoAreStringsEqualW(m_NtkoUserSigns[ic].bstrSignName,-1,m_pwszSignFilter,-1)
				//NtkoHasSubStringW(m_NtkoUserSigns[ic].bstrSignName,m_pwszSignFilter)
			)
		)
		{

			if( (!pwszFilter) || (0 == lstrlenW(pwszFilter)) || NtkoHasSubStringW(m_NtkoUserSigns[ic].bstrSignName,pwszFilter) )
			{
				int i =  NtkoInsertListViewItemW(hWndListView,LVIF_TEXT | LVIF_IMAGE|LVIF_PARAM, ic,wszCol1,0,ic);
				NtkoSetListViewItemTextW(hWndListView,i,1,m_NtkoUserSigns[ic].bstrSignName);
				NtkoSetListViewItemTextW(hWndListView,i,2,m_NtkoUserSigns[ic].bstrSignUser);
			}
			bInserted = TRUE;
		}
	}

	if(!bInserted) //插入所有印章
	{
		for(int ic=0;((ic<m_iUserSignsCount)&&(ic<NTKO_MAX_SIGNS));ic++)
		{
			WCHAR wszCol1[255];
#if !defined(NTKO_CUSTOM_JIANGSU_CA)
			NtkoFwsprintf(wszCol1,L"%d",ic+1);
#else
			if(m_IsJSCAUseSealSvr)
			{
				//NtkoFwsprintf(wszCol1,L"%s",m_NtkoUserSigns[ic].bstrSignURL);
				if(!m_NtkoUserSigns[ic].bIsESPSign)
				{
					NtkoFwsprintf(wszCol1,L"%s",m_NtkoUserSigns[ic].bstrSignURL);
				}			
				else
				{
					NtkoFwsprintf(wszCol1,L"RH%04d",ic+1);
				}
			}	
			else
			{
				NtkoFwsprintf(wszCol1,L"RH%04d",ic+1);
			}
#endif				
			if(m_NtkoUserSigns[ic].bstrSignName)
			{
				if( (!pwszFilter) || (0 == lstrlenW(pwszFilter)) || NtkoHasSubStringW(m_NtkoUserSigns[ic].bstrSignName,pwszFilter) )
				{
					int i =  NtkoInsertListViewItemW(hWndListView,LVIF_TEXT | LVIF_IMAGE|LVIF_PARAM, ic,wszCol1,0,ic);
					NtkoSetListViewItemTextW(hWndListView,i,1,m_NtkoUserSigns[ic].bstrSignName);
					NtkoSetListViewItemTextW(hWndListView,i,2,m_NtkoUserSigns[ic].bstrSignUser);
				}
			}
		}
	}

	AutoAdjustListViewColumn(hWndListView);
	return TRUE;
}

VOID CNTKOServerSignHelper::AutoAdjustListViewColumn(HWND hWndListView)
{
	HWND hWndHeader = ListView_GetHeader(hWndListView);
	//ShowWindow(hWndHeader,SW_HIDE);
	int colNumber = -1;
	int columnCount = NtkoGetListViewColumnsCount(hWndListView);
	
	//自动调整大小
	for(int Loop = 0;Loop<columnCount;Loop++)
	{
		ListView_SetColumnWidth(hWndListView,Loop,LVSCW_AUTOSIZE_USEHEADER);		
	}
	//ShowWindow(hWndHeader,SW_SHOW);
}

VOID CNTKOServerSignHelper::DestroyDlgListImageList()
{
	//ODS("CNTKOHandSignControl(%d)::DestroyDlgListImageList()!\n");
	if(m_hLargeSelSign)
	{
		ImageList_Destroy(m_hLargeSelSign); 
		m_hLargeSelSign = NULL;
	}
	if(m_hSmallSelSign)
	{
		ImageList_Destroy(m_hSmallSelSign);
		m_hSmallSelSign = NULL;
	}
	//ODS("CNTKOHandSignControl(%d)::DestroyDlgListImageList() end!\n");
}

BOOL CNTKOServerSignHelper::CheckEkeyToSignRel(LPWSTR ekeySN,LPWSTR signSN)
{
	HRESULT hr = S_OK;
	//VARIANT vResult;
	BOOL isOk = FALSE;
	WCHAR wszCPARA[2048];
	BSTR bstrRetXML = NULL;

	//构造参数
	ZeroMemory(&wszCPARA[0],sizeof(wszCPARA));

	lstrcatW(wszCPARA,L"&signsn=");
	lstrcatW(wszCPARA,signSN);

	lstrcatW(wszCPARA,L"&ekeysn=");
	lstrcatW(wszCPARA,ekeySN);		
	
	//向服务器提交请求
	ShowDelayDlgMUIStrW(IDSMUI_SIGN2EKEYCHECK);
	isOk = DoWebExecute2(m_rootSvrUrl,NTKO_HC_SIGN2EKEYCHECK_URL,wszCPARA,&bstrRetXML);		
	CloseDelayDlg();
	if(isOk && bstrRetXML)
	{			
		isOk = ParseXML(bstrRetXML);
		//MyMessageBoxW(hwndDlgParent, bstrRetXML,L"bstrRetXML" ,MB_OK|MB_ICONINFORMATION);
		if(isOk) //解析成功，但是服务器有可能有错误
		{ 
			if( 0 == m_iServerErrCode) 
			{
				if(NtkoAreStringsEqualW(m_bstrServerVersion,-1,NTKO_ServerSecsignControlProtocalVersion,-1))
				{
					isOk = TRUE;
				}
				else
				{
					ShowMsgBoxMUIStrW(IDSMUI_CHECKRIGHT_FAIL_INVALIDSERVER_VERSION,
						IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
					isOk = FALSE;
				}
			}
			else //服务器返回有错误
			{
				WCHAR wszBuf[1024];
				LPWSTR pwszFmt = LoadMUIStringW(IDSMUI_CHECKRIGHT_FAIL_SERVERFAILWITHCODE);
				NtkoFwsprintf(wszBuf,pwszFmt,m_iServerErrCode,m_bstrFailReason);
				SAFE_FREESTRING(pwszFmt);
				
				LPWSTR pwszMUITitle = LoadMUIStringW(IDSMUI_MSGBOXTITLE_TIP);
				NtkoMessageBoxW(m_hwndParent,wszBuf,pwszMUITitle ,MB_OK|MB_ICONINFORMATION);
				SAFE_FREESTRING(pwszMUITitle);

				if(1 == m_iServerErrCode)
				{
					m_IsLogin = FALSE;
				}
				isOk = FALSE;
			}
		}
		else //解析错误
		{
			ShowMsgBoxMUIStrW(IDSMUI_CHECKRIGHT_FAIL_PARSERESPONSEERROR,
				IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
			isOk = FALSE;
		}
	}
	else
	{			
		ShowMsgBoxMUIStrW(IDSMUI_CHECKRIGHT_FAIL,IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		isOk = FALSE;
	}

	if(isOk)
	{
		isOk = m_serverRightSettings.bUseEkeySign;
		if(!isOk)
		{
			ShowMsgBoxMUIStrW( IDSMUI_CHECKRIGHT_USER_NORIGHT,IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		}
		isOk = ParseSignOptions();
		if(!isOk)
		{
			ShowMsgBoxMUIStrW(IDSMUI_GETSIGNOPTIONS_FAIL_EKEYSIGNERROR_PARSEERROR,
				IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		}
		else
		{
			isOk = ParseNtkoCheckEkey();
			if(!isOk)
			{
				ShowMsgBoxMUIStrW(IDSMUI_CHECKRIGHT_FAIL_EKEYSIGNERROR_PARSEERROR,
					IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
			}
			else
			{
				isOk = m_CanUseThisEkey;
				if(!isOk)
				{
					ShowMsgBoxMUIStrW(IDSMUI_SIGN2EKEYCHECK_NORIGHT_EKEY,IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
				}
			}
		}
	}
	SAFE_FREEBSTRING(bstrRetXML);
	return isOk;
}

BOOL CNTKOServerSignHelper::GetTempNewFilePath(WCHAR** ppwszLocalFile,LPSTR preStr,LPSTR extStr)
{
	LPWSTR pwszTPath = NULL;
	CHAR   szTmpPath[MAX_PATH];
	CHAR	szTmpFile[MAX_PATH];
	
	if ((!ppwszLocalFile) || (!preStr) ) return FALSE;
	
	*ppwszLocalFile = NULL;
	
	if (GetTempPath(MAX_PATH, szTmpPath))
	{
		DWORD dwtlen = lstrlen(szTmpPath);
		if (dwtlen > 0 && szTmpPath[dwtlen-1] != '\\')
			lstrcat(szTmpPath, "\\");
		
		lstrcat(szTmpPath, NTKO_HC_TEMPFILEPATH);
		
		if (CreateDirectory(szTmpPath, NULL) || GetLastError() == ERROR_ALREADY_EXISTS)
		{
			lstrcat(szTmpPath, "\\");			
		}
	}
	
	if (lstrlen(szTmpPath))
	{
		char formatStr[MAX_PATH];
		
		for(long i = 0;i<2147483647;i++)
		{
			wsprintf(formatStr,"%%s%s%%d%s",preStr,extStr);
			TRACE1_NTKOLIB("GetTempNewFilePath:formatStr=%s\n",formatStr);
			wsprintf(szTmpFile,formatStr,szTmpPath,i);
			TRACE1_NTKOLIB("GetTempNewFilePath:szTmpFile=%s\n",szTmpFile);
			pwszTPath =  NtkoANSIToLPWSTR(szTmpFile);
			if(!NtkoFileExistsW(pwszTPath))
			{
				break;
			}
			SAFE_FREESTRING(pwszTPath);
		}
		*ppwszLocalFile = pwszTPath;
	}
	
	return (*ppwszLocalFile)==NULL?FALSE:TRUE;
}


VOID CNTKOServerSignHelper::FreeMyMem(LPWSTR p)
{
	SAFE_FREESTRING(p);
}

VOID CNTKOServerSignHelper::SetSignsFilter(LPWSTR pwszSignFilter)
{
	SAFE_FREESTRING(m_pwszSignFilter);
	m_pwszSignFilter = NtkoCopyStringW(pwszSignFilter);
}
VOID CNTKOServerSignHelper::ClearCheckSignServers()
{
	for(int i=0;i<NTKO_MAX_CHECKSIGN_SERVERS;i++)
	{
		SAFE_FREEBSTRING(m_NtkoCheckSignServers[i].bstrServerAppURL);
		SAFE_FREEBSTRING(m_NtkoCheckSignServers[i].bstrSignServerName);
		SAFE_FREESTRING(m_NtkoCheckSignServers[i].pwszPostPara);
	}
	ZeroMemory(&m_NtkoCheckSignServers[0],sizeof(m_NtkoCheckSignServers));
	m_iCheckSignServersCount = 0;
}

BOOL CNTKOServerSignHelper::ParseCheckSignServers()
{
	HRESULT hr = S_OK;
	BOOL isOk = FALSE;
	IXMLDOMNode *pIDOMNode = NULL;
	
	hr = m_pXMLDoc->selectSingleNode( OLESTR("//ResponseInfo"), &pIDOMNode);
	if(FAILED(hr) || (!pIDOMNode))
	{
		return FALSE;
	}
	
	long value = 0;
	IXMLDOMNodeList *pIDOMNodeList = NULL;
	
	hr = pIDOMNode->selectNodes( OLESTR("./CheckSignServerApp"), &pIDOMNodeList);
	if(SUCCEEDED(hr) && pIDOMNodeList)
	{
		hr = pIDOMNodeList->get_length(&value);
		if(SUCCEEDED(hr))
		{
			isOk = TRUE;
			pIDOMNodeList->reset();
			for(int ii = 0; (ii < value) && (ii < NTKO_MAX_CHECKSIGN_SERVERS); ii++)
			{
				IXMLDOMNode *pIDOMNode2 = NULL;
				hr = pIDOMNodeList->get_item(ii, &pIDOMNode2);
				if(pIDOMNode2)
				{
					IXMLDOMNamedNodeMap *pAttributeMap = NULL;
					hr = pIDOMNode2->get_attributes(&pAttributeMap);
					if(SUCCEEDED(hr) && pAttributeMap)
					{
						IXMLDOMNode *pIDOMNode3 = NULL;
						//signUNID
						hr = pAttributeMap->getNamedItem(L"serverId",&pIDOMNode3);
						if(SUCCEEDED(hr) && pIDOMNode3)
						{
							BSTR bstrTemp = NULL;
							pIDOMNode3->get_text(&bstrTemp);
							if(bstrTemp)
							{
								int iSignServerId = 0;
								if(NtkoStrToIntExW(bstrTemp,STIF_DEFAULT,&iSignServerId) && (iSignServerId>0))
								{
									m_NtkoCheckSignServers[ii].iSignServerId = iSignServerId;
									isOk = TRUE;
								}
								else
								{
									isOk = FALSE;
								}
							}
							SAFE_FREEBSTRING(bstrTemp);								
							RELEASE_INTERFACE(pIDOMNode3);
						}
						if(isOk)
						{
							isOk = FALSE;
							hr = pAttributeMap->getNamedItem(L"serverName",&pIDOMNode3);
							if(SUCCEEDED(hr) && pIDOMNode3)
							{
								pIDOMNode3->get_text(&m_NtkoCheckSignServers[ii].bstrSignServerName);			
								RELEASE_INTERFACE(pIDOMNode3);
								if(::SysStringByteLen(m_NtkoCheckSignServers[ii].bstrSignServerName))
								{
									isOk = TRUE;
								}
								else
								{
									isOk = FALSE;
								}
							}
						}						
						if(isOk)
						{
							isOk = FALSE;
							hr = pAttributeMap->getNamedItem(L"serverAppURL",&pIDOMNode3);
							if(SUCCEEDED(hr) && pIDOMNode3)
							{
								pIDOMNode3->get_text(&m_NtkoCheckSignServers[ii].bstrServerAppURL);			
								RELEASE_INTERFACE(pIDOMNode3);
								if(::SysStringByteLen(m_NtkoCheckSignServers[ii].bstrServerAppURL))
								{
									isOk = TRUE;
								}
								else
								{
									isOk = FALSE;
								}
							}
						}
						
						if(isOk)
						{
							m_iCheckSignServersCount++;
						}
						
						RELEASE_INTERFACE(pAttributeMap);
					}
					RELEASE_INTERFACE(pIDOMNode2);
				}
			}// end for
		}
		RELEASE_INTERFACE(pIDOMNodeList);
	}
	RELEASE_INTERFACE(pIDOMNode);
	return isOk;
}

void CNTKOServerSignHelper::ClearServerSigns()
{
	for(int i=0;i<NTKO_MAX_DOC_SERVER_SIGNS;i++)
	{
		SAFE_FREEBSTRING(m_NtkoServerSigns[i].bstrSignGUID);
		SAFE_FREEBSTRING(m_NtkoServerSigns[i].bstrSigner);
		SAFE_FREEBSTRING(m_NtkoServerSigns[i].bstrSignName);
		SAFE_FREEBSTRING(m_NtkoServerSigns[i].bstrSignUser);
		SAFE_FREEBSTRING(m_NtkoServerSigns[i].bstrSignSN);
		SAFE_FREEBSTRING(m_NtkoServerSigns[i].bstrEkeySN);
		SAFE_FREEBSTRING(m_NtkoServerSigns[i].bstrServerTime);
		SAFE_FREEBSTRING(m_NtkoServerSigns[i].bstrAppName);
		SAFE_FREEBSTRING(m_NtkoServerSigns[i].bstrClientIP);
	}
	ZeroMemory(&m_NtkoServerSigns[0],sizeof(m_NtkoServerSigns));
	m_iServerSignsCount = 0;
}

BOOL CNTKOServerSignHelper::ParseSignsCheckStatus()
{
	HRESULT hr = S_OK;
	BOOL isOk = FALSE;
	BOOL isFound = FALSE;
	IXMLDOMNode *pIDOMNode = NULL;
	
	hr = m_pXMLDoc->selectSingleNode( OLESTR("//ResponseInfo"), &pIDOMNode);
	if(FAILED(hr) || (!pIDOMNode))
	{
		return FALSE;
	}
	
	long value = 0;
	IXMLDOMNodeList *pIDOMNodeList = NULL;
	
	hr = pIDOMNode->selectNodes( OLESTR("./SignsCheckStatus"), &pIDOMNodeList);
	if(SUCCEEDED(hr) && pIDOMNodeList)
	{
		hr = pIDOMNodeList->get_length(&value);
		if(SUCCEEDED(hr))
		{
			isOk = TRUE;
			pIDOMNodeList->reset();
			for(int ii = 0; (ii < value) && (ii < NTKO_MAX_DOC_SERVER_SIGNS); ii++)
			{
				IXMLDOMNode *pIDOMNode2 = NULL;
				hr = pIDOMNodeList->get_item(ii, &pIDOMNode2);
				if(pIDOMNode2)
				{
					IXMLDOMNamedNodeMap *pAttributeMap = NULL;
					hr = pIDOMNode2->get_attributes(&pAttributeMap);
					if(SUCCEEDED(hr) && pAttributeMap)
					{
						IXMLDOMNode *pIDOMNode3 = NULL;
						//signUNID
						hr = pAttributeMap->getNamedItem(L"signUNID",&pIDOMNode3);
						if( SUCCEEDED(hr) && pIDOMNode3 )
						{
							pIDOMNode3->get_text(&m_NtkoServerSigns[m_iServerSignsCount].bstrSignGUID);			
							RELEASE_INTERFACE(pIDOMNode3);
							if(::SysStringByteLen(m_NtkoServerSigns[m_iServerSignsCount].bstrSignGUID))
							{
								isOk = TRUE;
							}
							else
							{
								isOk = FALSE;
							}
						}
						//found
						if(isOk)
						{
							isOk = FALSE;
							hr = pAttributeMap->getNamedItem(L"found",&pIDOMNode3);
							if(SUCCEEDED(hr) && pIDOMNode3)
							{
								BSTR bstrTemp = NULL;
								pIDOMNode3->get_text(&bstrTemp);
								if(bstrTemp)
								{
									if( NtkoAreStringsEqualW(bstrTemp,-1,L"true",-1) )
									{
										isOk = TRUE;
										isFound = TRUE;
										m_NtkoServerSigns[m_iServerSignsCount].bFound = TRUE;
									}
									else if( NtkoAreStringsEqualW(bstrTemp,-1,L"false",-1) )
									{
										isOk = TRUE;
										isFound = FALSE;
										m_NtkoServerSigns[m_iServerSignsCount].bFound = FALSE;
									}
									else
									{
										isOk = FALSE;
									}
								}
								SAFE_FREEBSTRING(bstrTemp);								
								RELEASE_INTERFACE(pIDOMNode3);
							}
						}
						//signer
						if(isOk && isFound)
						{
							isOk = FALSE;
							hr = pAttributeMap->getNamedItem(L"signer",&pIDOMNode3);
							if(SUCCEEDED(hr) && pIDOMNode3)
							{
								hr = pIDOMNode3->get_text(&m_NtkoServerSigns[m_iServerSignsCount].bstrSigner);			
								RELEASE_INTERFACE(pIDOMNode3);
								if(SUCCEEDED(hr))
								{
									isOk = TRUE;
								}
								else
								{
									isOk = FALSE;
								}
							}
						}
						//signName
						if(isOk && isFound)
						{
							isOk = FALSE;
							hr = pAttributeMap->getNamedItem(L"signName",&pIDOMNode3);
							if(SUCCEEDED(hr) && pIDOMNode3)
							{
								hr = pIDOMNode3->get_text(&m_NtkoServerSigns[m_iServerSignsCount].bstrSignName);			
								RELEASE_INTERFACE(pIDOMNode3);
								if(SUCCEEDED(hr))
								{
									isOk = TRUE;
								}
								else
								{
									isOk = FALSE;
								}
							}
						}
						//signUser
						if(isOk && isFound)
						{
							isOk = FALSE;
							hr = pAttributeMap->getNamedItem(L"signUser",&pIDOMNode3);
							if(SUCCEEDED(hr) && pIDOMNode3)
							{
								hr = pIDOMNode3->get_text(&m_NtkoServerSigns[m_iServerSignsCount].bstrSignUser);			
								RELEASE_INTERFACE(pIDOMNode3);
								if(SUCCEEDED(hr))
								{
									isOk = TRUE;
								}
								else
								{
									isOk = FALSE;
								}
							}
						}
						//signSN
						if(isOk && isFound)
						{
							hr = pAttributeMap->getNamedItem(L"signSN",&pIDOMNode3);
							if(SUCCEEDED(hr) && pIDOMNode3)
							{
								hr = pIDOMNode3->get_text(&m_NtkoServerSigns[m_iServerSignsCount].bstrSignSN);			
								RELEASE_INTERFACE(pIDOMNode3);
							}
						}
						
						//ekeySN
						if(isOk && isFound)
						{
							hr = pAttributeMap->getNamedItem(L"ekeySN",&pIDOMNode3);
							if(SUCCEEDED(hr) && pIDOMNode3)
							{
								hr = pIDOMNode3->get_text(&m_NtkoServerSigns[m_iServerSignsCount].bstrEkeySN);			
								RELEASE_INTERFACE(pIDOMNode3);
							}
						}
						
						//serverTime
						if(isOk && isFound)
						{
							isOk = FALSE;
							hr = pAttributeMap->getNamedItem(L"serverTime",&pIDOMNode3);
							if(SUCCEEDED(hr) && pIDOMNode3)
							{
								hr = pIDOMNode3->get_text(&m_NtkoServerSigns[m_iServerSignsCount].bstrServerTime);			
								RELEASE_INTERFACE(pIDOMNode3);
								if(SUCCEEDED(hr))
								{
									isOk = TRUE;
								}
								else
								{
									isOk = FALSE;
								}
							}
						}
						
						//appName
						if(isOk && isFound)
						{
							isOk = FALSE;
							hr = pAttributeMap->getNamedItem(L"appName",&pIDOMNode3);
							if(SUCCEEDED(hr) && pIDOMNode3)
							{
								hr = pIDOMNode3->get_text(&m_NtkoServerSigns[m_iServerSignsCount].bstrAppName);			
								RELEASE_INTERFACE(pIDOMNode3);
								if(SUCCEEDED(hr))
								{
									isOk = TRUE;
								}
								else
								{
									isOk = FALSE;
								}
							}
						}
						
						//clientIP
						if(isOk && isFound)
						{
							isOk = FALSE;
							hr = pAttributeMap->getNamedItem(L"clientIP",&pIDOMNode3);
							if(SUCCEEDED(hr) && pIDOMNode3)
							{
								hr = pIDOMNode3->get_text(&m_NtkoServerSigns[m_iServerSignsCount].bstrClientIP);			
								RELEASE_INTERFACE(pIDOMNode3);
								if(SUCCEEDED(hr))
								{
									isOk = TRUE;
								}
								else
								{
									isOk = FALSE;
								}
							}
						}
						
						if(isOk)
						{
							m_NtkoServerSigns[m_iServerSignsCount].iSignServerId = m_iCurServerId;
							m_iServerSignsCount++;
						}
						
						RELEASE_INTERFACE(pAttributeMap);
					}
					RELEASE_INTERFACE(pIDOMNode2);
				}
			}// end for
		}
		RELEASE_INTERFACE(pIDOMNodeList);
	}
	RELEASE_INTERFACE(pIDOMNode);
	return isOk;
}

CNTKOServerSignsInfo * CNTKOServerSignHelper::DoCheckSigns(PNtkoSecuritySignInfo pInfo,int count,int * outServersignCount)
{
	ShowDelayDlgMUIStrW(IDSMUI_GETTING_DOCSIGNS);
	
	if( 0==count || NULL==pInfo) //没有印章，或失败
	{
		ShowMsgBoxMUIStrW(IDSMUI_CHECKSIGNS_FAIL_NOSIGNS,IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		return NULL;
	}
	
	
	HRESULT hr = S_OK;
	BOOL isOk = FALSE;
	BSTR bstrRetXML = NULL;
	
	//获取签章验证服务器信息	
	ClearCheckSignServers();
	m_serverReqestType = NTKOServerRequestType_GetServerChangeInfo;

	ShowDelayDlgMUIStrW(IDSMUI_GETTING_CHANGED_SERVERS);
	isOk = DoWebGet(m_rootSvrUrl,NTKO_HC_GETCHGEDSVRS_URL,&bstrRetXML);	
	CloseDelayDlg();
	if(isOk && bstrRetXML)
	{			
		isOk = ParseXML(bstrRetXML);
		//MyMessageBoxW(hwndDlgParent, bstrRetXML,L"bstrRetXML" ,MB_OK|MB_ICONINFORMATION);
		if(isOk) //解析成功，但是服务器有可能有错误
		{ 
			if(0 == m_iServerErrCode)
			{
				if(NtkoAreStringsEqualW(m_bstrServerVersion,-1,NTKO_ServerSecsignControlProtocalVersion,-1))
				{
					isOk = TRUE;
				}
				else
				{
					ShowMsgBoxMUIStrW(IDSMUI_CHECKSIGNS_FAIL_INVALIDSERVER_VERSION,
						IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
					isOk = FALSE;
				}
			}
			else //服务器返回有错误
			{
				WCHAR wszBuf[1024];
				
				LPWSTR pwszFmt = LoadMUIStringW(IDSMUI_CHECKSIGNS_FAIL_SERVERFAILWITHCODE);
				NtkoFwsprintf(wszBuf,pwszFmt,m_iServerErrCode,m_bstrFailReason);
				SAFE_FREESTRING(pwszFmt);
				
				LPWSTR pwszMUITitle = LoadMUIStringW(IDSMUI_MSGBOXTITLE_TIP);
				NtkoMessageBoxW(m_hwndParent,wszBuf,pwszMUITitle ,MB_OK|MB_ICONINFORMATION);
				SAFE_FREESTRING(pwszMUITitle);
				
				isOk = FALSE;
			}
		}
		else //解析错误
		{
			ShowMsgBoxMUIStrW(IDSMUI_CHECKSIGNS_FAIL_PARSERESPONSEERROR,
				IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
			isOk = FALSE;
		}
	}
	else
	{			
		ShowMsgBoxMUIStrW(IDSMUI_CHECKSIGNS_FAIL_CONNECT,IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		isOk = FALSE;
	}
	if(isOk)
	{		
		isOk = ParseCheckSignServers();
		if(!isOk)
		{
			ShowMsgBoxMUIStrW(IDSMUI_CHECKSIGNS_FAIL_PARSERESPONSEERROR,
				IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		}	
		else
		{
			if(0 == m_iCheckSignServersCount) //没有印章，或失败
			{
				ShowMsgBoxMUIStrW(IDSMUI_CHECKSIGNS_FAIL_NOCHECKSERVERS,
					IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
				isOk = FALSE;
			}			
		}
	}
	SAFE_FREEBSTRING(bstrRetXML);
	
	if(!isOk)
	{
		return NULL;
	}
	
	ClearServerSigns();
	//构造参数
	//已经获得了签章验证服务器信息.看看需要向多少个服务器提交请求
	for(long i=0;(i<m_iCheckSignServersCount) && (i<NTKO_MAX_CHECKSIGN_SERVERS);i++)
	{
		BOOL isNeedGet = FALSE;
		for(long j=0;(j<count) && (j<NTKO_MAX_DOC_SERVER_SIGNS);j++)
		{
			//m_NtkoCheckSignServers[i]
			if(m_NtkoCheckSignServers[i].iSignServerId == pInfo[j].signServerId)
			{
				m_NtkoCheckSignServers[i].bIsUsedInThisDoc = TRUE;
				if(NULL == m_NtkoCheckSignServers[i].pwszPostPara)
				{
					m_NtkoCheckSignServers[i].pwszPostPara = NtkoCopyStringCatW(L"signguid=",pInfo[j].signUNID);
				}
				else
				{
					LPWSTR pwszTemp = NtkoCopyStringCatW(L"&signguid=",pInfo[j].signUNID);
					LPWSTR pwszTemp2 = NtkoCopyStringCatW(m_NtkoCheckSignServers[i].pwszPostPara,pwszTemp);
					SAFE_FREESTRING(pwszTemp);
					SAFE_FREESTRING(m_NtkoCheckSignServers[i].pwszPostPara);
					m_NtkoCheckSignServers[i].pwszPostPara = pwszTemp2;
				}
			}
		}
	}	
	
	m_serverReqestType = NTKOServerRequestType_CheckSign; //12检查印章
	
	isOk = TRUE;
	
	//向服务器提交请求
	ShowDelayDlgMUIStrW(IDSMUI_CHECKSIGNS_DOING_GETSIGNSINFO);
#if _MSC_VER >= 1500	// zhuguanglin add 20150313
	for(long i=0;(i<m_iCheckSignServersCount) && (i<NTKO_MAX_CHECKSIGN_SERVERS);i++)
#else
	for(i=0;(i<m_iCheckSignServersCount) && (i<NTKO_MAX_CHECKSIGN_SERVERS);i++)
#endif
	{				
		if(m_NtkoCheckSignServers[i].bIsUsedInThisDoc && m_NtkoCheckSignServers[i].pwszPostPara)
		{
			m_iCurServerId = m_NtkoCheckSignServers[i].iSignServerId; 			
			
			WCHAR wszSignServerNameInfo[MAX_PATH];
			LPWSTR pwszFmt = LoadMUIStringW(IDSMUI_CHECKSIGNS_GETSIGNSINFO_FROMSERVER);
			NtkoFwsprintf(wszSignServerNameInfo,pwszFmt,m_NtkoCheckSignServers[i].bstrSignServerName);
			SAFE_FREESTRING(pwszFmt);
			
			ShowDelayDlg(wszSignServerNameInfo);
			
			LPSTR pszTempServerURL = NtkoLPWSTRToANSI((LPWSTR)m_NtkoCheckSignServers[i].bstrServerAppURL);
			isOk = DoWebExecute2(pszTempServerURL,NTKO_HC_CHECKSIGNS_URL,m_NtkoCheckSignServers[i].pwszPostPara,&bstrRetXML);
			SAFE_FREESTRING(pszTempServerURL);
		}
		else
		{
			continue;
		}
		
		if(isOk && bstrRetXML)
		{			
			isOk = ParseXML(bstrRetXML);
			//MyMessageBoxW(hwndDlgParent, bstrRetXML,L"bstrRetXML" ,MB_OK|MB_ICONINFORMATION);
			if(isOk) //解析成功，但是服务器有可能有错误
			{ 
				if( 0 == m_iServerErrCode) 
				{
					if(NtkoAreStringsEqualW(m_bstrServerVersion,-1,NTKO_ServerSecsignControlProtocalVersion,-1))
					{
						isOk = TRUE;
					}
					else
					{
						isOk = FALSE;
					}
				}
				else //服务器返回有错误
				{
					isOk = FALSE;
				}
			}
			else //解析错误
			{
				isOk = FALSE;
			}
		}
		else
		{			
			isOk = FALSE;
		}
		if(isOk)
		{		
			isOk = ParseSignsCheckStatus();
		}
		SAFE_FREEBSTRING(bstrRetXML);		
	}
	CloseDelayDlg();
	if(outServersignCount)
	{
		(*outServersignCount) = m_iServerSignsCount;	
	}
	if(0 == m_iServerSignsCount) //没有印章，或失败
	{
		ShowMsgBoxMUIStrW(IDSMUI_CHECKSIGNS_FAIL_NOSIGNS_FROMSERVER,
			IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		return NULL;
	}
	else
	{
		return m_NtkoServerSigns;
	}
}

BOOL CNTKOServerSignHelper::ShowServerConfigDlg()
{
	int dlgRet = IDCANCEL;
	dlgRet = NtkoShowLargeOkCancelFrameDlg(m_hwndParent,CNTKOServerSignHelper::SysConfigSvrDlgHandler,this);

	if (IDOK == dlgRet)
	{	
		//保存系统设定
		WriteNtkoSecsignSettings(&m_secsignSettings);		
	}

	return TRUE;
}

STDMETHODIMP_(INT_PTR) CNTKOServerSignHelper::SysConfigSvrDlgHandler(HWND hDlg,HWND hwndTab,UINT Msg,WPARAM wParam,LPARAM lParam,LPVOID pUserParam)
{
	CNTKOServerSignHelper* pHelper = (CNTKOServerSignHelper*)pUserParam;
	
	switch (Msg) 
	{ 
	case WM_INITDIALOG:
		{
			if(pHelper)
			{
				LPWSTR pwszTitle = pHelper->LoadMUIStringW(IDSMUI_CFGDLG_SVRTITLE);	
				NtkoSetWindowTextW(hDlg,pwszTitle);
				SAFE_FREESTRING(pwszTitle);
				pwszTitle = pHelper->LoadMUIStringW(IDSMUI_CFGDLG_SYSCONF);
				NtkoAddDlgTabPage((HINSTANCE)g_hModule,hDlg,hwndTab,IDD_DLG_SYSCFG_SVR,
					(DLGPROC)SysConfigSvrDlgProc,pwszTitle,(LPARAM)pHelper);
				SAFE_FREESTRING(pwszTitle);
				
				NtkoShowDlgTabPage(hwndTab,0);
				
				return FALSE;
			}
			else
			{
				return TRUE;
			}
		}
		return TRUE;
	} 
	return FALSE;
}

STDMETHODIMP_(INT_PTR)  CNTKOServerSignHelper::SysConfigSvrDlgProc(HWND hDlg, UINT Msg, WPARAM wParam,LPARAM lParam)
{
#ifdef _WIN64	// zhuguanglin add 20150313
	CNTKOServerSignHelper* pHelper = (CNTKOServerSignHelper*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
#else
	CNTKOServerSignHelper* pHelper = (CNTKOServerSignHelper*)GetWindowLong(hDlg, GWL_USERDATA);
#endif
	HWND hDlgParent = GetParent(hDlg);
	
	switch (Msg) 
	{ 
	case WM_INITDIALOG:
		if (NULL != lParam)
		{
			pHelper = (CNTKOServerSignHelper*)lParam;
#ifdef _WIN64	// zhuguanglin add 20150313
			SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)pHelper);	
#else
			SetWindowLong(hDlg, GWL_USERDATA, (LONG)pHelper);	
#endif
		}
		if(pHelper) // get pen info
		{
			SetDlgItemText(hDlg,IDC_EDITSERVERNAME,pHelper->m_secsignSettings.serverURL);
			SendDlgItemMessage(hDlg,IDC_EDITSERVERNAME,EM_LIMITTEXT,MAX_PATH,0);
			SetWindowText(GetDlgItem(hDlg,IDC_STATICSERVERINFO),"");
			SendMessage(hDlg,WM_COMMAND,MAKEWPARAM(IDC_EDITSERVERNAME,EN_CHANGE),0);
			SetFocus(GetDlgItem(hDlg,IDC_EDITSERVERNAME));
			
			//初始化控件标题
			pHelper->SetMUIItemStrW(hDlg,IDC_STATIC_SVRCFG,IDCMUI_STATIC_SVRCFG);
			pHelper->SetMUIItemStrW(hDlg,IDC_STATICSERVERNAME,IDCMUI_STATICSERVERNAME);
			pHelper->SetMUIItemStrW(hDlg,IDC_STATICINFO_SVRCFG,IDCMUI_STATICINFO_SVRCFG);
			pHelper->SetMUIItemStrW(hDlg,IDC_BUTTON_TESTSERVER,IDCMUI_BUTTON_TESTSERVER);
			pHelper->SetMUIItemStrW(hDlg,IDC_CHECKUSEDEFPASS,IDCMUI_CHECKUSEDEFPASS);
			
			CheckDlgButton(hDlg,IDC_CHECKUSEDEFPASS,pHelper->m_secsignSettings.bUseDefaultsignPassowrd);
			SetDlgItemText(hDlg,IDC_EDITDEFPASS,pHelper->m_secsignSettings.defaultSignPassword);
			SendDlgItemMessage(hDlg,IDC_EDITDEFPASS,EM_LIMITTEXT,NTKO_MAX_PASSWORD,0);
			
			
			return FALSE;
		}
		else
		{
			return FALSE;
		}
		return TRUE;	
	case WM_CTLCOLORDLG:
		{
			if(pHelper)
			{
				HDC hdcControl = (HDC)wParam;
				SetBkMode(hdcControl,TRANSPARENT) ;
				return (int)GetStockObject(NTKO_TABEDDLG_BKCOLOR);
			}
		}
		break;
	case WM_CTLCOLORSTATIC:
		{
			int ictlID = GetDlgCtrlID((HWND)lParam);
			HDC hdc = (HDC)wParam;
			switch(ictlID)
			{
			case IDC_STATICSERVERINFO:
				{
					SetTextColor(hdc,NTKO_HILIGHTTIP_TEXTCOLOR);	
					SetBkMode(hdc, TRANSPARENT);
					return (int)GetSysColorBrush(COLOR_INFOBK);
				}
				break;
			case IDC_STATICINFO_SVRCFG:
				SetTextColor(hdc,NTKO_HILIGHTTIP_TEXTCOLOR);		
			default:
				SetBkMode(hdc, TRANSPARENT);
				return (int)GetStockObject(NTKO_TABEDDLG_BKCOLOR);
			}
		}
		break;
	case WM_COMMAND: 
		switch (LOWORD(wParam)) 
		{
		case IDOK:
			{
				BOOL isOk = FALSE;
				CHAR szTempSignServerName[MAX_PATH];
				ZeroMemory(&szTempSignServerName[0],sizeof(szTempSignServerName));
				szTempSignServerName[0] = '\0';
				GetDlgItemText(hDlg,IDC_EDITSERVERNAME,szTempSignServerName,MAX_PATH);	
				isOk = pHelper->CheckServerName(szTempSignServerName);
				if(!isOk)
				{
					pHelper->ShowMsgBoxMUIStrW(hDlg,IDSMUI_ERRORSIGNSERVERNAME,IDSMUI_MSGBOXTITLE_TIP,MB_OK | MB_ICONINFORMATION);
					SetFocus(GetDlgItem(hDlg,IDC_EDITSERVERNAME));
#ifdef _WIN64	// zhuguanglin add 20150312
					SetWindowLongPtr(hDlg,DWLP_MSGRESULT,TRUE);
#else
					SetWindowLong(hDlg,DWL_MSGRESULT,TRUE);
#endif
					return TRUE;
				}					
				else
				{
					int tlen = lstrlen(szTempSignServerName);
					if(szTempSignServerName[tlen-1] != '/')
					{
						szTempSignServerName[tlen] = '/'; //补充结尾的"/"
					}
					lstrcpyn(pHelper->m_secsignSettings.serverURL,szTempSignServerName,sizeof(pHelper->m_secsignSettings.serverURL));
					lstrcpyn(pHelper->m_rootSvrUrl,szTempSignServerName,sizeof(pHelper->m_rootSvrUrl)); //登录服务器默认和配置服务器相同
				}	
				pHelper->m_secsignSettings.bUseDefaultsignPassowrd = IsDlgButtonChecked(hDlg,IDC_CHECKUSEDEFPASS);
				ZeroMemory(pHelper->m_secsignSettings.defaultSignPassword,sizeof(pHelper->m_secsignSettings.defaultSignPassword));
				GetDlgItemText(hDlg,IDC_EDITDEFPASS,pHelper->m_secsignSettings.defaultSignPassword,NTKO_MAX_PASSWORD);
			}
			break;
		case IDC_BUTTON_TESTSERVER:
			{
				SetWindowText(GetDlgItem(hDlg,IDC_STATICSERVERINFO),"");
				BOOL isOk = FALSE;
				CHAR szTempSignServerNameBak[MAX_PATH];
				CHAR szTempSignServerName[MAX_PATH];
				ZeroMemory(&szTempSignServerName[0],sizeof(szTempSignServerName));
				ZeroMemory(&szTempSignServerNameBak[0],sizeof(szTempSignServerNameBak));
				szTempSignServerName[0] = '\0';
				GetDlgItemText(hDlg,IDC_EDITSERVERNAME,szTempSignServerName,MAX_PATH);	
				isOk = pHelper->CheckServerName(szTempSignServerName);
				if(isOk)
				{
					//备份之前的地址
					lstrcpyn(szTempSignServerNameBak,pHelper->m_secsignSettings.serverURL,sizeof(szTempSignServerNameBak));
					int tlen = lstrlen(szTempSignServerName);
					if(szTempSignServerName[tlen-1] != '/')
					{
						szTempSignServerName[tlen] = '/'; //补充结尾的"/"
					}
					
					//测试用户输入的地址
					lstrcpyn(pHelper->m_secsignSettings.serverURL,szTempSignServerName,sizeof(pHelper->m_secsignSettings.serverURL));
					isOk = pHelper->TestSignServer(hDlg,FALSE,TRUE,IDC_STATICSERVERINFO);
					//恢复之前的地址
					lstrcpyn(pHelper->m_secsignSettings.serverURL,szTempSignServerNameBak,sizeof(pHelper->m_secsignSettings.serverURL));
					
					if(isOk)
					{
						WCHAR info[2048];
						
						LPWSTR pwszFmt = pHelper->LoadMUIStringW(IDSMUI_TESTSERVER_INFO);
						NtkoFwsprintf(info,pwszFmt,pHelper->m_bstrServerInfoName,pHelper->m_bstrServerInfoVersion,
							pHelper->m_bstrServerInfoLicenseUserName,pHelper->m_bstrServerInfoLicenseCount);
						SAFE_FREESTRING(pwszFmt);
						
						NtkoSetWindowTextW(GetDlgItem(hDlg,IDC_STATICSERVERINFO),info);
					}
				}
				else
				{
					pHelper->ShowMsgBoxMUIStrW(hDlg,IDSMUI_ERRORSIGNSERVERNAME,IDSMUI_MSGBOXTITLE_TIP,MB_OK | MB_ICONINFORMATION);
					SetFocus(GetDlgItem(hDlg,IDC_EDITSERVERNAME));
				}
			}
			break;
		case IDC_EDITSERVERNAME:
			{
				if(EN_CHANGE == HIWORD(wParam))
				{
					BOOL isOk = FALSE;
					CHAR szTempSignServerName[MAX_PATH];
					ZeroMemory(&szTempSignServerName[0],sizeof(szTempSignServerName));
					szTempSignServerName[0] = '\0';
					GetDlgItemText(hDlg,IDC_EDITSERVERNAME,szTempSignServerName,MAX_PATH);	
					isOk = pHelper->CheckServerName(szTempSignServerName);
					EnableWindow(GetDlgItem(hDlgParent,IDOK),isOk);
					InvalidateRect(GetDlgItem(hDlgParent,IDOK),NULL,TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_TESTSERVER),isOk);
					break;
				}
			}
			break;
		} 
		break;
	} 
	return FALSE;
}

BOOL CNTKOServerSignHelper::CheckServerName(LPSTR szServerName)
{
	BOOL hasError = FALSE;
	int tlen = lstrlen(szServerName);
	if(tlen<13)
	{
		hasError = TRUE;
	}
	else
	{
		LPWSTR pwszSignServerName = NtkoANSIToLPWSTR(szServerName);
		if( (!NtkoStartWithW(pwszSignServerName,L"http://")) &&
			(!NtkoStartWithW(pwszSignServerName,L"https://"))
			)
		{
			hasError = TRUE;
		}
		SAFE_FREESTRING(pwszSignServerName);
	}
	return hasError?FALSE:TRUE;
}

BOOL CNTKOServerSignHelper::TestSignServer(HWND hwndParent,BOOL bShowPrompt,BOOL bSetDlgItemText,INT dlgItem)
{
	m_serverReqestType = NTKOServerRequestType_TestConnetServer; //10:测试服务器连接
	
	BOOL isOk = FALSE;
	BSTR bstrRetXML = NULL;
	HWND hwndDlgParent = hwndParent;
	
	LPWSTR pwszTip = LoadMUIStringW(IDSMUI_MSGBOXTITLE_TIP);
	
	ShowDelayDlgMUIStrW(IDSMUI_BEGIN_TESTSERVER);
	isOk = DoWebGet(m_secsignSettings.serverURL,NTKO_HC_GETSERVERINFO_URL,&bstrRetXML);	
	CloseDelayDlg();
	if(isOk && bstrRetXML)
	{
		isOk = ParseXML(bstrRetXML);
		if(isOk) //解析成功，但是服务器有可能有错误
		{ 
			if( 0 == m_iServerErrCode) 
			{
				if(NtkoAreStringsEqualW(m_bstrServerVersion,-1,NTKO_ServerSecsignControlProtocalVersion,-1))
				{
					isOk = TRUE;
				}
				else
				{
					LPWSTR pwszTitle = LoadMUIStringW(IDSMUI_SVRFAIL_INVALIDVER);			
					if(pwszTitle)
					{
						if(bShowPrompt)
						{
							NtkoShowMsgBox(hwndDlgParent,pwszTitle,pwszTip ,MB_OK|MB_ICONINFORMATION);
						}
						else if(bSetDlgItemText)
						{
							NtkoSetDlgItemTextW(hwndDlgParent,dlgItem,pwszTitle);
						}				
					}
					SAFE_FREESTRING(pwszTitle);
					isOk = FALSE;
				}
			}
			else //服务器返回有错误
			{
				LPWSTR pwszTitle = LoadMUIStringW(IDSMUI_SVRFAIL_WITHCODE);			
				if(pwszTitle)
				{
					WCHAR wszBuf[1024];
					NtkoFwsprintf(wszBuf,pwszTitle,m_iServerErrCode,m_bstrFailReason);
					if(bShowPrompt)
					{
						NtkoShowMsgBox(hwndDlgParent,pwszTitle,pwszTip ,MB_OK|MB_ICONINFORMATION);
					}
					else if(bSetDlgItemText)
					{
						NtkoSetDlgItemTextW(hwndDlgParent,dlgItem,pwszTitle);
					}				
				}
				SAFE_FREESTRING(pwszTitle);
				isOk = FALSE;
			}
		}
		else //解析错误
		{			
			LPWSTR pwszTitle = LoadMUIStringW(IDSMUI_SVRFAIL_PARSEERROR);			
			if(pwszTitle)
			{
				if(bShowPrompt)
				{
					NtkoShowMsgBox(hwndDlgParent,pwszTitle,pwszTip ,MB_OK|MB_ICONINFORMATION);
				}
				else if(bSetDlgItemText)
				{
					NtkoSetDlgItemTextW(hwndDlgParent,dlgItem,pwszTitle);
				}				
			}
			SAFE_FREESTRING(pwszTitle);
			isOk = FALSE;
		}
	}
	else
	{					
		LPWSTR pwszTitle = LoadMUIStringW(IDSMUI_SVRFAIL_CONNECT);			
		if(pwszTitle)
		{
			if(bShowPrompt)
			{
				NtkoShowMsgBox(hwndDlgParent,pwszTitle,pwszTip ,MB_OK|MB_ICONINFORMATION);
			}
			else if(bSetDlgItemText)
			{
				NtkoSetDlgItemTextW(hwndDlgParent,dlgItem,pwszTitle);
			}				
		}
		SAFE_FREESTRING(pwszTitle);
		isOk = FALSE;
	}
	if(isOk)
	{		
		isOk = ParseServerInfo();
		if(!isOk)
		{
			LPWSTR pwszTitle = LoadMUIStringW(IDSMUI_SVRFAIL_PARSEERROR);			
			if(pwszTitle)
			{
				if(bShowPrompt)
				{
					NtkoShowMsgBox(hwndDlgParent,pwszTitle,pwszTip ,MB_OK|MB_ICONINFORMATION);
				}
				else if(bSetDlgItemText)
				{
					NtkoSetDlgItemTextW(hwndDlgParent,dlgItem,pwszTitle);
				}				
			}
			SAFE_FREESTRING(pwszTitle);
		}	
	}
	SAFE_FREESTRING(pwszTip);
	SAFE_FREEBSTRING(bstrRetXML);
	return isOk;
}
// NtkoSignOpType_Add = 0,
// NtkoSignOpType_Lock,
// NtkoSignOpType_UnLock,
// NtkoSignOpType_Protect,
// NtkoSignOpType_Unprotect,
// NtkoSignOpType_EnableCheckDocChange,
// NtkoSignOpType_DisableCheckDocChange,
// NtkoSignOpType_Delete,
// NtkoSignOpType_Uncrypt

BOOL CNTKOServerSignHelper::DoServerSignLog(IDispatch* pDispSignObject,NtkoSignOpType signop,LPWSTR fileName,LPWSTR fileInfo)
{
	if(!pDispSignObject)
	{
		return FALSE;
	}
	m_serverReqestType = NTKOServerRequestType_DosignLog;// 7:签章审计日志记录

	HRESULT hr = S_OK;
	VARIANT vResult;
	BOOL isOk = FALSE;
	WCHAR wszCPARA[2048];
	BSTR bstrRetXML = NULL;
	WCHAR wszTemp[128];
	LPWSTR pwszTemp = NULL;

	long signType = 0;
	//构造参数
	ZeroMemory(&wszCPARA[0],sizeof(wszCPARA));

	//username==xxx&password=xxx
	lstrcatW(wszCPARA,L"username=");
	lstrcatW(wszCPARA,m_LoginUserName);

	lstrcatW(wszCPARA,L"&password=");
	lstrcatW(wszCPARA,m_LoginPassword);

	hr = GetProperty(pDispSignObject,OLESTR("SignName"),&vResult);
	if( FAILED(hr) || (!vResult.bstrVal) ) return FALSE;

	lstrcatW(wszCPARA,L"&signname=");
	lstrcatW(wszCPARA,vResult.bstrVal);
	VariantClear(&vResult);

	hr = GetProperty(pDispSignObject,OLESTR("SignUser"),&vResult);
	if( FAILED(hr) || (!vResult.bstrVal) ) return FALSE;
	lstrcatW(wszCPARA,L"&signuser=");
	lstrcatW(wszCPARA,vResult.bstrVal);
	VariantClear(&vResult);

	hr = GetProperty(pDispSignObject,OLESTR("SignSN"),&vResult);
	if( SUCCEEDED(hr) && (vResult.bstrVal) ) 
	{
		lstrcatW(wszCPARA,L"&signsn=");
		lstrcatW(wszCPARA,vResult.bstrVal);
	}
	VariantClear(&vResult);

	hr = GetProperty(pDispSignObject,OLESTR("SignGUID"),&vResult);
	if( FAILED(hr) || (!vResult.bstrVal) ) return FALSE;
	lstrcatW(wszCPARA,L"&signunid=");
	lstrcatW(wszCPARA,vResult.bstrVal);
	VariantClear(&vResult);

	hr = GetProperty(pDispSignObject,OLESTR("EkeySN"),&vResult);
	if( SUCCEEDED(hr) && (vResult.bstrVal) ) 
	{
		lstrcatW(wszCPARA,L"&ekeysn=");
		lstrcatW(wszCPARA,vResult.bstrVal);		
	}
	VariantClear(&vResult);


// 	lstrcatW(wszCPARA,L"&servertime=");
// 	lstrcatW(wszCPARA,m_bstrServerTime);
	hr = GetProperty(pDispSignObject,OLESTR("SignTime"),&vResult);
	if( SUCCEEDED(hr) && (vResult.bstrVal) ) 
	{
		lstrcatW(wszCPARA,L"&servertime=");
		lstrcatW(wszCPARA,vResult.bstrVal);		
	}
	VariantClear(&vResult);

	lstrcatW(wszCPARA,L"&appname=");

// 	NTKOAPPType_Other	= 0,	// "其它应用程序"
// 	NTKOAPPType_Word	= 1,	// "MS Word"
// 	NTKOAPPType_Excel	= 2,	// "MS Excel"
// 	NTKOAPPType_PDF		= 5,	// "NTKO Pdf"
// 	NTKOAPPType_WPS		= 6,	// "WPS 文字"
// 	NTKOAPPType_ETd		= 7		// "WPS 电子表格" 
	switch(m_appType)
	{
	case NTKOAPPType_Word:
		{
			lstrcatW(wszCPARA,L"MS Word");
		}
		break;
	case NTKOAPPType_Excel:
		{
			lstrcatW(wszCPARA,L"MS Excel");
		}
		break;
	case NTKOAPPType_PDF:
		{
			lstrcatW(wszCPARA,L"NTKO Pdf");
		}
		break;
	case NTKOAPPType_WPS:
		{
			lstrcatW(wszCPARA,L"WPS 文字");
		}
		break;
	case NTKOAPPType_ETd:
		{
			lstrcatW(wszCPARA,L"WPS 电子表格" );
		}
		break;
	case NTKOAPPType_Web:
		{
			lstrcatW(wszCPARA,L"HTML 印章" );
		}
		break;
	case NTKOAPPType_YoZo:
		{
			lstrcatW(wszCPARA,L"永中文字" );
		}
		break;
	case NTKOAPPType_Other:
	default:
		{
			lstrcatW(wszCPARA,L"");
		}
	}

	hr = GetProperty(pDispSignObject,OLESTR("CertIssuer"),&vResult);
	if( SUCCEEDED(hr) && (vResult.bstrVal) ) 
	{
		lstrcatW(wszCPARA,L"&cspreleasename=");
		lstrcatW(wszCPARA,vResult.bstrVal);		
	}
	VariantClear(&vResult);

	hr = GetProperty(pDispSignObject,OLESTR("CertUser"),&vResult);
	if( SUCCEEDED(hr) && (vResult.bstrVal) ) 
	{
		lstrcatW(wszCPARA,L"&cspusename=");
		lstrcatW(wszCPARA,vResult.bstrVal);		
	}
	VariantClear(&vResult);

	
	vResult.lVal = 0;
	hr = GetProperty(pDispSignObject,OLESTR("SignType"),&vResult);
	if( SUCCEEDED(hr)) 
	{		
		signType = vResult.lVal;
		ZeroMemory(wszTemp,sizeof(wszTemp));
		NtkoFwsprintf(wszTemp,L"&signtype=%d",vResult.lVal);
		lstrcatW(wszCPARA,wszTemp);	
	}
	VariantClear(&vResult);

	/*
	#define NTKO_HC_LOG_SIGNOP0			L"添加%ls"
	#define NTKO_HC_LOG_SIGNOP1			L"锁定%ls"
	#define NTKO_HC_LOG_SIGNOP2			L"解除%ls锁定"
	#define NTKO_HC_LOG_SIGNOP3			L"保护文档"
	#define NTKO_HC_LOG_SIGNOP4			L"解除文档保护"
	#define NTKO_HC_LOG_SIGNOP5			L"启用检查文档改变"
	#define NTKO_HC_LOG_SIGNOP6			L"停止检查文档改变"
	#define NTKO_HC_LOG_SIGNOP7			L"删除%ls"
	#define NTKO_HC_LOG_SIGNOP8			L"印章脱密"
	*/

	lstrcatW(wszCPARA,L"&signop=");	

	ZeroMemory(wszTemp,sizeof(wszTemp));
	LPWSTR pwszFmt = NULL;
	LPWSTR pwszInfo = NULL;

	switch(signop)
	{
	case 0:		
		pwszFmt = LoadMUIStringW(IDSMUI_HC_LOG_SIGNOP0);
		if(0 == signType)
		{			
			pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNTYPE0);
		}	
		else if(1 == signType)
		{
			pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNTYPE1);
		}
		else if(2 == signType)
		{
			pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNTYPE2);
		}
		else if(3 == signType)
		{
			pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNTYPE3);
		}
		else
		{
			pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNTYPEOTHER);
		}
		NtkoFwsprintf(wszTemp,pwszFmt,pwszInfo);
		SAFE_FREESTRING(pwszFmt);
		SAFE_FREESTRING(pwszInfo);
		lstrcatW(wszCPARA,wszTemp);	
		break;
	case 1:
		pwszFmt = LoadMUIStringW(IDSMUI_HC_LOG_SIGNOP1);
		if(0 == signType)
		{			
			pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNTYPE0);
		}	
		else if(1 == signType)
		{
			pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNTYPE1);
		}
		else if(2 == signType)
		{
			pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNTYPE2);
		}
		else if(3 == signType)
		{
			pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNTYPE3);
		}
		else
		{
			pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNTYPEOTHER);
		}
		NtkoFwsprintf(wszTemp,pwszFmt,pwszInfo);
		SAFE_FREESTRING(pwszFmt);
		SAFE_FREESTRING(pwszInfo);
		lstrcatW(wszCPARA,wszTemp);	
		break;
	case 2:
		pwszFmt = LoadMUIStringW(IDSMUI_HC_LOG_SIGNOP2);
		if(0 == signType)
		{			
			pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNTYPE0);
		}	
		else if(1 == signType)
		{
			pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNTYPE1);
		}
		else if(2 == signType)
		{
			pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNTYPE2);
		}
		else if(3 == signType)
		{
			pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNTYPE3);
		}
		else
		{
			pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNTYPEOTHER);
		}

		NtkoFwsprintf(wszTemp,pwszFmt,pwszInfo);
		SAFE_FREESTRING(pwszFmt);
		SAFE_FREESTRING(pwszInfo);
		lstrcatW(wszCPARA,wszTemp);
		break;
	case 3:
		pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNOP3);
		lstrcatW(wszCPARA,pwszInfo);	
		SAFE_FREESTRING(pwszInfo);
		break;
	case 4:
		pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNOP4);
		lstrcatW(wszCPARA,pwszInfo);	
		SAFE_FREESTRING(pwszInfo);
		break;
	case 5:
		pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNOP5);
		lstrcatW(wszCPARA,pwszInfo);	
		SAFE_FREESTRING(pwszInfo);
		break;
	case 6:
		pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNOP6);
		lstrcatW(wszCPARA,pwszInfo);	
		SAFE_FREESTRING(pwszInfo);
		break;
	case 7:
		pwszFmt = LoadMUIStringW(IDSMUI_HC_LOG_SIGNOP7);
		if(0 == signType)
		{			
			pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNTYPE0);
		}	
		else if(1 == signType)
		{
			pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNTYPE1);
		}
		else if(2 == signType)
		{
			pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNTYPE2);
		}
		else if(3 == signType)
		{
			pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNTYPE3);
		}
		else
		{
			pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNTYPEOTHER);
		}
		NtkoFwsprintf(wszTemp,pwszFmt,pwszInfo);
		SAFE_FREESTRING(pwszFmt);
		SAFE_FREESTRING(pwszInfo);
		lstrcatW(wszCPARA,wszTemp);	
		break;
	case 8:
		pwszInfo = LoadMUIStringW(IDSMUI_HC_LOG_SIGNOP8);
		lstrcatW(wszCPARA,pwszInfo);	
		SAFE_FREESTRING(pwszInfo);
		break;
	}

	lstrcatW(wszCPARA,L"&docfile=");
	lstrcatW(wszCPARA,fileName);		

	lstrcatW(wszCPARA,L"&docinfo=");
	lstrcatW(wszCPARA,fileInfo);		

	hr = GetProperty(pDispSignObject,OLESTR("SignPos"),&vResult);
	if( SUCCEEDED(hr) && (vResult.bstrVal) ) 
	{
		lstrcatW(wszCPARA,L"&signpos=");
		lstrcatW(wszCPARA,vResult.bstrVal);		
	}
	VariantClear(&vResult);


	
	//向服务器提交请求
	ShowDelayDlgMUIStrW(IDSMUI_DOINGLOGSIGN);
	isOk = DoWebExecute2(m_secsignSettings.serverURL,NTKO_HC_DOLOGSIGN_URL,wszCPARA,&bstrRetXML);		
	CloseDelayDlg();
	if(isOk && bstrRetXML)
	{			
		isOk = ParseXML(bstrRetXML);
		//MyMessageBoxW(hwndDlgParent, bstrRetXML,L"bstrRetXML" ,MB_OK|MB_ICONINFORMATION);
		if(isOk) //解析成功，但是服务器有可能有错误
		{ 
			if( 0 == m_iServerErrCode) 
			{
				if(NtkoAreStringsEqualW(m_bstrServerVersion,-1,NTKO_ServerSecsignControlProtocalVersion,-1))
				{
					isOk = TRUE;
				}
				else
				{
					ShowMsgBoxMUIStrW(IDSMUI_DOLOGSIGN_FAIL_INVALIDSERVER_VERSION,
						IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
					isOk = FALSE;
				}
			}
			else //服务器返回有错误
			{
				WCHAR wszBuf[1024];

				LPWSTR pwszFmt = LoadMUIStringW(IDSMUI_DOLOGSIGN_FAIL_SERVERFAILWITHCODE);
				NtkoFwsprintf(wszBuf,pwszFmt,m_iServerErrCode,m_bstrFailReason);
				SAFE_FREESTRING(pwszFmt);
				
				LPWSTR pwszMUITitle = LoadMUIStringW(IDSMUI_MSGBOXTITLE_TIP);
				NtkoShowMsgBox(m_hwndParent,wszBuf,pwszMUITitle ,MB_OK|MB_ICONINFORMATION);
				SAFE_FREESTRING(pwszMUITitle);

				if(1 == m_iServerErrCode)
				{
					m_IsLogin = FALSE;
				}
				isOk = FALSE;
			}
		}
		else //解析错误
		{
			ShowMsgBoxMUIStrW(IDSMUI_DOLOGSIGN_FAIL_PARSERESPONSEERROR,
				IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
			isOk = FALSE;
		}
	}
	else
	{			
// 		LPWSTR pwszMUITitle = LoadMUIStringW(IDSMUI_MSGBOXTITLE_TIP);
// 		NtkoMessageBoxW(hwndDlgParent,bstrRetXML,pwszMUITitle ,MB_OK|MB_ICONINFORMATION);
// 		SAFE_FREESTRING(pwszMUITitle);

		ShowMsgBoxMUIStrW(IDSMUI_DOLOGSIGN_FAIL_CONNECT,IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		isOk = FALSE;
	}

	SAFE_FREEBSTRING(bstrRetXML);
	return isOk;	
}

BOOL CNTKOServerSignHelper::DocheckSigns(CNTKODocSignsInfo * pNtkoDocSignsInfo, int len)
{
	if(0 == len || !pNtkoDocSignsInfo) //没有印章，或失败
	{
		ShowMsgBoxMUIStrW(IDSMUI_CHECKSIGNS_FAIL_NOSIGNS,IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		return FALSE;
	}

	m_iDocSignsCount = len;
	m_docSigns = pNtkoDocSignsInfo;
		
	HRESULT hr = S_OK;
	BOOL isOk = FALSE;
	BSTR bstrRetXML = NULL;

	//获取签章验证服务器信息	
	ClearCheckSignServers();
	m_serverReqestType = NTKOServerRequestType_GetServerChangeInfo;
	ShowDelayDlgMUIStrW(IDSMUI_GETTING_CHANGED_SERVERS);
	isOk = DoWebGet(m_rootSvrUrl,NTKO_HC_GETCHGEDSVRS_URL,&bstrRetXML);	
	CloseDelayDlg();
	if(isOk && bstrRetXML)
	{			
		isOk = ParseXML(bstrRetXML);
		//MyMessageBoxW(hwndDlgParent, bstrRetXML,L"bstrRetXML" ,MB_OK|MB_ICONINFORMATION);
		if(isOk) //解析成功，但是服务器有可能有错误
		{ 
			if( 0 == m_iServerErrCode) 
			{
				if(NtkoAreStringsEqualW(m_bstrServerVersion,-1,NTKO_ServerSecsignControlProtocalVersion,-1))
				{
					isOk = TRUE;
				}
				else
				{
					ShowMsgBoxMUIStrW(IDSMUI_CHECKSIGNS_FAIL_INVALIDSERVER_VERSION,
						IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
					isOk = FALSE;
				}
			}
			else //服务器返回有错误
			{
				WCHAR wszBuf[1024];

				LPWSTR pwszFmt = LoadMUIStringW(IDSMUI_CHECKSIGNS_FAIL_SERVERFAILWITHCODE);
				NtkoFwsprintf(wszBuf,pwszFmt,m_iServerErrCode,m_bstrFailReason);
				SAFE_FREESTRING(pwszFmt);

				LPWSTR pwszMUITitle = LoadMUIStringW(IDSMUI_MSGBOXTITLE_TIP);
				NtkoShowMsgBox(m_hwndParent,wszBuf,pwszMUITitle ,MB_OK|MB_ICONINFORMATION);
				SAFE_FREESTRING(pwszMUITitle);

				isOk = FALSE;
			}
		}
		else //解析错误
		{
			ShowMsgBoxMUIStrW(IDSMUI_CHECKSIGNS_FAIL_PARSERESPONSEERROR,
				IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
			isOk = FALSE;
		}
	}
	else
	{			
		ShowMsgBoxMUIStrW(IDSMUI_CHECKSIGNS_FAIL_CONNECT,IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		isOk = FALSE;
	}
	if(isOk)
	{		
		isOk = ParseCheckSignServers();
		if(!isOk)
		{
			ShowMsgBoxMUIStrW(IDSMUI_CHECKSIGNS_FAIL_PARSERESPONSEERROR,
				IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		}	
		else
		{
			if(0 == m_iCheckSignServersCount) //没有印章，或失败
			{
				ShowMsgBoxMUIStrW(IDSMUI_CHECKSIGNS_FAIL_NOCHECKSERVERS,
					IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
				isOk = FALSE;
			}			
		}
	}
	SAFE_FREEBSTRING(bstrRetXML);

	if(!isOk)
	{
		return FALSE;
	}

	ClearServerSigns();

	//构造参数
	//已经获得了签章验证服务器信息.看看需要向多少个服务器提交请求
	for(long i=0;(i<m_iCheckSignServersCount) && (i<NTKO_MAX_CHECKSIGN_SERVERS);i++)
	{
		BOOL isNeedGet = FALSE;
		for(long j=0;(j<m_iDocSignsCount) && (j<NTKO_MAX_DOC_SERVER_SIGNS);j++)
		{
			//m_NtkoCheckSignServers[i]
			if(m_NtkoCheckSignServers[i].iSignServerId == pNtkoDocSignsInfo[j].iSignServerId)
			{
				m_NtkoCheckSignServers[i].bIsUsedInThisDoc = TRUE;
				if(NULL == m_NtkoCheckSignServers[i].pwszPostPara)
				{
					m_NtkoCheckSignServers[i].pwszPostPara = NtkoCopyStringCatW(L"signguid=",pNtkoDocSignsInfo[j].bstrSignGUID);
				}
				else
				{
					LPWSTR pwszTemp = NtkoCopyStringCatW(L"&signguid=",pNtkoDocSignsInfo[j].bstrSignGUID);
					LPWSTR pwszTemp2 = NtkoCopyStringCatW(m_NtkoCheckSignServers[i].pwszPostPara,pwszTemp);
					SAFE_FREESTRING(pwszTemp);
					SAFE_FREESTRING(m_NtkoCheckSignServers[i].pwszPostPara);
					m_NtkoCheckSignServers[i].pwszPostPara = pwszTemp2;
				}
			}
		}
	}	

	m_serverReqestType = NTKOServerRequestType_CheckSign; //12检查印章

	isOk = TRUE;
	
	//向服务器提交请求
	ShowDelayDlgMUIStrW(IDSMUI_CHECKSIGNS_DOING_GETSIGNSINFO);
#if _MSC_VER >= 1500	// zhuguanglin add 20150313
	for(long i=0;(i<m_iCheckSignServersCount) && (i<NTKO_MAX_CHECKSIGN_SERVERS);i++)
#else
	for(i=0;(i<m_iCheckSignServersCount) && (i<NTKO_MAX_CHECKSIGN_SERVERS);i++)
#endif
	{				
		if(m_NtkoCheckSignServers[i].bIsUsedInThisDoc && m_NtkoCheckSignServers[i].pwszPostPara)
		{
			m_iCurServerId = m_NtkoCheckSignServers[i].iSignServerId; 			

			WCHAR wszSignServerNameInfo[MAX_PATH];
			LPWSTR pwszFmt = LoadMUIStringW(IDSMUI_CHECKSIGNS_GETSIGNSINFO_FROMSERVER);
			NtkoFwsprintf(wszSignServerNameInfo,pwszFmt,m_NtkoCheckSignServers[i].bstrSignServerName);
			SAFE_FREESTRING(pwszFmt);

			ShowDelayDlg(wszSignServerNameInfo);

			LPSTR pszTempServerURL = NtkoLPWSTRToANSI((LPWSTR)m_NtkoCheckSignServers[i].bstrServerAppURL);
			isOk = DoWebExecute2(pszTempServerURL,NTKO_HC_CHECKSIGNS_URL,m_NtkoCheckSignServers[i].pwszPostPara,&bstrRetXML);
			SAFE_FREESTRING(pszTempServerURL);
		}
		else
		{
			continue;
		}

		if(isOk && bstrRetXML)
		{			
			isOk = ParseXML(bstrRetXML);
			//MyMessageBoxW(hwndDlgParent, bstrRetXML,L"bstrRetXML" ,MB_OK|MB_ICONINFORMATION);
			if(isOk) //解析成功，但是服务器有可能有错误
			{ 
				if( 0 == m_iServerErrCode) 
				{
					if(NtkoAreStringsEqualW(m_bstrServerVersion,-1,NTKO_ServerSecsignControlProtocalVersion,-1))
					{
						isOk = TRUE;
					}
					else
					{
						isOk = FALSE;
					}
				}
				else //服务器返回有错误
				{
					isOk = FALSE;
				}
			}
			else //解析错误
			{
				isOk = FALSE;
			}
		}
		else
		{			
			isOk = FALSE;
		}
		if(isOk)
		{		
			isOk = ParseSignsCheckStatus();
		}
		SAFE_FREEBSTRING(bstrRetXML);		
	}
	CloseDelayDlg();

	if(0 == m_iServerSignsCount) //没有印章，或失败
	{
		ShowMsgBoxMUIStrW(IDSMUI_CHECKSIGNS_FAIL_NOSIGNS_FROMSERVER,
			IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
	}
	else
	{
		m_iSelectedSignIndex = -1;
		ShowCheckSignListDlg();
	}	
	return TRUE;
}

STDMETHODIMP CNTKOServerSignHelper::GetProperty(LPDISPATCH pDisp, LPOLESTR pszName, VARIANT* pvResult)
{
	if (NULL == pDisp) return E_POINTER;
	
	DISPID dwDispID;
	DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};
	
	//HRESULT hr = pDisp->GetIDsOfNames(IID_NULL, &pszName, 1, LOCALE_USER_DEFAULT, &dwDispID);
	HRESULT hr = NtkoGetIDsOfDispatchNames(pDisp, &pszName, &dwDispID);
	if(SUCCEEDED(hr)) {
		hr = pDisp->Invoke(dwDispID, IID_NULL, LOCALE_USER_DEFAULT,
			DISPATCH_METHOD | DISPATCH_PROPERTYGET, &dispparamsNoArgs,
			pvResult, NULL, NULL);
	}
	return hr;
}

BOOL CNTKOServerSignHelper::SetAppType(NTKOAPPType appType)
{
	m_appType = appType;
	return TRUE;
}

BOOL CNTKOServerSignHelper::SetSvrUrl(LPSTR url)
{
	int len = lstrlenA(url);
	if (0==len || 0>=MAX_PATH )
	{
		return FALSE;
	}

	ZeroMemory(m_secsignSettings.serverURL,MAX_PATH);
	CopyMemory(m_secsignSettings.serverURL, url ,len);

	ZeroMemory(m_rootSvrUrl, MAX_PATH);
	CopyMemory(m_rootSvrUrl, url, len);

	WriteRegisterInfo(HKEY_CURRENT_USER, "CLSID\\"NTKOSecsignConfigHk, NTKO_HC_SETTING_SecsignServerName,
		m_secsignSettings.serverURL);

	return TRUE;
}

void CNTKOServerSignHelper::ShowCheckSignListDlg()
{
	HWND hwndDlgParent = m_hwndParent;
	int dlgRet = IDCANCEL;
	HWND hwndDlg = NULL;
	
	m_hwndCheckSignDlg = NtkoMUICreateDialogParamW((HINSTANCE)g_hModule,IDD_DIALOGCHECKSIGNINDEX,
		hwndDlgParent, CNTKOServerSignHelper::CheckSignDlgWindowProc,(long)this); 
	if(m_hwndCheckSignDlg)
	{
		ShowWindow(m_hwndCheckSignDlg, SW_SHOW); 
		BringWindowToTop(m_hwndCheckSignDlg);
	}
}

STDMETHODIMP_(INT_PTR) CNTKOServerSignHelper::CheckSignDlgWindowProc(HWND hDlg, UINT Msg, WPARAM wParam,LPARAM lParam)
{
#ifdef _WIN64	// zhuguanglin add 20150313
	CNTKOServerSignHelper* pThis = (CNTKOServerSignHelper*)GetWindowLongPtr(hDlg, GWLP_USERDATA);	
#else
	CNTKOServerSignHelper* pThis = (CNTKOServerSignHelper*)GetWindowLong(hDlg, GWL_USERDATA);			
#endif
	switch (Msg) 
	{ 
		case WM_INITDIALOG:	
#ifdef _WIN64	// zhuguanglin add 20150313
			SetClassLongPtr(hDlg,GCLP_HICON,(LONG_PTR)g_signListDlgICON);
#else
			SetClassLong(hDlg,GCL_HICON,(LONG)g_signListDlgICON);
#endif
			if (NULL != lParam)
			{
				pThis = (CNTKOServerSignHelper*)lParam;
#ifdef _WIN64	// zhuguanglin add 20150313
				SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)pThis);	
#else
				SetWindowLong(hDlg, GWL_USERDATA, (LONG)pThis);	
#endif
				EnableWindow(GetDlgItem(hDlg,IDOK),FALSE);
				if(pThis)
				{

					PCNtkoCusDraw pNtkoCusDraw = CNtkoCusDraw::ConvertDlgToCusdraw(hDlg);
					if(pNtkoCusDraw)
					{
						pNtkoCusDraw->ApplyTheme(NTKO_DEFAULT_THEME);
					}

					pNtkoCusDraw = CNtkoCusDraw::ConvertDlgItemToCusdraw(hDlg,IDOK);
					if(pNtkoCusDraw)
					{
						pNtkoCusDraw->ApplyTheme(gNtkoLib_ThemeType);
					}
					pNtkoCusDraw = CNtkoCusDraw::ConvertDlgItemToCusdraw(hDlg,IDCANCEL);
					if(pNtkoCusDraw)
					{
						pNtkoCusDraw->ApplyTheme(gNtkoLib_ThemeType);
					}

					LPWSTR pwszTitle = pThis->LoadMUIStringW(IDSMUI_CHECKSIGN_DLG_TITLE);	
					NtkoSetWindowTextW(hDlg,pwszTitle);
					SAFE_FREESTRING(pwszTitle);

					//初始化控件标题
					pThis->SetMUIDlgItemStringW(hDlg,IDC_STATICDOCSIGNS_LIST,IDCMUI_STATICDOCSIGNS_LIST);
					pThis->SetMUIDlgItemStringW(hDlg,IDC_STATIC_SVRCHKINFO,IDCMUI_STATIC_SVRCHKINFO);
					pThis->SetMUIDlgItemStringW(hDlg,IDOK,IDCMUI_BUTTON_FINDPOS);
					pThis->SetMUIDlgItemStringW(hDlg,IDCANCEL,IDCMUI_IDCLOSE);

					HWND hWndListView = GetDlgItem(hDlg,IDC_LISTSIGNS);

					ListView_SetExtendedListViewStyleEx(hWndListView,
						LVS_EX_FULLROWSELECT|LVS_EX_REGIONAL,
						LVS_EX_FULLROWSELECT|LVS_EX_REGIONAL);

					 // Assign the image lists to the list-view control. 
					pThis->SetDlgListImageList(hWndListView,1);


					if(pThis->InitCheckSignListViewColumns(hWndListView))
					{
						pThis->FillCheckSignListWithSignsInfo(hWndListView,TRUE);				
					}


					ListView_SetSelectionMark(hWndListView,0);
					ListView_SetItemState(hWndListView, 0, LVIS_FOCUSED| LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);

					SetFocus(hWndListView);

					NMITEMACTIVATE niv;
					niv.hdr.code = NM_CLICK;
					niv.iItem = 0;

					pThis->m_hwndCheckSignDlg = hDlg;
					SendMessage(hDlg,WM_NOTIFY,IDC_LISTSIGNS,(LPARAM)&niv);					

					return FALSE; // we have set focus,so return false
				}
				else
				{
					//MessageBox(hDlg,"no pCtl","tip",MB_OK);
				}
			}
			else
			{
				//MessageBox(hDlg,"no win","tip",MB_OK);
			}
			return TRUE;
		case WM_NOTIFY:
			if(IDC_LISTSIGNS == (int)wParam)
			{
				LPNMITEMACTIVATE lpnmitem;
				lpnmitem = (LPNMITEMACTIVATE) lParam;
				UINT  iNotifyCode = lpnmitem->hdr.code;
				switch(iNotifyCode)
				{
				case NM_CLICK:
				case NM_DBLCLK:
					{
						BOOL isOk = !(BOOL)(lpnmitem->iItem<0);
						EnableWindow(GetDlgItem(hDlg,IDOK),isOk);
						InvalidateRect(GetDlgItem(hDlg,IDOK),NULL,TRUE);
						if(pThis && isOk)
						{
							HWND hWndListView = GetDlgItem(hDlg,IDC_LISTSIGNS);
							pThis->m_iSelectedSignIndex = NtkoGetListViewItemParamW(hWndListView,lpnmitem->iItem);
							if(pThis->m_iSelectedSignIndex >=0 )
							{
								pThis->ShowSelectedDocSignInfo(TRUE);
							}
						}
					
						if( (NM_DBLCLK == iNotifyCode) && isOk )
						{
							SendMessage(hDlg,WM_COMMAND,(WPARAM)IDOK,0);
						}
					}
					break;
				}
			}
			break;
		case WM_DESTROY:
			{				
				if(pThis)pThis->DestroyDlgListImageList();
#ifdef _WIN64	// zhuguanglin add 20150313
				SetClassLongPtr(hDlg,GCLP_HICON,NULL);	
#else
				SetClassLong(hDlg,GCL_HICON,NULL);	
#endif
				pThis->m_hwndCheckSignDlg = NULL;
				return 0;
			}
		case WM_COMMAND: 
			switch (LOWORD(wParam)) 
			{ 
			case IDOK: 	
				{
					int i = (pThis)?pThis->m_iSelectedSignIndex:-1;
					if(i<0)
					{
						pThis->ShowMsgBoxMUIStrW(hDlg,IDSMUI_SIGNSLISTDLG_PLZSELECTSIGN,IDSMUI_MSGBOXTITLE_TIP,MB_OK | MB_ICONINFORMATION);
						break;
					}				
					else
					{
						//查找选择的印章并定位
						if(pThis->m_iSelectedSignIndex >=0 )
						{
							pThis->GotoSelectedDocSign();
						}
					}
				}
				break;
			case IDCANCEL:
				pThis->m_iSelectedSignIndex = -1;
				DestroyWindow(hDlg);//非模式对话框,需要这个
				pThis->m_hwndCheckSignDlg = NULL;
				break;
			} 
			break;
	} 
	return FALSE;
}

void CNTKOServerSignHelper::SetMUIDlgItemStringW(HWND hDlg,UINT idItem,DWORD dwKey)
{
	LPWSTR pwszText = LoadMUIStringW(dwKey);
	if(pwszText)
	{
		NtkoSetDlgItemTextW(hDlg,idItem,pwszText);
		SAFE_FREESTRING(pwszText);
	}
	else
	{
		NtkoSetDlgItemTextW(hDlg,idItem,NTKO_DEFAULT_NOFOUND_TEXT);
	}
}

BOOL CNTKOServerSignHelper::InitCheckSignListViewColumns(HWND hWndListView)
{
	LPWSTR pwszText = NULL;
	int colWidth = 100;
	
	pwszText = LoadMUIStringW(IDSMUI_LVSELSIGNSFIRSTCOLUMN0);
	NtkoInsertListViewColumnW(hWndListView,LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM,
		0,LVCFMT_LEFT, pwszText,40);
	SAFE_FREESTRING(pwszText);
	
	pwszText = LoadMUIStringW(IDSMUI_DOCSIGNSFIRSTCOLUMN2);
	NtkoInsertListViewColumnW(hWndListView,LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM,
		1,LVCFMT_LEFT, pwszText,colWidth);
	SAFE_FREESTRING(pwszText);
	
	pwszText = LoadMUIStringW(IDSMUI_DOCSIGNSFIRSTCOLUMN1);
	NtkoInsertListViewColumnW(hWndListView,LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM,
		2,LVCFMT_LEFT, pwszText,colWidth);
	SAFE_FREESTRING(pwszText);
	
	return TRUE;
}

BOOL CNTKOServerSignHelper::FillCheckSignListWithSignsInfo(HWND hWndListView,BOOL bIsForServerSigns)
{
	ListView_DeleteAllItems(hWndListView);
	
	//for(long ic=0;(ic<m_iServerSignsCount)&& (ic<NTKO_MAX_DOC_SERVER_SIGNS);ic++)
	for(long ic=0;(ic<m_iDocSignsCount)&& (ic<NTKO_MAX_DOC_SERVER_SIGNS);ic++)
	{
		WCHAR wszCol1[255];
		NtkoFwsprintf(wszCol1,L"%d",ic+1);
		
		if( (!bIsForServerSigns) || m_docSigns[ic].bstrSignGUID)
		{
			int i = 0;
			if(m_docSigns[ic].bIsSignChecked)
			{
				i = NtkoInsertListViewItemW(hWndListView,LVIF_TEXT | LVIF_IMAGE|LVIF_PARAM, ic,wszCol1,0,ic);
			}
			else
			{
				i = NtkoInsertListViewItemW(hWndListView,LVIF_TEXT | LVIF_IMAGE|LVIF_PARAM, ic,wszCol1,1,ic);
			}
			NtkoSetListViewItemTextW(hWndListView,i,1,m_docSigns[ic].bstrSignName);
			NtkoSetListViewItemTextW(hWndListView,i,2,m_docSigns[ic].bstrSigner);
		}
	}	
	AutoAdjustListViewColumn(hWndListView);
	return TRUE;
}

VOID CNTKOServerSignHelper::ShowSelectedDocSignInfo(BOOL bIsForServerSigns)
{
	if(m_iSelectedSignIndex<0)
	{
		return;
	}
	
	if(!m_hwndCheckSignDlg)
	{
		return;
	}
	
	HWND hWndTemp = GetDlgItem(m_hwndCheckSignDlg,IDC_STATICTITLE);	
	if(hWndTemp)
	{
		NtkoSetWindowTextW(hWndTemp,m_docSigns[m_iSelectedSignIndex].bstrSignCheckInfoTitle);
	}
	hWndTemp = GetDlgItem(m_hwndCheckSignDlg,IDC_STATICINFODOC);	
	if(hWndTemp)
	{
		NtkoSetWindowTextW(hWndTemp,m_docSigns[m_iSelectedSignIndex].bstrSignCheckInfoText);
	}
	
	if(m_docSigns[m_iSelectedSignIndex].bIsSignChecked)
	{
		SendDlgItemMessage(m_hwndCheckSignDlg,IDC_STATICBMP,STM_SETIMAGE,(WPARAM)IMAGE_BITMAP,(LPARAM)m_hbmpOk);
	}
	else
	{
		SendDlgItemMessage(m_hwndCheckSignDlg,IDC_STATICBMP,STM_SETIMAGE,(WPARAM)IMAGE_BITMAP,(LPARAM)m_hbmpError);
	}
	
	if(!bIsForServerSigns)
	{
		return;
	}
	
	long foundServerSignIndex = -1;
	long foundCheckServerIndex = -1;
	//在服务器印章中查找
	for(long i=0;(i<m_iServerSignsCount)&& (i<NTKO_MAX_DOC_SERVER_SIGNS);i++)
	{
		if( (m_NtkoServerSigns[i].bFound )&& 
			(NtkoAreStringsEqualW(m_NtkoServerSigns[i].bstrSignGUID,-1,m_docSigns[m_iSelectedSignIndex].bstrSignGUID,-1))
			)
		{
			foundServerSignIndex = i;
			break;
		}
	}
	
	//构造服务器印章信息字符串
	WCHAR	wszServerInfo[4096];
	ZeroMemory(&wszServerInfo[0],sizeof(wszServerInfo));
	
	if(foundServerSignIndex >=0 ) //found
	{
		for(long i=0;(i<m_iCheckSignServersCount) && (i<NTKO_MAX_CHECKSIGN_SERVERS);i++)
		{
			if(m_NtkoCheckSignServers[i].iSignServerId == m_NtkoServerSigns[foundServerSignIndex].iSignServerId)
			{
				foundCheckServerIndex = i;
				break;
			}
		}
	}
	
	if(foundCheckServerIndex >=0 ) // all found
	{
		if(m_NtkoServerSigns[foundServerSignIndex].bstrSignSN && ::SysStringByteLen(m_NtkoServerSigns[foundServerSignIndex].bstrSignSN)
			&& (!(m_dwHiddenCheckItems & NTKO_HIDDENCHECK_SIGNSN)))
		{
			LPWSTR pwszMUITmp = LoadMUIStringW(IDSMUI_SIGNINFO_SIGNSN_W);			
			NtkoStringCatW(wszServerInfo,pwszMUITmp,sizeof(wszServerInfo));
			SAFE_FREESTRING(pwszMUITmp);
			NtkoStringCatW(wszServerInfo,m_NtkoServerSigns[foundServerSignIndex].bstrSignSN,sizeof(wszServerInfo));
			NtkoStringCatW(wszServerInfo,L"\n",sizeof(wszServerInfo));
		}
		if(m_NtkoServerSigns[foundServerSignIndex].bstrEkeySN && ::SysStringByteLen(m_NtkoServerSigns[foundServerSignIndex].bstrEkeySN)
			&& (!(m_dwHiddenCheckItems & NTKO_HIDDENCHECK_EKEYSN)))
		{
			LPWSTR pwszMUITmp = LoadMUIStringW(IDSMUI_SIGNINFO_EKEYSN_W);
			NtkoStringCatW(wszServerInfo,pwszMUITmp,sizeof(wszServerInfo));
			SAFE_FREESTRING(pwszMUITmp);
			NtkoStringCatW(wszServerInfo,m_NtkoServerSigns[foundServerSignIndex].bstrEkeySN,sizeof(wszServerInfo));
			NtkoStringCatW(wszServerInfo,L"\n",sizeof(wszServerInfo));
		}		
		if(m_NtkoServerSigns[foundServerSignIndex].bstrSignGUID && (!(m_dwHiddenCheckItems & NTKO_HIDDENCHECK_SIGNGUID)) )
		{
			LPWSTR pwszMUITmp = LoadMUIStringW(IDSMUI_SIGNINFO_SIGNGUID_W);
			NtkoStringCatW(wszServerInfo,pwszMUITmp,sizeof(wszServerInfo));
			SAFE_FREESTRING(pwszMUITmp);
			NtkoStringCatW(wszServerInfo,m_NtkoServerSigns[foundServerSignIndex].bstrSignGUID,sizeof(wszServerInfo));
			NtkoStringCatW(wszServerInfo,L"\n",sizeof(wszServerInfo));
		}
		///////********  这儿有点不同，查询服务器列表获得服务器名称 *****************
		if(m_NtkoCheckSignServers[foundCheckServerIndex].bstrSignServerName && (!(m_dwHiddenCheckItems & NTKO_HIDDENCHECK_SIGNSERVERNAME)))
		{
			LPWSTR pwszMUITmp = LoadMUIStringW(IDSMUI_SIGNINFO_SIGNSERVERNAME_W);
			NtkoStringCatW(wszServerInfo,pwszMUITmp,sizeof(wszServerInfo));
			SAFE_FREESTRING(pwszMUITmp);
			NtkoStringCatW(wszServerInfo,m_NtkoCheckSignServers[foundCheckServerIndex].bstrSignServerName,sizeof(wszServerInfo));
			NtkoStringCatW(wszServerInfo,L"\n",sizeof(wszServerInfo));
		}
		if(m_NtkoServerSigns[foundServerSignIndex].bstrSigner && (!(m_dwHiddenCheckItems & NTKO_HIDDENCHECK_SIGNER)) )
		{
			LPWSTR pwszMUITmp = LoadMUIStringW(IDSMUI_SIGNINFO_USERNAME_W);
			NtkoStringCatW(wszServerInfo,pwszMUITmp,sizeof(wszServerInfo));
			SAFE_FREESTRING(pwszMUITmp);
			NtkoStringCatW(wszServerInfo,m_NtkoServerSigns[foundServerSignIndex].bstrSigner,sizeof(wszServerInfo));
			NtkoStringCatW(wszServerInfo,L"\n",sizeof(wszServerInfo));
		}
		if(m_NtkoServerSigns[foundServerSignIndex].bstrSignName && (!(m_dwHiddenCheckItems & NTKO_HIDDENCHECK_SIGNNAME)) )
		{
			LPWSTR pwszMUITmp = LoadMUIStringW(IDSMUI_SIGNINFO_SIGNNAME_W);
			NtkoStringCatW(wszServerInfo,pwszMUITmp,sizeof(wszServerInfo));
			SAFE_FREESTRING(pwszMUITmp);
			NtkoStringCatW(wszServerInfo,m_NtkoServerSigns[foundServerSignIndex].bstrSignName,sizeof(wszServerInfo));
			NtkoStringCatW(wszServerInfo,L"\n",sizeof(wszServerInfo));
		}
		if(m_NtkoServerSigns[foundServerSignIndex].bstrSignUser  && (!(m_dwHiddenCheckItems & NTKO_HIDDENCHECK_SIGNUSER)) )
		{
			LPWSTR pwszMUITmp = LoadMUIStringW(IDSMUI_SIGNINFO_SIGNUSER_W);
			NtkoStringCatW(wszServerInfo,pwszMUITmp,sizeof(wszServerInfo));
			SAFE_FREESTRING(pwszMUITmp);
			NtkoStringCatW(wszServerInfo,m_NtkoServerSigns[foundServerSignIndex].bstrSignUser,sizeof(wszServerInfo));
			NtkoStringCatW(wszServerInfo,L"\n",sizeof(wszServerInfo));
		}		
		if(m_NtkoServerSigns[foundServerSignIndex].bstrServerTime  && (!(m_dwHiddenCheckItems & NTKO_HIDDENCHECK_SERVERTIME)) )
		{
			LPWSTR pwszMUITmp = LoadMUIStringW(IDSMUI_SIGNINFO_TIMESTAMP_W);
			NtkoStringCatW(wszServerInfo,pwszMUITmp,sizeof(wszServerInfo));
			SAFE_FREESTRING(pwszMUITmp);
			NtkoStringCatW(wszServerInfo,m_NtkoServerSigns[foundServerSignIndex].bstrServerTime,sizeof(wszServerInfo));
			NtkoStringCatW(wszServerInfo,L"\n",sizeof(wszServerInfo));
		}	
		if(m_NtkoServerSigns[foundServerSignIndex].bstrAppName && (!(m_dwHiddenCheckItems & NTKO_HIDDENCHECK_APPNAME)) )
		{
			LPWSTR pwszMUITmp = LoadMUIStringW(IDSMUI_SIGNINFO_APPNAME_W);
			NtkoStringCatW(wszServerInfo,pwszMUITmp,sizeof(wszServerInfo));
			SAFE_FREESTRING(pwszMUITmp);
			NtkoStringCatW(wszServerInfo,m_NtkoServerSigns[foundServerSignIndex].bstrAppName,sizeof(wszServerInfo));
			NtkoStringCatW(wszServerInfo,L"\n",sizeof(wszServerInfo));
		}		
		if(m_NtkoServerSigns[foundServerSignIndex].bstrClientIP && (!(m_dwHiddenCheckItems & NTKO_HIDDENCHECK_CLIENTIP)) )
		{
			LPWSTR pwszMUITmp = LoadMUIStringW(IDSMUI_SIGNINFO_CLIENTIP_W);
			NtkoStringCatW(wszServerInfo,pwszMUITmp,sizeof(wszServerInfo));
			SAFE_FREESTRING(pwszMUITmp);
			NtkoStringCatW(wszServerInfo,m_NtkoServerSigns[foundServerSignIndex].bstrClientIP,sizeof(wszServerInfo));
			//CopyStringCat2(wszServerInfo,L"\n");
		}	
		
		SendDlgItemMessage(m_hwndCheckSignDlg,IDC_STATICSVRBMP,STM_SETIMAGE,(WPARAM)IMAGE_BITMAP,(LPARAM)m_hbmpOk);
		hWndTemp = GetDlgItem(m_hwndCheckSignDlg,IDC_STATICTITLESERVER);	
		if(hWndTemp)
		{
			LPWSTR pwszMUITmp = LoadMUIStringW(IDSMUI_CHECKSIGNS_SERVER_OK);
			NtkoSetWindowTextW(hWndTemp,pwszMUITmp);
			SAFE_FREESTRING(pwszMUITmp);
		}
	}
	else
	{
		LPWSTR pwszMUITmp = LoadMUIStringW(IDSMUI_CHECKSIGNS_NOSIGN_ON_SERVER);
		NtkoStringCatW(wszServerInfo,pwszMUITmp,sizeof(wszServerInfo));
		SAFE_FREESTRING(pwszMUITmp);
		
		SendDlgItemMessage(m_hwndCheckSignDlg,IDC_STATICSVRBMP,STM_SETIMAGE,(WPARAM)IMAGE_BITMAP,(LPARAM)m_hbmpError);
		hWndTemp = GetDlgItem(m_hwndCheckSignDlg,IDC_STATICTITLESERVER);	
		if(hWndTemp)
		{
			LPWSTR pwszMUITmp = LoadMUIStringW(IDSMUI_CHECKSIGNS_SERVER_FAIL);
			NtkoSetWindowTextW(hWndTemp,pwszMUITmp);
			SAFE_FREESTRING(pwszMUITmp);
		}
	}
	
	hWndTemp = GetDlgItem(m_hwndCheckSignDlg,IDC_STATICINFOSERVER);	
	if(hWndTemp)
	{
		NtkoSetWindowTextW(hWndTemp,wszServerInfo);
	}
}

BOOL CNTKOServerSignHelper::GetCertificateLogonInfo(HWND hwndDlgParent,LPWSTR* ppwszCertName,LPWSTR* ppwszCertSN)
{
	HCRYPTPROV		hSignCryptProv = NULL; // sign provider
	HCERTSTORE       hCertStore = NULL;  //store 
	PCCERT_CONTEXT   pCurSignCert = NULL; //用户选择的证书

	DWORD dwKeySpec = 0;
	DWORD dwLastErrorCode = 0;
	BOOL retValue = FALSE;
	BOOL isNeedFreeHProv = FALSE;
	LPWSTR pwszCertsSubject = NULL;
	LPWSTR pwszCertsSN = NULL;

	if ( !(hCertStore = CertOpenSystemStore( NULL, "MY")))	
	{
		return FALSE;
	}

	if(!NtkoSelectCertOptionalDlg(hwndDlgParent,hCertStore,FALSE,NULL,TRUE,TRUE,1,&pCurSignCert,NULL))
	{
		goto exit0;
	}
	if(!pCurSignCert)
	{
		goto exit0;
	}

	hSignCryptProv = NtkoGetSignCryptProv(pCurSignCert);
	if(!hSignCryptProv)
	{		 
		 dwLastErrorCode = GetLastError();
		 TRACE1_NTKOLIB("Error using CryptAcquireCertificatePrivateKey.error=%d\n",dwLastErrorCode);
// 		 WCHAR errMes[100];
// 		 NtkoFwsprintf(errMes,L"CryptAcquireCertificatePrivateKey error 0x%lx",dwLastErrorCode);
// 		 HWND hDlgParentWnd = m_pCtl?(m_pCtl->get_Wnd()):NULL;
// 		 m_pCtl->MyMessageBoxUseMUIStrW(hDlgParentWnd,errMes,L"error",MB_OK | MB_ICONINFORMATION);

		ShowMsgBoxMUIStrW(hwndDlgParent,IDSMUI_LOGINSERVER_FAIL_INVALIDCERT,
						IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		 goto exit0;
	}

	if(ppwszCertName)
	{
		*ppwszCertName = NtkoCertGetNameStringW(pCurSignCert,CERT_NAME_SIMPLE_DISPLAY_TYPE,0);
	}
	if(ppwszCertSN)
	{
		*ppwszCertSN = NtkoCertGetSNStringW(pCurSignCert,NULL);
	}

	retValue = TRUE;
	
exit0:

	if(pCurSignCert)
	{
		CertFreeCertificateContext(pCurSignCert);
		pCurSignCert = NULL;
	}
	if(hCertStore)
	{
		CertCloseStore(hCertStore, CERT_CLOSE_STORE_CHECK_FLAG);
		hCertStore = NULL;
	}
	if(hSignCryptProv)
	{
		CryptReleaseContext(hSignCryptProv, 0);	
		hSignCryptProv = NULL;
	}
	return retValue;
}

void CNTKOServerSignHelper::NTKO_Custom_ClearCompanyInfo()
{
#if !defined(NTKO_CUSTOM_JIANGSU_CA)
	return;
#endif

	SAFE_FREESTRING(m_Custom_jiangshuCA_CompanyInfo.companyCode);
	SAFE_FREESTRING(m_Custom_jiangshuCA_CompanyInfo.companyId);
	SAFE_FREESTRING(m_Custom_jiangshuCA_CompanyInfo.companyName);
}

void CNTKOServerSignHelper::NTKO_Custom_IniteCompanyInfo()
{
#if !defined(NTKO_CUSTOM_JIANGSU_CA)
	return;
#endif
	m_Custom_jiangshuCA_CompanyInfo.companyId = NULL;
	m_Custom_jiangshuCA_CompanyInfo.companyCode = NULL;
	m_Custom_jiangshuCA_CompanyInfo.companyName = NULL;
}

BOOL CNTKOServerSignHelper::NTKO_Custom_CreateKey(LPWSTR * md5, LPWSTR * time)
{
#if !defined(NTKO_CUSTOM_JIANGSU_CA)
	return FALSE;
#endif

	if (!md5 || !time)
	{
		return FALSE;
	}
	
	*time = NULL;
	*md5 = NULL;

	CONST INT NTKO_TOTALKEY_LEN = 20;
	CONST INT NTKO_TIME_STR_LEN = 14;
	CONST INT NTKO_POST_FIX_LEN = 5;

	SYSTEMTIME localTime;
	GetLocalTime(&localTime);
	char TIME_STRING[NTKO_TOTALKEY_LEN] = {0};

	sprintf(TIME_STRING,"%04d%02d%02d%02d%02d%02d",localTime.wYear, localTime.wMonth,localTime.wDay,
		localTime.wHour,localTime.wMinute,localTime.wSecond);

	// jsgds
	TIME_STRING[NTKO_TIME_STR_LEN + 0] = 'j';
	TIME_STRING[NTKO_TIME_STR_LEN + 1] = 's';
	TIME_STRING[NTKO_TIME_STR_LEN + 2] = 'g';
	TIME_STRING[NTKO_TIME_STR_LEN + 3] = 'd';
	TIME_STRING[NTKO_TIME_STR_LEN + 4] = 's';
	
	MD5 classMD5;
	classMD5.update(TIME_STRING, NTKO_TOTALKEY_LEN-1);	
	*md5 = NtkoANSIToLPWSTR( classMD5.toString() );

	TIME_STRING[NTKO_TIME_STR_LEN + 0] = '\0'; //截断字符串

	*time = NtkoANSIToLPWSTR(TIME_STRING);
	
	if ( !(*time) || !(*md5) )
	{
		if ( *time ) SAFE_FREESTRING((*time));
		if ( *md5 ) SAFE_FREESTRING((*md5))
		return FALSE;
	}
	
	return TRUE;
}

BOOL CNTKOServerSignHelper::NTKO_Custom_SetComName(LPWSTR comName)
{
#if !defined(NTKO_CUSTOM_JIANGSU_CA)
	return FALSE;
#endif

	m_Custom_jiangshuCA_CompanyInfo.companyName = NtkoCopyStringW(comName);
	return TRUE;
}

LPWSTR CNTKOServerSignHelper::GetSelectedSignName()
{
	if( (0<=m_selectedSignIndex) && (m_selectedSignIndex<m_iUserSignsCount) )
	{
		return m_NtkoUserSigns[m_selectedSignIndex].bstrSignName;
	}
	return NULL;
}
LPWSTR CNTKOServerSignHelper::GetSelectedSignUser()
{
	if( (0<=m_selectedSignIndex) && (m_selectedSignIndex<m_iUserSignsCount) )
	{
		return m_NtkoUserSigns[m_selectedSignIndex].bstrSignUser;
	}
	return NULL;
}

LPWSTR CNTKOServerSignHelper::GetSelectedSignSN()
{
	if( (0<=m_selectedSignIndex) && (m_selectedSignIndex<m_iUserSignsCount) )
	{
		if(!m_NtkoUserSigns[m_selectedSignIndex].bIsESPSign)
		{
			return m_NtkoUserSigns[m_selectedSignIndex].bstrSignURL;
		}		
	}
	return NULL;
}

BOOL CNTKOServerSignHelper::GetSelectedSignIsESP()
{
	if( (0<=m_selectedSignIndex) && (m_selectedSignIndex<m_iUserSignsCount) )
	{
		return m_NtkoUserSigns[m_selectedSignIndex].bIsESPSign;
	}
	return TRUE;
}
LPWSTR CNTKOServerSignHelper::GetLogonCertSubject()
{
	if(m_pCurSignCert)
	{
		LPWSTR pwszCertName = NtkoCertGetNameStringW(m_pCurSignCert,CERT_NAME_SIMPLE_DISPLAY_TYPE,0);
		return pwszCertName;
	}
	return NULL;
}

PCRYPT_HASH_BLOB CNTKOServerSignHelper::GetLogonCertSHA1Hash()
{
	if(m_pCurSignCert)
	{
		return NtkoGetCertSHA1Hash(m_pCurSignCert);
	}
	return NULL;
}

VOID CNTKOServerSignHelper::SetIsJSCAUseSealSvr(BOOL bUse)
{
	m_IsJSCAUseSealSvr = bUse;
}

BOOL CNTKOServerSignHelper::GetIsJSCAUseSealSvr()
{
	return m_IsJSCAUseSealSvr;
}


/*
BOOL CNTKOServerSignHelper::RefreshServerSignSettings(NTKOServerRequestType )
{
	BOOL isOk = FALSE;
	BSTR bstrRetXML = NULL;
	
	ShowDelayDlgMUIStrW(IDSMUI_CHECKING_USERRIGHTS);
	isOk = DoWebGet(m_rootSvrUrl,NTKO_HC_GETRIGHTS_URL,&bstrRetXML);	
	CloseDelayDlg();
	if(isOk && bstrRetXML)
	{
		isOk = ParseXML(bstrRetXML);
		if(isOk) //解析成功，但是服务器有可能有错误
		{ 
			if( 0 == m_iServerErrCode) 
			{
				if(NtkoAreStringsEqualW(m_bstrServerVersion,-1,NTKO_ServerSecsignControlProtocalVersion,-1))
				{
					isOk = TRUE;
				}
				else
				{
					ShowMsgBoxMUIStrW(IDSMUI_CHECKRIGHT_FAIL_INVALIDSERVER_VERSION,
						IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
					isOk = FALSE;
				}
			}
			else //服务器返回有错误
			{
				WCHAR wszBuf[1024];
				LPWSTR pwszFmt = LoadMUIStringW(IDSMUI_CHECKRIGHT_FAIL_SERVERFAILWITHCODE);
				NtkoFwsprintf(wszBuf,pwszFmt,m_iServerErrCode,m_bstrFailReason);
				SAFE_FREESTRING(pwszFmt);
				
				LPWSTR pwszMUITitle = LoadMUIStringW(IDSMUI_MSGBOXTITLE_TIP);
				NtkoMessageBoxW(m_hwndParent,wszBuf,pwszMUITitle ,MB_OK|MB_ICONINFORMATION);
				SAFE_FREESTRING(pwszMUITitle);
				
				if(1 == m_iServerErrCode)
				{
					m_IsLogin = FALSE;
				}
				isOk = FALSE;
			}
		}
		else //解析错误
		{
			ShowMsgBoxMUIStrW(IDSMUI_CHECKRIGHT_FAIL_PARSERESPONSEERROR,
				IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
			isOk = FALSE;
		}
	}
	else
	{			
		ShowMsgBoxMUIStrW(IDSMUI_CHECKRIGHT_FAIL,IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		isOk = FALSE;
	}
	
	if(isOk)
	{
		isOk = ParseSignOptions();
		if(!isOk)
		{
			ShowMsgBoxMUIStrW(IDSMUI_GETSIGNOPTIONS_FAIL_HANDSIGNERROR_PARSEERROR,
				IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		}
	}
	SAFE_FREEBSTRING(bstrRetXML);
	return isOk;
}
*/


void CNTKOServerSignHelper::ReleaseCurCert()
{
	if(m_pCurSignCert)
	{
		CertFreeCertificateContext(m_pCurSignCert);
		m_pCurSignCert = NULL;
	}
}

void CNTKOServerSignHelper::ReleaseCurStore()
{
	if(m_hCertStore)
	{
		CertCloseStore(m_hCertStore, CERT_CLOSE_STORE_CHECK_FLAG);
		m_hCertStore = NULL;
	}
}
BOOL CNTKOServerSignHelper::SelectLogonCert(HINTERNET hReq)
{
	HWND hwndDlgParent = (m_hwndParent?m_hwndParent:GetDesktopWindow());

	if(m_pCurSignCert)
	{
		return TRUE;
	}

	if(!hReq)
	{
		return FALSE;
	}

#if 0
	INTERNET_CERTIFICATE_INFO serverCertInfo;
	DWORD dwSize = 0;
	LPWSTR pwszServerIssuer = NULL; 
	
	dwSize = sizeof(serverCertInfo);
	if(InternetQueryOption(hReq,INTERNET_OPTION_SECURITY_CERTIFICATE_STRUCT, &serverCertInfo, &dwSize))
	{
		pwszServerIssuer = NtkoANSIToLPWSTR(serverCertInfo.lpszIssuerInfo);
		if ( serverCertInfo.lpszEncryptionAlgName )
		{
			LocalFree( serverCertInfo.lpszEncryptionAlgName);
		}
		if ( serverCertInfo.lpszIssuerInfo )
		{
			LocalFree( serverCertInfo.lpszIssuerInfo );
		}
		if ( serverCertInfo.lpszProtocolName )
		{
			LocalFree( serverCertInfo.lpszProtocolName );
		}
		if ( serverCertInfo.lpszSignatureAlgName )
		{
			LocalFree( serverCertInfo.lpszSignatureAlgName );
		}
		if ( serverCertInfo.lpszSubjectInfo )
		{
			LocalFree( serverCertInfo.lpszSubjectInfo );
		}
	}
#endif

	if (!m_hCertStore)
	{
		m_hCertStore = CertOpenSystemStore( NULL, NTKODS_STORENAMEA);
		if (!m_hCertStore)
		{
			return FALSE;
		}
	}

	LPWSTR pwszServerIssuer = NtkoANSIToLPWSTR(m_secsignSettings.CertIssuerName);

	HideUIDelayDlg();
	if(!NtkoSelectCertOptionalDlg(hwndDlgParent,m_hCertStore,FALSE,pwszServerIssuer,TRUE,TRUE,1,&m_pCurSignCert,NULL))
	{
		SAFE_FREESTRING(pwszServerIssuer);
		return FALSE;
	}

	SAFE_FREESTRING(pwszServerIssuer);

	if(!m_pCurSignCert)
	{
		return FALSE;
	}

	DWORD dwKeySpec = 0;
	HCRYPTPROV hSignCryptProv = NtkoGetSignCryptProv(m_pCurSignCert,&dwKeySpec);
	if(!hSignCryptProv)
	{		 
		 DWORD dwLastErrorCode = GetLastError();
		 TRACE1_NTKOLIB("Error using CryptAcquireCertificatePrivateKey.error=%d\n",dwLastErrorCode);
// 		 WCHAR errMes[100];
// 		 NtkoFwsprintf(errMes,L"CryptAcquireCertificatePrivateKey error 0x%lx",dwLastErrorCode);
// 		 HWND hDlgParentWnd = m_pCtl?(m_pCtl->get_Wnd()):NULL;
// 		 m_pCtl->MyMessageBoxUseMUIStrW(hDlgParentWnd,errMes,L"error",MB_OK | MB_ICONINFORMATION);

		ShowMsgBoxMUIStrW(hwndDlgParent,IDSMUI_LOGINSERVER_FAIL_INVALIDCERT,
						IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		ReleaseCurCert();
		return FALSE;
	}

	BYTE   pbData[256];       // 1000 will hold the longest key container name.
	DWORD cbData = 256;
	ZeroMemory(pbData,sizeof(pbData));


#if defined(NTKO_CUSTOM_SHANXI_CA) //这儿暂时不需要了。不需要用EKEY序列号
	cbData = 256;
	if(CryptGetProvParam(hSignCryptProv, PP_IMPTYPE, 	pbData, &cbData, 0))
	{
		if(pbData[0] != CRYPT_IMPL_SOFTWARE)
		{
			NtkoCmnDlgParam dlgParam;
			ZeroMemory(&dlgParam,sizeof(dlgParam));

			dlgParam.dwFlags = NTKO_CDPF_VERIFYEKEY | NTKO_CDPF_GETEKEYSN;
			dlgParam.EkeyType = m_secsignSettings.ekeyType;
			dlgParam.EkeyVersion = NTKO_EKEYVER_AUTO;
			dlgParam.EkeyIndex = -1;

			int dlgRet = IDCANCEL;
			//提示选择EKEY
			LPWSTR pwszTitle = LoadMUIStringW(IDSMUI_SELEKEYDLGTITLE);
			dlgRet = NtkoShowGetEkeyAndPassDlg(hwndDlgParent,m_EKEYPassword,NTKO_MINEKEY_PASSWORD,NTKO_MAXEKEY_PASSWORD,pwszTitle,&dlgParam);
			SAFE_FREESTRING(pwszTitle);
			if ((IDOK != dlgRet) || (!CheckEkeyUsageWithMes(dlgParam.dwEkeyUsage)) )
			{			
				SAFE_FREEBSTRING(dlgParam.bstrEkeySN);			
				return FALSE;
			}
			SAFE_FREEBSTRING(m_bstrEKEYSN);
			m_bstrEKEYSN = ::SysAllocString(dlgParam.bstrEkeySN);
			SAFE_FREEBSTRING(dlgParam.bstrEkeySN);	

			//设定key加密pin码
			CryptSetProvParam(hSignCryptProv,PP_SIGNATURE_PIN,(LPBYTE)m_EKEYPassword,0);
		}
	}
#endif
	

	//签名一些数据,用于提示ekey口令对话框

	CHAR	szTempSignData[11]; //临时数据
	DWORD	cbTempSignDataSize; //数据长度
	cbTempSignDataSize = sizeof(szTempSignData);
	ZeroMemory(szTempSignData,cbTempSignDataSize);
	lstrcpyn(szTempSignData,"ntkontkont",cbTempSignDataSize);

	HCRYPTHASH hHash = NULL;
	LPBYTE pSignedData = NULL;
	DWORD dwSignedDataSize = 0;
	BOOL isPinOk = FALSE;

	if(CryptCreateHash(
	   hSignCryptProv, 
	   CALG_SHA1, 
	   0, 
	   0, 
	   &hHash)) 
	{
		if(CryptHashData(
		   hHash, 
		   (BYTE*)szTempSignData, 
		   cbTempSignDataSize, 
		   0)) 
		{	
			if(CryptSignHash(
			   hHash, 
			   dwKeySpec,//AT_KEYEXCHANGE,//AT_SIGNATURE, 
			   NULL, //szDescription, This parameter is no longer used and must be set to NULL 
			   0, 
			   NULL, 
			   &dwSignedDataSize)) 
			{
				 pSignedData = (BYTE *)MemAlloc(dwSignedDataSize);
				 
				 if(CryptSignHash(
				   hHash, 
				   dwKeySpec,//AT_KEYEXCHANGE,//AT_SIGNATURE, 
				   NULL, //szDescription, This parameter is no longer used and must be set to NULL 
				   0, 
				   pSignedData, 
				   &dwSignedDataSize))
				 {
					 isPinOk = TRUE;
				 }
				if(pSignedData)
				{
					MemFree(pSignedData);
					pSignedData = NULL;
				}
			}			
		}
		if(hHash) 
		{
			CryptDestroyHash(hHash);
			hHash = NULL;
		}
	}

	CryptReleaseContext(hSignCryptProv, 0);	

	if(!isPinOk)
	{
		ReleaseCurCert();
		return FALSE;
	}

	if (!InternetSetOption(hReq, INTERNET_OPTION_CLIENT_CERT_CONTEXT, 
				(void *)m_pCurSignCert, sizeof(CERT_CONTEXT)) )
	{
		ReleaseCurCert();
		return FALSE;
	}
	ShowUIDelayDlgAgain();
	return TRUE;
}


BOOL CNTKOServerSignHelper::SelectLogonCertNoHTTPS(BOOL bSignSomeData)
{
	HWND hwndDlgParent = (m_hwndParent?m_hwndParent:GetDesktopWindow());

	if(m_pCurSignCert)
	{
		return TRUE;
	}

	if (!m_hCertStore)
	{
		m_hCertStore = CertOpenSystemStore( NULL, NTKODS_STORENAMEA);
		if (!m_hCertStore)
		{
			return FALSE;
		}
	}

	LPWSTR pwszServerIssuer = NtkoANSIToLPWSTR(m_secsignSettings.CertIssuerName);

	HideUIDelayDlg();
	if(!NtkoSelectCertOptionalDlg(hwndDlgParent,m_hCertStore,FALSE,pwszServerIssuer,TRUE,TRUE,1,&m_pCurSignCert,NULL))
	{
		SAFE_FREESTRING(pwszServerIssuer);
		return FALSE;
	}

	SAFE_FREESTRING(pwszServerIssuer);

	if(!m_pCurSignCert)
	{
		return FALSE;
	}

	DWORD dwKeySpec = 0;
	HCRYPTPROV hSignCryptProv = NtkoGetSignCryptProv(m_pCurSignCert,&dwKeySpec);
	if(!hSignCryptProv)
	{		 
		 DWORD dwLastErrorCode = GetLastError();
		 TRACE1_NTKOLIB("Error using CryptAcquireCertificatePrivateKey.error=%d\n",dwLastErrorCode);
// 		 WCHAR errMes[100];
// 		 NtkoFwsprintf(errMes,L"CryptAcquireCertificatePrivateKey error 0x%lx",dwLastErrorCode);
// 		 HWND hDlgParentWnd = m_pCtl?(m_pCtl->get_Wnd()):NULL;
// 		 m_pCtl->MyMessageBoxUseMUIStrW(hDlgParentWnd,errMes,L"error",MB_OK | MB_ICONINFORMATION);

		ShowMsgBoxMUIStrW(hwndDlgParent,IDSMUI_LOGINSERVER_FAIL_INVALIDCERT,
						IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		ReleaseCurCert();
		return FALSE;
	}

	if(!bSignSomeData) //如果不需要签名数据
	{
		CryptReleaseContext(hSignCryptProv, 0);	
		ShowUIDelayDlgAgain();
		return TRUE;
	}

	BYTE   pbData[256];       // 1000 will hold the longest key container name.
	DWORD cbData = 256;
	ZeroMemory(pbData,sizeof(pbData));


	//签名一些数据,用于提示ekey口令对话框

	CHAR	szTempSignData[11]; //临时数据
	DWORD	cbTempSignDataSize; //数据长度
	cbTempSignDataSize = sizeof(szTempSignData);
	ZeroMemory(szTempSignData,cbTempSignDataSize);
	lstrcpyn(szTempSignData,"ntkontkont",cbTempSignDataSize);

	HCRYPTHASH hHash = NULL;
	LPBYTE pSignedData = NULL;
	DWORD dwSignedDataSize = 0;
	BOOL isPinOk = FALSE;

	if(CryptCreateHash(
	   hSignCryptProv, 
	   CALG_SHA1, 
	   0, 
	   0, 
	   &hHash)) 
	{
		if(CryptHashData(
		   hHash, 
		   (BYTE*)szTempSignData, 
		   cbTempSignDataSize, 
		   0)) 
		{	
			if(CryptSignHash(
			   hHash, 
			   dwKeySpec,//AT_KEYEXCHANGE,//AT_SIGNATURE, 
			   NULL, //szDescription, This parameter is no longer used and must be set to NULL 
			   0, 
			   NULL, 
			   &dwSignedDataSize)) 
			{
				 pSignedData = (BYTE *)MemAlloc(dwSignedDataSize);
				 
				 if(CryptSignHash(
				   hHash, 
				   dwKeySpec,//AT_KEYEXCHANGE,//AT_SIGNATURE, 
				   NULL, //szDescription, This parameter is no longer used and must be set to NULL 
				   0, 
				   pSignedData, 
				   &dwSignedDataSize))
				 {
					 isPinOk = TRUE;
				 }
				if(pSignedData)
				{
					MemFree(pSignedData);
					pSignedData = NULL;
				}
			}			
		}
		if(hHash) 
		{
			CryptDestroyHash(hHash);
			hHash = NULL;
		}
	}

	CryptReleaseContext(hSignCryptProv, 0);	

	if(!isPinOk)
	{
		ReleaseCurCert();
		return FALSE;
	}
	ShowUIDelayDlgAgain();
	return TRUE;
}

LPWSTR CNTKOServerSignHelper::GenTempSignedPKCS7Msg()
{
	HWND hwndDlgParent = (m_hwndParent?m_hwndParent:GetDesktopWindow());
	if(!m_pCurSignCert)
	{
		return NULL;
	}


	DWORD dwKeySpec = 0;
	HCRYPTPROV hSignCryptProv = NtkoGetSignCryptProv(m_pCurSignCert,&dwKeySpec);
	if(!hSignCryptProv)
	{		 
		 DWORD dwLastErrorCode = GetLastError();
		 TRACE1_NTKOLIB("Error using CryptAcquireCertificatePrivateKey.error=%d\n",dwLastErrorCode);
// 		 WCHAR errMes[100];
// 		 NtkoFwsprintf(errMes,L"CryptAcquireCertificatePrivateKey error 0x%lx",dwLastErrorCode);
// 		 HWND hDlgParentWnd = m_pCtl?(m_pCtl->get_Wnd()):NULL;
// 		 m_pCtl->MyMessageBoxUseMUIStrW(hDlgParentWnd,errMes,L"error",MB_OK | MB_ICONINFORMATION);

		ShowMsgBoxMUIStrW(hwndDlgParent,IDSMUI_LOGINSERVER_FAIL_INVALIDCERT,
						IDSMUI_MSGBOXTITLE_TIP ,MB_OK|MB_ICONINFORMATION);
		return NULL;
	}

	//签名一些数据,用于提示ekey口令对话框

	CHAR	szTempSignData[10]; //临时数据
	DWORD	cbTempSignDataSize; //数据长度
	cbTempSignDataSize = sizeof(szTempSignData);
	ZeroMemory(szTempSignData,cbTempSignDataSize);
	CopyMemory(szTempSignData,"ntkontkont",cbTempSignDataSize);

	LPBYTE encodedData = NULL;
	DWORD encodedDataLen = 0;
	LPSTR pszResultData = NULL;
	LPWSTR pwszResultData = NULL;

	if(!NtkoSignDigestPKCS7((LPBYTE)szTempSignData, cbTempSignDataSize, &encodedData, &encodedDataLen, hSignCryptProv, m_pCurSignCert))
	{
		CryptReleaseContext(hSignCryptProv, 0);	
		return NULL;
	}
	else
	{
		pszResultData =  binaryASCIIHexEncoding((char*)encodedData, encodedDataLen);
	}

	if(pszResultData)
	{
#ifdef _DEBUG
	#if 1
		HANDLE hDebugFile = INVALID_HANDLE_VALUE;
		NtkoOpenLocalFileW(L"c:\\ntkoSubmitPKCS7_bin.bin", GENERIC_WRITE, 0, OPEN_ALWAYS, &hDebugFile);
		if (INVALID_HANDLE_VALUE != hDebugFile)
		{
			DWORD dwWritten = 0;
			WriteFile(hDebugFile, (void*)encodedData, encodedDataLen, &dwWritten, NULL);
		}
		CloseHandle(hDebugFile);
	#endif
	#if 1
		HANDLE hDebugFileB2H = INVALID_HANDLE_VALUE;
		NtkoOpenLocalFileW(L"c:\\ntkoSubmitPKCS7_B2Hex.txt", GENERIC_WRITE, 0, OPEN_ALWAYS, &hDebugFileB2H);
		if (INVALID_HANDLE_VALUE != hDebugFileB2H)
		{
			DWORD dwWritten = 0;
			DWORD dwBytesToWrite = lstrlen(pszResultData);
			WriteFile(hDebugFileB2H, (void*)pszResultData,dwBytesToWrite , &dwWritten, NULL);
		}
		CloseHandle(hDebugFileB2H);
	#endif
#endif

		pwszResultData = NtkoANSIToLPWSTR(pszResultData);
		SAFE_FREESTRING(pszResultData);
	}
	SAFE_FREESTRING(encodedData);
	CryptReleaseContext(hSignCryptProv, 0);	
	return pwszResultData;
}

VOID CNTKOServerSignHelper::SetEkeyPassword(LPSTR pszEkeyPass)
{
	if(pszEkeyPass && lstrlen(pszEkeyPass))
	{
		ZeroMemory(m_EKEYPassword,sizeof(m_EKEYPassword));
		lstrcpyn(m_EKEYPassword,pszEkeyPass,sizeof(m_EKEYPassword));
	}		
}