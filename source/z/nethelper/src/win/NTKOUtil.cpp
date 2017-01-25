#include "ntkomemstr.h"
#include "NTKOUtil.h"
#include <windows.h>
#include <comcat.h>
#include "NTKOServerSignHelper.h"

EXTERN NtkoSecsignSettings g_DefaultSecsignSettings;

BOOL ReadRegisterInfo(HKEY hkParent, LPCSTR pszSubKey,LPSTR infoName,LPSTR resultBuf,DWORD dwBufSize)
{
	if( (!hkParent) || (!pszSubKey) || (!infoName) || (!resultBuf) )
	{
		return FALSE;
	}

	HKEY    hklmOfficeAddin;

	DWORD   dwret;
	DWORD	dwBufLen = dwBufSize;
	DWORD	dwResult;
	if ((dwret = RegOpenKey(hkParent, pszSubKey, &hklmOfficeAddin)) == ERROR_SUCCESS)
	{        
		dwResult = RegQueryValueEx(hklmOfficeAddin, infoName, 0, NULL, 
			(BYTE *)resultBuf, &dwBufLen);
		RegCloseKey(hklmOfficeAddin);
		if(ERROR_SUCCESS == dwResult )
		{
			return TRUE;
		}		
	}
	//*resultBuf = '\0'; 不覆盖默认值
	return FALSE;
}
BOOL WriteRegisterInfo(HKEY hkParent, LPCSTR pszSubKey,LPSTR infoName,LPSTR infoBuf)
{
	if( (!hkParent) || (!pszSubKey) || (!infoName) || (!infoBuf) )
	{
		return FALSE;
	}

	HKEY    hklmOfficeAddin;
	DWORD   dwret;
	DWORD	dwResult;

	if ((dwret = RegCreateKeyEx(hkParent, pszSubKey, 
		0, NULL, 0, KEY_WRITE, NULL, &hklmOfficeAddin, NULL)) == ERROR_SUCCESS)
	{        
		dwResult = RegSetValueEx(hklmOfficeAddin, infoName, 0, 
			REG_SZ, (BYTE *)infoBuf, lstrlen(infoBuf)+1);
		RegCloseKey(hklmOfficeAddin);
		if(ERROR_SUCCESS == dwResult )
		{
			return TRUE;
		}	
	}
	return FALSE;
}

BOOL ReadRegisterBinary(HKEY hkParent, LPCSTR pszSubKey,LPSTR infoName,BYTE* resultBuf,DWORD* pdwBufSize)
{
	if( (!hkParent) || (!pszSubKey) || (!infoName) || (!resultBuf) )
	{
		return FALSE;
	}

	HKEY    hklmOfficeAddin;

	DWORD   dwret;
	DWORD	dwBufLen = 0;
	DWORD	dwResult;

	if(pdwBufSize)
	{
		dwBufLen = *pdwBufSize;
	}


	if ((dwret = RegOpenKey(hkParent, pszSubKey, &hklmOfficeAddin)) == ERROR_SUCCESS)
	{        
		dwResult = RegQueryValueEx(hklmOfficeAddin, infoName, 0, NULL, 
			resultBuf, &dwBufLen);
		RegCloseKey(hklmOfficeAddin);
		if(ERROR_SUCCESS == dwResult )
		{
			if(pdwBufSize)
			{
				*pdwBufSize = dwBufLen;
			}
			return TRUE;
		}	
		else
		{
			if(pdwBufSize)
			{
				*pdwBufSize = 0;
			}
		}
	}
	else
	{
		if(pdwBufSize)
		{
			*pdwBufSize = 0;
		}
	}

	//*resultBuf = '\0'; 不覆盖默认值
	return FALSE;
}
BOOL WriteRegisterBinary(HKEY hkParent, LPCSTR pszSubKey,LPSTR infoName,BYTE* infoBuf,DWORD dwBufSize)
{
	if( (!hkParent) || (!pszSubKey) || (!infoName) || (!infoBuf) )
	{
		return FALSE;
	}

	HKEY    hklmOfficeAddin;
	DWORD   dwret;
	DWORD	dwResult;

	if ((dwret = RegCreateKeyEx(hkParent, pszSubKey, 
		0, NULL, 0, KEY_WRITE, NULL, &hklmOfficeAddin, NULL)) == ERROR_SUCCESS)
	{        
		dwResult = RegSetValueEx(hklmOfficeAddin, infoName, 0, 
			REG_BINARY,infoBuf, dwBufSize);
		RegCloseKey(hklmOfficeAddin);
		if(ERROR_SUCCESS == dwResult )
		{
			return TRUE;
		}	
	}
	return FALSE;
}

