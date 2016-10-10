//
//  K5SOFApp.h
//
//  Created by Longmai.
//  Copyright (c) 2014年 Longmai. All rights reserved.
//



@protocol K5SOFAppDelegate <NSObject>

-(void) didDeviceDiscovered:(NSString * )devName;

-(void) didDeviceConnected:(NSString *)devName  error:(NSError *)error;

-(void) didDeviceDisconnected:(NSString*)devName;

@end



@interface K5SOFApp : NSObject
{

   
}

@property (nonatomic,assign)  id <K5SOFAppDelegate> delegate;


-(NSString*) SOF_GetVersion;

-(int) SOF_GetBLEState;

-(int) SOF_EnumDevices:(NSMutableArray*)foundDevices timeout:(NSInteger)timeout;

-(int) SOF_StartEnumDevices;

-(int) SOF_StopEnumDevices;

-(int) SOF_Connect:(NSString*)devName authData:(NSData*)authData application:(NSString*)appName;

-(int) SOF_Connect:(NSString*)devName authCode:(NSString*)authCode application:(NSString*)appName;

-(int) SOF_ConnectAsyn:(NSString*)devName authCode:(NSString*)authCode application:(NSString*)appName;

-(int) SOF_Connect:(NSString*)devName application:(NSString*)appName;

-(int) SOF_Connect:(NSString*)devName authData:(NSData*)authData;

-(int) SOF_Connect:(NSString*)devName authCode:(NSString*)authCode;

-(int) SOF_OpenApplication:(NSString * )appName;

-(int) SOF_Disconnect;

-(int) SOF_GetDeviceInfo:(NSMutableString *)Label SerialNumber:(NSMutableString *)SerialNumber Version:(NSMutableString*)version;

-(int) SOF_GetBatteryLevel:(NSInteger*)level;

-(int) SOF_GetLastError;

-(int) SOF_GenRandom:(int)randomLen randomData:(NSMutableData*)data;

-(int) SOF_Login:(NSString*)userPin  leftRetry:(NSInteger*)left;

-(int) SOF_Login:(NSInteger)pinType userPin:(NSString*)userPin  leftRetry:(NSInteger*)left;

-(int) SOF_Logout;

-(int) SOF_ChangePassWd:(NSString*)oldUpin newUpin:(NSString*)newUpin retry:(NSInteger*)retry;

-(int) SOF_ChangePassWd:(NSInteger)pinType  oldPin:(NSString*)oldPin newPin:(NSString*)newPin retry:(NSInteger*)retry;

-(int) SOF_UnblockPIN:(NSString*)adminPin newUpin:(NSString *)newUpin retry:(NSInteger*)retry;

-(int) SOF_CreateContainer:(NSString*)containerName containerId:(NSInteger*)containerId;

-(int) SOF_DeleteContainer:(NSString*)containerName;

-(int) SOF_EnumContainers:(NSMutableArray*)containers;

-(int) SOF_GetContainerInfo:(NSString*)containerName containerType:(NSInteger*)containerType
               signKeyLen:(NSInteger*)signKeyLen  exchKeyLen:(NSInteger*)exchKeyLen
               signCertFlag:(NSInteger*)signCertFlag exchCertFlag:(NSInteger*)exchCertFlag;

-(int) SOF_OpenContainerKeyPair:(NSString*)containerName containerType:(NSInteger*)containerType hasSignKeyPair:(BOOL*)hasSignKeyPair
          hasExchKeyPair:(BOOL*)hasExchKeyPair containerId:(NSInteger*)containerId;

-(int) SOF_ExportUserCert:(NSString *)containerName signFlag:(NSInteger)signFlag certData:(NSMutableData*)certData;

-(int) SOF_ImportUserCert:(NSString *)containerName signFlag:(NSInteger)signFlag certData:(NSData*)certData;

-(int) SOF_ExportPublicKeyBlob:(NSString*)containerName signFlag:(NSInteger)signFlag keyAlgo:(NSInteger*)keyAlgo pubkeyBlob:(NSMutableData*)pubkeyBlob;

-(int) SOF_GenKeyPair:(NSString *)containerName  signFlag:(NSInteger)signFlag algoId:(NSInteger)algoId bits:(NSInteger)bits  pubKeyBlob:(NSMutableData*)pubkeyBlob;

-(int) SOF_ImportRSAKeyPair:(NSString *)containerName symAlgId:(NSInteger)symAlgId
              wrappedKey:(NSData*)wrappedKey bits:(NSInteger)bits envelopedKeyBlob:(NSData*)envelopedKeyBlob;

-(int) SOF_ImportECCKeyPair:(NSString *)containerName envelopedKeyBlob:(NSData*)envelopedKeyBlob;

-(int) SOF_SignData:(NSString*)containerName signFlag:(NSInteger)signFlag  hashAlgo:(NSInteger)hashAlgo  data:(NSData*)data  signature:(NSMutableData*)signature;

-(int) SOF_SignData:(NSString*)containerName signFlag:(NSInteger)signFlag  hashAlgo:(NSInteger)hashAlgo  timeOut:(NSInteger)timeOut data:(NSData*)data  signature:(NSMutableData*)signature;

-(int) SOF_SignDataCancel:(NSString*)containerName signFlag:(NSInteger)signFlag  hashAlgo:(NSInteger)hashAlgo;

-(int) SOF_VerifySignedData:(NSData*)pubKeyBlob algoId:(NSInteger)algoId hashAlgo:(NSInteger)hashAlgo data:(NSData*)data signature:(NSData*)signature;

