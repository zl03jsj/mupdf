#include "windows.h"
#include "..\..\..\00ntkocommon\inc\ntkosigndef.h"
#include "..\..\..\00ntkocommon\inc\ntkosignekey.h "
#include "..\NTKOSignHelperDll.h"

// #define NTKOSignHelper_GetClassObjMethodName "DllGetClassObject"
// #define NTKOSignHelper_DelClassObjMethodName "DllDelClassObject"
int main(void)
{
	LPSTR dllPath = NULL;
#if defined(_DEBUG)
	dllPath = "..\\OutPutDll\\signHelper_D.dll";
#else
	dllPath = "..\\OutPutDll\\signHelper_D.dll";
#endif

	HMODULE hModule = NULL;
	hModule = LoadLibrary(dllPath);
	if (!hModule)
		return 0;
	DllGetClassObjFunPoint GetClassObj = (DllGetClassObjFunPoint)GetProcAddress(hModule, NTKOSignHelper_GetClassObjMethodName);
	INTKOServerSignHelper * pIHelper = GetClassObj();
	pIHelper->InitNew(NULL,NULL);

	return 0;
}