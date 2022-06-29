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

TEST(AddAImm, "BotnamiTest.M6809.Arithmetic")
{
    // TODO: Implement tests for carry, overflow, and half-carry flags
    BotnamiTest test;

    vector<uint8_t> expected_values;

    for (int i = 1; i < 256; i++)
    {
	expected_values.push_back(i);
    }

    for (int i = 0; i < 5; i++)
    {
	expected_values.push_back(i);
    }

    vector<uint8_t> code;

    for (int i = 0; i < 260; i++)
    {
	// ADDA #1
	code.push_back(0x8B);
	code.push_back(0x01);
    }

    test.init(code, 0x1000);

    for (int i = 0; i < 260; i++)
    {
	test.run();
	auto status = test.getCore().getStatus();
	uint8_t expected_value = expected_values.at(i);

	botassert::eq(expected_value, status.rega);

	if (expected_value == 0)
	{
	    botassert::is_true(test.getCore().is_zero());
	}
	else
	{
	    botassert::is_false(test.getCore().is_zero());
	}

	if ((expected_value >= 128) && (expected_value <= 255))
	{
	    botassert::is_true(test.getCore().is_sign());
	}
	else
	{
	    botassert::is_false(test.getCore().is_sign());
	}

	if (expected_value == 128)
	{
	    botassert::is_true(test.getCore().is_overflow());
	}
	else
	{
	    botassert::is_false(test.getCore().is_overflow());
	}

	if (expected_value == 0)
	{
	    botassert::is_true(test.getCore().is_carry());
	}
	else
	{
	    botassert::is_false(test.getCore().is_carry());
	}

	if ((expected_value % 16) == 0)
	{
	    botassert::is_true(test.getCore().is_half());
	}
	else
	{
	    botassert::is_false(test.getCore().is_half());
	}
    }

    test.shutdown();
}

TEST(EncTest, "BotnamiTest.Konami1")
{
    BotnamiKonami1Test test;

    test.init({
	0x5D, 0x10, 0x87, // CLR $1087
	0x92, 0xEC, 0x1F, 0x80, // LDS #$1F80
	0x04, 0x28, // LDA #$28
	0x37, 0x8B, // TFR A, DP
	0xF7, 0x10, 0x80, // CLR $1080
	0xD7, // CLRB
	0x3F, 0x10, 0x00 // STA $1000
    }, 0);

    vector<int> addresses = 
    {
	0x0, 0x3, 0x4, 0x7,
	0x9, 0xB, 0xE, 0xF
    };

    vector<uint8_t> expected_opcodes = 
    {
	0x7F, 0x10, 0xCE, 0x86,
	0x1F, 0x7F, 0x5F, 0xB7
    };

    for (int i = 0; i < 8; i++)
    {
	uint8_t opcode = test.getCore().readOpcode(addresses.at(i));
	botassert::eq(opcode, expected_opcodes.at(i));
    }

    test.shutdown();
}

TEST(LslATest, "Botnami.Konami2.Shift")
{
    BotnamiKonami2Test test;

    vector<uint8_t> code;

    for (int i = 0; i < 260; i++)
    {
	code.push_back(0x10);
	code.push_back((i & 0xFF));
	code.push_back(0x9C);
    }

    test.init(code, 0x1000);

    for (int i = 0; i < 260; i++)
    {
	test.run(2);
	auto status = test.getCore().getStatus();

	uint8_t expected_value = ((i << 1) & 0xFF);

	botassert::eq(expected_value, status.rega);

	if (expected_value == 0)
	{
	    botassert::is_true(test.getCore().is_zero());
	}
	else
	{
	    botassert::is_false(test.getCore().is_zero());
	}

	if ((expected_value >= 128) && (expected_value <= 255))
	{
	    botassert::is_true(test.getCore().is_sign());
	}
	else
	{
	    botassert::is_false(test.getCore().is_sign());
	}

	if ((i >= 64) && (i <= 191))
	{
	    botassert::is_true(test.getCore().is_overflow());
	}
	else
	{
	    botassert::is_false(test.getCore().is_overflow());
	}

	if ((i >= 128) && (i <= 255))
	{
	    botassert::is_true(test.getCore().is_carry());
	}
	else
	{
	    botassert::is_false(test.getCore().is_carry());
	}
    }

    test.shutdown();
}

