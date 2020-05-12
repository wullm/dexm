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
#include <string.h>
#include <hdf5.h>
#include <assert.h>
#include <math.h>
#include "../include/input.h"
#include "../include/fft.h"

int readParams(struct params *pars, const char *fname) {
     pars->Seed = ini_getl("Random", "Seed", 1, fname);

     pars->GridSize = ini_getl("Box", "GridSize", 64, fname);
     pars->BoxLen = ini_getd("Box", "BoxLen", 1.0, fname);
     pars->Splits = ini_getl("Box", "Splits", 1, fname);
     pars->GridX = ini_getl("Box", "GridX", 0, fname);
     pars->GridY = ini_getl("Box", "GridY", 0, fname);
     pars->GridZ = ini_getl("Box", "GridZ", 0, fname);

     pars->MaxParticleTypes = ini_getl("Simulation", "MaxParticleTypes", 1, fname);
     pars->NumParticleTypes = 0; //should not be read, but inferred
     pars->Homogeneous = ini_getbool("Simulation", "Homogeneous", 0, fname);
     pars->SlabSize = ini_getl("Read", "SlabSize", 8000000, fname);

     /* Read strings */
     int len = DEFAULT_STRING_LENGTH;
     pars->OutputDirectory = malloc(len);
     pars->Name = malloc(len);
     pars->TransferFunctionsFile = malloc(len);
     pars->TransferFunctionsFormat = malloc(len);
     pars->InputFilename = malloc(len);
     pars->OutputFilename = malloc(len);
     ini_gets("Output", "Directory", "./output", pars->OutputDirectory, len, fname);
     ini_gets("Simulation", "Name", "No Name", pars->Name, len, fname);
     ini_gets("TransferFunctions", "File", "", pars->TransferFunctionsFile, len, fname);
     ini_gets("TransferFunctions", "Format", "Plain", pars->TransferFunctionsFormat, len, fname);
     ini_gets("Output", "Filename", "particles.hdf5", pars->OutputFilename, len, fname);
     ini_gets("Read", "Filename", "", pars->InputFilename, len, fname);

     return 0;
}

int readUnits(struct units *us, const char *fname) {
    /* Internal units */
    us->UnitLengthMetres = ini_getd("Units", "UnitLengthMetres", 1.0, fname);
    us->UnitTimeSeconds = ini_getd("Units", "UnitTimeSeconds", 1.0, fname);
    us->UnitMassKilogram = ini_getd("Units", "UnitMassKilogram", 1.0, fname);

    /* Get the transfer functions format */
    char format[DEFAULT_STRING_LENGTH];
    ini_gets("TransferFunctions", "Format", "Plain", format, DEFAULT_STRING_LENGTH, fname);

    /* Format of the transfer functions */
    int default_h_exponent;
    int default_k_exponent;
    int default_sign;
    if (strcmp(format, "CLASS") == 0) {
        default_h_exponent = 1;
        default_k_exponent = 0;
        default_sign = -1;
    } else {
        default_h_exponent = 0;
        default_k_exponent = -2;
        default_sign = +1;
    }
    us->TransferUnitLengthMetres = ini_getd("TransferFunctions", "UnitLengthMetres", MPC_METRES, fname);
    us->Transfer_hExponent = ini_getl("TransferFunctions", "hExponent", default_h_exponent, fname);
    us->Transfer_kExponent = ini_getl("TransferFunctions", "kExponent", default_k_exponent, fname);
    us->Transfer_Sign = ini_getl("TransferFunctions", "Sign", default_sign, fname);

    return 0;
}

int readCosmology(struct cosmology *cosmo, const char *fname) {
     cosmo->h = ini_getd("Cosmology", "h", 0.70, fname);
     cosmo->n_s = ini_getd("Cosmology", "n_s", 0.97, fname);
     cosmo->A_s = ini_getd("Cosmology", "A_s", 2.215e-9, fname);
     cosmo->k_pivot = ini_getd("Cosmology", "k_pivot", 0.05, fname);

     return 0;
}

int cleanParams(struct params *pars) {
    free(pars->OutputDirectory);
    free(pars->Name);
    free(pars->TransferFunctionsFile);
    free(pars->TransferFunctionsFormat);

    return 0;
}

