#include "smbios.h"
#include "base.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MaxType 18

int getPnPInfo(PnPICDS * pnpicds)
{

	unsigned char data[0x22], sum = 0;
	unsigned char far *bios;
	unsigned i, j;
	int find = 0;

	bios = (unsigned char far *) (MK_FP(0xf000, 0));
	i = 0;

	while (find == 0 && i < 0xfff0) {
		if (bios[i] == '$' && bios[i + 1] == 'P'
		    && bios[i + 2] == 'n' && bios[i + 3] == 'P') {
			for (j = 0; j < 0x22; j++)
				sum += bios[i + j];

			find = 1;

		} else
			i += 16;
	}

	if (find == 1) {
		for (j = 0; j < 0x22; j++)
			data[j] = bios[i + j];

		memcpy(pnpicds, data, 0x22);
	}

	if (pnpicds->Signature[0] != '$' || pnpicds->Signature[1] != 'P'
	    || pnpicds->Signature[2] != 'n'
	    || pnpicds->Signature[3] != 'P') {
		strcpy(errorInfo,
		       "can not find 'pnp bios' data structure");
		return -1;
	}

	return find;
}

int getBiosByHb(PNP_FUNCTION_0x50_FRAME * frame, u32 signature,
		u32 * result)
{
	__asm__ __volatile__(
				    //PUSH_REGS
				    "mov $0x47, %%eax\n\t"
				    "mov $0x82f, %%edx\n\t"
				    "out %%al, %%dx\n\t"
				    "xor %%ebx, %%ebx\n\t"
				    "mov %%ax, %%bx\n\t"
				    "mov %%ebx, (%%edi)\n\t"
				    //POP_REGS
				    ::"b"(signature), "D"(result),
				    "S"(frame)
				    :"eax", "memory");
	return 1;
}

int getSMBiosInfo(const PnPICDS * PnP, unsigned char *dmiBIOSRevision,
		  unsigned short *numStructures,
		  unsigned short *structureSize,
		  unsigned long *dmiStorageBase,
		  unsigned short *dmiStorageSize,
		  unsigned short *biosSelector)
{
	short rlt;
	char str[10];
	smbios_func50 func50;

	*biosSelector = PnP->RM_BIOS_Data_Segment;

	func50 =
	    (smbios_func50) MK_FP(PnP->RM_EP_Segment, PnP->RM_EP_Offset);

	rlt = func50(0x50, dmiBIOSRevision, numStructures, structureSize,
		     dmiStorageBase, dmiStorageSize, *biosSelector);

	if (rlt != 0x00) {
		strcpy(errorInfo,
		       "smbios function 0x50 call error,error number is:");
		sprintf(str, "%d", rlt);
		strcat(errorInfo, str);
		return 1;
	}

	return 0;

}

int getSMBiosInfo1( /* const PnPICDS *PnP, */ unsigned char
		   *dmiBIOSRevision, unsigned short *numStructures,
		   unsigned short *structureSize,
		   unsigned long *dmiStorageBase,
		   unsigned short *dmiStorageSize,
		   unsigned short *biosSelector)
{
	u32 rlt = 0;
	char str[10] = { 0 };

	PNP_FUNCTION_0x50_FRAME biosInfoFunc;
	memset(&biosInfoFunc, 0, sizeof(biosInfoFunc));
	biosInfoFunc.BiosSelector = (u32) biosSelector;
	biosInfoFunc.dmiBIOSRevision = dmiBIOSRevision;
	biosInfoFunc.NumStructures = numStructures;
	biosInfoFunc.StructureSize = structureSize;
	biosInfoFunc.dmiStorageBase = dmiStorageBase;
	biosInfoFunc.dmiStorageSize = dmiStorageSize;
	biosInfoFunc.GenericEntryFrame.Function = 0x00000050;

	getBiosByHb(&biosInfoFunc, SIGNATURE_32('$', 'I', 'S', 'B'), &rlt);

	if (rlt != 0) {
		strcpy(errorInfo, "smbios function 0x50 call error : ");
		sprintf(str, "0x%x", rlt);
		strcat(errorInfo, str);
		return 1;
	}

	return 0;

}

