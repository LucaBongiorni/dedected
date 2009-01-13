#ifndef SC14421_FIRMWARE_H
#define SC14421_FIRMWARE_H

extern unsigned char sc14421_II_sniff_scan_fw[509];
/* sc14421_II_sniff_scan.asm-Includefile für C-Programm */
/* Ende Includefile für C-Programm */
extern unsigned char sc14421_II_sniff_sync_fw[509];
/* sc14421_II_sniff_sync.asm-Includefile für C-Programm */
#define PP0 0x4
#define PP2 0x6
#define PP4 0x8
#define PP6 0xA
#define PP8 0xC
#define PP10 0xE
#define PP12 0x11
#define PP14 0x13
#define PP16 0x15
#define PP18 0x17
#define PP20 0x19
#define PP22 0x1B
#define JP0 0x3
#define JP2 0x5
#define JP4 0x7
#define JP6 0x9
#define JP8 0xB
#define JP10 0xD
#define JP12 0x10
#define JP14 0x12
#define JP16 0x14
#define JP18 0x16
#define JP20 0x18
#define JP22 0x1A
#define sync_label_D1 0xC8
#define sync_label_D4 0xCB
#define sync_label_28 0x1F
/* Ende Includefile für C-Programm */

#endif
