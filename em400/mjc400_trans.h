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

#ifndef MJC400_TRANS_H
#define MJC400_TRANS_H

#include <inttypes.h>

int mjc400_trans_illegal(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans(uint16_t* memptr, char **buf);

int mjc400_trans_lw(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_tw(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_ls(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_ri(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_rw(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_pw(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_rj(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_is(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_bb(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_bm(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_bs(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_bc(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_bn(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_ou(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_in(uint8_t op, uint16_t* memptr, char **buf);

int mjc400_trans_37(uint8_t op, uint16_t* memptr, char **buf);

int mjc400_trans_aw(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_ac(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_sw(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_cw(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_or(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_om(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_nr(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_nm(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_er(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_em(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_xr(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_xm(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_cl(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_lb(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_rb(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_cb(uint8_t op, uint16_t* memptr, char **buf);

int mjc400_trans_awt(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_trb(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_irb(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_drb(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_cwt(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_lwt(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_lws(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_rws(uint8_t op, uint16_t* memptr, char **buf);

int mjc400_trans_70(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_71(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_72(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_73(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_74(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_75(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_76(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_77(uint8_t op, uint16_t* memptr, char **buf);

int mjc400_trans_37_ad(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_37_sd(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_37_mw(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_37_dw(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_37_af(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_37_sf(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_37_mf(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_37_df(uint8_t op, uint16_t* memptr, char **buf);

int mjc400_trans_70_ujs(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_70_jls(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_70_jes(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_70_jgs(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_70_jvs(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_70_jxs(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_70_jys(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_70_jcs(uint8_t op, uint16_t* memptr, char **buf);

int mjc400_trans_71_blc(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_71_exl(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_71_brc(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_71_nrf(uint8_t op, uint16_t* memptr, char **buf);

int mjc400_trans_72_ric(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_72_zlb(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_72_sxu(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_72_nga(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_72_slz(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_72_sly(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_72_slx(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_72_sry(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_72_ngl(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_72_rpc(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_72_shc(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_72_rky(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_72_zrb(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_72_sxl(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_72_ngc(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_72_svz(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_72_svy(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_72_svx(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_72_srx(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_72_srz(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_72_lpc(uint8_t op, uint16_t* memptr, char **buf);

int mjc400_trans_73_hlt(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_73_mcl(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_73_cit(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_73_sil(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_73_siu(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_73_sit(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_73_giu(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_73_gil(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_73_lip(uint8_t op, uint16_t* memptr, char **buf);

int mjc400_trans_74_uj(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_74_jl(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_74_je(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_74_jg(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_74_jz(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_74_jm(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_74_jn(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_74_lj(uint8_t op, uint16_t* memptr, char **buf);

int mjc400_trans_75_ld(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_75_lf(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_75_la(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_75_ll(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_75_td(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_75_tf(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_75_ta(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_75_tl(uint8_t op, uint16_t* memptr, char **buf);

int mjc400_trans_76_rd(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_76_rf(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_76_ra(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_76_rl(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_76_pd(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_76_pf(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_76_pa(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_76_pl(uint8_t op, uint16_t* memptr, char **buf);

int mjc400_trans_77_mb(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_77_im(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_77_ki(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_77_fi(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_77_sp(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_77_md(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_77_rz(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_trans_77_ib(uint8_t op, uint16_t* memptr, char **buf);

#endif

// vim: tabstop=4