int getBiosStructrue(const PnPICDS * PnP,
		     unsigned char far * dmiStrucBuffer,
		     unsigned char type, unsigned long dmiStorageBase,
		     unsigned short biosSelector)
{
	unsigned char typeNo;

	short rlt;

	unsigned short structureNo;
	unsigned short dmiSelector;
	char str[10];
	smbios_func51 func51;

	func51 =
	    (smbios_func51) MK_FP(PnP->RM_EP_Segment, PnP->RM_EP_Offset);

	dmiSelector = (unsigned short) (dmiStorageBase >> 4);

	structureNo = 0;

	while ((structureNo) != 0xffff) {
		rlt =
		    func51(0x51, &structureNo, dmiStrucBuffer, dmiSelector,
			   biosSelector);

		if (rlt != 0) {
			strcpy(errorInfo,
			       "smbios function 0x51 call error,error number is:");
			sprintf(str, "%d", rlt);
			strcat(errorInfo, str);
			return 1;
		}

		typeNo = (unsigned char) dmiStrucBuffer[0];

		if (typeNo < MaxType && typeNo == type) {
			break;
		}
	}

	if (typeNo != type) {
		strcpy(errorInfo,
		       "smbios function 0x51 call error,no such bios type:");
		sprintf(str, "%d", type);
		strcat(errorInfo, str);
		return 1;

	} else
		return 0;
}

int setBiosStructrue(const PnPICDS * PnP, const SMBiosHeader * head,
		     const void *value, int size, unsigned char type,
		     unsigned long dmiStorageBase,
		     unsigned long dmiStorageSize,
		     unsigned short biosSelector, int command)
{

	unsigned short dmiSelector;
	unsigned char *dmiWorkBuffer = 0;
	char str[10];
	SMBiosSetValue *setValueBuffer;
	int control = 1;
	int rlt;
	smbios_func52 func52;

	if (command == 5) {
		setValueBuffer =
		    (SMBiosSetValue *) malloc(sizeof(SMBiosSetValue) +
					      size);
		strcpy(setValueBuffer->structureData,
		       (const char *) value);
		setValueBuffer->data_length = size - 1;
	} else if (command == 6) {
		setValueBuffer =
		    (SMBiosSetValue *) malloc(sizeof(SMBiosSetValue) +
					      size);
		setValueBuffer->data_length = size;
		memcpy(setValueBuffer->structureData, value, size);
	} else {
		strcpy(errorInfo,
		       "smbios function 0x52 call error,not implemented:");
		sprintf(str, "%d", command);
		strcat(errorInfo, str);
		return 1;
	}

	dmiWorkBuffer = (unsigned char *) malloc(dmiStorageSize);
	dmiSelector = (unsigned short) (dmiStorageBase >> 4);

	setValueBuffer->command = command;
	setValueBuffer->change_mask = 0x00;
	setValueBuffer->change_value = 0;
	setValueBuffer->header = *head;

	/*TODO : field_ofs可能有问题,没明白什么意思 */
	setValueBuffer->field_ofs = 0x7;

	func52 =
	    (smbios_func52) MK_FP(PnP->RM_EP_Segment, PnP->RM_EP_Offset);

	rlt =
	    func52(0x52, (unsigned char *) setValueBuffer, dmiWorkBuffer,
		   control, dmiSelector, biosSelector);

	free(setValueBuffer);
	free(dmiWorkBuffer);

	if (rlt != 0) {
		strcpy(errorInfo,
		       "smbios function 0x52 call error,error number is:");
		sprintf(str, "%d", rlt);
		strcat(errorInfo, str);

		return 1;
	}

	return 0;
}

