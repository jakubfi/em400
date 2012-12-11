//  Copyright (c) 2012 Jakub Filipowicz <jakubf@gmail.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc.,
//  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

enum op_result {
	OP_ILLEGAL = -1,
	OP_OK,
	OP_MD
};

int op_illegal();

int op_lw();
int op_tw();
int op_ls();
int op_ri();
int op_rw();
int op_pw();
int op_rj();
int op_is();
int op_bb();
int op_bm();
int op_bs();
int op_bc();
int op_bn();
int op_ou();
int op_in();

int op_37();

int op_aw();
int op_ac();
int op_sw();
int op_cw();
int op_or();
int op_om();
int op_nr();
int op_nm();
int op_er();
int op_em();
int op_xr();
int op_xm();
int op_cl();
int op_lb();
int op_rb();
int op_cb();

int op_awt();
int op_trb();
int op_irb();
int op_drb();
int op_cwt();
int op_lwt();
int op_lws();
int op_rws();

int op_70();
int op_71();
int op_72();
int op_73();
int op_74();
int op_75();
int op_76();
int op_77();

int op_37_ad();
int op_37_sd();
int op_37_mw();
int op_37_dw();
int op_37_af();
int op_37_sf();
int op_37_mf();
int op_37_df();

int op_70_ujs();
int op_70_jls();
int op_70_jes();
int op_70_jgs();
int op_70_jvs();
int op_70_jxs();
int op_70_jys();
int op_70_jcs();

int op_71_blc();
int op_71_exl();
int op_71_brc();
int op_71_nrf();

int op_72_ric();
int op_72_zlb();
int op_72_sxu();
int op_72_nga();
int op_72_slz();
int op_72_sly();
int op_72_slx();
int op_72_sry();
int op_72_ngl();
int op_72_rpc();
int op_72_shc();
int op_72_rky();
int op_72_zrb();
int op_72_sxl();
int op_72_ngc();
int op_72_svz();
int op_72_svy();
int op_72_svx();
int op_72_srx();
int op_72_srz();
int op_72_lpc();

int op_73_hlt();
int op_73_mcl();
int op_73_cit();
int op_73_sil();
int op_73_siu();
int op_73_sit();
int op_73_giu();
int op_73_gil();
int op_73_lip();
int op_73_six();
int op_73_cix();

int op_74_uj();
int op_74_jl();
int op_74_je();
int op_74_jg();
int op_74_jz();
int op_74_jm();
int op_74_jn();
int op_74_lj();

int op_75_ld();
int op_75_lf();
int op_75_la();
int op_75_ll();
int op_75_td();
int op_75_tf();
int op_75_ta();
int op_75_tl();

int op_76_rd();
int op_76_rf();
int op_76_ra();
int op_76_rl();
int op_76_pd();
int op_76_pf();
int op_76_pa();
int op_76_pl();

int op_77_mb();
int op_77_im();
int op_77_ki();
int op_77_fi();
int op_77_sp();
int op_77_md();
int op_77_rz();
int op_77_ib();

#endif

// vim: tabstop=4
