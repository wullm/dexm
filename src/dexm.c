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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/dexm.h"

const char *fname;

int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("No parameter file specified.\n");
        return 0;
    }

    /* Read options */
    const char *fname = argv[1];
    printf("The parameter file is %s\n", fname);

    struct params pars;
    struct units us;
    struct particle_type *types = NULL;
    struct cosmology cosmo;
    struct transfer trs;

    readParams(&pars, fname);
    readUnits(&us, fname);
    readCosmology(&cosmo, fname);
    readTypes(&pars, &types, fname);

    int err;
    err = readTransfers(&pars, &us, &cosmo, &trs);
    if(err) {
        return 0;
    }

    printf("Creating initial conditions for: \"%s\".\n", pars.Name);

    /* Seed the random number generator */
    srand(pars.Seed);

    if (pars.Homogeneous) {
        printf("We are in homogeneous mode.\n");

        struct particle **parts = malloc(pars.NumParticleTypes * sizeof(*parts));

        /* Allocate enough memory for one chunk */
        allocParticles(&parts[0], &pars, &types[0]);

        for (int chunk=0; chunk<types[0].Chunks; chunk++) {
            printf("Generating chunk %d.\n", chunk);
            genParticles_FromGrid(&parts[0], &pars, &us, &cosmo, &types[0], chunk);
        }

        /* Clean the particles up */
        cleanParticles(&parts[0], &pars, &types[0]);
    }

    /* Clean up */
    cleanTransfers(&trs);
    cleanTypes(&pars, &types);

}