int controlBios(const PnPICDS * PnP, short subfunc, void far * data,
		unsigned char control, unsigned short dmiSelector,
		unsigned short biosSelector)
{
	int rlt;
	char str[10];
	smbios_func54 func54;

	func54 =
	    (smbios_func54) MK_FP(PnP->RM_EP_Segment, PnP->RM_EP_Offset);

	rlt =
	    func54(0x54, subfunc, data, control, dmiSelector,
		   biosSelector);

	if (rlt != 0) {
		strcpy(errorInfo,
		       "smbios function 0x54 call error,error number is:");
		sprintf(str, "%d", rlt);
		strcat(errorInfo, str);

		return 1;
	}
	return 0;
}

void event_clear(PNP_FUNCTION_0x54_FRAME * frame, u32 signature,
		 u32 * result)
{
	__asm__ __volatile__(
				    //PUSH_REGS
				    "mov $0x47, %%eax\n\t"
				    "mov $0x82f, %%edx\n\t"
				    "out %%al, %%dx\n\t"
				    "xor %%ebx, %%ebx\n\t"
				    "mov %%ax, %%bx\n\t"
				    "mov %%ebx, (%%edi)\n\t"
				    //POP_REGS
				    ::"b"(signature), "D"(result),
				    "S"(frame)
				    :"eax", "memory");
}

int controlBios1(u16 dmiSelector, u16 biosSelector)
{
	unsigned int rlt = 0;
	char str[10] = { 0 };

	PNP_FUNCTION_0x54_FRAME controlbios;
	memset(&controlbios, 0, sizeof(controlbios));

	controlbios.DmiSelector = dmiSelector;
	controlbios.BiosSelector = biosSelector;
	controlbios.Control = 1;
	controlbios.GenericEntryFrame.Function = 0x00000054;

	event_clear(&controlbios, SIGNATURE_32('$', 'I', 'S', 'B'), &rlt);
	if (rlt != 0) {
		strcpy(errorInfo,
		       "smbios function 0x54 call error,error number is:");
		sprintf(str, "0x%x", rlt);
		strcat(errorInfo, str);

		return 1;
	}
	return 0;
}

int readDataFormBios(char *result, int type, unsigned char offset)
{
	PnPICDS *pnp;
	int rst;

	unsigned char dmiBIOSRevision;
	unsigned short numStructures;
	unsigned short structureSize;
	unsigned long dmiStorageBase;
	unsigned short dmiStorageSize;
	unsigned short biosSelector;
	unsigned char far *dmiStrucBuffer;
	unsigned short dmiSelector;

	char str[17];

	SMBiosType1 *biosType1;
	SMBiosType2 *biosType2;
	SMBiosType3 *biosType3;

	pnp = (PnPICDS *) malloc(sizeof(PnPICDS));
	rst = getPnPInfo(pnp);

	if (rst != 1) {
		free(pnp);
		return 1;
	}

	rst =
	    getSMBiosInfo(pnp, &dmiBIOSRevision, &numStructures,
			  &structureSize, &dmiStorageBase, &dmiStorageSize,
			  &biosSelector);

	if (rst == 1) {
		free(pnp);
		return 1;
	}

	dmiSelector = (unsigned short) (dmiStorageBase >> 4);
	dmiStrucBuffer =
	    (unsigned char far *) malloc(sizeof(char) * (structureSize));

	rst =
	    getBiosStructrue(pnp, dmiStrucBuffer, type, dmiStorageBase,
			     biosSelector);

	if (rst == 1) {
		free(pnp);
		free(dmiStrucBuffer);
		return 1;
	}

	/* 获取结果 */
	/*  这里使用char*进行转换是否正确？ */
	if (type == 1) {
		biosType1 = (SMBiosType1 *) dmiStrucBuffer;
		if (offset == 5) {
			strcpy(result,
			       (char *) (u32) biosType1->product_name_str);
		} else if (offset == 7) {
			strcpy(result,
			       (char *) (u32)
			       biosType1->serial_number_str);
		} else if (offset == 6) {
			strcpy(result,
			       (char *) (u32) biosType1->version_str);
		} else if (offset == 8) {
			memcpy(str, biosType1->uuid, 16);
			str[16] = '\0';
			strcpy(result, str);
		} else {

		}

		if (type == 2) {
			biosType2 = (SMBiosType2 *) dmiStrucBuffer;
			if (offset == 6) {
				strcpy(result,
				       (char *) (u32)
				       biosType2->version_str);
			} else {

			}
		}

		if (type == 3) {
			biosType3 = (SMBiosType3 *) dmiStrucBuffer;
			if (offset == 7) {
				strcpy(result,
				       (char *) (u32)
				       biosType3->serial_number_str);
			} else if (offset == 8) {
				strcpy(result,
				       (char *) (u32)
				       biosType3->asset_tag_number_str);
			} else {

			}
		}
	}

	free(pnp);
	free(dmiStrucBuffer);
	return 0;
}

