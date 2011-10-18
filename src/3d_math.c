/*
 dEngine Source Code
 Copyright (C) 2009 - Fabien Sanglard

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 *  matrix.c
 *  dEngine
 *
 *  Created by fabien sanglard on 09/08/09.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define PIOVER180       (M_PI / 180.0f)

#include "global.h"
#include "3d_math.h"
#include "engine.h"

/**
 Matrices here are colmn major

 i
 0   4   8  12
 1   5   9  13
 j       2   6  10  14
 3   7  11  15

 for i=column
 for j=row,

 m[ij] = m[  j + (i*4)  ]

 */

void matrix_copy(matrix_t from, matrix_t to) {
    int i;

    for (i = 0; i < 16; i++)
        to[i] = from[i];
}

void matrix_multiply_vertex_by_matrix(vec3_t pos, matrix_t mvp, vec3_t dest) {
    static vec4_t tmp;

    //Premultiply
    /*
     tmp[0] = pos[0] * mvp[0] + pos[1] * mvp[1] + pos[2] * mvp[2] + 1 * mvp[3];
     tmp[1] = pos[0] * mvp[4] + pos[1] * mvp[5] + pos[2] * mvp[6] + 1 * mvp[7];
     tmp[2] = pos[0] * mvp[8] + pos[1] * mvp[9] + pos[2] * mvp[10] + 1 * mvp[11];
     tmp[3] = pos[0] * mvp[12] + pos[1] * mvp[13] + pos[2] * mvp[14] + 1 * mvp[15];
     */

    //Postmultiply
    tmp[0] = pos[0] * mvp[0] + pos[1] * mvp[4] + pos[2] * mvp[8] + 1 * mvp[12];
    tmp[1] = pos[0] * mvp[1] + pos[1] * mvp[5] + pos[2] * mvp[9] + 1 * mvp[13];
    tmp[2] = pos[0] * mvp[2] + pos[1] * mvp[6] + pos[2] * mvp[10] + 1 * mvp[14];
    tmp[3] = pos[0] * mvp[3] + pos[1] * mvp[7] + pos[2] * mvp[11] + 1 * mvp[15];

    //W divide in advance
    dest[0] = tmp[0] / tmp[3];
    dest[1] = tmp[1] / tmp[3];
    dest[2] = tmp[2] / tmp[3];
}

void matrix_multiply(const matrix_t m1, const matrix_t m2, matrix_t result) {
    //Multiply matrix as column major indexation

    int i;
    int row = 0;
    int colum = 0;
    for (i = 0; i < 16; i++) {
        colum = (i / 4) * 4;
        row = i % 4;

        result[i] = m1[row] * m2[colum] + m1[row + 4] * m2[colum + 1] + m1[row + 8] * m2[colum + 2] + m1[row + 12] * m2[colum + 3];
    }

}

void matrix_multiply3x3(const matrix3x3_t m1, const matrix3x3_t m2, matrix3x3_t dest) {
    int i;
    int row = 0;
    int colum = 0;
    for (i = 0; i < 9; i++) {
        colum = (i / 3) * 3;
        row = i % 3;

        dest[i] = m1[row] * m2[colum] + m1[row + 3] * m2[colum + 1] + m1[row + 6] * m2[colum + 2];
    }

}

void matrix_transform_vec4t(const matrix_t m1, const vec4_t vect, vec4_t dest) {
    dest[0] = m1[0] * vect[0] + m1[4] * vect[1] + m1[8] * vect[2] + m1[12] * vect[3];
    dest[1] = m1[1] * vect[0] + m1[5] * vect[1] + m1[9] * vect[2] + m1[13] * vect[3];
    dest[2] = m1[2] * vect[0] + m1[6] * vect[1] + m1[10] * vect[2] + m1[14] * vect[3];
    dest[3] = m1[3] * vect[0] + m1[7] * vect[1] + m1[11] * vect[2] + m1[15] * vect[3];
}

void matrix_transform_vec3t(const matrix3x3_t m1, const vec3_t vect, vec3_t dest) {
    dest[0] = m1[0] * vect[0] + m1[3] * vect[1] + m1[6] * vect[2];
    dest[1] = m1[1] * vect[0] + m1[4] * vect[1] + m1[7] * vect[2];
    dest[2] = m1[2] * vect[0] + m1[5] * vect[1] + m1[8] * vect[2];
}

void matrix_print(matrix_t m) {
    int i, j;

    printf("-----------------\n");
    for (i = 0; i < 4; i++) //Column dest
            {
        for (j = 0; j < 4; j++) //Row dest
                {
            // m1 j row * m2 i column
            printf(" %.4f ", m[i + j * 4]);
        }
        printf("\n");
    }
    printf("-----------------\n");
}

