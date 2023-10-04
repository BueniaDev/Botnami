	cpu 6809

	include "lib/vectors.inc"

	org $C000

main:
    lds #$7FFE
    lda #$0D
    bsr test
    sync

test:
    cmpa #$0D
    beq testout
    cmpa #$0A
    beq testout
    sta, x+
testout:
    clr, x+
    rts

swi_intr:
	sync