long ReadRegisterLong(HKEY hkParent, LPCSTR pszSubKey,LPSTR infoName,long ifFailReturnThis)
{
	if( (!hkParent) || (!pszSubKey) || (!infoName) )
	{
		return ifFailReturnThis;
	}
	HKEY    hklmOfficeAddin;

	BYTE	resultBuf[256];
	DWORD   dwret;
	DWORD	dwBufLen = sizeof(resultBuf);
	DWORD	dwResult;

	if ((dwret = RegOpenKey(hkParent, pszSubKey, &hklmOfficeAddin)) == ERROR_SUCCESS)
	{        
		dwResult = RegQueryValueEx(hklmOfficeAddin, infoName, 0, NULL, 
			(BYTE *)resultBuf, &dwBufLen);
		RegCloseKey(hklmOfficeAddin);
		if(ERROR_SUCCESS == dwResult )
		{
			long retValue = *((long*)(&resultBuf[0]));
			return retValue;
		}		
	}
	return ifFailReturnThis;
}

BOOL WriteRegisterLong(HKEY hkParent, LPCSTR pszSubKey,LPSTR infoName,long lValue)
{
	if( (!hkParent) || (!pszSubKey) || (!infoName) )
	{
		return FALSE;
	}
	HKEY    hklmOfficeAddin;
	DWORD   dwret;
	DWORD	dwResult;

	if ((dwret = RegCreateKeyEx(hkParent, pszSubKey, 
		0, NULL, 0, KEY_WRITE, NULL, &hklmOfficeAddin, NULL)) == ERROR_SUCCESS)
	{        
		dwResult = RegSetValueEx(hklmOfficeAddin, infoName, 0, 
			REG_DWORD, (BYTE *)(&lValue), sizeof(lValue));
		RegCloseKey(hklmOfficeAddin);
		if(ERROR_SUCCESS == dwResult )
		{
			return TRUE;
		}
	}
	return FALSE;
}

