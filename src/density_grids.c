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

#include <hdf5.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "../include/density_grids.h"
#include "../include/fft.h"
#include "../include/fft_kernels.h"
#include "../include/output.h"
#include "../include/titles.h"

/* Generate a density grid for each particle type by applying the transfer
 * function to the random phases. The spline struct is used to interpolate
 * the transfer functions.
 */
int generateDensityGrids(const struct params *pars, const struct units *us,
                         const struct cosmology *cosmo,
                         const struct perturb_spline *spline,
                         struct particle_type *types,
                         const fftw_complex *grf) {

    /* Grid dimensions */
    const int N = pars->GridSize;
    const double boxlen = pars->BoxLen;

    /* Find the interpolation index along the time dimension */
    double log_tau = cosmo->log_tau_ini; //log of conformal time
    int tau_index; //greatest lower bound bin index
    double u_tau; //spacing between subsequent bins
    perturbSplineFindTau(spline, log_tau, &tau_index, &u_tau);

    /* Create complex 3D arrays */
    fftw_complex *fbox = (fftw_complex*) fftw_malloc(N*N*(N/2+1)*sizeof(fftw_complex));

    /* For each particle type, create the corresponding density field */
    for (int pti = 0; pti < pars->NumParticleTypes; pti++) {
        /* The current particle type */
        struct particle_type *ptype = types + pti;
        const char *Identifier = ptype->Identifier;

        /* The user-defined title of the density transfer function */
        const char *title = ptype->TransferFunctionDensity;

        /* Find the title among the transfer functions */
        int index_src = findTitle(spline->ptdat->titles, title, spline->ptdat->n_functions);
        if (index_src < 0) {
            printf("Error: transfer function '%s' not found (%d).\n", title, index_src);
            return 1;
        }

        /* Copy the complex random field into the complex array */
        memcpy(fbox, grf, N*N*(N/2+1)*sizeof(fftw_complex));

        /* Package the spline parameters */
        struct spline_params sp = {spline, index_src, tau_index, u_tau};

        /* Apply the transfer function to fbox */
        fft_apply_kernel(fbox, fbox, N, boxlen, kernel_transfer_function, &sp);

        /* Export the real box */
        char dbox_fname[DEFAULT_STRING_LENGTH];
        sprintf(dbox_fname, "%s/%s%s%s", pars->OutputDirectory, "density_", Identifier, ".hdf5");
        fft_c2r_export(fbox, N, boxlen, dbox_fname);
        printf("Density field '%s' exported to '%s'.\n", title, dbox_fname);
    }

    /* Free all the FFT objects */
    fftw_free(fbox);

    return 0;
}

/* Generate an energy flux grid for each particle type by applying the transfer
 * function to the random phases. The spline struct is used to interpolate
 * the transfer functions. The energy flux is theta = div.velocity.
 */
int generateEnergyFluxGrids(const struct params *pars, const struct units *us,
                           const struct cosmology *cosmo,
                           const struct perturb_spline *spline,
                           struct particle_type *types,
                           const fftw_complex *grf) {

    /* Grid dimensions */
    const int N = pars->GridSize;
    const double boxlen = pars->BoxLen;

    /* How many grids have we computed? */
    int counter = 0;

    /* Find the interpolation index along the time dimension */
    double log_tau = cosmo->log_tau_ini; //log of conformal time
    int tau_index; //greatest lower bound bin index
    double u_tau; //spacing between subsequent bins
    perturbSplineFindTau(spline, log_tau, &tau_index, &u_tau);

    /* Create complex 3D arrays */
    fftw_complex *fbox = (fftw_complex*) fftw_malloc(N*N*(N/2+1)*sizeof(fftw_complex));

    /* For each particle type, create the corresponding density field */
    for (int pti = 0; pti < pars->NumParticleTypes; pti++) {
        /* The current particle type */
        struct particle_type *ptype = types + pti;
        const char *Identifier = ptype->Identifier;

        /* The user-defined title of the velocity transfer function */
        const char *title = ptype->TransferFunctionVelocity;

        /* Skip if not specified. We will use Zel'dovich instead. */
        if (strcmp("", title) == 0) continue;
        counter++;

        /* Find the title among the transfer functions */
        int index_src = findTitle(spline->ptdat->titles, title, spline->ptdat->n_functions);
        if (index_src < 0) {
            printf("Error: transfer function '%s' not found (%d).\n", title, index_src);
            return 1;
        }

        /* Copy the complex random field into the complex array */
        memcpy(fbox, grf, N*N*(N/2+1)*sizeof(fftw_complex));

        /* Package the spline parameters */
        struct spline_params sp = {spline, index_src, tau_index, u_tau};

        /* Apply the transfer function to fbox */
        fft_apply_kernel(fbox, fbox, N, boxlen, kernel_transfer_function, &sp);

        /* Export the real box */
        char dbox_fname[DEFAULT_STRING_LENGTH];
        sprintf(dbox_fname, "%s/%s%s%s", pars->OutputDirectory, "theta_", Identifier, ".hdf5");
        fft_c2r_export(fbox, N, boxlen, dbox_fname);
        printf("Energy flux field '%s' exported to '%s'.\n", title, dbox_fname);
    }

    /* Free all the FFT objects */
    fftw_free(fbox);

    if (counter == 0) {
        printf("No velocity fields requested (specify TransferFunctionVelocity).\n");
    }

    return 0;
}