void matrix_print3x3(matrix3x3_t m) {
    int i, j;

    printf("-----------------\n");
    for (i = 0; i < 3; i++) //Column dest
            {
        for (j = 0; j < 3; j++) //Row dest
                {
            // m1 j row * m2 i column
            printf(" %.4f ", m[i + j * 3]);
        }
        printf("\n");
    }
    printf("-----------------\n");
}

void matrix_load_identity(matrix_t m) {
    int i;
    for (i = 0; i < 16; i++)
        m[i] = 0;

    m[0] = 1;
    m[5] = 1;
    m[10] = 1;
    m[15] = 1;
}

void vector_cross_product(const vec3_t v1, const vec3_t v2, vec3_t cross) {
    cross[0] = v1[1] * v2[2] - v1[2] * v2[1]; // X
    cross[1] = v1[2] * v2[0] - v1[0] * v2[2]; // Y
    cross[2] = v1[0] * v2[1] - v1[1] * v2[0]; // Z
}

void normalize(vec3_t v) {
    float length, ilength;

    length = (float) sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);

//	printf("Length = %.3f\n",length);

    if (length) {
        ilength = 1 / length;
        v[0] *= ilength;
        v[1] *= ilength;
        v[2] *= ilength;
    }
}

void vector_interpolate(const vec3_t v1, const vec3_t v2, float f, vec3_t dest) {
    dest[0] = (1 - f) * v1[0] + f * v2[0];
    dest[1] = (1 - f) * v1[1] + f * v2[1];
    dest[2] = (1 - f) * v1[2] + f * v2[2];
}

void Quat_fromEuler(float pitch, float yaw, float roll, quat4_t dest) {
    float p = pitch * PIOVER180 / 2.0;
    float y = yaw * PIOVER180 / 2.0;
    float r = roll * PIOVER180 / 2.0;

    float sinp = sin(p);
    float siny = sin(y);
    float sinr = sin(r);
    float cosp = cos(p);
    float cosy = cos(y);
    float cosr = cos(r);

    dest[X] = sinr * cosp * cosy - cosr * sinp * siny;
    dest[Y] = cosr * sinp * cosy + sinr * cosp * siny;
    dest[Z] = cosr * cosp * siny - sinr * sinp * cosy;
    dest[W] = cosr * cosp * cosy + sinr * sinp * siny;

    Quat_normalize(dest);
}

void Quat_to_matrix(quat4_t q, matrix_t matrix) {
    float x2 = q[X] * q[X];
    float y2 = q[Y] * q[Y];
    float z2 = q[Z] * q[Z];
    float xy = q[X] * q[Y];
    float xz = q[X] * q[Z];
    float yz = q[Y] * q[Z];
    float wx = q[W] * q[X];
    float wy = q[W] * q[Y];
    float wz = q[W] * q[Z];

    matrix[0] = 1.0f - 2.0f * (y2 + z2);
    matrix[1] = 2.0f * (xy - wz);
    matrix[2] = 2.0f * (xz + wy);
    matrix[3] = 0.0f;
    matrix[4] = 2.0f * (xy + wz);
    matrix[5] = 1.0f - 2.0f * (x2 + z2);
    matrix[6] = 2.0f * (yz - wx);
    matrix[7] = 0.0f;
    matrix[8] = 2.0f * (xz - wy);
    matrix[9] = 2.0f * (yz + wx);
    matrix[10] = 1.0f - 2.0f * (x2 + y2);
    matrix[11] = 0.0f;
    matrix[12] = 0.0f;
    matrix[13] = 0.0f;
    matrix[14] = 0.0f;
    matrix[15] = 1.0f;
}

void Quat_computeW(quat4_t q) {
    float t = 1.0f - (q[X] * q[X]) - (q[Y] * q[Y]) - (q[Z] * q[Z]);

    if (t < 0.0f)
        q[W] = 0.0f;
    else
        q[W] = -sqrt(t);
}

void Quat_normalize(quat4_t q) {
    /* compute magnitude of the quaternion */
    float mag = sqrt((q[X] * q[X]) + (q[Y] * q[Y]) + (q[Z] * q[Z]) + (q[W] * q[W]));

    /* check for bogus length, to protect against divide by zero */
    if (mag > 0.0f) {
        /* normalize it */
        float oneOverMag = 1.0f / mag;

        q[X] *= oneOverMag;
        q[Y] *= oneOverMag;
        q[Z] *= oneOverMag;
        q[W] *= oneOverMag;
    }
}

