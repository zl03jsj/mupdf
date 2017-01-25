#if !defined(_NTKOSignHelperDll_h__) 
#define _NTKOSignHelperDll_h__

#ifdef NTKOSERVERSIGNHELPER_EXPORTS
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

#define		EXTERN		extern
#define		CONST		const

#define NTKOSignHelper_GetClassObjMethodName "NtkoDllGetClassObject"
#define NTKOSignHelper_DelClassObjMethodName "NtkoDllDelClassObject"

#include "ntkosigndef.h"

enum NTKOSecsignLineMode{
	LineMode_NoPress = 0,
	LineMode_ImitatePress,
	LineMode_RealPress
};
// 是垂直方向的位置
enum NTKOSecsignCommentLocationV{
	NTKOSignCommentLocationV0 = 0,
	NTKOSignCommentLocationV1,
	NTKOSignCommentLocationV2,
	NTKOSignCommentLocationV3,
	NTKOSignCommentLocationV4,
	NTKOSignCommentLocationV5,
	NTKOSignCommentLocationV6,
	NTKOSignCommentLocationV7,
	NTKOSignCommentLocationV8,
	NTKOSignCommentLocationV9,
	NTKOSignCommentLocationVA,
	NTKOSignCommentLocationVB
};
// 是垂直方向的位置
enum NTKOSecsignCommentLocationH{
	NTKOSignCommentLocationH_0 = 0,
	NTKOSignCommentLocationH_1,
	NTKOSignCommentLocationH_2,
};
enum NTKOSecsignTimeAlignV{
	TIMEAlignV_NO = 0,	// "印章内顶部"
	TIMEAlignV_N1,		// "中间偏上"
	TIMEAlignV_N2,		// "中间偏下"
	TIMEAlignV_N3,		// "印章内底部"
	TIMEAlignV_N4,		// "印章下方"
	TIMEAlignV_N5,		// "不显示"
	TIMEAlignV_N6,		// "印章内居中"
	TIMEAlignV_N7,		// "自定义位置"
};

enum NTKOSecsignTimeAlignH{
	TIMEAlignH_NO = 0,	// "印章内顶部"
	TIMEAlignH_N1,		// "中间偏上"
	TIMEAlignH_N2,		// "中间偏下"
};
enum NTKOSecsignTimeMode{
	NTKOSignTimeMode_0 = 0, //二〇〇九年一月二十二日
	NTKOSignTimeMode_1,		//2009年1月22日
	NTKOSignTimeMode_2,		//2009-1-22
	NTKOSignTimeMode_3,		//2009.1.22
};

//0: Other; 1:Word, 2:Excel,6:WPS,7 ET电子表,5 pdf
enum NTKOAPPType{
	NTKOAPPType_Other	= 0,	// "Other Application"
	NTKOAPPType_Word	= 1,	// "MS Word"
	NTKOAPPType_Excel	= 2,	// "MS Excel"
	NTKOAPPType_Web		= 3,	// "HTML 印章"
	NTKOAPPType_PDF		= 5,	// "NTKO Pdf"
	NTKOAPPType_WPS		= 6,	// "WPS 文字"
	NTKOAPPType_ETd		= 7,	// "WPS 电子表格" 
	NTKOAPPType_YoZo	= 8		// "永中文字"
};

