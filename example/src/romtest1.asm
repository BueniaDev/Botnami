	cpu 6809

	include "lib/vectors.inc"

	org $C000

main:
	lda #0
	sta $8000
	ldb #$80
delay1:
	decb
	bne delay1
	lda #1
	sta $8000
	ldb #$80
delay2:
	decb
	bne delay2
	bra end

end:
	sync

swi_intr:
	sync
	
	