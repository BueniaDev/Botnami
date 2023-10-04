	cpu 6809

	include "lib/vectors.inc"

	org $C000

ram	equ $0000
ramend 	equ $7FFF
led	equ $8000

main:
	lds #ramend
	bsr led_off
	ldb #80
	bsr delay
	bsr led_on
	bsr delay
	bra end

led_on:
	lda #1
	sta led
	rts

led_off:
	lda #0
	sta led
	rts

delay:
	decb
	bne delay
	rts

end:
	sync

swi_intr:
	sync