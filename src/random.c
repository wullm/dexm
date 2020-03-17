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

#include <stdlib.h>
#include <math.h>

#include "../include/random.h"
#include "../include/dexm.h"

/* Generate standard normal variable with Box-Mueller */
double sampleNorm() {
    double u = (double) rand()/RAND_MAX;
    double v = (double) rand()/RAND_MAX;

    double z0 = sqrt(-2 * log(u)) * cos(2 * M_PI * v);
    //double z1 = sqrt(-2 * log(u)) * sin(2 * M_PI * v);

    return z0;
}
