#  Copyright (c) 2012-2013 Jakub Filipowicz <jakubf@gmail.com>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc.,
#  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

MODE_MERA400 = 0
MODE_K202 = 1

# ------------------------------------------------------------------------
def m400_get_opcode(i, d, a, b, c, mode):

    # try to find opcode, return invalid if nonexistent
    try:
        code = m400_opcodes[i][mode]
        group = m400_opcodes[i][2]
        desc = m400_opcodes[i][4]
    except:
        return "---", OP_INVALID, ""

    # if code is empty in main dictionary, get it from the group function
    if m400_opcodes[i][3] is not None:
        try:
            code, desc = m400_opcodes[i][3](i, d, a, b, c, mode)
        except:
            return "---", OP_INVALID, ""

#    # basic opcodes have suffixes in MERA-400 mode
#    if mode == MODE_MERA400 and m400_opcodes[i][2] in (OP_NORM2, OP_NORM1):
#        # basic opcode, no direct arg
#        if c != 0:
#            if d == 0:
#                suffix = "R"
#            if d == 1:
#                suffix = "A"
#        # basic opcode with direct 16-bit arg
#        if c == 0:
#            if d == 0:
#                suffix = "D"
#            if d == 1:
#                suffix = "I"    
#        code += suffix

    return code, group, desc

# ------------------------------------------------------------------------
def m400_name_i37(i, d, a, b, c, mode):
    return op_basic_i37_a[a][mode], op_basic_i37_a[a][2]

# ------------------------------------------------------------------------
def m400_name_i70(i, d, a, b, c, mode):
    # treated by assembler as UJS, 0
    if (d==0) and (a==0) and (b==0) and (c==0):
        return 'NOP', 'NO Operation'
    else:
        return op_short_arg_i70_a[a][mode], op_short_arg_i70_a[a][2]

# ------------------------------------------------------------------------
def m400_name_i71(i, d, a, b, c, mode):
    da2 = (d<<1) | (a>>2)
    return op_byte_arg_i71_da2[da2][mode], op_byte_arg_i71_da2[da2][2]

# ------------------------------------------------------------------------
def m400_name_i72(i, d, a, b, c, mode):
    dbc = (d<<6) | (b<<3) | (c)
    if (b == 0b010):
        if mode == MODE_MERA400:
            return "SHC", "SHift Cyclic"
        else:
            return "shc", "SHift Cyclic"
    else:
        return op_no_arg_i72_dbc[dbc][mode], op_no_arg_i72_dbc[dbc][2]

# ------------------------------------------------------------------------
def m400_name_i73(i, d, a, b, c, mode):
    # sil, sit, cit, ciu
    if a == 2:
        dc = (d<<3) | c;
        return op_no_arg_i73_dc[dc][mode], op_no_arg_i73_dc[dc][2]
    else:
        da = (d<<3) | a;
        return op_no_arg_i73_da[da][mode], op_no_arg_i73_da[da][2]

# ------------------------------------------------------------------------
def m400_name_i74(i, d, a, b, c, mode):
    return op_basic_i74_a[a][mode], op_basic_i74_a[a][2]

# ------------------------------------------------------------------------
def m400_name_i75(i, d, a, b, c, mode):
    return op_basic_i75_a[a][mode], op_basic_i75_a[a][2]

# ------------------------------------------------------------------------
def m400_name_i76(i, d, a, b, c, mode):
    return op_basic_i76_a[a][mode], op_basic_i76_a[a][2]

# ------------------------------------------------------------------------
def m400_name_i77(i, d, a, b, c, mode):
    return op_basic_i77_a[a][mode], op_basic_i77_a[a][2]

OP_INVALID = -1
OP_NORM2 = 0
OP_NORM1 = 1
OP_SHORT2 = 2
OP_SHORT1 = 3
OP_BYTE = 4
OP_NO = 5
OP_NO2 = 6

