	cpu 6809

	include "lib/hardware.inc"
	include "lib/vectors.inc"

	org 0
inputbuffer	rmb 256
outputbuffer	rmb 256

	org $C000

	include "lib/serial.asm"
	include "lib/strings.asm"

greetingmsg
	fcc 'KujoSerial test'
	fcb cr
	fcb lf
	fcb cr
	fcb lf
	fcb null
promptmsg
	fcc '> '
	fcb null
typedmsg
	fcc 'You typed: '
	fcb null
newlinemsg
	fcb cr
	fcb lf
	fcb null

main:
	lds #ramend+1
	ldx #ramstart
zeroram:
	clr ,x+
	cmpx #ramend+1
	bne zeroram

	lbsr serialinit
	ldx #greetingmsg
	lbsr serialputstr
mainloop:
	ldx #promptmsg
	lbsr serialputstr
	ldx #inputbuffer
	lbsr serialgetstr
	ldx #outputbuffer
	ldy #newlinemsg
	lbsr concatstr
	ldy #typedmsg
	lbsr concatstr
	ldy #inputbuffer
	lbsr concatstr
	ldy #newlinemsg
	lbsr concatstr
	clr ,x+

	ldx #outputbuffer
	lbsr serialputstr
	
	bra mainloop

swi_intr:
	sync
	