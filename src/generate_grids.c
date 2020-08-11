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

#include <hdf5.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "../include/generate_grids.h"
#include "../include/mitos.h"

/* Generate a perturbation theory grid for each particle type by applying the
 * desired transfer function (given by the titles array) to the random phases.
 * The spline struct is used to interpolate the transfer functions.
 */
int generatePerturbationGrids(const struct params *pars, const struct units *us,
                              const struct cosmology *cosmo,
                              const struct perturb_spline *spline,
                              struct particle_type *types, char **titles,
                              const char *grf_fname, const char *grid_name) {

    /* Grid dimensions */
    const int N = pars->GridSize;
    const double boxlen = pars->BoxLen;

    /* Find the interpolation index along the time dimension */
    double log_tau = cosmo->log_tau_ini; //log of conformal time
    int tau_index; //greatest lower bound bin index
    double u_tau; //spacing between subsequent bins
    perturbSplineFindTau(spline, log_tau, &tau_index, &u_tau);

    /* For each particle type, create the corresponding density field */
    for (int pti = 0; pti < pars->NumParticleTypes; pti++) {
        /* The current particle type */
        struct particle_type *ptype = types + pti;
        const char *Identifier = ptype->Identifier;

        /* The user-defined title of the density transfer function */
        const char *title = titles[pti];

        /* Skip if not specified */
        if (strcmp("", title) == 0) continue;

        /* Find the title among the transfer functions */
        int index_src = findTitle(spline->ptdat->titles, title, spline->ptdat->n_functions);
        if (index_src < 0) {
            printf("Error: transfer function '%s' not found (%d).\n", title, index_src);
            return 1;
        }

        /* Create 3D arrays */
        double *box = (double*) fftw_malloc(N*N*N*sizeof(double));
        fftw_complex *fbox = (fftw_complex*) fftw_malloc(N*N*(N/2+1)*sizeof(fftw_complex));

        /* Load the Gaussian random field */
        int err = readGRF_inPlace_H5(box, grf_fname);
        if (err > 0) return err;

        /* Create FFT plans (destroys input) */
        fftw_plan r2c = fftw_plan_dft_r2c_3d(N, N, N, box, fbox, FFTW_ESTIMATE | FFTW_DESTROY_INPUT);

        /* Execute and normalize */
        fft_execute(r2c);
        fft_normalize_r2c(fbox,N,boxlen);

        /* Free the destroyed real-space box */
        fftw_free(box);
        fftw_destroy_plan(r2c);

        /* Package the spline parameters */
        struct spline_params sp = {spline, index_src, tau_index, u_tau};

        /* Apply the transfer function to fbox */
        fft_apply_kernel(fbox, fbox, N, boxlen, kernel_transfer_function, &sp);

        /* Export the real box */
        char dbox_fname[DEFAULT_STRING_LENGTH];
        sprintf(dbox_fname, "%s/%s_%s%s", pars->OutputDirectory, grid_name, Identifier, ".hdf5");
        fft_c2r_export_and_free(fbox, N, boxlen, dbox_fname); //frees fbox
        printf("Perturbation field '%s' exported to '%s'.\n", title, dbox_fname);
    }

    return 0;
}
