#include "ntkomemstr.h"
#include "NTKOServerSignHelper.h"
#include <windows.h>
#include "ntkomuiHelper.h"
#include "ntkocusdraw.h"
#include "ntkosecstr.h"
#include "ntkosignekey.h"
#include "res/resource.h"

// 进程间共享的登录状态
// #pragma data_seg(".NTKOSSSrd")
// volatile BOOL	sg_isLogIn				= FALSE;
// volatile BOOL	sg_hasRight_handSign	= FALSE;
// volatile BOOL	sg_hasRight_keyComment	= FALSE;
// volatile BOOL	sg_hasRight_sign		= FALSE;
// volatile BOOL	sg_hasRight_keySign		= FALSE;
// #pragma data_seg()
// 
// __declspec(allocate(".NTKOSSSrd")) volatile  WCHAR sg_LoginUserName[NTKO_MAX_LOGINUSERNAME+1];
// __declspec(allocate(".NTKOSSSrd")) volatile  WCHAR sg_LoginPassword[NTKO_MAX_PASSWORD+1];
// __declspec(allocate(".NTKOSSSrd")) volatile  char  sg_EkeyPassword [NTKO_MAX_PASSWORD+1];
// 
// #pragma comment(linker,"/SECTION:.NTKOSSSrd,RWS")
//CNtkoMuiHelper * v_pNtkoMUIHelper = NULL;

NtkoSecsignSettings g_DefaultSecsignSettings = {
	100,				// 手写签名图片缩放比例
	LineMode_NoPress,	// 笔模式
	3,					// 笔宽
	RGB(0xFF,0x00,0x00),		// 颜色
	NTKOSignCommentLocationV0,	// 批注位置
	NULL,				// 批注字体
	TIMEAlignV_N1,		// 日期位置
	NTKOSignTimeMode_0,	// 日期风格
	NULL,	// 日期字体
	TRUE,	// 签章位于文字上方
	TRUE,	// 自动接收修订再盖章
	{'\0'},	// 数字证书发布者名字
	GetSystemDefaultLangID(),
	FALSE,
	FALSE,
	TRUE,	// 检查文档改变
	TRUE,	// 签章时手写签名
	TRUE,	// 添加批注	
	TRUE,	// 添加日期
	TRUE,	// 键盘批注边框
	NTKOSecsignPrintMode_Origin ,// 打印 模式
	TRUE,	// 是否使用
	FALSE,	// 印章是否虚化
	FALSE,	// 检查字体改变
	TRUE,	// 是否显示UI
	TRUE,	// 是否允许锁定时盖章(用于word,excel等文档)
	FALSE,
	NULL, 
	FALSE,
	{'\0'},
	FALSE,
	{'\0'},
	ntkoSptKeyHaiTai
};

HANDLE g_hModule		= NULL;
BOOL   v_fUnicodeAPI    = FALSE;  // Flag to determine if we should us Unicode API
BOOL   v_fWindows2KPlus = FALSE;
BOOL   v_fWindowsXPPlus = FALSE;
CNTKOServerSignHelper * g_pHelper = NULL;
DWORD  v_RefCount		= 0;
HICON  g_signListDlgICON = NULL;

BOOL APIENTRY DllMain( HANDLE hModule, 
					  DWORD  ul_reason_for_call, 
					  LPVOID lpReserved
					  )
{
    switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			g_hModule = hModule;
			OSVERSIONINFO osvi;
			osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
			GetVersionEx (&osvi);
			v_fUnicodeAPI = (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT); //unicode support
			v_fWindows2KPlus = ((v_fUnicodeAPI) && (osvi.dwMajorVersion  > 4)); //2k plus
			v_fWindowsXPPlus = ((v_fWindows2KPlus) && (osvi.dwMinorVersion  > 0)); //xp,2003
			g_signListDlgICON = (HICON)LoadImage((HINSTANCE)g_hModule, MAKEINTRESOURCE(IDI_ICONLOGO), IMAGE_ICON, 16, 16, 0);
		}
		break;
	case DLL_PROCESS_DETACH:
		{
			if (g_signListDlgICON)
			{
				DestroyIcon(g_signListDlgICON);
				g_signListDlgICON = NULL;
			}
		}
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
    }
    return TRUE;
}

INTKOServerSignHelper * WINAPI NtkoDllGetClassObject()
{
	if (!g_pHelper)
	{
		g_pHelper = new CNTKOServerSignHelper;
	}
	v_RefCount++;
	TRACE1_NTKOLIB("NtkoDllGetClassObject,ref=%d\n",v_RefCount);
	return (PINTKOServerSignHelper)g_pHelper;
}

void WINAPI NtkoDllDelClassObject()
{
	v_RefCount--;
	TRACE1_NTKOLIB("NtkoDllDelClassObject,ref=%d\n",v_RefCount);
	if(0 == v_RefCount)
	{
		NTKO_SAFEFreePoint(g_pHelper);
		ODS_NTKOLIB("NtkoDllDelClassObject, object deleted,end\n");
	}	
}
