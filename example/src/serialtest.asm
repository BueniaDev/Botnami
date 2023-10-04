	cpu 6809

	include "lib/hardware.inc"
	include "lib/vectors.inc"

	org 0
inputbuffer	rmb 256
outputbuffer	rmb 256

	org $C000
	include "lib/serial.asm"

greetingmsg
	fcc 'Hello from Karen-chan!'
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
	bra end

end:
	sync

swi_intr:
	sync
	