//  Copyright (c) 2012-2013 Jakub Filipowicz <jakubf@gmail.com>
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

void op_lw();
void op_tw();
void op_ls();
void op_ri();
void op_rw();
void op_pw();
void op_rj();
void op_is();
void op_bb();
void op_bm();
void op_bs();
void op_bc();
void op_bn();
void op_ou();
void op_in();

struct opdef * op_37();

void op_aw();
void op_ac();
void op_sw();
void op_cw();
void op_or();
void op_om();
void op_nr();
void op_nm();
void op_er();
void op_em();
void op_xr();
void op_xm();
void op_cl();
void op_lb();
void op_rb();
void op_cb();

void op_awt();
void op_trb();
void op_irb();
void op_drb();
void op_cwt();
void op_lwt();
void op_lws();
void op_rws();

struct opdef * op_70();
struct opdef * op_71();
struct opdef * op_72();
struct opdef * op_73();
struct opdef * op_74();
struct opdef * op_75();
struct opdef * op_76();
struct opdef * op_77();

void op_37_ad();
void op_37_sd();
void op_37_mw();
void op_37_dw();
void op_37_af();
void op_37_sf();
void op_37_mf();
void op_37_df();

void op_70_ujs();
void op_70_jls();
void op_70_jes();
void op_70_jgs();
void op_70_jvs();
void op_70_jxs();
void op_70_jys();
void op_70_jcs();

void op_71_blc();
void op_71_exl();
void op_71_brc();
void op_71_nrf();

void op_72_ric();
void op_72_zlb();
void op_72_sxu();
void op_72_nga();
void op_72_slz();
void op_72_sly();
void op_72_slx();
void op_72_sry();
void op_72_ngl();
void op_72_rpc();
void op_72_shc();
void op_72_rky();
void op_72_zrb();
void op_72_sxl();
void op_72_ngc();
void op_72_svz();
void op_72_svy();
void op_72_svx();
void op_72_srx();
void op_72_srz();
void op_72_lpc();

void op_73_hlt();
void op_73_mcl();
void op_73_cit();
void op_73_sil();
void op_73_siu();
void op_73_sit();
void op_73_giu();
void op_73_gil();
void op_73_lip();
void op_73_sint();
void op_73_sind();
void op_73_cron();

void op_74_uj();
void op_74_jl();
void op_74_je();
void op_74_jg();
void op_74_jz();
void op_74_jm();
void op_74_jn();
void op_74_lj();

void op_75_ld();
void op_75_lf();
void op_75_la();
void op_75_ll();
void op_75_td();
void op_75_tf();
void op_75_ta();
void op_75_tl();

void op_76_rd();
void op_76_rf();
void op_76_ra();
void op_76_rl();
void op_76_pd();
void op_76_pf();
void op_76_pa();
void op_76_pl();

void op_77_mb();
void op_77_im();
void op_77_ki();
void op_77_fi();
void op_77_sp();
void op_77_md();
void op_77_rz();
void op_77_ib();

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