enum NTKOServerRequestType{
	NTKOServerRequestType_GetRootServerInfo			= 1,	// 1:获取根服务器信息。
	NTKOServerRequestType_UNKOWN					= 2,
	NTKOServerRequestType_Login						= 3,	// 3:用户登录。
	NTKOServerRequestType_GetSignList				= 4,	// 4:获取服务器软件印章列表
	NTKOServerRequestType_GetEkeyUseableState		= 5,	// 5.获取用户EKEY可用状态
	NTKOServerRequestType_CheckRightHandsign		= 6,	// 6:检查手写签名权限
	NTKOServerRequestType_CheckRightKeyBoradComment = 7,	// 7:检查键盘批注权限	
	NTKOServerRequestType_DosignLog					= 8,	// 8:签章审计日志记录
	NTKOServerRequestType_GetBinaryFile				= 9,	// 9:二进制文件请求,未使用
	NTKOServerRequestType_Logout					= 10,	// 10:用户注销请求
	NTKOServerRequestType_TestConnetServer			= 11,	// 11:测试服务器连接
	NTKOServerRequestType_GetserverTime				= 12,	// 12:获取服务器时间
	NTKOServerRequestType_CheckSign					= 13,	// 13:检查印章,
	NTKOServerRequestType_GetServerChangeInfo		= 14,	// 14:获取服务器变更(过期)信息
	NTKOServerRequestType_GetRightOfEkeyAndSign		= 15,	// 15:获取用户EKEY及印章的关联权限
	NTKOServerRequestType_GetServerComments			= 16,	// 16:获取服务器批注列表
	NTKOServerRequestType_SetServerComments			= 17,	// 17:设定服务器批注列表
	NTKOServerRequestType_KeepSession				= 18,	// 18:保持session
	NTKOServerRequestType_CheckRightBarcode			= 20,	// 20:检查二维码权限
};

enum NtkoSignOpType {
	NtkoSignOpType_Add = 0,
	NtkoSignOpType_Lock,
	NtkoSignOpType_UnLock,
	NtkoSignOpType_Protect,
	NtkoSignOpType_Unprotect,
	NtkoSignOpType_EnableCheckDocChange,
	NtkoSignOpType_DisableCheckDocChange,
	NtkoSignOpType_Delete,
	NtkoSignOpType_Uncrypt
};

// 本地印章相关设定
struct NtkoSecsignSettings
{
	int		ScalePic;				// 手写签名图片缩放比例
	NTKOSecsignLineMode	LineMode;	// 笔模式
	int		PenWidth;				// 笔宽
	DWORD	PenColor;				// 颜色
	NTKOSecsignCommentLocationV		CommentLocationV;	// 批注位置
	HFONT	HFontComment;				// 批注字体
	NTKOSecsignTimeAlignV	TimeAlignV;	// 日期位置
	NTKOSecsignTimeMode		TimeMode;	// 日期风格
	HFONT	HFontTime;					// 日期字体

	BOOL	bSignOverText;	  // 签章位于文字上方
	BOOL	bAutoAcceptRivise;// 自动接收修订再盖章
	CHAR	CertIssuerName[MAX_PATH];	  // 数字证书发布者名字
	LANGID	CurLangId;		  // 语言ID
	// 兼容以前版本的属性
	BOOL bLockSign;
	BOOL bLockDoc;

	// sign setting:function setting
	BOOL bCheckDocChange;// 检查文档改变
	BOOL bHandsign;		 // 签章时手写签名
	BOOL bComment;		 // 添加批注	
	BOOL bAddTime;		 // 添加日期
	BOOL bKeboardBorder; // 键盘批注边框

	// sign setting:safety setting
	NTKOSecsignPrintMode  PrintMode;		// 打印 模式
	BOOL bUseCert;		// 是否使用
	BOOL bFuzzyPic;		// 印章是否虚化
	BOOL bCheckFont;	// 检查字体改变
	BOOL bShowUI;		// 是否显示UI
	BOOL bAllowLockAdd;	// 是否允许锁定时盖章(用于word,excel等文档)

	BOOL	bUseEkeyLogin;									// 是否使用ekey登录
	CHAR	serverURL[MAX_PATH];							// 服务器URL
	BOOL	bUseDefaultsignPassowrd;						// 是否使用默认密码
	CHAR	defaultSignPassword[NTKO_MAX_PASSWORD+1];		// 默认密码
	BOOL	bRememberUserName;								// 是否记住用户名
	CHAR	rememberedUserName[NTKO_MAX_LOGINUSERNAME+1];	// 用户名

