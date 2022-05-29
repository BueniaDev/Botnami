TEST(DecA, "BotnamiTest.M6809.Zero")
{
    BotnamiTest test;
    
    test.init({
	0x86, 0x01, // LDA $01
	0x4A, // DECA
    });
    botassert::is_false(test.getCore().is_zero());

    test.run(2);
    botassert::is_true(test.getCore().is_zero());
    test.shutdown();
}

TEST(DecB, "BotnamiTest.M6809.Zero")
{
    BotnamiTest test;
    
    test.init({
	0xC6, 0x01, // LDB $01
	0x5A, // DECB
    });
    botassert::is_false(test.getCore().is_zero());

    test.run(2);
    botassert::is_true(test.getCore().is_zero());
    test.shutdown();
}

TEST(AddA, "BotnamiTest.M6809.Zero")
{
    BotnamiTest test;
    
    test.init({
	0x86, 0xFF, // LDA $FF
	0x8B, 0x01, // ADDA #1
    });
    botassert::is_false(test.getCore().is_zero());

    test.run(2);
    botassert::is_true(test.getCore().is_zero());
    test.shutdown();
}

TEST(CmpA, "BotnamiTest.M6809.Zero")
{
    BotnamiTest test;
    
    test.init({
	0x86, 0x00, // LDA $00
	0x81, 0x00, // CMPA %00
    });
    botassert::is_false(test.getCore().is_zero());

    test.run(2);
    botassert::is_true(test.getCore().is_zero());
    test.shutdown();
}


