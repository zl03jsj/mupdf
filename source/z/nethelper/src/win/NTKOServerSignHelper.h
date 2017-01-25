#if !defined(_NTKOServerSignHelper_h__) 
#define _NTKOServerSignHelper_h__

#include <windows.h>
#include "ntkokeyintfc.h"
#include <wininet.h>
#include "NTKOUtil.h"
#include "ntkosignekey.h"
#include "NTKOSignHelperDll.h"
#include <commctrl.h>
class CNtkoSecStrs;
class CNtkoSignEkey;
class CNtkoUIDelayDlg;
class CNtkoMuiHelper;


//************************************
// Class:	CNTKOServerSignHelper
// Describe:服务器版电子印章网络相关功能的帮助类
//************************************
class CNTKOServerSignHelper:public INTKOServerSignHelper{
public:
	void  SetSignSvrLicType(NtkoSignSvrLicType iSignSvrLicType);
	BOOL  InitNew (HWND hwndParent,HMODULE hModuleIncludePublishData);// 初始化,必须调用.
	BOOL  DoLogin (LPWSTR username, LPWSTR password, BOOL bUseEkey);  // 登录服务器
	BOOL  DoLogout();					// 从服务器注销
	BOOL  IsLogin(){return m_IsLogin;}; // 获取是否已经登录
	CONST NtkoServerSecsignRightSettings * GetServerSetting(){return &m_serverRightSettings;};// 获取服务器配置印章权限
	CONST NtkoPublishRightSettings * GetPublishSetting(){return &m_publishRightSettings;};	// 获取发布配置印章权限
	CONST NtkoSecsignSettings * GetSecsignSetting(){return &m_secsignSettings;};			// 获取印章设置
	CONST LPWSTR GetServerTime();	// 从服务器获取时间字符串				
	CONST LPWSTR GetLoginUserName(){return m_LoginUserName;};		// 返回登录用户名.
	CONST LPWSTR GetLoginRealName(){return m_LoginRealName;};		//返回登录返回的实际名称
	BOOL  CheckOpRight(NTKOServerRequestType opType);				// 操作权限检查
	LPWSTR GetLoadedToLocalsignPath(NtkoSignType signtype);			// pdf 印章服务器版本日志函数
	BOOL CheckEkeyToSignRel(LPWSTR ekeySN,LPWSTR signSN);			// 检查ekey和印章的关联关系.(即ekey是否可以盖此印章)
	//////////////////////////////////////////////////////////////////////////
	BOOL DoServerSignLog(NtkoSecuritySignInfo  * pSignInfo,NtkoSignOpType opType,LPWSTR fileName,LPWSTR fileInfo);	// pdf印章日志记录
	CNTKOServerSignsInfo * DoCheckSigns(PNtkoSecuritySignInfo pInfo,int count,int * outServersignCount);			// 检查pdf印章
	//////////////////////////////////////////////////////////////////////////
	BOOL DoServerSignLog(IDispatch* pSecsignControl,NtkoSignOpType opType,LPWSTR fileName,LPWSTR fileInfo);
	BOOL DocheckSigns(CNTKODocSignsInfo * pNtkoDocSignsInfo, int len);
	//////////////////////////////////////////////////////////////////////////
	LONG GetLogServerId()  {return m_iThisServerInfoServerId;};		// 返回配置服务器ID
	LPWSTR GetLogServerName(){return m_bstrThisServerInfoName;};	// 返回配置服务器名字.
	BOOL ShowServerConfigDlg(); // 显示服务器配置对话框
	BOOL SetAppType(NTKOAPPType appType);
	BOOL SetSvrUrl(LPSTR url);
	VOID FreeMyMem(LPWSTR p);	// 释放dll内部分配的内存

