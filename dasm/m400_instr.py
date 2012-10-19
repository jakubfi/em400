#  Copyright (c) 2012 Jakub Filipowicz <jakubf@gmail.com>
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

# ------------------------------------------------------------------------
def m400_get_opcode(i, d, a, b, c):
    # all except basic opcodes are in dictionaries
    code = m400_opcodes[i][0]
    group = m400_opcodes[i][1]
    desc = m400_opcodes[i][3]
    if code == '':
        code, desc = m400_opcodes[i][2](i, d, a, b, c)

    # basic opcodes have suffixes
    if m400_opcodes[i][1] in (OP_NORM2, OP_NORM1):
        # basic opcode, no direct arg
        if c != 0:
            if d == 0:
                suffix = "R"
            if d == 1:
                suffix = "A"
        # basic opcode with direct 16-bit arg
        if c == 0:
            if d == 0:
                suffix = "D"
            if d == 1:
                suffix = "I"    
        code += suffix

    return code, group, desc

# ------------------------------------------------------------------------
def m400_name_i37(i, d, a, b, c):
    return op_basic_i37_a[a][0], op_basic_i37_a[a][1]

# ------------------------------------------------------------------------
def m400_name_i70(i, d, a, b, c):
    # treated by assembler as UJS, 0
    if (d==0) and (a==0) and (b==0) and (c==0):
        return 'NOP', 'NO Operation'
    else:
        return op_short_arg_i70_a[a][0], op_short_arg_i70_a[a][1]

# ------------------------------------------------------------------------
def m400_name_i71(i, d, a, b, c):
    da2 = (d<<1) | (a>>2)
    if (da2 == 0b11) and (a != 7):
        return "ILLEGAL", "ILLEGAL"
    else:
        return op_byte_arg_i71_da2[da2][0], op_byte_arg_i71_da2[da2][1]

# ------------------------------------------------------------------------
def m400_name_i72(i, d, a, b, c):
    dbc = (d<<6) | (b<<3) | (c)
    if (b == 0b010):
        return "SHC", "SHift Cyclic"
    else:
        try:
            return op_no_arg_i72_dbc[dbc][0], op_no_arg_i72_dbc[dbc][1]
        except Exception, e:
            return "ILLEGAL", "ILLEGAL"

# ------------------------------------------------------------------------
def m400_name_i73(i, d, a, b, c):
    if a != 2:
        try:
            return op_no_arg_i73_a[a][0], op_no_arg_i73_a[a][1]
        except Exception, e:
            return "ILLEGAL", "ILLEGAL"
    else:
        c10 = c & 0b11
        try:
            return op_no_arg_i73_c10[c10][0], op_no_arg_i73_c10[c10][1]
        except Exception, e:
            return "ILLEGAL", "ILLEGAL"

# ------------------------------------------------------------------------
def m400_name_i74(i, d, a, b, c):
    return op_basic_i74_a[a][0], op_basic_i74_a[a][1]

# ------------------------------------------------------------------------
def m400_name_i75(i, d, a, b, c):
    return op_basic_i75_a[a][0], op_basic_i75_a[a][1]

# ------------------------------------------------------------------------
def m400_name_i76(i, d, a, b, c):
    return op_basic_i76_a[a][0], op_basic_i76_a[a][1]

# ------------------------------------------------------------------------
def m400_name_i77(i, d, a, b, c):
    return op_basic_i77_a[a][0], op_basic_i77_a[a][1]

OP_NORM2 = 0
OP_NORM1 = 1
OP_SHORT2 = 2
OP_SHORT1 = 3
OP_BYTE = 4
OP_NO = 5
OP_NO2 = 6

