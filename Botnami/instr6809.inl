read();
memAccess();
instIncomplete();

switch (inst_state)
{
    case Reset:
    {
	setAddress(0xFFFF);

	ea_next = 0xFFFF;

	rega_next = 0;
	regb_next = 0;
	reg_cc_next = 0x50;
	read();
	memAccess();
	instIncomplete();
	int_type_next = None;
	setState(Reset0);
    }
    break;
    case Reset0:
    {
	setAddress(0xFFFE);
	reg_pc_next = ((reg_pc_next & 0xFF) | (getData() << 8));
	memAccess();
	instComplete();
	setState(Reset2);
    }
    break;
    case Reset2:
    {
	setAddress(0xFFFF);
	reg_pc_next = ((reg_pc_next & 0xFF00) | getData());
	memAccess();
	instComplete();
	setState(FetchI1);
    }
    break;
    case FetchI1:
    {
	// TODO: Implement interrupts and DMABREQ/HALT pins
	if (false)
	{
	}
	else
	{
	    addr_next = reg_pc;
	    reg_pc_next = (reg_pc + 1);

	    read();
	    mapInstr();
	    is_page2_next = false;
	    is_page3_next = false;
	    
	    if (instr1_next == 0x10)
	    {
		is_page2_next = true;
		memAccess();
		setState(FetchI1V2);
	    }
	    else if (instr1_next == 0x11)
	    {
		is_page3_next = true;
		memAccess();
		setState(FetchI1V2);
	    }
	    else
	    {
		memAccess();
		setState(FetchI2);
	    }
	}
    }
    break;
    case FetchI1V2:
    {
	addr_next = reg_pc;
	reg_pc_next = (reg_pc + 1);

	read();
	mapInstr();

	if (instr1_next == 0x10)
	{
	    if (!is_page3)
	    {
		is_page2_next = true;
	    }

	    memAccess();
	    setState(FetchI1V2);
	}
	else if (instr1_next == 0x11)
	{
	    if (!is_page2)
	    {
		is_page3_next = true;
	    }

	    memAccess();
	    setState(FetchI1V2);
	}
	else
	{
	    memAccess();
	    setState(FetchI2);
	}
    }
    break;
    case FetchI2:
    {
	incAddress();
	reg_pc_next = (reg_pc + 1);
	instr2_next = getData();
	memAccess();
	setState(FetchI1);

	decodeInstruction(instr1);

	switch (addr_mode)
	{
	    case Indexed:
	    {
		memAccess();
		setState(IndexedBase);
	    }
	    break;
	    case Extended:
	    {
		ea_next = ((ea_next & 0xFF) | (instr2_next << 8));
		memAccess();
		setState(ExtendedAddrLow);
	    }
	    break;
	    case Direct:
	    {
		ea_next = ((reg_dp << 8) | instr2_next);
		syncAccess();
		setState(DirectDontCare);
	    }
	    break;
	    case Inherent:
	    {
		switch (instr1)
		{
		    // NOP
		    case 0x12:
		    {
			memAccess();
			instComplete();
			setState(FetchI1);
		    }
		    break;
		    // SYNC
		    case 0x13:
		    {
			instComplete();
			syncAccess();
			setState(Sync);
		    }
		    break;
		    // RTS
		    case 0x39:
		    {
			memAccess();
			setState(RtsHigh);
		    }
		    break;
		    // SWI
		    case 0x3F:
		    {
			memAccess();
			setState(SwiStart);
		    }
		    break;
		    case 0x19:
		    case 0x3D:
		    case 0x3B:
		    case 0x3C:
		    case 0x1D:
		    case 0x3A:
		    {
			cout << "Unrecognized instruction of " << hex << int(instr1) << endl;
			throw runtime_error("Botnami error");
		    }
		    break;
		    default:
		    {
			fetchALU8(instr1);

			if (is_rega)
			{
			    alu8_a = rega;
			}
			else
			{
			    alu8_a = regb;
			}

			alu8_b = 0;
			alu8_cc = reg_cc;

			runALU8();

			reg_cc_next = alu_cc_out;

			if (alu8_wb)
			{
			    if (is_rega)
			    {
				rega_next = alu8_res;
			    }
			    else
			    {
				regb_next = alu8_res;
			    }
			}

			memAccess();
			instComplete();
			setState(FetchI1);

			if (is_one_byte)
			{
			    reg_pc_next = reg_pc;
			}
		    }
		    break;
		}
	    }
	    break;
	    case Immediate:
	    {
		if (is_special_imm)
		{
		    switch (instr1)
		    {
			case 0x34:
			case 0x36:
			{
			    reg_pc_next = (reg_pc + 1);
			    temp_next = changebit(instr2_next, 14, testbit(instr1, 1));
			    syncAccess();
			    setNextState(FetchI1);
			    setState(PushDontCare1);
			}
			break;
			case 0x35:
			case 0x37:
			{
			    reg_pc_next = (reg_pc + 1);
			    temp_next = changebit(instr2_next, 14, testbit(instr1, 1));
			    syncAccess();
			    setNextState(FetchI1);
			    setState(PullDontCare1);
			}
			break;
			default:
			{
			    cout << "Unrecognized special immediate instruction of " << hex << int(instr1) << endl;
			    throw runtime_error("Botnami error");
			}
			break;
		    }
		}
		else if (is_alu_8bit)
		{
		    fetchALU8(instr1);

		    if (is_rega)
		    {
			alu8_a = rega;
		    }
		    else
		    {
			alu8_a = regb;
		    }

		    alu8_b = instr2_next;
		    alu8_cc = reg_cc;

		    runALU8();

		    reg_cc_next = alu_cc_out;

		    if (alu8_wb)
		    {
			if (is_rega)
			{
			    rega_next = alu8_res;
			}
			else
			{
			    regb_next = alu8_res;
			}
		    }

		    instComplete();
		    memAccess();
		    setState(FetchI1);
		}
		else
		{
		    fetchALU16(is_page2, is_page3, instr1);

		    if (alu16_op != InvalidALU16)
		    {
			memAccess();
			setState(Imm16Lo);
		    }
		}
	    }
	    break;
	    case Relative:
	    {
		if (is_page2 || (instr1 == 0x16) || (instr1 == 0x17))
		{
		    memAccess();
		    setState(LBraOffsetLow);
		}
		else
		{
		    syncAccess();
		    setState(BraDontCare);
		}
	    }
	    break;
	    default:
	    {
		cout << "Unrecognized addressing mode of " << dec << int(addr_mode) << endl;
		throw runtime_error("Botnami error");
	    }
	    break;
	}
    }
    break;
    case Sync:
    {
	setAddress(0xFFFF);
	syncAccess();
	instComplete();
    }
    break;
    case Imm16Lo:
    {
	setAddress(reg_pc);
	reg_pc_next = (reg_pc + 1);

	fetchALU16(is_page2, is_page3, instr1);
	alu16_cc = reg_cc;
	alu16_b = ((instr2 << 8) | getData());

	switch (alu16_reg)
	{
	    case RegX: alu16_a = regx; break;
	    case RegY: alu16_a = regy; break;
	    case RegS: alu16_a = reg_ssp; break;
	    default:
	    {
		cout << "Unrecognized ALU16 register of " << dec << int(alu16_reg) << endl;
		throw runtime_error("Botnami error");
	    }
	    break;
	}

	runALU16();

	reg_cc_next = alu_cc_out;

	if (alu16_wb)
	{
	    switch (alu16_reg)
	    {
		case RegX: regx_next = alu16_res; break;
		case RegY: regy_next = alu16_res; break;
		case RegS: reg_ssp_next = alu16_res; break;
		default:
		{
		    cout << "Unrecognized ALU16 writeback register of " << dec << int(alu16_reg) << endl;
		    throw runtime_error("Botnami error");
		}
		break;
	    }
	}

	if (alu16_op == Ld16)
	{
	    instComplete();
	    memAccess();
	    setState(FetchI1);
	}
	else
	{
	    syncAccess();
	    setState(Imm16DontCare);
	}
    }
    break;
    case Imm16DontCare:
    {
	setAddress(0xFFFF);
	instComplete();
	memAccess();
	setState(FetchI1);
    }
    break;
    case ExtendedAddrLow:
    {
	addr_next = reg_pc;
	reg_pc_next = (reg_pc + 1);
	ea_next = ((ea_next & 0xFF00) | getData());
	syncAccess();
	setState(ExtendedDontCare);
    }
    break;
    case ExtendedDontCare:
    {
	if (is_jump)
	{
	    cout << "Jump instruction extended" << endl;
	    throw runtime_error("Botnami error");
	}
	else
	{
	    memAccess();
	    setState(AluEA);
	}
    }
    break;
    case DirectDontCare:
    {
	setAddress(0xFFFF);

	if (is_jump)
	{
	    cout << "Jump instruction direct" << endl;
	    throw runtime_error("Botnami error");
	}
	else
	{
	    memAccess();
	    setState(AluEA);
	}
    }
    break;
    case IndexedBase:
    {
	setAddress(reg_pc);
	instr3_next = getData();
	decodeIndexed(instr2);

	switch (indexed_reg)
	{
	    case IdxRegX: alu16_a = regx; break;
	    case IdxRegY: alu16_a = regy; break;
	    case IdxRegU: alu16_a = reg_usp; break;
	    case IdxRegS: alu16_a = reg_ssp; break;
	    default:
	    {
		cout << "Unrecognized indexed register of " << dec << int(indexed_reg) << endl;
		throw runtime_error("Botnami error");
	    }
	    break;
	}

	alu16_op = Add16;

	switch (indexed_mode)
	{
	    case NoOffs:
	    {
		switch (indexed_reg)
		{
		    case IdxRegX: ea_next = regx; break;
		    case IdxRegY: ea_next = regy; break;
		    case IdxRegU: ea_next = reg_usp; break;
		    case IdxRegS: ea_next = reg_ssp; break;
		    default: ea_next = 0; break;
		}

		if (is_idx_indirect)
		{
		    cout << "Indirect indexed instruction" << endl;
		    throw runtime_error("Botnami error");
		}
		else if (is_jump)
		{
		    reg_pc_next = ea_next;
		    instComplete();
		    memAccess();
		    setState(FetchI1);
		}
		else
		{
		    memAccess();
		    setState(AluEA);
		}
	    }
	    break;
	    case Bit5Offs:
	    {
		uint16_t offs = (instr2 & 0x1F);

		// Sign extend offset to 16-bits
		if (testbit(offs, 4))
		{
		    offs |= 0xFFE0;
		}

		alu16_b = offs;
		runALU16();
		ea_next = alu16_res;
		syncAccess();
		setState(IdxDontCare3);
	    }
	    break;
	    case PostInc1:
	    {
		alu16_b = 1;
		ea_next = alu16_a;
		runALU16();

		switch (indexed_reg)
		{
		    case IdxRegX: regx_next = alu16_res; break;
		    case IdxRegY: regy_next = alu16_res; break;
		    default:
		    {
			cout << "Unrecognized indexed writeback register of " << dec << int(indexed_reg) << endl;
			throw runtime_error("Botnami error");
		    }
		    break;
		}

		syncAccess();
		setState(Idx16OffsDontCare2);
	    }
	    break;
	    case PostInc2:
	    {
		alu16_b = 2;
		ea_next = alu16_a;
		runALU16();

		switch (indexed_reg)
		{
		    case IdxRegX: regx_next = alu16_res; break;
		    case IdxRegY: regy_next = alu16_res; break;
		    default:
		    {
			cout << "Unrecognized indexed writeback register of " << dec << int(indexed_reg) << endl;
			throw runtime_error("Botnami error");
		    }
		    break;
		}

		syncAccess();
		setState(Idx16OffsDontCare0);
	    }
	    break;
	    default:
	    {
		cout << "Unrecognized indexed mode of " << dec << int(indexed_mode) << endl;
		throw runtime_error("Botnami error");
	    }
	    break;
	}
    }
    break;
    case IdxDontCare3:
    {
	setAddress(0xFFFF);

	if (is_idx_indirect)
	{
	    cout << "Indirect instruction" << endl;
	    throw runtime_error("Botnami error");
	}
	else
	{
	    if (is_jump)
	    {
		cout << "Jump indirect instruction" << endl;
		throw runtime_error("Botnami error");
	    }
	    else
	    {
		memAccess();
		setState(AluEA);
	    }
	}
    }
    break;
    case Idx16OffsDontCare0:
    {
	setAddress(0xFFFF);
	syncAccess();
	setState(Idx16OffsDontCare2);
    }
    break;
    case Idx16OffsDontCare2:
    {
	setAddress(0xFFFF);

	if (indexed_reg == IdxRegPC)
	{
	    cout << "Indexed reg is PC" << endl;
	    throw runtime_error("Botnami error");
	}
	else
	{
	    syncAccess();
	    setState(Idx16OffsDontCare3);
	}
    }
    break;
    case Idx16OffsDontCare3:
    {
	setAddress(0xFFFF);

	if (is_idx_indirect)
	{
	    cout << "Indirect 16 instruction" << endl;
	    throw runtime_error("Botnami error");
	}
	else
	{
	    if (is_jump)
	    {
		cout << "Jump indirect 16 instruction" << endl;
		throw runtime_error("Botnami error");
	    }
	    else
	    {
		memAccess();
		setState(AluEA);
	    }
	}
    }
    break;
    case AluEA:
    {
	fetchALU16(is_page2, is_page3, instr1);
	if (is_alu8_set1)
	{
	    setAddress(ea);
	    fetchALU8(instr1);
	    alu8_b = getData();
	    alu8_cc = reg_cc;

	    if (is_rega)
	    {
		alu8_a = rega;
	    }
	    else
	    {
		alu8_a = regb;
	    }

	    runALU8();
	    reg_cc_next = alu_cc_out;

	    if (alu8_wb)
	    {
		if (is_rega)
		{
		    rega_next = alu8_res;
		}
		else
		{
		    regb_next = alu8_res;
		}
	    }

	    instComplete();
	    memAccess();
	    setState(FetchI1);
	}
	else if (is_store8)
	{
	    setAddress(ea);
	    write();

	    alu8_op = Ld;
	    alu8_a = 0;
	    alu8_cc = reg_cc;

	    if (is_store_b)
	    {
		setData(regb);
		alu8_b = regb;
	    }
	    else
	    {
		setData(rega);
		alu8_b = rega;
	    }

	    runALU8();
	    reg_cc_next = alu_cc_out;

	    instComplete();
	    memAccess();
	    setState(FetchI1);
	}
	else if (alu16_op == Ld16)
	{
	    setAddress(ea);
	    ea_next = (ea + 1);

	    switch (alu16_reg)
	    {
		case RegX:
		{
		    regx_next = ((regx_next & 0xFF) | (getData() << 8));
		}
		break;
		case RegY:
		{
		    regy_next = ((regy_next & 0xFF) | (getData() << 8));
		}
		break;
		case RegD:
		{
		    rega_next = getData();
		}
		break;
		default:
		{
		    cout << "Unrecognized ld16 ALU register of " << dec << int(alu16_reg) << endl;
		    throw runtime_error("Botnami error");
		}
		break;
	    }

	    memAccess();
	    setState(Ld16Low);
	}
	else if (is_store16)
	{
	    cout << "Store16" << endl;
	    throw runtime_error("Botnami error");
	}
	else if (is_alu8_set0)
	{
	    setAddress(ea);
	    fetchALU8(instr1);
	    alu8_a = getData();
	    alu8_cc = reg_cc;

	    runALU8();

	    temp_next = alu8_res;
	    reg_cc_next = alu_cc_out;

	    if (alu8_op == Tst)
	    {
		syncAccess();
		setState(TstDontCare1);
	    }
	    else
	    {
		syncAccess();
		setState(AluDontCare);
	    }
	}
	else if (((alu16_op != InvalidALU16) && (alu16_op != Ld16)) && ((instr1 < 0x30) || (instr1 > 0x33)))
	{
	    setAddress(ea);
	    ea_next = (ea + 1);

	    temp_next = ((temp_next & 0xFF) | (getData() << 8));
	    memAccess();
	    setState(Alu16Low);
	}
	else
	{
	    switch (instr1)
	    {
		case 0x9D:
		case 0xAD:
		case 0xBD:
		{
		    reg_pc_next = ea;
		    setAddress(ea);
		    temp_next = reg_pc;
		    syncAccess();
		    setState(JsrDontCare);
		}
		break;
		case 0x30:
		case 0x31:
		case 0x32:
		case 0x33:
		{
		    alu16_cc = reg_cc;
		    alu16_a = ea;

		    runALU16();

		    switch (alu16_reg)
		    {
			case RegX:
			{
			    reg_cc_next = alu_cc_out;
			    regx_next = alu16_res;
			}
			break;
			case RegY:
			{
			    reg_cc_next = alu_cc_out;
			    regy_next = alu16_res;
			}
			break;
			case RegU:
			{
			    reg_usp_next = alu16_res;
			}
			break;
			case RegS:
			{
			    reg_ssp_next = alu16_res;
			}
			break;
			default: break;
		    }

		    instComplete();
		    memAccess();
		    setState(FetchI1);
		}
		break;
		default:
		{
		    cout << "Unrecognized instruction of " << hex << int(instr1) << endl;
		    throw runtime_error("Botnami error");
		}
		break;
	    }
	}
    }
    break;
    case AluDontCare:
    {
	setAddress(0xFFFF);
	memAccess();
	setState(AluWriteback);
    }
    break;
    case Alu16Low:
    {
	setAddress(ea);
	fetchALU16(is_page2, is_page3, instr1);
	alu16_cc = reg_cc;
	alu16_b = ((temp_next & 0xFF00) | getData());

	switch (alu16_reg)
	{
	    case RegX: alu16_a = regx; break;
	    case RegY: alu16_a = regy; break;
	    case RegS: alu16_a = reg_ssp; break;
	    case RegU: alu16_a = reg_usp; break;
	    case RegD: alu16_a = ((rega << 8) | regb); break;
	    default: alu16_a = 0; break;
	}

	runALU16();

	reg_cc_next = alu_cc_out;

	if (alu16_wb)
	{
	    switch (alu16_reg)
	    {
		case RegX: regx_next = alu16_res; break;
		case RegY: regy_next = alu16_res; break;
		case RegU: reg_usp_next = alu16_res; break;
		case RegS: reg_ssp_next = alu16_res; break;
		case RegD:
		{
		    rega = (alu16_res >> 8);
		    regb = (alu16_res & 0xFF);
		}
		break;
	    }
	}

	syncAccess();
	setState(Alu16DontCare);
    }
    break;
    case Alu16DontCare:
    {
	setAddress(0xFFFF);
	instComplete();
	memAccess();
	setState(FetchI1);
    }
    break;
    case AluWriteback:
    {
	setAddress(ea);
	write();
	setData((temp & 0xFF));
	instComplete();
	memAccess();
	setState(FetchI1);
    }
    break;
    case Ld16Low:
    {
	setAddress(ea);

	uint8_t alu16_b_high = 0;

	switch (alu16_reg)
	{
	    case RegD:
	    {
		regb_next = getData();
		alu16_b_high = rega;
	    }
	    break;
	    case RegX:
	    {
		regx_next = ((regx_next & 0xFF00) | getData());
		alu16_b_high = (regx >> 8);
	    }
	    break;
	    case RegY:
	    {
		regy_next = ((regy_next & 0xFF00) | getData());
		alu16_b_high = (regy >> 8);
	    }
	    break;
	    default:
	    {
		cout << "Unrecognized ld16 low ALU register of " << dec << int(alu16_reg) << endl;
		throw runtime_error("Botnami error");
	    }
	    break;
	}

	fetchALU16(is_page2, is_page3, instr1);
	alu16_cc = reg_cc;

	alu16_a = 0;
	alu16_b = ((alu16_b_high << 8) | getData());
	runALU16();

	reg_cc_next = alu_cc_out;

	instComplete();
	memAccess();
	setState(FetchI1);
    }
    break;
    case BraDontCare:
    {
	setAddress(0xFFFF);
	temp_next = reg_pc;

	fetchBranch(instr1);

	if (is_branch_cond)
	{
	    reg_pc_next = (reg_pc + int8_t(instr2));

	    if (instr1 == 0x8D)
	    {
		memAccess();
		setState(BsrDontCare1);
	    }
	    else
	    {
		instComplete();
		memAccess();
		setState(FetchI1);
	    }
	}
	else
	{
	    instComplete();
	    memAccess();
	    setState(FetchI1);
	}
    }
    break;
    case BsrDontCare1:
    {
	setAddress(reg_pc);
	syncAccess();
	setState(BsrDontCare2);
    }
    break;
    case BsrDontCare2:
    {
	setAddress(0xFFFF);
	memAccess();
	setState(BsrReturnLow);
    }
    break;
    case BsrReturnLow:
    {
	reg_ssp_next = (reg_ssp - 1);
	setAddress(reg_ssp_next);
	setData((temp & 0xFF));
	write();
	memAccess();
	setState(BsrReturnHigh);
    }
    break;
    case BsrReturnHigh:
    {
	reg_ssp_next = (reg_ssp - 1);
	setAddress(reg_ssp_next);
	setData((temp >> 8));
	write();
	instComplete();
	memAccess();
	setState(FetchI1);
    }
    break;
    case JsrDontCare:
    {
	setAddress(0xFFFF);
	memAccess();
	setState(JsrReturnLow);
    }
    break;
    case JsrReturnLow:
    {
	reg_ssp_next = (reg_ssp - 1);
	setAddress(reg_ssp_next);
	setData((temp & 0xFF));
	write();
	memAccess();
	setState(JsrReturnHigh);
    }
    break;
    case JsrReturnHigh:
    {
	reg_ssp_next = (reg_ssp - 1);
	setAddress(reg_ssp_next);
	setData((temp >> 8));
	write();
	instComplete();
	memAccess();
	setState(FetchI1);
    }
    break;
    case RtsHigh:
    {
	setAddress(reg_ssp);
	reg_ssp_next = (reg_ssp + 1);
	reg_pc_next = ((reg_pc_next & 0xFF) | (getData() << 8));
	memAccess();
	setState(RtsLow);
    }
    break;
    case RtsLow:
    {
	setAddress(reg_ssp);
	reg_ssp_next = (reg_ssp + 1);
	reg_pc_next = ((reg_pc_next & 0xFF00) | getData());
	syncAccess();
	setState(RtsDontCare2);
    }
    break;
    case RtsDontCare2:
    {
	setAddress(0xFFFF);
	instComplete();
	memAccess();
	setState(FetchI1);
    }
    break;
    case LBraOffsetLow:
    {
	setAddress(reg_pc);
	reg_pc_next = (reg_pc + 1);
	instr3_next = getData();
	syncAccess();
	setState(LBraDontCare);
    }
    break;
    case LBraDontCare:
    {
	setAddress(0xFFFF);

	fetchBranch(instr1);

	if (is_branch_cond)
	{
	    syncAccess();
	    setState(LBraDontCare2);
	}
	else
	{
	    instComplete();
	    memAccess();
	    setState(FetchI1);
	}
    }
    break;
    case LBraDontCare2:
    {
	temp_next = reg_pc;
	setAddress(0xFFFF);

	int16_t offs = ((instr2 << 8) | instr3);

	reg_pc_next = (reg_pc + offs);

	if (instr1 == 0x17)
	{
	    memAccess();
	    setState(BsrDontCare1);
	}
	else
	{
	    instComplete();
	    memAccess();
	    setState(FetchI1);
	}
    }
    break;
    case SwiStart:
    {
	setAddress(reg_pc);
	temp_next = 0xFF;

	setNextState(IrqDontCare2);
	memAccess();
	setState(IrqDontCare);

	if (is_page3)
	{
	    int_type_next = Swi3;
	}
	else if (is_page2)
	{
	    int_type_next = Swi2;
	}
	else
	{
	    int_type_next = Swi;
	}

	reg_cc_next = setbit(reg_cc_next, 7);
    }
    break;
    case IrqDontCare:
    {
	// nmi_clear_next = false;
	setAddress(0xFFFF);
	memAccess();
	setState(PushAction);
    }
    break;
    case IrqDontCare2:
    {
	setAddress(0xFFFF);
	memAccess();
	setState(IrqVectorHigh);
	instComplete();
    }
    break;
    case IrqVectorHigh:
    {
	switch (int_type)
	{
	    case Swi:
	    {
		setAddress(0xFFFA);
	    }
	    break;
	    default:
	    {
		cout << "Unrecognized IRQ type of " << dec << int(int_type) << endl;
		throw runtime_error("Botnami error");
	    }
	    break;
	}

	reg_pc_next = ((reg_pc_next & 0xFF) | (getData() << 8));
	memAccess();
	// setBusy();
	instComplete();
	setState(IrqVectorLow);
    }
    break;
    case IrqVectorLow:
    {
	switch (int_type)
	{
	    case Swi:
	    {
		setAddress(0xFFFB);
		reg_cc_next = setbit(reg_cc_next, 6);
		reg_cc_next = setbit(reg_cc_next, 4);
		instComplete();
	    }
	    break;
	    default:
	    {
		cout << "Unrecognized IRQ type of " << dec << int(int_type) << endl;
		throw runtime_error("Botnami error");
	    }
	    break;
	}

	reg_pc_next = ((reg_pc_next & 0xFF00) | getData());
	memAccess();
	instComplete();
	setState(IntDontCare);
    }
    break;
    case IntDontCare:
    {
	setAddress(0xFFFF);
	memAccess();
	instComplete();
	setState(FetchI1);
    }
    break;
    case PushDontCare1:
    {
	setAddress(0xFFFF);
	syncAccess();
	setState(PushDontCare2);
    }
    break;
    case PushDontCare2:
    {
	setAddress(0xFFFF);
	memAccess();
	setState(PushDontCare3);
    }
    break;
    case PushDontCare3:
    {
	uint16_t addr = testbit(instr1, 1) ? reg_usp : reg_ssp;
	setAddress(addr);
	setState(PushAction);
    }
    break;
    case PushAction:
    {
	memAccess();

	if (testbit(temp, 7) && !testbit(temp, 15))
	{
	    decSP();

	    setData((reg_pc & 0xFF));
	    write();
	    temp_next = setbit(temp_next, 15);
	}
	else if (testbit(temp, 7) && testbit(temp, 15))
	{
	    decSP();
	    setData((reg_pc >> 8));
	    write();
	    temp_next = resetbit(temp_next, 15);
	    temp_next = resetbit(temp_next, 7);
	}
	else if (testbit(temp, 6) && !testbit(temp, 15))
	{
	    decSP();

	    uint16_t sp = testbit(temp, 14) ? reg_ssp : reg_usp;

	    setData((sp & 0xFF));
	    write();
	    temp_next = setbit(temp_next, 15);
	}
	else if (testbit(temp, 6) && testbit(temp, 15))
	{
	    decSP();
	    uint16_t sp = testbit(temp, 14) ? reg_ssp : reg_usp;

	    setData((sp >> 8));
	    write();
	    temp_next = resetbit(temp_next, 15);
	    temp_next = resetbit(temp_next, 6);
	}
	else if (testbit(temp, 5) && !testbit(temp, 15))
	{
	    decSP();

	    setData((regy & 0xFF));
	    write();
	    temp_next = setbit(temp_next, 15);
	}
	else if (testbit(temp, 5) && testbit(temp, 15))
	{
	    decSP();

	    setData((regy >> 8));
	    write();
	    temp_next = resetbit(temp_next, 15);
	    temp_next = resetbit(temp_next, 5);
	}
	else if (testbit(temp, 4) && !testbit(temp, 15))
	{
	    decSP();

	    setData((regx & 0xFF));
	    write();
	    temp_next = setbit(temp_next, 15);
	}
	else if (testbit(temp, 4) && testbit(temp, 15))
	{
	    decSP();

	    setData((regx >> 8));
	    write();
	    temp_next = resetbit(temp_next, 15);
	    temp_next = resetbit(temp_next, 4);
	}
	else if (testbit(temp, 3))
	{
	    decSP();
	    setData(reg_dp);
	    write();
	    temp_next = resetbit(temp_next, 3);
	}
	else if (testbit(temp, 2))
	{
	    decSP();
	    setData(regb);
	    write();
	    temp_next = resetbit(temp_next, 2);
	}
	else if (testbit(temp, 1))
	{
	    decSP();
	    setData(rega);
	    write();
	    temp_next = resetbit(temp_next, 1);
	}
	else if (testbit(temp, 0))
	{
	    decSP();
	    setData(reg_cc);
	    write();
	    temp_next = resetbit(temp_next, 0);
	}

	if (testbit(temp, 13))
	{
	    instComplete();
	}

	bool is_empty = ((temp_next & 0xFF) == 0);

	if (is_empty)
	{
	    if (next_state == FetchI1)
	    {
		memAccess();
		instComplete();
	    }
	    else
	    {
		syncAccess();
	    }

	    setState(next_state);
	}
    }
    break;
    case PullDontCare1:
    {
	setAddress(0xFFFF);
	syncAccess();
	setState(PullDontCare2);
    }
    break;
    case PullDontCare2:
    {
	setAddress(0xFFFF);
	memAccess();
	setState(PullAction);
    }
    break;
    case PullAction:
    {
	memAccess();

	if (testbit(temp, 0))
	{
	    incSP();
	    reg_cc_next = getData();

	    if (testbit(temp, 12))
	    {
		if (testbit(getData(), 7))
		{
		    temp_next = ((temp_next & 0xFF00) | 0xFE);
		}
		else
		{
		    temp_next = ((temp_next & 0xFF00) | 0x80);
		}
	    }
	    else
	    {
		temp_next = resetbit(temp_next, 0);
	    }
	}
	else if (testbit(temp, 1))
	{
	    incSP();
	    rega_next = getData();
	    temp_next = resetbit(temp_next, 1);
	}
	else if (testbit(temp, 2))
	{
	    incSP();
	    regb_next = getData();
	    temp_next = resetbit(temp_next, 2);
	}
	else if (testbit(temp, 3))
	{
	    incSP();
	    reg_dp_next = getData();
	    temp_next = resetbit(temp_next, 2);
	}
	else if (testbit(temp, 4) && !testbit(temp, 15))
	{
	    incSP();
	    regx_next = ((regx_next & 0xFF) | (getData() << 8));
	    temp_next = setbit(temp_next, 15);
	}
	else if (testbit(temp, 4) && testbit(temp, 15))
	{
	    incSP();
	    regx_next = ((regx_next & 0xFF00) | getData());
	    temp_next = resetbit(temp_next, 4);
	    temp_next = resetbit(temp_next, 15);
	}
	else if (testbit(temp, 5) && !testbit(temp, 15))
	{
	    incSP();
	    regy_next = ((regy_next & 0xFF) | (getData() << 8));
	    temp_next = setbit(temp_next, 15);
	}
	else if (testbit(temp, 5) && testbit(temp, 15))
	{
	    incSP();
	    regy_next = ((regy_next & 0xFF00) | getData());
	    temp_next = resetbit(temp_next, 5);
	    temp_next = resetbit(temp_next, 15);
	}
	else if (testbit(temp, 6) && !testbit(temp, 15))
	{
	    incSP();

	    uint16_t &sp = testbit(temp, 14) ? reg_ssp_next : reg_usp_next;
	    sp = ((sp & 0xFF) | (getData() << 8));
	    temp_next = setbit(temp_next, 15);
	}
	else if (testbit(temp, 6) && testbit(temp, 15))
	{
	    incSP();
	    uint16_t &sp = testbit(temp, 14) ? reg_ssp_next : reg_usp_next;
	    sp = ((sp & 0xFF00) | getData());
	    temp_next = resetbit(temp_next, 6);
	    temp_next = resetbit(temp_next, 15);
	}
	else if (testbit(temp, 7) && !testbit(temp, 15))
	{
	    incSP();
	    reg_pc_next = ((regy_next & 0xFF) | (getData() << 8));
	    temp_next = setbit(temp_next, 15);
	}
	else if (testbit(temp, 7) && testbit(temp, 15))
	{
	    incSP();
	    reg_pc_next = ((regy_next & 0xFF00) | getData());
	    temp_next = resetbit(temp_next, 7);
	    temp_next = resetbit(temp_next, 15);
	}
	else
	{
	    uint16_t addr = testbit(temp, 14) ? reg_usp : reg_ssp;
	    setAddress(addr);

	    if (next_state == FetchI1)
	    {
		memAccess();
		instComplete();
	    }
	    else
	    {
		syncAccess();
	    }

	    setState(next_state);
	}
    }
    break;
    default:
    {
	cout << "Unrecognized state of " << dec << int(inst_state) << endl;
	throw runtime_error("Botnami error");
    }
    break;
}