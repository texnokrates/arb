/*=============================================================================

    This file is part of ARB.

    ARB is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    ARB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ARB; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

=============================================================================*/
/******************************************************************************

    Copyright (C) 2012 Fredrik Johansson

******************************************************************************/

#include "flint/double_extras.h"
#include "acb_mat.h"
#include "bool_mat.h"

slong _arb_mat_exp_choose_N(const mag_t norm, slong prec);

/* evaluates the truncated Taylor series (assumes no aliasing) */
void
_acb_mat_exp_taylor(acb_mat_t S, const acb_mat_t A, slong N, slong prec)
{
    if (N == 1)
    {
        acb_mat_one(S);
    }
    else if (N == 2)
    {
        acb_mat_one(S);
        acb_mat_add(S, S, A, prec);
    }
    else if (N == 3)
    {
        acb_mat_t T;
        acb_mat_init(T, acb_mat_nrows(A), acb_mat_nrows(A));
        acb_mat_sqr(T, A, prec);
        acb_mat_scalar_mul_2exp_si(T, T, -1);
        acb_mat_add(S, A, T, prec);
        acb_mat_one(T);
        acb_mat_add(S, S, T, prec);
        acb_mat_clear(T);
    }
    else
    {
        slong i, lo, hi, m, w, dim;
        acb_mat_struct * pows;
        acb_mat_t T, U;
        fmpz_t c, f;

        dim = acb_mat_nrows(A);
        m = n_sqrt(N);
        w = (N + m - 1) / m;

        fmpz_init(c);
        fmpz_init(f);
        pows = flint_malloc(sizeof(acb_mat_t) * (m + 1));
        acb_mat_init(T, dim, dim);
        acb_mat_init(U, dim, dim);

        for (i = 0; i <= m; i++)
        {
            acb_mat_init(pows + i, dim, dim);
            if (i == 0)
                acb_mat_one(pows + i);
            else if (i == 1)
                acb_mat_set(pows + i, A);
            else
                acb_mat_mul(pows + i, pows + i - 1, A, prec);
        }

        acb_mat_zero(S);
        fmpz_one(f);

        for (i = w - 1; i >= 0; i--)
        {
            lo = i * m;
            hi = FLINT_MIN(N - 1, lo + m - 1);

            acb_mat_zero(T);
            fmpz_one(c);

            while (hi >= lo)
            {
                acb_mat_scalar_addmul_fmpz(T, pows + hi - lo, c, prec);
                if (hi != 0)
                    fmpz_mul_ui(c, c, hi);
                hi--;
            }

            acb_mat_mul(U, pows + m, S, prec);
            acb_mat_scalar_mul_fmpz(S, T, f, prec);
            acb_mat_add(S, S, U, prec);
            fmpz_mul(f, f, c);
        }

        acb_mat_scalar_div_fmpz(S, S, f, prec);

        fmpz_clear(c);
        fmpz_clear(f);
        for (i = 0; i <= m; i++)
            acb_mat_clear(pows + i);
        flint_free(pows);
        acb_mat_clear(T);
        acb_mat_clear(U);
    }
}

static slong
_acb_mat_count_is_zero(const acb_mat_t A)
{
    slong nz, i, j;
    for (nz = 0, i = 0; i < acb_mat_nrows(A); i++)
        for (j = 0; j < acb_mat_ncols(A); j++)
            nz += acb_is_zero(acb_mat_entry(A, i, j));
    return nz;
}

static void
_acb_mat_exp_diagonal(acb_mat_t B, const acb_mat_t A, slong prec)
{
    slong n, i;
    n = acb_mat_nrows(A);
    if (B != A)
    {
        acb_mat_zero(B);
    }
    for (i = 0; i < n; i++)
    {
        acb_exp(acb_mat_entry(B, i, i), acb_mat_entry(A, i, i), prec);
    }
}

void
acb_mat_exp(acb_mat_t B, const acb_mat_t A, slong prec)
{
    slong i, j, dim, nz;
    bool_mat_t C;

    if (!acb_mat_is_square(A))
    {
        flint_printf("acb_mat_exp: a square matrix is required!\n");
        abort();
    }

    if (acb_mat_is_empty(A))
        return;

    dim = acb_mat_nrows(A);

    if (dim == 1)
    {
        acb_exp(acb_mat_entry(B, 0, 0), acb_mat_entry(A, 0, 0), prec);
        return;
    }

    nz = _acb_mat_count_is_zero(A);

    if (nz == dim * dim)
    {
        acb_mat_one(B);
        return;
    }

    if (nz == 0)
    {
        bool_mat_init(C, dim, dim);
        bool_mat_complement(C, C);
    }
    else
    {
        bool_mat_t S;
        bool_mat_init(S, dim, dim);
        for (i = 0; i < dim; i++)
            for (j = 0; j < dim; j++)
                bool_mat_set_entry(S, i, j, !acb_is_zero(acb_mat_entry(A, i, j)));
        if (bool_mat_is_diagonal(S))
        {
            _acb_mat_exp_diagonal(B, A, prec);
            bool_mat_clear(S);
            return;
        }
        else
        {
            bool_mat_init(C, dim, dim);
            bool_mat_transitive_closure(C, S);
            bool_mat_clear(S);
        }
    }

    /* evaluate using scaling and squaring of truncated taylor series */
    {
        slong wp, N, q, r;
        mag_t norm, err;
        acb_mat_t T;
        int is_real;

        is_real = acb_mat_is_real(A);

        wp = prec + 3 * FLINT_BIT_COUNT(prec);

        mag_init(norm);
        mag_init(err);
        acb_mat_init(T, dim, dim);

        acb_mat_bound_inf_norm(norm, A);

        q = pow(wp, 0.25);  /* wanted magnitude */

        if (mag_cmp_2exp_si(norm, 2 * wp) > 0) /* too big */
            r = 2 * wp;
        else if (mag_cmp_2exp_si(norm, -q) < 0) /* tiny, no need to reduce */
            r = 0;
        else
            r = FLINT_MAX(0, q + MAG_EXP(norm)); /* reduce to magnitude 2^(-r) */

        acb_mat_scalar_mul_2exp_si(T, A, -r);
        mag_mul_2exp_si(norm, norm, -r);

        N = _arb_mat_exp_choose_N(norm, wp);
        mag_exp_tail(err, norm, N);

        _acb_mat_exp_taylor(B, T, N, wp);

        if (is_real)
        {
            for (i = 0; i < dim; i++)
                for (j = 0; j < dim; j++)
                    if (bool_mat_get_entry(C, i, j))
                        arb_add_error_mag(acb_realref(acb_mat_entry(B, i, j)), err);
        }
        else
        {
            for (i = 0; i < dim; i++)
                for (j = 0; j < dim; j++)
                    if (bool_mat_get_entry(C, i, j))
                        acb_add_error_mag(acb_mat_entry(B, i, j), err);
        }

        for (i = 0; i < r; i++)
        {
            acb_mat_sqr(T, B, wp);
            acb_mat_swap(T, B);
        }

        for (i = 0; i < dim; i++)
            for (j = 0; j < dim; j++)
                acb_set_round(acb_mat_entry(B, i, j),
                    acb_mat_entry(B, i, j), prec);

        mag_clear(norm);
        mag_clear(err);
        acb_mat_clear(T);
    }

    bool_mat_clear(C);
}
