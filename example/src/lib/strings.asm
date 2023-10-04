nibtoaschex:
	anda #$0F
	adda #$30
	cmpa #$39
	ble nibtoaschexout
	adda #$07
nibtoaschexout:
	sta, x+
	rts

bytetoaschex:
	pshs a
	lsra
	lsra
	lsra
	lsra
	bsr nibtoaschex
	puls a
	bsr nibtoaschex
	rts

wordtoaschex:
	pshs b
	bsr bytetoaschex
	puls a
	bsr bytetoaschex
	rts

aschextonib:
	lda ,x+
	suba $30
	cmpa $10
	blo aschextonibout
	suba $07
	cmpa $10
	blo aschextonibout
	suba $20
aschextonibout:
	rts

aschextobyte:
	leas -1,s
	bsr aschextonib
	lsla
	lsla
	lsla
	lsla
	sta ,s
	bsr aschextonib
	ora, s
	leas 1,s
	rts

aschextoword:
	bsr aschextobyte
	tfr a,b
	bsr aschextobyte
	exg a,b
	rts

skipspaces:
	lda, x+
	cmpa #$20
	beq skipspaces
	leax -1,x
	rts

parseinput:
	ldx #inputbuffer+1
	ldy #parambuffer
nextparseinput:
	lda, x
	beq parseinputout
	bsr skipspaces
	lda, x
	cmpa #$22
	beq parsestring
	lda 2,x
	beq parsebyte
	cmpa #$20
	beq parsebyte
	lda 4,x
	beq parseword
	cmpa #$20
	beq parseword
	bra parseinputout
parsebyte:
	lda #1
	sta, y+
	bsr aschextobyte
	sta, y+
	bra nextparseinput
parseword:
	lda #2
	sta, y+
	bsr aschextoword
	std, y++
	bra nextparseinput
parsestring:
	lda #3
	sta, y+
	leax 1,x
stringloop:
	lda, x+
	beq parsestringout
	cmpa #$22
	beq parsestringout
	sta, y+
	bne stringloop
parsestringout:
	clr, y+
	bra nextparseinput
parseinputout:
	clr, y+
	ldy #parambuffer
	rts

concatstr:
	lda, y+
	beq concatstrout
	sta, x+
	bra concatstr
concatstrout:
	rts