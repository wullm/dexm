/*******************************************************************************
 * This file is part of Mitos.
 * Copyright (c) 2020 Willem Elbers (whe@willemelbers.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

#ifndef FIREBOLT_INTERFACE_H
#define FIREBOLT_INTERFACE_H

#include <fftw3.h>
#include <firebolt_nano.h>
#include "input.h"
#include "perturb_data.h"

struct firebolt_interface {
    struct grids *grids_ref;
    int q_size;
    double log_q_min;
    double log_q_max;
};

int initFirebolt(const struct params *pars, const struct cosmology *cosmo,
                 const struct units *us, const struct perturb_data *ptdat,
                 const struct perturb_spline *spline,
                 struct firebolt_interface *firebolt, const fftw_complex *grf,
                 double M_nu_eV, double T_nu_eV);
int cleanFirebolt(void);

#endif