int writeDataToBios(const void *data, int type, int command,
		    unsigned char offset)
{
	PnPICDS *pnp;
	int rst;
	int len = 0;
	SMBiosHeader head;

	unsigned char dmiBIOSRevision;
	unsigned short numStructures;
	unsigned short structureSize;
	unsigned long dmiStorageBase;
	unsigned short dmiStorageSize;
	unsigned short biosSelector;
	unsigned char far *dmiStrucBuffer;
	unsigned short dmiSelector;

	pnp = (PnPICDS *) malloc(sizeof(PnPICDS));
	rst = getPnPInfo(pnp);

	if (rst != 1) {
		free(pnp);
		return 1;
	}

	rst =
	    getSMBiosInfo(pnp, &dmiBIOSRevision, &numStructures,
			  &structureSize, &dmiStorageBase, &dmiStorageSize,
			  &biosSelector);

	if (rst == 1) {
		free(pnp);
		return 1;
	}

	dmiSelector = (unsigned short) (dmiStorageBase >> 4);
	dmiStrucBuffer =
	    (unsigned char far *) malloc(sizeof(char) * (structureSize));

	rst =
	    getBiosStructrue(pnp, dmiStrucBuffer, type, dmiStorageBase,
			     biosSelector);

	if (rst == 1) {
		free(pnp);
		free(dmiStrucBuffer);
		return 1;
	}

	if (command == 5) {
		len = strlen((const char *) data) + 1;
	}
	if (command == 6 && type == 1) {
		len = 16;
	}

	memcpy(&head, dmiStrucBuffer, sizeof(SMBiosHeader));

	rst =
	    setBiosStructrue(pnp, &head, data, len, type, dmiStorageBase,
			     dmiStorageSize, biosSelector, command);

	free(pnp);
	free(dmiStrucBuffer);

	return rst;
}

int clearBiosEventLog()
{
	int rst;

	unsigned char dmiBIOSRevision = 0;
	unsigned short numStructures = 0;
	unsigned short structureSize = 0;
	unsigned long dmiStorageBase = 0;
	unsigned short dmiStorageSize = 0;
	unsigned short biosSelector = 0;
	unsigned short dmiSelector = 0;

#if 0
	PnPICDS *pnp;
	pnp = (PnPICDS *) malloc(sizeof(PnPICDS));
	rst = getPnPInfo(pnp);


	if (rst != 1) {
		free(pnp);
		return 1;
	}

	rst =
	    getSMBiosInfo(pnp, &dmiBIOSRevision, &numStructures,
			  &structureSize, &dmiStorageBase, &dmiStorageSize,
			  &biosSelector);

	if (rst == 1) {
		free(pnp);
		return 1;
	}
#endif

	rst =
	    getSMBiosInfo1(&dmiBIOSRevision, &numStructures,
			   &structureSize, &dmiStorageBase,
			   &dmiStorageSize, &biosSelector);
	if (rst)
		return -1;

	dmiSelector = (unsigned short) (dmiStorageBase >> 4);

	rst = controlBios1(dmiSelector, biosSelector);

	return rst;
}