m400_opcodes = {
    020: ['LW', 'lo',   OP_NORM2, None, 'Load Word'],
    021: ['TW', 'lob',  OP_NORM2, None, 'Take Word'],
    022: ['LS', 'lom',  OP_NORM2, None, 'Load Selective'],
    023: ['RI', 'los',  OP_NORM2, None, 'Remember and Increment'],
    024: ['RW', 'st',   OP_NORM2, None, 'Remember Word'],
    025: ['PW', 'stb',  OP_NORM2, None, 'Put Word'],
    026: ['RJ', 'jpar', OP_NORM2, None, 'Return Jump'],
    027: ['IS', 'is',   OP_NORM2, None, 'Install Semaphore'],
    030: ['BB', 'clbo', OP_NORM2, None, 'Branch on Bits'],
    031: ['BM', 'bm',   OP_NORM2, None, 'Branch on bits in Memory'],
    032: ['BS', 'clmo', OP_NORM2, None, 'Branch Selective'],
    033: ['BC', 'bc',   OP_NORM2, None, 'Branch if not all Conditions'],
    034: ['BN', 'bn',   OP_NORM2, None, 'Branch if No conditions'],
    035: ['OU', 'ou',   OP_NORM2, None, 'OUtput data'],
    036: ['IN', 'in',   OP_NORM2, None, 'INput data'],
    037: ['',   '',     OP_NORM1, m400_name_i37, ''],
    040: ['AW', 'ad',   OP_NORM2, None, 'Add Word'],
    041: ['AC', 'adc',  OP_NORM2, None, 'Add word with Carry'],
    042: ['SW', 'su',   OP_NORM2, None, 'Subtract Word'],
    043: ['CW', 'co',   OP_NORM2, None, 'Compare Word'],
    044: ['OR', 'or',   OP_NORM2, None, 'Or Register'],
    045: ['OM', 'om',   OP_NORM2, None, 'Or Memory'],
    046: ['NR', 'and',  OP_NORM2, None, 'aNd in Register'],
    047: ['NM', 'nm',   OP_NORM2, None, 'aNd in Memory'],
    050: ['ER', 'orn',  OP_NORM2, None, 'Erase bits in Register'],
    051: ['EM', 'em',   OP_NORM2, None, 'Erase bits in Memory'],
    052: ['XR', 'xr',   OP_NORM2, None, 'eXclusive or in Register'],
    053: ['XM', 'xm',   OP_NORM2, None, 'eXclusive or in Memory'],
    054: ['CL', 'cl',   OP_NORM2, None, 'Compare Logically'],
    055: ['LB', 'lb',   OP_NORM2, None, 'Load Byte'],
    056: ['RB', 'wrb',  OP_NORM2, None, 'Remember Byte'],
    057: ['CB', 'cb',   OP_NORM2, None, 'Compare Byte'],
    060: ['AWT','adt',  OP_SHORT2, None, 'Add to Word parameTer'],
    061: ['TRB','adot', OP_SHORT2, None, 'parameTer to Register and Branch'],
    062: ['IRB','adjt', OP_SHORT2, None, 'Increment Register and Branch'],
    063: ['DRB','zdrb', OP_SHORT2, None, 'Decrease Register and Branch'],
    064: ['CWT','cot',  OP_SHORT2, None, 'Compare Word to parameTer'],
    065: ['LWT','lot',  OP_SHORT2, None, 'Load to Word paremeTer'],
    066: ['LWS','lts',  OP_SHORT2, None, 'Load to Word Short'],
    067: ['RWS','sts',  OP_SHORT2, None, 'Remember Word Short'],
    070: ['',   '', OP_SHORT1, m400_name_i70, ''],
    071: ['',   '', OP_BYTE,  m400_name_i71, ''],
    072: ['',   '', OP_NO2,   m400_name_i72, ''],
    073: ['',   '', OP_NO,    m400_name_i73, ''],
    074: ['',   '', OP_NORM1, m400_name_i74, ''],
    075: ['',   '', OP_NORM1, m400_name_i75, ''],
    076: ['',   '', OP_NORM1, m400_name_i76, ''],
    077: ['',   '', OP_NORM1, m400_name_i77, '']
}

op_basic_i37_a = {
    0: ['AD', 'add', 'Add Double word'],
    1: ['SD', 'sd',  'Subtract Double word'],
    2: ['MW', 'mw',  'Multiply Words'],
    3: ['DW', 'dw',  'Divide Words'],
    4: ['AF', 'adf', 'Add Floating point'],
    5: ['SF', 'sbf', 'Subtract Floating point'],
    6: ['MF', 'mlf', 'Multiply Floating point'],
    7: ['DF', 'dvf', 'Divide Floating point']
}

op_short_arg_i70_a = {
    0: ['UJS', 'jpt',  'Unconditional Jump Short'],
    1: ['JLS', 'jptl', 'Jump if Less Short'],
    2: ['JES', 'jpte', 'Jump if Equal Short'],
    3: ['JGS', 'jptg', 'Jump if Greater Short'],
    4: ['JVS', 'jptv', 'Jump if oVerflow Short'],
    5: ['JXS', 'jptx', 'Jump if X Short'],
    6: ['JYS', 'jtpy', 'Jump if Y Short'],
    7: ['JCS', 'jcs',  'Jump if Carry Short']
}

op_byte_arg_i71_da2 = {
    0: ['BLC', 'blc', 'Branch if not Left Conditions'],
    1: ['EXL', 'exl', 'EXtra code Legal'],
    2: ['BRC', 'brc', 'Branch if not Right Conditions'],
    3: ['NRF', 'nlz', 'NoRmalize Floating point']
}