	NtkoSptKeyType ekeyType;
};

// 江苏CA定制公司信息结构体
struct NTKO_CUSTOM_jiangshuCA_CompanyInfo{
	LPWSTR companyId;
	LPWSTR companyName;
	LPWSTR companyCode;
};

// 权限配置
struct NtkoSecsignRightSettings
{
	BOOL bUseHandsign;
	BOOL bUseKeyboardsign;
	BOOL bUseNormalsign;
	BOOL bUseEkeySign;
	BOOL bUseBarcode;
	BOOL bUseFingerPrint;
	BOOL bUseMutisignEkey;		// 多页章是否使用EKEY
	BOOL bUseMuticrosssignEkey; // 骑缝章是否使用EKEY
	BOOL bUseMutisign;
	BOOL bUseMutiCrossPageSign;
	BOOL bUseDbCrossPagesign;
};

#define NtkoServerSecsignRightSettings	NtkoSecsignRightSettings
#define NtkoPublishRightSettings		NtkoSecsignRightSettings

typedef struct CNTKOSignsInfo
{
	BSTR	bstrSignName;	// 江苏CA定制版本中,作为协议中的signname
	BSTR	bstrSignUser;	// 江苏CA定制版本中,作为协议中的Type
	BSTR	bstrSignURL;	// 江苏CA定制版本中,作为协议中的ID
	BOOL	bIsESPSign;		// 是esp印章，还是江苏ca的图片章
}CNTKOSignsInfo,*PCNTKOSignsInfo;


typedef struct CNTKODocSignsInfo
{
	BSTR	bstrSignGUID;
	BSTR	bstrSigner;
	BSTR	bstrSignName;
	long	iSignServerId;
	BOOL	bIsSignChecked;
	BSTR	bstrSignCheckInfoTitle;
	BSTR	bstrSignCheckInfoText;
	DWORD	dwIndexInDoc;
}CNTKODocSignsInfo,*PCNTKODocSignsInfo;

typedef struct CNTKOServerSignsInfo
{
	BSTR	bstrSignGUID;
	BOOL	bFound;
	BSTR	bstrSigner;
	BSTR	bstrSignName;
	BSTR	bstrSignUser;
	BSTR	bstrSignSN;
	BSTR	bstrEkeySN;
	BSTR	bstrServerTime;
	BSTR	bstrAppName;
	BSTR	bstrClientIP;
	long	iSignServerId;
}CNTKOServerSignsInfo,*PCNTKOServerSignsInfo;

typedef struct CNTKOCheckSignServersInfo
{
	long	iSignServerId;
	BSTR	bstrSignServerName;
	BSTR	bstrServerAppURL;
	BOOL	bIsUsedInThisDoc;
	LPWSTR	pwszPostPara;
}CNTKOCheckSignServersInfo,*PCNTKOCheckSignServersInfo;