BOOL ReadNtkoSecsignSettings(NtkoSecsignSettings * pSecsignSettings)
{
	BOOL isok = FALSE;
	pSecsignSettings->ScalePic = ReadRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NKTO_HC_SETTING_HandSignScale,
		g_DefaultSecsignSettings.ScalePic);
	pSecsignSettings->LineMode = (NTKOSecsignLineMode)ReadRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NKTO_HC_SETTING_LineStyle,
		(LONG)g_DefaultSecsignSettings.LineMode);
	pSecsignSettings->PenWidth = (INT)ReadRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NKTO_HC_SETTING_LineWidth,
		(LONG)g_DefaultSecsignSettings.PenWidth);
	pSecsignSettings->PenColor = (DWORD)ReadRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NKTO_HC_SETTING_LineColor,
		(LONG)g_DefaultSecsignSettings.PenColor);
	pSecsignSettings->CommentLocationV = (NTKOSecsignCommentLocationV)ReadRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NKTO_HC_SETTING_CommentLocationV,
		(LONG)g_DefaultSecsignSettings.CommentLocationV);
	pSecsignSettings->HFontComment = NULL;
	pSecsignSettings->TimeAlignV = (NTKOSecsignTimeAlignV)ReadRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NKTO_HC_SETTING_TimeLocationV,
		(LONG)g_DefaultSecsignSettings.TimeAlignV);
	pSecsignSettings->TimeMode = (NTKOSecsignTimeMode)ReadRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NKTO_HC_SETTING_TimeStyle,
		(LONG)g_DefaultSecsignSettings.TimeAlignV);
	pSecsignSettings->HFontTime = NULL;
	pSecsignSettings->bSignOverText = (BOOL)ReadRegisterLong(HKEY_CURRENT_USER, 
	"CLSID\\"NTKOSecsignConfigHk,NTKO_HC_SETTING_IsAddSginTop,
		g_DefaultSecsignSettings.bSignOverText);
	pSecsignSettings->bAutoAcceptRivise = (BOOL)ReadRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk, NTKO_HC_SETTING_IsAutoAcceptRev,
		g_DefaultSecsignSettings.bAutoAcceptRivise);
	ReadRegisterInfo(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NTKO_HC_SETTING_CertIssuerName,
		pSecsignSettings->CertIssuerName,sizeof(pSecsignSettings->CertIssuerName));
	pSecsignSettings->CurLangId = (LANGID)ReadRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NTKO_HC_SETTING_LangID,
			g_DefaultSecsignSettings.CurLangId);
	pSecsignSettings->bLockSign = (BOOL)ReadRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk, NTKO_HC_SETTING_ISAUTOLOCKSIGN,
		g_DefaultSecsignSettings.bLockSign);
	pSecsignSettings->bLockDoc = (BOOL)ReadRegisterLong(HKEY_CURRENT_USER, 
	"CLSID\\"NTKOSecsignConfigHk,NTKO_HC_SETTING_ISAUTOLOCKSIGN,
		g_DefaultSecsignSettings.bLockDoc);
	pSecsignSettings->bCheckDocChange = (BOOL)ReadRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NTKO_HC_SETTING_IsCheckDocChange,
		g_DefaultSecsignSettings.bCheckDocChange);
	pSecsignSettings->bHandsign = (BOOL)ReadRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NKTO_HC_SETTING_IsAddHandSignOnSign,
		g_DefaultSecsignSettings.bHandsign);
	pSecsignSettings->bComment = (BOOL)ReadRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NKTO_HC_SETTING_IsAddCommentOnSign,
		g_DefaultSecsignSettings.bComment);
	pSecsignSettings->bAddTime = (BOOL)ReadRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NKTO_HC_SETTING_IsAddTimeOnSign,
		g_DefaultSecsignSettings.bAddTime);
	pSecsignSettings->bKeboardBorder = (BOOL)ReadRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NKTO_HC_SETTING_IsUseKeyboardborder,
		g_DefaultSecsignSettings.bKeboardBorder);
	pSecsignSettings->PrintMode = (NTKOSecsignPrintMode)ReadRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NKTO_HC_SETTING_PrintMode,
		(LONG)g_DefaultSecsignSettings.PrintMode);
	pSecsignSettings->bUseCert = (BOOL)ReadRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NKTO_HC_SETTING_IsUseCert,
		g_DefaultSecsignSettings.bUseCert);
	pSecsignSettings->bFuzzyPic = (BOOL)ReadRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NKTO_HC_SETTING_IsFuzzySignPic,
		g_DefaultSecsignSettings.bFuzzyPic);
	pSecsignSettings->bCheckFont = (BOOL)ReadRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NKTO_HC_SETTING_IsCheckFont,
		g_DefaultSecsignSettings.bCheckFont);
	pSecsignSettings->bShowUI = (BOOL)ReadRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NKTO_HC_SETTING_IsShowUI,
		g_DefaultSecsignSettings.bCheckDocChange);
	pSecsignSettings->bAllowLockAdd = (BOOL)ReadRegisterLong(HKEY_CURRENT_USER, 
	"CLSID\\"NTKOSecsignConfigHk,NTKO_HC_SETTING_IsAllowLockAdd,
		g_DefaultSecsignSettings.bAllowLockAdd);
	pSecsignSettings->bUseEkeyLogin = (BOOL)ReadRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk, NTKO_HC_SETTING_IsUseEkeyLogin,
		g_DefaultSecsignSettings.bUseEkeyLogin);
	
	ReadRegisterInfo(HKEY_CURRENT_USER, "CLSID\\"NTKOSecsignConfigHk, NTKO_HC_SETTING_SecsignServerName,
		pSecsignSettings->serverURL,sizeof(g_DefaultSecsignSettings.serverURL));
	
	pSecsignSettings->bUseDefaultsignPassowrd = (BOOL)ReadRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk, NTKO_HC_SETTING_IsUseDefaultPassword,
		g_DefaultSecsignSettings.bUseDefaultsignPassowrd);
	
	BYTE tmpDefPass[NTKO_MAX_PASSWORD];
	ZeroMemory(tmpDefPass,sizeof(tmpDefPass));
	DWORD passLen = sizeof(tmpDefPass);
	ReadRegisterBinary(HKEY_CURRENT_USER, "CLSID\\"NTKOSecsignConfigHk, NTKO_HC_SETTING_DefaultSignPassword,
		tmpDefPass, &passLen);
	
	for(DWORD i=0;i<passLen;i++)
	{
		pSecsignSettings->defaultSignPassword[i] = (CHAR)((tmpDefPass[i])^(0xFE-passLen+i));
	}
	
	pSecsignSettings->bRememberUserName = (BOOL)ReadRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk, NTKO_HC_SETTING_IsRememberUserName,
		g_DefaultSecsignSettings.bRememberUserName);
	
	ReadRegisterInfo(HKEY_CURRENT_USER, "CLSID\\"NTKOSecsignConfigHk, NTKO_HC_SETTING_RememberedUserName,
		pSecsignSettings->rememberedUserName,sizeof(pSecsignSettings->rememberedUserName) );
	
	return TRUE;
}

