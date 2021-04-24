; Modified of xtide universal bios to initialize ISA Plug and Play SB AWE 64 IDE so
; one can boot from the sound card.
;
; This uses static logical device numbers etc. spesific to my card. These have been
; checked with pnpdump.exe in dos prior to building this file.
;
; After compiling this source with nasm, one must run xtidecfg.com to update the
; checksums so the ROM extension will run. If using some other version of xtide
; ROM, one must make sure the jump addresses are ok for that particular rom, and
; also make sure that our added code will be placed in an empty region so it
; doesn't interfere with the normal function of the ROM.

	incbin  "ide_xt.bin", 0, 3
	jmp     0x19F0
	incbin  "ide_xt.bin", $, 0x19F0-6

%macro outp 2
	mov     dx, %1
	mov     al, %2
	out     dx, al
%endmacro

	; WaitForKey();
	outp    0x279, 2
	outp    0xA79, 2

	; SendKey();
	outp    0x279, 0
	outp    0x279, 0
	outp    0x279, 0x6A
	outp    0x279, 0xB5
	outp    0x279, 0xDA
	outp    0x279, 0xED
	outp    0x279, 0xF6
	outp    0x279, 0xFB
	outp    0x279, 0x7D
	outp    0x279, 0xBE
	outp    0x279, 0xDF
	outp    0x279, 0x6F
	outp    0x279, 0x37
	outp    0x279, 0x1B
	outp    0x279, 0x0D
	outp    0x279, 0x86
	outp    0x279, 0xC3
	outp    0x279, 0x61
	outp    0x279, 0xB0
	outp    0x279, 0x58
	outp    0x279, 0x2C
	outp    0x279, 0x16
	outp    0x279, 0x8B
	outp    0x279, 0x45
	outp    0x279, 0xA2
	outp    0x279, 0xD1
	outp    0x279, 0xE8
	outp    0x279, 0x74
	outp    0x279, 0x3A
	outp    0x279, 0x9D
	outp    0x279, 0xCE
	outp    0x279, 0xE7
	outp    0x279, 0x73
	outp    0x279, 0x39

	; Wake(0);
	outp    0x279, 3
	outp    0xA79, 0

	; WriteCsn(1);
	outp    0x279, 6
	outp    0xA79, 1

	; Wake(1);
	outp    0x279, 3
	outp    0xA79, 1

	; WriteByte(7, 3); // select logical device 3
	outp    0x279, 7
	outp    0xA79, 3

	; reg[0x30] = 0x01;
	outp    0x279, 0x30
	outp    0xA79, 0x01

	; reg[0x60] = 0x0170;
	outp    0x279, 0x61
	outp    0xA79, 0x70
	outp    0x279, 0x60
	outp    0xA79, 0x01

	; reg[0x62] = 0x0376;
	outp    0x279, 0x63
	outp    0xA79, 0x76
	outp    0x279, 0x62
	outp    0xA79, 0x03

	jmp     0x0432

	incbin  "ide_xt.bin", $
