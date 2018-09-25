#ifndef _SMBIOS_H

#define _SMBIOS_H

#include "data.h"

#ifdef __DOS__

#include <dos.h>

#else

#define far

#endif
#define SIGNATURE_16(A, B)        ((A) | (B << 8))
#define SIGNATURE_32(A, B, C, D)  (SIGNATURE_16 (A, B) | (SIGNATURE_16 (C, D) << 16))

typedef struct PnP_ICDS 
{                                         
	char Signature[4];
	char Structure_Version;
	char Structure_Length;
	unsigned  BIOS_Control_Field;
	char Structure_Checksum;
	void  far *Event_NF_Addr;
	unsigned short RM_EP_Offset;          
	unsigned short RM_EP_Segment;
	unsigned short PM_EP_Offset;
	unsigned long PM_EP_Segment;
	unsigned long OEM_Device_ID;
	unsigned short RM_BIOS_Data_Segment;
	unsigned long PM_BIOS_Data_Segment;
}PnPICDS;

typedef struct smbios_structure_header {
	uint8_t type;
	uint8_t length;
	uint16_t handle;
} SMBiosHeader;

typedef struct smbios_type_1 {
	struct smbios_structure_header header;
	uint8_t manufacturer_str;
	uint8_t product_name_str;
	uint8_t version_str;
	uint8_t serial_number_str;
	uint8_t uuid[16];
	uint8_t wake_up_type;
	uint8_t sku_number_str;
	uint8_t family_str;
} SMBiosType1;

typedef struct smbios_type_2 {
	struct smbios_structure_header header;
	uint8_t manufacturer_str;
	uint8_t product_name_str;
	uint8_t version_str;
	uint8_t serial_number_str;
	uint8_t assert_tag;
	uint8_t feature_flag;
	uint8_t location_in_chassis;
	uint16_t chassis_handle;
	uint8_t board_type;
	uint8_t contained_object_handle_count;
	/*紧跟着被包含的Object Handle*/
} SMBiosType2;

typedef struct smbios_type_3 {
	struct smbios_structure_header header;
	uint8_t manufacturer_str;
	uint8_t type;
	uint8_t version_str;
	uint8_t serial_number_str;
	uint8_t asset_tag_number_str;
	uint8_t boot_up_state;
	uint8_t power_supply_state;
	uint8_t thermal_state;
	uint8_t security_status;
	uint32_t oem_defined;
	uint8_t height;
	uint8_t number_of_power_cords;
	uint8_t contained_element_count;
	/*紧跟着被包含的元素*/
} SMBiosType3;

typedef struct bios_set_struct {
	unsigned char command;
	unsigned char field_ofs;
	unsigned long change_mask;
	unsigned long change_value;	
	unsigned data_length;
	SMBiosHeader header;
	void           *structureData;
} SMBiosSetValue;

typedef struct _PNP_FAR_PTR {
	u16                                   Offset;
	u16                                   Segment;
} PNP_FAR_PTR;

typedef struct {
	u16                                   PushedBp;
	PNP_FAR_PTR                           ReturnAddress;
	s16                                   Function;
} PNP_GENERIC_ENTRY_FRAME;

typedef struct _PNP_FUNCTION_0x50_FRAME {
	PNP_GENERIC_ENTRY_FRAME               GenericEntryFrame;
	unsigned char  far *dmiBIOSRevision;
	unsigned short far *NumStructures;
	unsigned short far *StructureSize;
	unsigned long  far *dmiStorageBase;
	unsigned short far *dmiStorageSize;
	u16                                   BiosSelector;
} PNP_FUNCTION_0x50_FRAME;

typedef struct _PNP_FUNCTION_0x54_FRAME {
	PNP_GENERIC_ENTRY_FRAME               GenericEntryFrame;
	s16                                   SubFunction;
	void far *                           Data;
	u8                                    Control;
	u16                                   DmiSelector;
	u16                                   BiosSelector;
} PNP_FUNCTION_0x54_FRAME;

typedef short far (*smbios_func50) ( short Function, unsigned char  far *dmiBIOSRevision,  unsigned short far *NumStructures,
		unsigned short far *StructureSize,  unsigned long  far *dmiStorageBase,  
		unsigned short far *dmiStorageSize,   unsigned short    BiosSelector );

typedef short far (*smbios_func51) (  short Function, unsigned short far *Structure, 
		unsigned char far  *dmiStrucBuffer, unsigned short     dmiSelector, 
		unsigned short     BiosSelector );

typedef short far (*smbios_func52) (   short Function,  unsigned char far *dmiDataBuffer, unsigned char far *dmiWorkBuffer, 
		unsigned char     Control,  unsigned short    dmiSelector,  unsigned short    BiosSelector  );

typedef short far (*smbios_func54) (   short Function,  short subfunc, void far *data, 	unsigned char     Control,  
		unsigned short    dmiSelector,  unsigned short    BiosSelector  );

int getPnPInfo(PnPICDS *pnpicds);

int getSMBiosInfo(const PnPICDS *PnP, unsigned char  *dmiBIOSRevision,    unsigned short *numStructures, unsigned short *structureSize, 
		unsigned long   *dmiStorageBase, unsigned short *dmiStorageSize, unsigned short *biosSelector);

int getSMBiosInfo1(/*const PnPICDS *PnP, */unsigned char  *dmiBIOSRevision,    unsigned short *numStructures, unsigned short *structureSize, 
		unsigned long   *dmiStorageBase, unsigned short *dmiStorageSize, unsigned short *biosSelector);

int getBiosStructrue(const PnPICDS *PnP, unsigned char  far *dmiStrucBuffer, unsigned char type, unsigned long   dmiStorageBase,
		unsigned short biosSelector);

int setBiosStructrue ( const PnPICDS *PnP, const  SMBiosHeader *head, const void  *value, int size, unsigned char type, unsigned long   dmiStorageBase,
		unsigned long   dmiStorageSize,unsigned short biosSelector, int command );

int controlBios(const PnPICDS *PnP, short subfunc, void far *data, 	unsigned char     control,  
	 	unsigned short    dmiSelector,  unsigned short    biosSelector);

int controlBios1(unsigned short dmiSelector, unsigned short biosSelector); 

int clearBiosEventLog();

int readDataFormBios(char *result, int type, unsigned char offset);

int writeDataToBios(const void *data, int type, int command, unsigned char offset);

void event_clear(PNP_FUNCTION_0x54_FRAME *Frame,u32  Signature,u32* Result);
int getBiosByHb(PNP_FUNCTION_0x50_FRAME *Frame,u32  Signature,u32* Result);

#endif
