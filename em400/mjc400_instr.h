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

#define OP_OK 0
#define OP_ILLEGAL 1
#define OP_MD 2

int mjc400_op_illegal();

int mjc400_op_lw();
int mjc400_op_tw();
int mjc400_op_ls();
int mjc400_op_ri();
int mjc400_op_rw();
int mjc400_op_pw();
int mjc400_op_rj();
int mjc400_op_is();
int mjc400_op_bb();
int mjc400_op_bm();
int mjc400_op_bs();
int mjc400_op_bc();
int mjc400_op_bn();
int mjc400_op_ou();
int mjc400_op_in();

int mjc400_op_37();

int mjc400_op_aw();
int mjc400_op_ac();
int mjc400_op_sw();
int mjc400_op_cw();
int mjc400_op_or();
int mjc400_op_om();
int mjc400_op_nr();
int mjc400_op_nm();
int mjc400_op_er();
int mjc400_op_em();
int mjc400_op_xr();
int mjc400_op_xm();
int mjc400_op_cl();
int mjc400_op_lb();
int mjc400_op_rb();
int mjc400_op_cb();

int mjc400_op_awt();
int mjc400_op_trb();
int mjc400_op_irb();
int mjc400_op_drb();
int mjc400_op_cwt();
int mjc400_op_lwt();
int mjc400_op_lws();
int mjc400_op_rws();

int mjc400_op_70();
int mjc400_op_71();
int mjc400_op_72();
int mjc400_op_73();
int mjc400_op_74();
int mjc400_op_75();
int mjc400_op_76();
int mjc400_op_77();

int mjc400_op_37_ad();
int mjc400_op_37_sd();
int mjc400_op_37_mw();
int mjc400_op_37_dw();
int mjc400_op_37_af();
int mjc400_op_37_sf();
int mjc400_op_37_mf();
int mjc400_op_37_df();

int mjc400_op_70_ujs();
int mjc400_op_70_jls();
int mjc400_op_70_jes();
int mjc400_op_70_jgs();
int mjc400_op_70_jvs();
int mjc400_op_70_jxs();
int mjc400_op_70_jys();
int mjc400_op_70_jcs();

int mjc400_op_71_blc();
int mjc400_op_71_exl();
int mjc400_op_71_brc();
int mjc400_op_71_nrf();

int mjc400_op_72_ric();
int mjc400_op_72_zlb();
int mjc400_op_72_sxu();
int mjc400_op_72_nga();
int mjc400_op_72_slz();
int mjc400_op_72_sly();
int mjc400_op_72_slx();
int mjc400_op_72_sry();
int mjc400_op_72_ngl();
int mjc400_op_72_rpc();
int mjc400_op_72_shc();
int mjc400_op_72_rky();
int mjc400_op_72_zrb();
int mjc400_op_72_sxl();
int mjc400_op_72_ngc();
int mjc400_op_72_svz();
int mjc400_op_72_svy();
int mjc400_op_72_svx();
int mjc400_op_72_srx();
int mjc400_op_72_srz();
int mjc400_op_72_lpc();

int mjc400_op_73_hlt();
int mjc400_op_73_mcl();
int mjc400_op_73_cit();
int mjc400_op_73_sil();
int mjc400_op_73_siu();
int mjc400_op_73_sit();
int mjc400_op_73_giu();
int mjc400_op_73_gil();
int mjc400_op_73_lip();

int mjc400_op_74_uj();
int mjc400_op_74_jl();
int mjc400_op_74_je();
int mjc400_op_74_jg();
int mjc400_op_74_jz();
int mjc400_op_74_jm();
int mjc400_op_74_jn();
int mjc400_op_74_lj();

int mjc400_op_75_ld();
int mjc400_op_75_lf();
int mjc400_op_75_la();
int mjc400_op_75_ll();
int mjc400_op_75_td();
int mjc400_op_75_tf();
int mjc400_op_75_ta();
int mjc400_op_75_tl();

int mjc400_op_76_rd();
int mjc400_op_76_rf();
int mjc400_op_76_ra();
int mjc400_op_76_rl();
int mjc400_op_76_pd();
int mjc400_op_76_pf();
int mjc400_op_76_pa();
int mjc400_op_76_pl();

int mjc400_op_77_mb();
int mjc400_op_77_im();
int mjc400_op_77_ki();
int mjc400_op_77_fi();
int mjc400_op_77_sp();
int mjc400_op_77_md();
int mjc400_op_77_rz();
int mjc400_op_77_ib();