op_no_arg_i72_dbc = {
    0b0000000: ['RIC', 'ric',  'Read Instruction Counter'],
    0b0000001: ['ZLB', 'zlb',  'Zero to Left Byte'],
    0b0000010: ['SXU', 'stxa', 'Set X as Upper bit'],
    0b0000011: ['NGA', 'nega', 'NeGation Arithmetic'],
    0b0000100: ['SLZ', 'slz',  'Shift Left add Zero'],
    0b0000101: ['SLY', 'shly', 'Shift Left add Y'],
    0b0000110: ['SLX', 'shlx', 'Shift Left add X'],
    0b0000111: ['SRY', 'sry',  'Shift Right, add Y'],
    0b0001000: ['NGL', 'neg',  'NeGate Logically'],
    0b0001001: ['RPC', 'rpc',  'Remember Program Conditions'],
#   0bTAAA010: ['SHC', '', ''], 
    0b1000000: ['RKY', 'rkey', 'Read KeYboard'],
    0b1000001: ['ZRB', 'zrb',  'Zero to Right Byte'],
    0b1000010: ['SXL', 'stxz', 'Set X as Lower bit'],
    0b1000011: ['NGC', 'nec',  'NeGation with Carry'],
    0b1000100: ['SVZ', 'shv',  'Shift left, check oVerflow, add Zero'],
    0b1000101: ['SVY', 'shvy', 'Shift left, check oVerflow, add Y'],
    0b1000110: ['SVX', 'shvx', 'Shift left, check oVerflow, add X'],
    0b1000111: ['SRX', 'shrx', 'Shift Right, add X'],
    0b1001000: ['SRZ', 'shr',  'Shift Right, add Zero'],
    0b1001001: ['LPC', 'lpc',  'Load Program Conditions']
}

op_no_arg_i73_da = {
    0b0000: ['HLT', 'stop', 'HaLT'],
    0b1000: ['HLT', 'stop', 'HaLT'],

    0b0001: ['MCL', 'mcl',  'Master CLear'],
    0b1001: ['MCL', 'mcl',  'Master CLear'],

    0b0011: ['GIU', 'giu',  'Generate Interrupt Upper'],
    0b1011: ['GIL', 'gil',  'Generate Interrupt Lower'],

    0b0100: ['LIP', 'lip',  'Leave to Interrupted Program'],
    0b1100: ['LIP', 'lip',  'Leave to Interrupted Program'],
    0b0101: ['CRON','cron', 'CPU modification ON']
}

op_no_arg_i73_dc = {
    0b0000: ['CIT',  'cit',  'Clear InTerrupts'],
    0b1000: ['CIT',  'cit',  'Clear InTerrupts'],
    0b0001: ['SIL',  'sil',  'Set Interrupt Lower'],
    0b1001: ['SIL',  'sil',  'Set Interrupt Lower'],
    0b0010: ['SIU',  'siu',  'Set Interrupt Upper'],
    0b1010: ['SIU',  'siu',  'Set Interrupt Upper'],
    0b0011: ['SIT',  'sit',  'Set InTerrupts'],
    0b1011: ['SIT',  'sit',  'Set InTerrupts'],
    0b0100: ['SINT', 'sint', 'Software INterrupt T'],
    0b1100: ['SIND', 'sind', 'Software INterrupt D']
}

op_basic_i74_a = {
    0: ['UJ', 'jp',  'Unconditional Jump'],
    1: ['JL', 'jpl', 'Jump if Less'],
    2: ['JE', 'jpe', 'Jump if Equal'],
    3: ['JG', 'jpg', 'Jump if Greater'],
    4: ['JZ', 'jz',  'Jump if Zero'],
    5: ['JM', 'jm',  'Jump if Minus'],
    6: ['JN', 'jn',  'Jump if Not equal'],
    7: ['LJ', 'jpr', 'Link Jump']
}    

op_basic_i75_a = {
    0: ['LD', 'ldd',  'Load Double word'],
    1: ['LF', 'ldf',  'Load Floating point'],
    2: ['LA', 'ldr',  'Load All registers'],
    3: ['LL', 'ldm',  'Load Last three registers'],
    4: ['TD', 'lddb', 'Take Double word'],
    5: ['TF', 'ldfb', 'Take Floating point'],
    6: ['TA', 'ldrb', 'Take to All registers'],
    7: ['TL', 'ldmb', 'Take to Last three registers']
}

op_basic_i76_a = {
    0: ['RD', 'std',  'Remember Double word'],
    1: ['RF', 'stf',  'Remember Floating point'],
    2: ['RA', 'str',  'Remember All registers'],
    3: ['RL', 'stm',  'Remember Last three registers'],
    4: ['PD', 'stdb', 'Put Double word'],
    5: ['PF', 'stfb', 'Put Floating point'],
    6: ['PA', 'strb', 'Put All registers'],
    7: ['PL', 'stmb', 'Put Last three registers']
}

op_basic_i77_a = {
    0: ['MB', 'mb',   'Modify Block arr. register'],
    1: ['IM', 'im',   'load Interrupt Mask'],
    2: ['KI', 'ki',   'Kill Interrupts'],
    3: ['FI', 'fi',   'Fetch Interrupts'],
    4: ['SP', 'sp',   'Start Program'],
    5: ['MD', 'mod',  'MoDify next instruction'],
    6: ['RZ', 'zs',   'Remember Zero'],
    7: ['IB', 'ados', 'Increment and Branch']
}


# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
