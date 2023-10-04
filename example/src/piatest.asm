	cpu 6809

	include "lib/hardware.inc"
	include "lib/vectors.inc"


	org $C000

	include "lib/spi.asm"

main:
	lds #ramend+1
	ldx #ramstart
zeroram:
	clr ,x+
	cmpx #ramend+1
	bne zeroram

	lbsr spiinit
	sync


swi_intr:
	sync