-(int) SOF_ExtPublicEncrypt:(NSData*)pubkeyBlob algoId:(NSInteger)algoId input:(NSData*)input output:(NSMutableData*)output;

-(int) SOF_PublicEncrypt:(NSString*)containerName signFlag:(NSInteger)signFlag input:(NSData*)input output:(NSMutableData*)output;

-(int) SOF_PrivateDecrypt:(NSString*)containerName signFlag:(NSInteger)signFlag input:(NSData*)input output:(NSMutableData*)output;

-(int) SOF_PrivateOperation:(NSString*)containerName signFlag:(NSInteger)signFlag input:(NSData*)input output:(NSMutableData*)output;

-(int) SOF_PublicOperation:(NSString*)containerName signFlag:(NSInteger)signFlag input:(NSData*)input output:(NSMutableData*)output;

-(int) SOF_EncryptData:(NSInteger)algoId key:(NSData*)key padding:(BOOL)padding iv:(NSData*)iv plain:(NSData*)plain cipher:(NSMutableData*)cipher;

-(int) SOF_DecryptData:(NSInteger)algoId key:(NSData*)key padding:(BOOL)padding iv:(NSData*)iv cipher:(NSData*)cipher plain:(NSMutableData*)plain;

-(int) SOF_DigestData:(NSInteger)algoId data:(NSData*)data hashVal:(NSMutableData*)hashVal;

-(int) SOF_DigestInit:(NSInteger)algoId;

-(int) SOF_DigestInit_Display:(NSInteger)algoId display:(BOOL)display;

-(int) SOF_DigestUpdate:(NSData *)data;

-(int) SOF_DigestUpdate_Display:(NSData *)data display:(BOOL)display;

-(int) SOF_DIgestFinal:(NSMutableData *)hashVal;

-(int) SOF_EnumFiles:(NSMutableArray*)files;

-(int) SOF_GetFileInfo:(NSString*)fileName fileSize:(NSInteger*)fileSize
         readRights:(NSInteger*)readRights writeRights:(NSInteger*)writeRights;


-(int) SOF_ReadFile:(NSString*)fileName offset:(NSInteger)offset readLen:(NSInteger)readLen data:(NSMutableData*)data;

-(int) SOF_WriteFile:(NSString*)fileName offset:(NSInteger)offset writeLen:(NSInteger)writeLen data:(NSData*)data;

-(int) SOF_CreateFile:(NSString*)fileName  fileSize:(NSInteger)fileSize  readRights:(NSInteger)readRights writeRight:(NSInteger)writeRights;

-(int) SOF_DeleteFile:(NSString*)fileName;

-(int) SOF_SetLabel:(NSString *)label;

-(int) SOF_GetPINInfo:(NSInteger)pinType maxRetry:(NSInteger*)maxRetry remainRetry:(NSInteger*)remainRetry isDefault:(NSInteger *)isDefault;

-(int) SOF_ImportExtRSAKeyPair:(NSString*)containerName signFlag:(NSInteger)signFlag priKeyBlob:(NSData*)priKeyBlob;

-(int) SOF_InternalAuth;

-(NSData*) SM3_GetZ:(NSData*)eccPubKey;

-(int) SOF_DevAuth:(NSString*)authKey retryCount:(NSInteger*)retryCount;

-(int) SOF_CreateApplication:(NSString *)appName  adminPin:(NSString*)adminPin adminPinRetryCount:(NSInteger)adminPinRetryCount
                              userPin:(NSString*)userPin userPinRetryCount:(NSInteger)userPinRetryCount createFileRights:(NSInteger)createFileRights;

-(int) SOF_DeleteApplication:(NSString*)appName;

-(int) SOF_EnumApplications:(NSMutableArray*)appNames;

-(NSString *) SOF_GetConnectedDev;

-(NSData *) SOF_SoftDigestData:(NSInteger)algoId data:(NSData*)data;

-(int) SOF_SoftDigestInit:(NSInteger)algoId eccPubKey:(NSData*)eccPubKey;

-(int) SOF_SoftDigestUpdate:(NSData*)data;

-(NSData *) SOF_SoftDigestFinal;


-(int) SOF_SetSymtricKey:(NSInteger)algoId key:(NSData*)key keyId:(NSInteger*)keyId;

-(int) SOF_DestroySymtricKey:(NSInteger)keyId;

-(int) SOF_EncryptInit:(NSInteger)keyId iv:(NSData*)iv padding:(NSInteger)padding;

-(int) SOF_EncryptUpdate:(NSInteger)keyId plain:(NSData*)plain cipher:(NSMutableData*)cipher;

-(int) SOF_EncryptFinal:(NSInteger)keyId plain:(NSData*)plain cipher:(NSMutableData*)cipher;

-(int) SOF_DecryptInit:(NSInteger)keyId iv:(NSData*)iv padding:(NSInteger)padding;

-(int) SOF_DecryptUpdate:(NSInteger)keyId cipher:(NSData*)cipher plain:(NSMutableData*)plain;

-(int) SOF_DecryptFinal:(NSInteger)keyId cipher:(NSData*)cipher plain:(NSMutableData*)plain;

-(int) SOF_ExtRSAPubKeyOperation:(NSData*)pubkeyBlob input:(NSData*)input output:(NSData*)output;

-(int) SOF_DeleteCertificate:(NSString*)containerName signFlag:(NSInteger)signFlag;

-(int) SOF_DeleteKeyPair:(NSString *)containerName signFlag:(NSInteger)signFlag;



@end
