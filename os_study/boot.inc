;------------------------------------------loader and kernel----------------------------------------------
LOADER_BASE_ADDR equ 0x900
LOADER_START_SECTOR equ 0x2

;--------------------------------------gdt descriptor properties------------------------------------------
;31                   24  23  22  21  20 19        16  15 14  13  12 11         8 7                      0
;---------------------------------------------------------------------------------------------------------|
;      base 31-24       | G |D/B| L |AVL|limit 19-16 | P |  DPL | S |    TYPE    |       base 23-16       |
;---------------------------------------------------------------------------------------------------------|
;                     base 15-0                      |                  limit 15-0                        |
;---------------------------------------------------------------------------------------------------------|
DESC_G_4K               equ 0x800000
DESC_D_32               equ 0x400000
DESC_L                  equ 0x200000
DESC_AVL                equ 0           ;cpu不用此位，置为0
DESC_LIMIT_CODE2        equ 0x0f0000
DESC_LIMIT_DATA2        equ DESC_LIMIT_CODE2
DESC_LIMIT_VIDEO2       equ 0
DESC_P              equ 0x8000
DESC_DPL_0          equ 0
DESC_DPL_1          equ 0x2000
DESC_DPL_2          equ 0x4000
DESC_DPL_3          equ 0x6000
DESC_S_CODE         equ 0x1000
DESC_S_DATA         equ DESC_S_CODE
DESC_S_SYS          equ 0
;x=1可执行，c=0非一致性，r=0不可读，a=0非已访问
DESC_TYPE_CODE      equ 0x0800
;x=0非可执行，e=0向上扩展，w=1可写，a=0非已访问
DESC_TYPE_DATA      equ 0x0200
DESC_CODE_HIGH4     equ DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_CODE2 + DESC_P + DESC_DPL_0 + DESC_S_CODE + DESC_TYPE_CODE
DESC_DATA_HIGH4     equ DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_DATA2 + DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA
DESC_VIDEO_HIGH4    equ DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_VIDEO2 + DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA
;--------------------------------------------selector properties-------------------------------------------
RPL0                equ 0
RPL1                equ 1
RPL2                equ 2
RPL3                equ 3
TI_GDT              equ 0
TI_LDT              equ 4
