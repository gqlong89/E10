/*tfs.h
* 2018-02-28
* Copyright(C) 2018
* liutao@chargerlink.com
*/
#ifndef __TFS_H__
#define __TFS_H__
#include "includes.h"


#define BUF_MAX 256
#define	TFS_ID2_LEN							24
#define TFS_ID2_AES_LEN						64
#define TFS_TIMESTAMP_AUTH_CODE_LEN			67
#define TFS_CHALLENGE_AUTH_CODE_LEN			86
#define	TFS_EXTA_LEN						8
#define CIPHER_SIZE  						72


extern uint8_t AesKey[17];
extern const uint8_t *extraData;

extern int TfsGetId2(uint8_t id[TFS_ID2_LEN]);
extern int TfsId2Decrypt(const uint8_t *cipherText, uint8_t cipherLen, uint8_t *out);
extern int TfsId2GetChallengeAuthCode(const uint8_t *challenge, uint8_t *extra, uint8_t extraLen, uint8_t *out);
extern int TfsId2GetTimeStampAuthCode(uint64_t timestamp, uint8_t *extra, uint8_t extraLen, uint8_t *out);


#endif //__TFS_H__


