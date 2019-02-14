/*tfs.c
* 2018-02-28
* Copyright(C) 2018
* liutao@chargerlink.com
*/

#include "tfs.h"
#include "BswDrv_Usart.h"
#include "GM510.h"
#include "BswDrv_sc8042b.h"




uint8_t AesKey[17] = {0};
const uint8_t *extraData = "CL123456";



int TfsGetId2(uint8_t id[TFS_ID2_LEN])
{
	if (GM510SendCmdnospace("AT+CTFSGETID\r", "OK", 10, 200, id)==0) {
		CL_LOG("tfs get id2 ok(%d): %s.\n", TFS_ID2_LEN, id);
		return CL_OK;
	}
    OptFailNotice(31);
	CL_LOG("tfs get id2 error.\n");
	return CL_FAIL;
}


int TfsId2Decrypt(const uint8_t *cipherText, uint8_t cipherLen, uint8_t *out)
{
	char decryptCmdReq[256] = {0};
	char tmp[256]={0};
	char tmp1[2]={0};

	for (int i=0; i<cipherLen; i++) {
		memset(&tmp1, 0, 1);
		sprintf((char*)tmp1, "%02x", cipherText[i]);
		strcat(tmp, tmp1);
	}
	sprintf(decryptCmdReq,"AT+CTFSDECRYPT=\"%s\"\r", tmp);
	if (GM510SendCmdnospace(decryptCmdReq, "OK", 10, 200, out)==0) {
		CL_LOG("tfs id2 decrypt success.\n");
		return CL_OK;
	}
	CL_LOG("tfs id2 decrypt fail.\n");
	return CL_FAIL;
}

int TfsId2GetTimeStampAuthCode(uint64_t timestamp, uint8_t *extra, uint8_t extraLen, uint8_t *out)
{
	char authCmdReq[256] = {0};
	uint8_t mode = 1;
    uint8_t timestamp_str[20 + 1] = {0};

	sprintf((void*)timestamp_str, "%llu", timestamp);
	sprintf(authCmdReq,"AT+CTFSAUTH=%d,\"%s\",\"%s\"\r", mode, timestamp_str, extra);
	if (GM510SendCmdnospace(authCmdReq, "OK", 10, 200, out)==0) {
		CL_LOG("id2 get timestamp auth code(%d): %s.\n", TFS_TIMESTAMP_AUTH_CODE_LEN, out);
		return CL_OK;
	}

	CL_LOG("tfs id2 get timestamp auth code fail.\n");
	return CL_FAIL;
}