STDAPI_(void) ConvertANSIToUTF8Hex(LPSTR pszANSI)
{
	if(!pszANSI)
	{
		return;
	}

	BYTE	result[1024];
	LPWSTR  pszW = NtkoANSIToLPWSTR(pszANSI);
	LPSTR	pszUTF8 = NtkoLPWSTRToUTF8(pszW);
	SAFE_FREESTRING(pszW);
	
	char	tBuf[5];
	ZeroMemory(result,1024);
	
	int		outlen = lstrlen(pszUTF8);
	if(outlen>1023)return;
	
	
	BYTE *pByte = (BYTE*)pszUTF8;
	BYTE *pResult = &result[0];
	
	//寻找最后一个"/"
	BYTE *pByteSrc = pByte + outlen - 1;
	while( (*pByteSrc) && (pByteSrc >= pByte) )
	{
		if((*pByteSrc) == 47 ) // chr(47) = "/"
		{
			break;
		}
		pByteSrc--;
	}
	
	while(*pByte && (pByte<(BYTE*)pszUTF8+1023) )
	{
		BYTE b = (*pByte);
		//以下的(95 == b) 是为了防止Domino的_f9pa4mjqv9t34cia38k9sth0jmmqh7fuo2eufs4tvl89rf8gjnn9h7fuq2epcs4tvnhb38c1g60_.rar不能下载的错误
		if( ( b > 127) || ( (95 == b)&& (pByte>pByteSrc) )  )
		{
			wsprintf(tBuf,"%%%2X",b);
			StrNCat((char*)result,tBuf,5);
			pResult += 3;
		}
		else
		{
			*pResult = b;
			pResult++;
		}
		pByte++;
	}
	ZeroMemory(pszANSI,outlen);
	StrCpyN(pszANSI,(char*)result,1024);
	SAFE_FREESTRING(pszUTF8);
}

STDAPI_(LPWSTR)	ConvertLPWSTRToUTF8HexW(LPWSTR pwsz)
{
	if(!pwsz)
	{
		return NULL;
	}
	BYTE	result[1024];
	LPSTR	pszUTF8 = NtkoLPWSTRToUTF8(pwsz);
	
	char	tBuf[5];
	ZeroMemory(result,1024);
	
	int		outlen = lstrlen(pszUTF8);
	if(outlen>1023)return NULL;
	
	
	BYTE *pByte = (BYTE*)pszUTF8;
	BYTE *pResult = &result[0];
	
	//寻找最后一个"/"
	BYTE *pByteSrc = pByte + outlen - 1;
	while( (*pByteSrc) && (pByteSrc >= pByte) )
	{
		if((*pByteSrc) == 47 ) // chr(47) = "/"
		{
			break;
		}
		pByteSrc--;
	}
	
	while(*pByte && (pByte<(BYTE*)pszUTF8+1023) )
	{
		BYTE b = (*pByte);
		//以下的(95 == b) 是为了防止Domino的_f9pa4mjqv9t34cia38k9sth0jmmqh7fuo2eufs4tvl89rf8gjnn9h7fuq2epcs4tvnhb38c1g60_.rar不能下载的错误
		if( ( b > 127) || ( (95 == b)&& (pByte>pByteSrc) )  )
		{
			wsprintf(tBuf,"%%%2X",b);
			StrNCat((char*)result,tBuf,5);
			pResult += 3;
		}
		else
		{
			*pResult = b;
			pResult++;
		}
		pByte++;
	}
	SAFE_FREESTRING(pszUTF8);

	return NtkoANSIToLPWSTR((char*)result);
}