int readGRF_H5(double **box, int *N, double *box_len, const char *fname) {
    /* Create the hdf5 file */
    hid_t h_file = H5Fopen(fname, H5F_ACC_RDONLY, H5P_DEFAULT);

    /* Create the Header group */
    hid_t h_grp = H5Gopen(h_file, "Header", H5P_DEFAULT);

    /* Read the size of the field */
    hid_t h_attr, h_err;
    double boxsize[3];

    /* Open and read out the attribute */
    h_attr = H5Aopen(h_grp, "BoxSize", H5P_DEFAULT);
    h_err = H5Aread(h_attr, H5T_NATIVE_DOUBLE, &boxsize);
    assert(h_err >= 0);

    /* It should be a cube */
    assert(boxsize[0] == boxsize[1]);
    assert(boxsize[1] == boxsize[2]);
    *box_len = boxsize[0];

    /* Close the attribute, and the Header group */
    H5Aclose(h_attr);
    H5Gclose(h_grp);

    /* Open the Field group */
    h_grp = H5Gopen(h_file, "Field", H5P_DEFAULT);

    /* Open the Field dataset */
    hid_t h_data = H5Dopen2(h_grp, "Field", H5P_DEFAULT);

    /* Open the dataspace and fetch the grid dimensions */
    hid_t h_space = H5Dget_space(h_data);
    int ndims = H5Sget_simple_extent_ndims(h_space);
    hsize_t *dims = malloc(ndims * sizeof(hsize_t));
    H5Sget_simple_extent_dims(h_space, dims, NULL);

    /* We should be in 3D */
    assert(ndims == 3);
    /* It should be a cube */
    assert(dims[0] == dims[1]);
    assert(dims[1] == dims[2]);
    *N = dims[0];

    /* Allocate the array */
    *box = malloc(dims[0] * dims[1] * dims[2] * sizeof(double));

    /* Read out the data */
    h_err = H5Dread(h_data, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, *box);

    /* Close the dataspace and dataset */
    H5Sclose(h_space);
    H5Dclose(h_data);

    /* Close the Field group */
    H5Gclose(h_grp);

    /* Close the file */
    H5Fclose(h_file);

    return 0;
}

/* Read the box without any checks, assuming we have sufficient memory
 * allocated. */
int readGRF_inPlace_H5(double *box, const char *fname) {
    /* Create the hdf5 file */
    hid_t h_file = H5Fopen(fname, H5F_ACC_RDONLY, H5P_DEFAULT);

    /* Open the Field group */
    hid_t h_grp = H5Gopen(h_file, "Field", H5P_DEFAULT);

    /* Open the Field dataset */
    hid_t h_data = H5Dopen2(h_grp, "Field", H5P_DEFAULT);

    /* Open the dataspace and fetch the grid dimensions */
    hid_t h_space = H5Dget_space(h_data);
    int ndims = H5Sget_simple_extent_ndims(h_space);
    hsize_t *dims = malloc(ndims * sizeof(hsize_t));
    H5Sget_simple_extent_dims(h_space, dims, NULL);

    /* Read out the data */
    hid_t h_err = H5Dread(h_data, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, box);

    /* Close the dataspace and dataset */
    H5Sclose(h_space);
    H5Dclose(h_data);

    /* Close the Field group */
    H5Gclose(h_grp);

    /* Close the file */
    H5Fclose(h_file);

    return 0;
}


int readFieldChunk_H5(double *chunk_data, int N, int num_chunks, int chunk_id,
                      const char *fname) {

    /* Number of chunks along one dimension */
    int L = cbrt(num_chunks);

    /* Size of a chunk along one dimension */
    int M = N/L;

    /* Verify chunking dimensions */
    if (L*L*L != num_chunks || M*L != N) {
        printf("Error: chunk dimensions do not divide grid dimensions.\n");
        return 1;
    }

    /* Open the hdf5 file */
    hid_t h_file = H5Fopen(fname, H5F_ACC_RDWR, H5P_DEFAULT);

    /* Open the Header group */
    hid_t h_grp = H5Gopen(h_file, "Field", H5P_DEFAULT);

    /* Open the Field dataset */
    hid_t h_data = H5Dopen2(h_grp, "Field", H5P_DEFAULT);

    /* Get the file dataspace */
    hid_t h_space = H5Dget_space(h_data);

    /* The chunk in question */
    const hsize_t chunk_rank = 3;
    const hsize_t chunk_dims[3] = {M, M, M}; //3D space

    /* The chunk position */
    int chunk_x, chunk_y, chunk_z;
    inverse_row_major(chunk_id, &chunk_x, &chunk_y, &chunk_z, L);

    /* Offset of the chunk inside the grid */
    const hsize_t chunk_offset[3] = {chunk_x * M, chunk_y * M, chunk_z * M};

    printf("%lld %lld %lld\n", chunk_offset[0], chunk_offset[1], chunk_offset[2]);
    printf("%lld %lld %lld\n", chunk_offset[0]+chunk_dims[0], chunk_offset[1]+chunk_dims[1], chunk_offset[2]+chunk_dims[2]);

    /* Create memory space for the chunk */
    hid_t h_memspace = H5Screate_simple(chunk_rank, chunk_dims, NULL);
    H5Sselect_hyperslab(h_space, H5S_SELECT_SET, chunk_offset, NULL, chunk_dims, NULL);

    /* Read the data */
    hid_t h_err = H5Dread(h_data, H5T_NATIVE_DOUBLE, h_memspace, h_space, H5P_DEFAULT, chunk_data);
    if (h_err < 0) {
        printf("Error: reading chunk of hdf5 data.\n");
        return 1;
    }

    /* Close the dataset, corresponding dataspace, and the Field group */
    H5Dclose(h_data);
    H5Sclose(h_space);
    H5Sclose(h_memspace);
    H5Gclose(h_grp);

    /* Close the file */
    H5Fclose(h_file);

    return 0;
}
