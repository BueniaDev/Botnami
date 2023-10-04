
serialinit:
	lda #%00000011 ; Master reset
	sta serialcr
	; Divider = 16, Databits = 8n1, no rts and no interrupts
	lda #%00010101
	sta serialcr
	rts

serialputchar:
	ldb serialsr
	andb #%00000010 ; Transmit empty
	beq serialputchar ; Wait for port to be idle
	sta serialtx ; Output the char
	rts

serialputstr:
	lda ,x+ ; Fetch the next character
	beq serialputstrout ; If null character is found, exit
	bsr serialputchar ; Output the character
	bra serialputstr ; More characters
serialputstrout:
	rts

serialgetchar:
	lda serialsr
	anda #%00000001
	beq serialgetchar
	lda serialrx
	bsr serialputchar
	rts

serialgetstr:
	bsr serialgetchar
	cmpa #cr
	beq serialgetstrout
	cmpa #lf
	beq serialgetstrout
	sta ,x+
	bra serialgetstr
serialgetstrout:
	clr ,x+
	rts