m400_opcodes = {
    020: ['LW',  OP_NORM2, None, 'Load Word'],
    021: ['TW',  OP_NORM2, None, 'Take Word'],
    022: ['LS',  OP_NORM2, None, 'Load Selective'],
    023: ['RI',  OP_NORM2, None, 'Remember and Increment'],
    024: ['RW',  OP_NORM2, None, 'Remember Word'],
    025: ['PW',  OP_NORM2, None, 'Put Word'],
    026: ['RJ',  OP_NORM2, None, 'Return Jump'],
    027: ['IS',  OP_NORM2, None, 'Install Semaphore'],
    030: ['BB',  OP_NORM2, None, 'Branch on Bits'],
    031: ['BM',  OP_NORM2, None, 'Branch on bits in Memory'],
    032: ['BS',  OP_NORM2, None, 'Branch Selective'],
    033: ['BC',  OP_NORM2, None, 'Branch if not all Conditions'],
    034: ['BN',  OP_NORM2, None, 'Branch if No conditions'],
    035: ['OU',  OP_NORM2, None, 'OUtput data'],
    036: ['IN',  OP_NORM2, None, 'INput data'],
    037: ['',    OP_NORM1, m400_name_i37, ''],
    040: ['AW',  OP_NORM2, None, 'Add Word'],
    041: ['AC',  OP_NORM2, None, 'Add word with Carry'],
    042: ['SW',  OP_NORM2, None, 'Subtract Word'],
    043: ['CW',  OP_NORM2, None, 'Compare Word'],
    044: ['OR',  OP_NORM2, None, 'Or Register'],
    045: ['OM',  OP_NORM2, None, 'Or Memory'],
    046: ['NR',  OP_NORM2, None, 'aNd in Register'],
    047: ['NM',  OP_NORM2, None, 'aNd in Memory'],
    050: ['ER',  OP_NORM2, None, 'Erase bits in Register'],
    051: ['EM',  OP_NORM2, None, 'Erase bits in Memory'],
    052: ['XR',  OP_NORM2, None, 'eXclusive or in Register'],
    053: ['XM',  OP_NORM2, None, 'eXclusive or in Memory'],
    054: ['CL',  OP_NORM2, None, 'Compare Logically'],
    055: ['LB',  OP_NORM2, None, 'Load Byte'],
    056: ['RB',  OP_NORM2, None, 'Remember Byte'],
    057: ['CB',  OP_NORM2, None, 'Compare Byte'],
    060: ['AWT', OP_SHORT2, None, 'Add to Word parameTer'],
    061: ['TRB', OP_SHORT2, None, 'parameTer to Register and Branch'],
    062: ['IRB', OP_SHORT2, None, 'Increment Register and Branch'],
    063: ['DRB', OP_SHORT2, None, 'Decrease Register and Branch'],
    064: ['CWT', OP_SHORT2, None, 'Compare Word to parameTer'],
    065: ['LWT', OP_SHORT2, None, 'Load to Word paremeTer'],
    066: ['LWS', OP_SHORT2, None, 'Load to Word Short'],
    067: ['RWS', OP_SHORT2, None, 'Remember Word Short'],
    070: ['',    OP_SHORT1, m400_name_i70, ''],
    071: ['',    OP_BYTE,  m400_name_i71, ''],
    072: ['',    OP_NO2,   m400_name_i72, ''],
    073: ['',    OP_NO,    m400_name_i73, ''],
    074: ['',    OP_NORM1, m400_name_i74, ''],
    075: ['',    OP_NORM1, m400_name_i75, ''],
    076: ['',    OP_NORM1, m400_name_i76, ''],
    077: ['',    OP_NORM1, m400_name_i77, '']
}

op_basic_i37_a = {
    0: ['AD', 'Add Double word'],
    1: ['SD', 'Subtract Double word'],
    2: ['MW', 'Multiply Words'],
    3: ['DW', 'Divide Words'],
    4: ['AF', 'Add Floating point'],
    5: ['SF', 'Subtract Floating point'],
    6: ['MF', 'Multiply Floating point'],
    7: ['DF', 'Divide Floating point']
}

op_short_arg_i70_a = {
    0: ['UJS', 'Unconditional Jump Short'],
    1: ['JLS', 'Jump if Less Short'],
    2: ['JES', 'Jump if Equal Short'],
    3: ['JGS', 'Jump if Greater Short'],
    4: ['JVS', 'Jump if oVerflow Short'],
    5: ['JXS', 'Jump if X Short'],
    6: ['JYS', 'Jump if Y Short'],
    7: ['JCS', 'Jump if Carry Short']
}