	VOID SetSignsFilter(LPWSTR pwszSignFilter);
	LPWSTR GetSignsFilter(){return m_pwszSignFilter;};

//////////////////////////////////////////////////////////////////////////
//  以下为江苏CA定制版本协议解析
	BOOL NTKO_Custom_SetComName(LPWSTR comName);
//////////////////////////////////////////////////////////////////////////
	LPWSTR GetSelectedSignName();	//不需要释放返回值
	LPWSTR GetSelectedSignUser();	//不需要释放返回值
	LPWSTR GetSelectedSignSN();	//不需要释放返回值
	BOOL   GetSelectedSignIsESP();	//返回最后所选的印章是否是ESP印章
	LPWSTR GetLogonCertSubject();	//需要释放返回值
	PCRYPT_HASH_BLOB GetLogonCertSHA1Hash(); //需要SAFE_FREESTRING释放返回值及其pbData数据
	VOID	SetIsJSCAUseSealSvr(BOOL bUse); //江苏CA是否使用章模服务器
	BOOL	GetIsJSCAUseSealSvr();

	//川大智胜定制所需接口,返回用户对应的证书序号,不需要释放返回值.
	//2014.06.19 修改为返回全部主题，逗号分割
	LPWSTR GetUserCertSN(){return m_bstr_custom_ChuanDaZhiSheng_UserCertSN;};

	VOID SetHiddenCheckItems(DWORD HiddenFlag){m_dwHiddenCheckItems = HiddenFlag;};
	DWORD GetHiddenCheckItems(){return m_dwHiddenCheckItems;};
	VOID SetEkeyPassword(LPSTR ekeyPass);

public:
	CNTKOServerSignHelper(void);
	~CNTKOServerSignHelper();
private:
	INT ShowMsgBoxMUIStrW(DWORD idText,DWORD idCaption,UINT type);
	INT ShowMsgBoxMUIStrW(HWND hwndParent,DWORD idText,DWORD idCaption,UINT type);
	void SetMUIDlgItemStringW(HWND hDlg,UINT idItem,DWORD dwKey);
	LPWSTR LoadMUIStringW(DWORD dwKey);
	VOID SetMUIItemStrW(HWND hDlg,UINT idItem,DWORD dwKey);
	BOOL CreateHiddenWindowForMsg();
	BOOL ShowDelayDlg(LPWSTR pwszMes);
	BOOL CloseDelayDlg();

	void HideUIDelayDlg();
	void ShowUIDelayDlgAgain();

	VOID ShowDelayDlgMUIStrW(UINT idText);
	VOID ShowCheckSignListDlg();
	BOOL CheckServerName(LPSTR szServerName);
	// 网络相关函数
	BOOL DoWebGet(LPSTR pszServerApp,LPWSTR pwszURL,BSTR* pbstr);
	BOOL GetHTTPReturnData(HINTERNET hReq,BSTR* pbstr);
	BOOL DownLoadURLToILockBytes(LPSTR pszServerApp,LPWSTR pwszURL,ILockBytes* pILockBytes);
	BOOL DoWebExecute2(LPSTR pszServerApp,LPWSTR pwszURL,LPWSTR pwszCPARA,BSTR* pbstr);
	BOOL IsInternetSettingEnableHTTP11();
	BOOL DownLoadURLToFile(LPSTR pszServerApp,LPWSTR pwszURL, LPWSTR pwszFile);
	BOOL TestSignServer(HWND hwndParent,BOOL bShowPrompt,BOOL bSetDlgItemText,INT dlgItem);
	// 解析 xml
	BOOL ParseXML(BSTR bstrXML);
	BOOL ParseRootServerInfo();
	BOOL ParseThisServerInfo();
	BOOL ParseThisUserInfo();
//////////////////////////////////////////////////////////////////////////
//  以下为江苏CA定制版本协议解析
	BOOL NTKO_Custom_ParseThisSealServerInfo();
	void NTKO_Custom_ClearCompanyInfo();
	void NTKO_Custom_IniteCompanyInfo();
	BOOL NTKO_Custom_ParseSigns();
	BOOL NTKO_Custom_ParseSigns_New();
	BOOL NTKO_Custom_CreateKey(LPWSTR * md5, LPWSTR * timestamp);
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//  以下为川大智胜定制版本协议解析
	BOOL NTKO_Custom_ParseCertSN();
//////////////////////////////////////////////////////////////////////////


//数字证书SSL双向验证相关
	void		ReleaseCurCert();
	void		ReleaseCurStore();
	BOOL		SelectLogonCert(HINTERNET hReq);
	BOOL		SelectLogonCertNoHTTPS(BOOL bSignSomeData);
	LPWSTR		GenTempSignedPKCS7Msg();
//////////////////////////////////////////////////////////////////////////