BOOL WriteNtkoSecsignSettings(NtkoSecsignSettings * pSecsignSettings)
{
	BOOL isok = FALSE;
	WriteRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NKTO_HC_SETTING_HandSignScale,
		pSecsignSettings->ScalePic);
	WriteRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NKTO_HC_SETTING_LineStyle,
		(LONG)pSecsignSettings->LineMode);
	WriteRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NKTO_HC_SETTING_LineWidth,
		(LONG)pSecsignSettings->PenWidth);
	WriteRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NKTO_HC_SETTING_LineColor,
		(LONG)pSecsignSettings->PenColor);
	WriteRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NKTO_HC_SETTING_CommentLocationV,
		(LONG)pSecsignSettings->CommentLocationV);
	pSecsignSettings->HFontComment = NULL;
	WriteRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NKTO_HC_SETTING_TimeLocationV,
		(LONG)pSecsignSettings->TimeAlignV);
	WriteRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NKTO_HC_SETTING_TimeStyle,
		(LONG)pSecsignSettings->TimeMode);

	pSecsignSettings->HFontTime = NULL;

	WriteRegisterLong(HKEY_CURRENT_USER, 
	"CLSID\\"NTKOSecsignConfigHk,NTKO_HC_SETTING_IsAddSginTop,
		pSecsignSettings->bSignOverText);
	WriteRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk, NTKO_HC_SETTING_IsAutoAcceptRev,
		pSecsignSettings->bAutoAcceptRivise);
	WriteRegisterInfo(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NTKO_HC_SETTING_CertIssuerName,
		pSecsignSettings->CertIssuerName);
	WriteRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NTKO_HC_SETTING_LangID,
			pSecsignSettings->CurLangId);
	WriteRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk, NTKO_HC_SETTING_ISAUTOLOCKSIGN,
		pSecsignSettings->bLockSign);
	WriteRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NTKO_HC_SETTING_ISAUTOLOCKSIGN,
		pSecsignSettings->bLockDoc);
	WriteRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NTKO_HC_SETTING_IsCheckDocChange,
		pSecsignSettings->bCheckDocChange);
	WriteRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NKTO_HC_SETTING_IsAddHandSignOnSign,
		pSecsignSettings->bHandsign);
	WriteRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NKTO_HC_SETTING_IsAddCommentOnSign,
		pSecsignSettings->bComment);
	WriteRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NKTO_HC_SETTING_IsAddTimeOnSign,
		pSecsignSettings->bAddTime);
	WriteRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NKTO_HC_SETTING_IsUseKeyboardborder,
		pSecsignSettings->bKeboardBorder);
	WriteRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NKTO_HC_SETTING_PrintMode,
		(LONG)pSecsignSettings->PrintMode);
	WriteRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NKTO_HC_SETTING_IsUseCert,
		pSecsignSettings->bUseCert);
	WriteRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NKTO_HC_SETTING_IsFuzzySignPic,
		pSecsignSettings->bFuzzyPic);
	WriteRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NKTO_HC_SETTING_IsCheckFont,
		pSecsignSettings->bCheckFont);
	WriteRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk,NKTO_HC_SETTING_IsShowUI,
		pSecsignSettings->bShowUI);
	WriteRegisterLong(HKEY_CURRENT_USER, 
	"CLSID\\"NTKOSecsignConfigHk,NTKO_HC_SETTING_IsAllowLockAdd,
		pSecsignSettings->bAllowLockAdd);
	WriteRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk, NTKO_HC_SETTING_IsUseEkeyLogin,
		pSecsignSettings->bUseEkeyLogin );
	
	WriteRegisterInfo(HKEY_CURRENT_USER, "CLSID\\"NTKOSecsignConfigHk, NTKO_HC_SETTING_SecsignServerName,
		pSecsignSettings->serverURL);
	
	 WriteRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk, NTKO_HC_SETTING_IsUseDefaultPassword,
		pSecsignSettings->bUseDefaultsignPassowrd);
	
	BYTE tmpDefPass[NTKO_MAX_PASSWORD];
	ZeroMemory(tmpDefPass,sizeof(tmpDefPass));
	int passLen = strlen(pSecsignSettings->defaultSignPassword);
	for(int i=0;i<passLen;i++)
	{
		tmpDefPass[i] = (CHAR)((pSecsignSettings->defaultSignPassword[i])^(0xFE-passLen+i));
	}
	WriteRegisterBinary(HKEY_CURRENT_USER, "CLSID\\"NTKOSecsignConfigHk, NTKO_HC_SETTING_DefaultSignPassword,
		tmpDefPass, (DWORD)passLen);
	

	
	 (BOOL)WriteRegisterLong(HKEY_CURRENT_USER, 
		"CLSID\\"NTKOSecsignConfigHk, NTKO_HC_SETTING_IsRememberUserName,
		pSecsignSettings->bRememberUserName);
	
	WriteRegisterInfo(HKEY_CURRENT_USER, "CLSID\\"NTKOSecsignConfigHk, NTKO_HC_SETTING_RememberedUserName,
		pSecsignSettings->rememberedUserName);
	
	return TRUE;	
}