op_byte_arg_i71_da2 = {
    0: ['BLC', 'Branch if not Left Conditions'],
    1: ['EXL', 'EXtra code Legal'],
    2: ['BRC', 'Branch if not Right Conditions'],
    3: ['NRF', 'NoRmalize Floating point']
}

op_no_arg_i72_dbc = {
    0b0000000: ['RIC', 'Read Instruction Counter'],
    0b0000001: ['ZLB', 'Zero to Left Byte'],
    0b0000010: ['SXU', 'Set X as Upper bit'],
    0b0000011: ['NGA', 'NeGation Arithmetic'],
    0b0000100: ['SLZ', 'Shift Left add Zero'],
    0b0000101: ['SLY', 'Shift Left add Y'],
    0b0000110: ['SLX', 'Shift Left add X'],
    0b0000111: ['SRY', 'Shift Right, add Y'],
    0b0001000: ['NGL', 'NeGate Logically'],
    0b0001001: ['RPC', 'Remember Program Conditions'],
#   0bTAAA010: ['SHC'],
    0b1000000: ['RKY', 'Read KeYboard'],
    0b1000001: ['ZRB', 'Zero to Right Byte'],
    0b1000010: ['SXL', 'Set X as Lower bit'],
    0b1000011: ['NGC', 'NeGation with Carry'],
    0b1000100: ['SVZ', 'Shift left, check oVerflow, add Zero'],
    0b1000100: ['SVY', 'Shift left, check oVerflow, add Y'],
    0b1000110: ['SVX', 'Shift left, check oVerflow, add X'],
    0b1000111: ['SRX', 'Shift Right, add X'],
    0b1001000: ['SRZ', 'Shift Right, add Zero'],
    0b1001001: ['LPC', 'Load Program Conditions']

}

op_no_arg_i73_a = {
    0: ['HLT', 'HaLT'],
    1: ['MCL', 'Master CLear'],
    3: ['GIU', 'Generate Interrupt Upper'],
    4: ['LIP', 'Leave to Interrupted Program'],
    5: ['GIL', 'Generate Interrupt Lower']
}

op_no_arg_i73_c10 = {
    0: ['CIT', 'Clear InTerrupts'],
    1: ['SIL', 'Set Interrupt Lower'],
    2: ['SIU', 'Set Interrupt Upper'],
    3: ['SIT', 'Set InTerrupts']
}

op_basic_i74_a = {
    0: ['UJ', 'Unconditional Jump'],
    1: ['JL', 'Jump if Less'],
    2: ['JE', 'Jump if Equal'],
    3: ['JG', 'Jump if Greater'],
    4: ['JZ', 'Jump if Zero'],
    5: ['JM', 'Jump if Minus'],
    6: ['JN', 'Jump if Not equal'],
    7: ['LJ', 'Link Jump']
}    

op_basic_i75_a = {
    0: ['LD', 'Load Double word'],
    1: ['LF', 'Load Floating point'],
    2: ['LA', 'Load All registers'],
    3: ['LL', 'Load Last three registers'],
    4: ['TD', 'Take Double word'],
    5: ['TF', 'Take Floating point'],
    6: ['TA', 'Take to All registers'],
    7: ['TL', 'Take to Last three registers']
}

op_basic_i76_a = {
    0: ['RD', 'Remember Double word'],
    1: ['RF', 'Remember Floating point'],
    2: ['RA', 'Remember All registers'],
    3: ['RL', 'Remember Last three registers'],
    4: ['PD', 'Put Double word'],
    5: ['PF', 'Put Floating point'],
    6: ['PA', 'Put All registers'],
    7: ['PL', 'Put Last three registers']
}

op_basic_i77_a = {
    0: ['MB', 'Modify Block arr. register'],
    1: ['IM', 'load Interrupt Mask'],
    2: ['KI', 'Kill Interrupts'],
    3: ['FI', 'Fetch Interrupts'],
    4: ['SP', 'Start Program'],
    5: ['MD', 'MoDify next instruction'],
    6: ['RZ', 'Remember Zero'],
    7: ['IB', 'Increment and Branch']
}




# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