	BOOL ParseUserRights();
	BOOL ParseResponseStatus();
	BOOL ParseSignOptions();
	BOOL ParseNtkoCheckEkey();
	BOOL ParseServerTime();
	BOOL ParseServerInfo();
	BOOL ParseNtkoUserSigns();
	BOOL ParseCheckSignServers();
	BOOL ParseSignsCheckStatus();
	// 权限检查
	BOOL CheckEKEYUsageWithMes(DWORD ekeyUsage,DWORD usage=NTKO_EKEYUSAGE_EKEY_LOGIN);//DWORD usage=NTKO_EKEYUSAGE_EKEY_LOGIN=1
	BOOL CheckServerBeforeHandSign ();
	BOOL CheckServerBeforeKeyboard ();
	BOOL CheckServerBeforeEkeySign ();
	BOOL CheckServerBeforeNomalSign();
	BOOL CheckServerBeforeBarcode();
	// BOOL RefreshServerSignSettings();
	void ClearServerSigns();
	VOID ClearCheckSignServers();
private:
	VOID SetDlgListImageList(HWND hWndListView,int iDlgType);
	BOOL InitSelSignListViewColumns(HWND hWndListView); 
	BOOL FillSelSignListWithSignsInfo(HWND hWndListView,BOOL isIncludeBlankSigns,LPWSTR pwszFilter);
	VOID AutoAdjustListViewColumn(HWND hWndListView);
	VOID DestroyDlgListImageList();
	VOID ClearUserSigns();
	BOOL InitCheckSignListViewColumns(HWND hWndListView); 
	BOOL FillCheckSignListWithSignsInfo(HWND hWndListView,BOOL bIsForServerSigns);
	VOID ShowSelectedDocSignInfo(BOOL bIsForServerSigns);
	VOID GotoSelectedDocSign(){return;};
	BOOL GetCertificateLogonInfo(HWND hwndDlgParent,LPWSTR* ppwszCertName,LPWSTR* ppwszCertSN);
public:
	static BOOL FSSFindMSDocWinProc(HWND hwndChild, LPARAM lParam);
	static STDMETHODIMP_(INT_PTR) LoginDlgFrameHandler(HWND hDlg,HWND hwndTab,UINT Msg,WPARAM wParam,LPARAM lParam,LPVOID pUserParam);
	static STDMETHODIMP_(INT_PTR) LoginDlgTabProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static DWORD WINAPI Notify_SlotChanged(BOOL bIsOk,LPVOID pParam,BOOL bIsInserted);
	static LRESULT CALLBACK HiddenWindowForMsgProcess(HWND hwnd, UINT uMsg,WPARAM wParam,LPARAM lParam);
	static STDMETHODIMP_(INT_PTR) SelSignIndexDlgFrameHandler(HWND hDlg,HWND hwndTab,UINT Msg,WPARAM wParam,LPARAM lParam,LPVOID pUserParam);
	static STDMETHODIMP_(INT_PTR) SelSignIndexDlgTabProc(HWND hDlg, UINT Msg, WPARAM wParam,LPARAM lParam);
	static BOOL GetTempNewFilePath(WCHAR** ppwszLocalFile,LPSTR preStr,LPSTR extStr);
	static STDMETHODIMP_(INT_PTR) SysConfigSvrDlgHandler(HWND hDlg,HWND hwndTab,UINT Msg,WPARAM wParam,LPARAM lParam,LPVOID pUserParam);
	static STDMETHODIMP_(INT_PTR) SysConfigSvrDlgProc(HWND hDlg, UINT Msg, WPARAM wParam,LPARAM lParam);
	static STDMETHODIMP_(INT_PTR) CheckSignDlgWindowProc(HWND hDlg, UINT Msg, WPARAM wParam,LPARAM lParam);
	static STDMETHODIMP GetProperty(LPDISPATCH pDisp, LPOLESTR pszName, VARIANT* pvResult); // 获取idispatch接口的属性.

private:
	NtkoSecsignSettings  m_secsignSettings;
	NtkoServerSecsignRightSettings m_serverRightSettings;
	NtkoPublishRightSettings m_publishRightSettings;
	BOOL	m_IsLogin;
	WCHAR	m_LoginUserName[NTKO_MAX_LOGINUSERNAME+1];
	WCHAR	m_LoginPassword[NTKO_MAX_PASSWORD+1];
	WCHAR	m_LoginRealName[NTKO_MAX_LOGINUSERNAME+1];
	CHAR	m_rootSvrUrl[MAX_PATH]; //登录服务器(root Server url),配置的服务器在m_secsignSetting->serverurl中.
	// 江苏CA定制版本请求印章列表URL
	CHAR	m_CustomSignServerUrl[MAX_PATH];
	CNtkoSecStrs   * m_pNTkoSecstrPublishInfo;
	CNtkoSignEkey  * m_pNtkoSignEkey;
	CNtkoUIDelayDlg* m_pNtkoUIDelayDlg;	
	CNtkoMuiHelper * m_pNtkoMuiHelper;
	BOOL			m_PublishInfo_AUTOUseEkeyLogin;
	HWND			m_HwndHiddenForMsg;
	HWND			m_hwndParent;
	char			m_EKEYPassword[NTKO_MAX_PASSWORD+1];
	BSTR			m_bstrEKEYSN;
	NTKOAPPType		m_appType;
	IXMLDOMDocument * m_pXMLDoc;
	BSTR					m_bstrServerVersion;
	BSTR					m_bstrServerInfoVersion;
	BSTR					m_bstrServerInfoName;
	BSTR					m_bstrServerInfoLicenseUserName;
	BSTR					m_bstrServerInfoLicenseCount;
	BSTR					m_bstrThisServerInfoName;
	int						m_iThisServerInfoServerId;
	int						m_iServerErrCode;
	BSTR					m_bstrFailReason;
	BOOL					m_CanUseThisEkey;
	BSTR					m_bstrServerTime;
	CNTKOSignsInfo			m_NtkoUserSigns[NTKO_MAX_SIGNS]; // 保存用户可用服务器上软件印章
	long					m_iUserSignsCount;
	int						m_iSelectedSignIndex;// 弹出印章检查列表后选择的印章序号.
	CNTKOCheckSignServersInfo	m_NtkoCheckSignServers[NTKO_MAX_CHECKSIGN_SERVERS];  // 保存服务器改变信息
	long					m_iCheckSignServersCount;
	CNTKOServerSignsInfo	m_NtkoServerSigns[NTKO_MAX_DOC_SERVER_SIGNS]; // 保存服务器返回的印章信息
	long					m_iServerSignsCount;
	CNTKODocSignsInfo	*   m_docSigns;
	long					m_iDocSignsCount;
	long					m_iCurServerId; 
	HWND					m_hwndCheckSignDlg;
	DWORD m_dwReceiveDataTimeOut;
	DWORD m_dwConnectServerTimeOut;
	HIMAGELIST		m_hLargeSelSign;   // image list for icon view 
    HIMAGELIST		m_hSmallSelSign;   // image list for other views 
	LONG			m_selectedSignIndex;
	NTKOServerRequestType m_serverReqestType;
	HMODULE			m_hModulePublishInfo;

	HBITMAP			m_hbmpError;
	HBITMAP			m_hbmpOk;

	LPWSTR			m_pwszSignFilter;

	DWORD			m_dwHiddenCheckItems;

	NtkoSignSvrLicType m_iSignSvrLicType;


	NTKO_CUSTOM_jiangshuCA_CompanyInfo m_Custom_jiangshuCA_CompanyInfo;
	BOOL			m_IsJSCAUseSealSvr;

//数字证书SSL双向验证相关
	HCERTSTORE    m_hCertStore;  //store 
	PCCERT_CONTEXT   m_pCurSignCert; //用户选择的证书
//////////////////////////////////////////////////////////////////////////

	BSTR		m_bstr_custom_ChuanDaZhiSheng_UserCertSN;
};

#endif
