	cpu 6809
	include "lib/hardware.inc"
	include "lib/vectors.inc"

	org 0
userregs	rmb 11
inputbuffer	rmb 256
outputbuffer	rmb 256
parambuffer	rmb 256
dumpreadpointer	rmb 2


	org $C000

	include "lib/serial.asm"
	include "lib/strings.asm"

greetingmsg
	fcc 'Kujo6809 Monitor v0.1'
	fcb cr
	fcb lf
	fcb cr
	fcb lf
	fcb null
newlinemsg
	fcb cr
	fcb lf
	fcb null
breakatmsg
	fcc '***Break at '
	fcb null
promptmsg
	fcc 'Monitor: > '
	fcb null
youtypedmsg
	fcc 'You typed: '
	fcb null
nosuchmsg
	fcc 'No such command'
	fcb cr
	fcb lf
	fcb null
commfailedmsg
	fcc 'Command failed, possibly bad syntax...'
	fcb cr
	fcb lf
	fcb null
badexitmsg
	fcc 'Internal error, exiting monitor...'
	fcb cr
	fcb lf
	fcb null

commandarray
	fdb showhelp
	fcc 'h'
	fdb showhelp
	fcc '?'
	fdb resetmonitor
	fcc 'x'
	fdb latchout
	fcc 'c'

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
	swi

; We should never get here - it means that rti was called without patching up the return address, so print error and loop
	ldx #badexitmsg
	lbsr serialputstr
badexitloop:
	bra badexitloop

swi_intr:
	ldx #outputbuffer
	ldy #breakatmsg
	lbsr concatstr
	leay, s
	ldd 10,y
	lbsr wordtoaschex
	ldy #newlinemsg
	lbsr concatstr
	clr ,x+
	ldx #outputbuffer
	lbsr serialputstr

mainloop:
	ldx #promptmsg
	lbsr serialputstr

	ldx #inputbuffer
	lbsr serialgetstr

	ldx #outputbuffer

	ldy #newlinemsg
	lbsr concatstr

	ldy #youtypedmsg
	lbsr concatstr
	ldy #inputbuffer
	lbsr concatstr
	ldy #newlinemsg
	lbsr concatstr
	clr, x+

	ldx #outputbuffer
	lbsr serialputstr

	lda inputbuffer
	ldx #commandarray
nextcommand:
	ldy, x++
	ldb, x
	beq commandarrayend
	cmpa, x+
	bne nextcommand
	jsr, y
	bne commanderror
	bra mainloop
commanderror:
	ldx #commfailedmsg
	lbsr serialputstr
	bra mainloop

commandarrayend:
	ldx #nosuchmsg
	lbsr serialputstr
	bra mainloop

generalerror:
	lda #1
	rts

helpmsg
	fcc 'Commands:'
	fcb cr
	fcb lf
	fcc '  x : Reset the monitor'
	fcb cr
	fcb lf
	fcb '  c OO : Output OO on the debug latch'
	fcb cr
	fcb lf
	fcc '  h or ? : Show this help guide'
	fcb cr
	fcb lf
	fcb cr
	fcb lf
	fcb null

showhelp:
	ldx #greetingmsg
	lbsr serialputstr
	ldx #helpmsg
	lbsr serialputstr
	clra
	rts

latchout:
	lbsr parseinput
	lda ,y+
	cmpa #1
	lbne generalerror

	lda ,y+
	sta latch

	clra
	rts

resetmonitor:
	ldx $fffe
	jmp, x