//************************************
// Class:	CNTKOServerSignHelper
// Describe:服务器版电子印章网络相关功能的帮助类
//************************************
class INTKOServerSignHelper {
public:
	virtual void  SetSignSvrLicType(NtkoSignSvrLicType iSignSvrLicType) = 0;
	virtual BOOL  InitNew (HWND hwndParent,HMODULE hModuleIncludePublishData)=0;
	virtual BOOL  DoLogin (LPWSTR username, LPWSTR password, BOOL bUseEkey)=0;
	virtual BOOL  DoLogout()=0;
	virtual VOID  ClearUserSigns()=0;
	virtual BOOL  IsLogin()=0;
	virtual CONST NtkoServerSecsignRightSettings * GetServerSetting() = 0;
	virtual CONST NtkoPublishRightSettings * GetPublishSetting() = 0;
	virtual CONST NtkoSecsignSettings * GetSecsignSetting()=0;
	virtual CONST LPWSTR GetServerTime()=0;
	virtual CONST LPWSTR GetLoginUserName()=0;
	virtual CONST LPWSTR GetLoginRealName()=0;
	virtual BOOL CheckOpRight(NTKOServerRequestType opType)=0;
	virtual LPWSTR GetLoadedToLocalsignPath(NtkoSignType signtype)=0;// 需要调用MemFree
	virtual BOOL CheckEkeyToSignRel(LPWSTR ekeySN,LPWSTR signSN) = 0;
	//////////////////////////////////////////////////////////////////////////
	virtual BOOL DoServerSignLog(NtkoSecuritySignInfo  * pSignInfo,NtkoSignOpType opType,LPWSTR fileName,LPWSTR fileInfo)=0;// pdf印章日志记录
	// 传入文档中的所有印章信息,传出印章服务器上的所有印章信息!!!
	virtual CNTKOServerSignsInfo * DoCheckSigns(PNtkoSecuritySignInfo pInfo,int len,int * outServersignCount)=0;// 检查pdf印章
	//////////////////////////////////////////////////////////////////////////
	virtual BOOL DoServerSignLog(IDispatch* pSecsignControl,NtkoSignOpType opType,LPWSTR fileName,LPWSTR fileInfo)=0;
	virtual BOOL DocheckSigns(CNTKODocSignsInfo * pNtkoDocSignsInfo, int len)=0;
	//////////////////////////////////////////////////////////////////////////
	virtual LONG GetLogServerId() = 0;
	virtual LPWSTR GetLogServerName() = 0;
	virtual BOOL ShowServerConfigDlg() = 0;
	virtual BOOL SetAppType(NTKOAPPType appType)=0;
	virtual BOOL SetSvrUrl(LPSTR url) = 0;
	virtual VOID FreeMyMem(LPWSTR p) = 0;

	virtual	VOID SetSignsFilter(LPWSTR pwszSignFilter) = 0;
	virtual LPWSTR GetSignsFilter() = 0;
	virtual BOOL NTKO_Custom_SetComName(LPWSTR comName) = 0;
	virtual LPWSTR GetSelectedSignName() = 0; //不需要释放返回值
	virtual LPWSTR GetSelectedSignUser() = 0; //不需要释放返回值
	virtual LPWSTR GetSelectedSignSN() = 0; //不需要释放返回值
	virtual BOOL   GetSelectedSignIsESP() = 0;	//不需要释放返回值
	virtual LPWSTR GetLogonCertSubject() = 0; //需要SAFE_FREESTRING释放返回值
	virtual PCRYPT_HASH_BLOB GetLogonCertSHA1Hash() = 0; //需要SAFE_FREESTRING释放返回值及其pbData数据
	virtual VOID SetIsJSCAUseSealSvr(BOOL bUse) = 0; //江苏CA是否使用章模服务器
	virtual BOOL GetIsJSCAUseSealSvr() = 0;
	
	//川大智胜定制所需接口,返回用户对应的证书序号,不需要释放返回值.
	//2014.06.19 修改为返回全部主题，逗号分割
	virtual LPWSTR GetUserCertSN() = 0;

	virtual VOID SetHiddenCheckItems(DWORD HiddenFlag) = 0; //设定服务器版本检查印章对话框中的隐藏项目
	virtual DWORD GetHiddenCheckItems() = 0;
	virtual VOID SetEkeyPassword(LPSTR ekeyPass) = 0;
};

typedef INTKOServerSignHelper* PINTKOServerSignHelper;

INTKOServerSignHelper * WINAPI  NtkoDllGetClassObject();
void WINAPI NtkoDllDelClassObject();

typedef INTKOServerSignHelper* (WINAPI *DllGetClassObjFunPoint)(void);
typedef void(WINAPI * DllDelClassObjFunPoint)();


struct NtkosignHelperFuncs
{
	DllGetClassObjFunPoint GetClassObj;
	DllDelClassObjFunPoint DelClassObj;
};

#endif // _NTKOSignHelperDll_h__