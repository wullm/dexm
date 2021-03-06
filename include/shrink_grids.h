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

#ifndef SHRINK_GRIDS_H
#define SHRINK_GRIDS_H

#include "distributed_grid.h"

/* Shrink an N*N*N distributed grid into an M*M*M grid, where M divides N */
int shrinkGrid_dg(double *out, struct distributed_grid *in, int M, int N);

/* Shrink an N*N*N grid into an M*M*M grid, where M divides N */
int shrinkGrid(double *out, const double *in, int M, int N);


#endif