void Quat_mult_quat(const quat4_t qa, const quat4_t qb, quat4_t out) {
    out[W] = (qa[W] * qb[W]) - (qa[X] * qb[X]) - (qa[Y] * qb[Y]) - (qa[Z] * qb[Z]);
    out[X] = (qa[X] * qb[W]) + (qa[W] * qb[X]) + (qa[Y] * qb[Z]) - (qa[Z] * qb[Y]);
    out[Y] = (qa[Y] * qb[W]) + (qa[W] * qb[Y]) + (qa[Z] * qb[X]) - (qa[X] * qb[Z]);
    out[Z] = (qa[Z] * qb[W]) + (qa[W] * qb[Z]) + (qa[X] * qb[Y]) - (qa[Y] * qb[X]);
}

void Quat_mult_vec(const quat4_t q, const vec3_t v, quat4_t out) {
    out[W] = -(q[X] * v[X]) - (q[Y] * v[Y]) - (q[Z] * v[Z]);
    out[X] = (q[W] * v[X]) + (q[Y] * v[Z]) - (q[Z] * v[Y]);
    out[Y] = (q[W] * v[Y]) + (q[Z] * v[X]) - (q[X] * v[Z]);
    out[Z] = (q[W] * v[Z]) + (q[X] * v[Y]) - (q[Y] * v[X]);
}

void Quat_rotate_short_point(const quat4_t q, const vec3s_t in, vec3_t out) {
    vec3_t inFloat;

    inFloat[0] = in[0];
    inFloat[1] = in[1];
    inFloat[2] = in[2];

    Quat_rotate_point(q, inFloat, out);

}

void Quat_rotate_point(const quat4_t q, const vec3_t in, vec3_t out) {
    quat4_t tmp, inv, final;

    inv[X] = -q[X];
    inv[Y] = -q[Y];
    inv[Z] = -q[Z];
    inv[W] = q[W];

    //Quat_normalize (inv);

    Quat_mult_vec(q, in, tmp);
    Quat_mult_quat(tmp, inv, final);

    out[X] = final[X];
    out[Y] = final[Y];
    out[Z] = final[Z];
}

void multiply_by_invert_quaternion(const vec3_t v1, const quat4_t quat, vec3_t dest) {
    static quat4_t inv;

    inv[0] = -quat[0];
    inv[1] = -quat[1];
    inv[2] = -quat[2];
    inv[3] = quat[3];

    Quat_rotate_point(inv, v1, dest);
}

void Quat_create_from_mat3x3(const matrix3x3_t matrix, quat4_t out) {
    float trace;
    float s;

    trace = 1.0 + matrix[0] + matrix[4] + matrix[8];

    if (trace > 0) {
        s = 2 * sqrt(trace);
        out[0] = (matrix[7] - matrix[5]) / s;
        out[1] = (matrix[2] - matrix[6]) / s;
        out[2] = (matrix[3] - matrix[1]) / s;
        out[3] = s / 4;
    } else if (matrix[0] > matrix[4] && matrix[0] > matrix[8]) { // Column 0:
        s = sqrt(1.0 + matrix[0] - matrix[4] - matrix[8]) * 2;
        out[0] = s / 4;
        out[1] = (matrix[1] + matrix[3]) / s;
        out[2] = (matrix[2] + matrix[6]) / s;
        out[3] = (matrix[7] - matrix[5]) / s;
    } else if (matrix[4] > matrix[8]) { // Column 1:
        s = sqrt(1.0 + matrix[4] - matrix[0] - matrix[8]) * 2;
        out[0] = (matrix[1] + matrix[3]) / s;
        out[1] = s / 4;
        out[2] = (matrix[5] + matrix[7]) / s;
        out[3] = (matrix[2] - matrix[6]) / s;
    } else { // Column 2:
        s = sqrt(1.0 + matrix[8] - matrix[0] - matrix[4]) * 2;
        out[0] = (matrix[2] + matrix[6]) / s;
        out[1] = (matrix[5] + matrix[7]) / s;
        out[2] = s / 4;
        out[3] = (matrix[3] - matrix[1]) / s;
    }

    Quat_normalize(out);

}

void Quat_convert_to_mat3x3(matrix3x3_t matrix, const quat4_t out) {
    matrix[0] = 1 - 2 * (out[Y] * out[Y] + out[Z] * out[Z]);
    matrix[1] = 2 * (out[X] * out[Y] - out[W] * out[Z]);
    matrix[2] = 2 * (out[X] * out[Z] + out[W] * out[Y]);
    matrix[3] = 2 * (out[X] * out[Y] + out[W] * out[Z]);
    matrix[4] = 1 - 2 * (out[X] * out[X] + out[Z] * out[Z]);
    matrix[5] = 2 * (out[Y] * out[Z] - out[W] * out[X]);
    matrix[6] = 2 * (out[X] * out[Z] - out[W] * out[Y]);
    matrix[7] = 2 * (out[Y] * out[Z] + out[W] * out[X]);
    matrix[8] = 1 - 2 * (out[X] * out[X] + out[Y] * out[Y]);
}
