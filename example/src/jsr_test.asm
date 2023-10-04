	cpu 6809
	include "lib/vectors.inc"

	org $C000

main:
	ldy #testbr
	jsr, y

testbr:
	sync


swi_intr:
	sync