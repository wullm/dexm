/*******************************************************************************
 * This file is part of DEXM.
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

#include <math.h>
#include "../include/primordial.h"
#include "../include/transfer_interp.h"

const struct cosmology *cosmology;

int initPrimordial(const struct params *pars, const struct cosmology *cosmo) {
    cosmology = cosmo;

    return 0;
}

double primordialPower(double k) {
    if (k == 0) return 0;

    double A_s = cosmology->A_s;
    double n_s = cosmology->n_s;
    double k_pivot = cosmology->k_pivot;

    return A_s * pow(k/k_pivot, n_s);
}

/* The full power spectrum P(k) = P_primo(k) * Transfer(k)^2 */
double fullPower(double k) {
    if (k == 0) return 0;

    double Pr = primordialPower(k);
    double Tr = tr_func_at_k(k);

    return Pr * Tr * Tr;
}