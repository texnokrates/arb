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

    Copyright (C) 2015 Fredrik Johansson

******************************************************************************/

#include "flint/fmpz_poly.h"
#include "acb_modular.h"

/* reference values h_(-d)(2015) mod 31337 computed using sage */
static const short hilbert_poly_values[1000] = {
    0, 0, 0, 2015, 287, 0, 0, 5390, 25352, 0, 0, 3446, 10689, 0, 0, 7571,
    27889, 0, 0, 9315, 17503, 0, 0, 20694, 1734, 0, 0, 5911, 29250, 0, 0,
    6194, 3783, 0, 0, 6982, 30449, 0, 0, 10645, 11069, 0, 0, 494, 29262,
    0, 0, 10595, 22835, 0, 0, 18309, 24632, 0, 0, 6433, 5042, 0, 0, 9174,
    24082, 0, 0, 3463, 30027, 0, 0, 11406, 22491, 0, 0, 16831, 8457, 0, 0,
    21721, 19275, 0, 0, 23777, 28295, 0, 0, 13585, 26717, 0, 0, 632,
    20965, 0, 0, 9886, 852, 0, 0, 18674, 12461, 0, 0, 17204, 23887, 0, 0,
    132, 11124, 0, 0, 29609, 2760, 0, 0, 29227, 28978, 0, 0, 29543, 20831,
    0, 0, 12260, 15153, 0, 0, 18215, 29267, 0, 0, 13864, 4648, 0, 0, 9591,
    3948, 0, 0, 29683, 20283, 0, 0, 30212, 23760, 0, 0, 10290, 17465, 0,
    0, 31207, 21341, 0, 0, 19076, 29310, 0, 0, 740, 11697, 0, 0, 20446,
    7258, 0, 0, 16515, 29406, 0, 0, 19948, 20065, 0, 0, 8576, 19014, 0, 0,
    1647, 1354, 0, 0, 19468, 6933, 0, 0, 2065, 17883, 0, 0, 19430, 5539,
    0, 0, 5738, 25233, 0, 0, 21651, 6815, 0, 0, 23927, 28399, 0, 0, 3108,
    23302, 0, 0, 17409, 7911, 0, 0, 26800, 31051, 0, 0, 14360, 11091, 0,
    0, 10984, 11954, 0, 0, 26389, 3557, 0, 0, 3298, 18516, 0, 0, 22211,
    2972, 0, 0, 22613, 5274, 0, 0, 27799, 13362, 0, 0, 8002, 23366, 0, 0,
    31122, 20593, 0, 0, 356, 19130, 0, 0, 10024, 2373, 0, 0, 19390, 28768,
    0, 0, 25504, 4440, 0, 0, 14798, 8726, 0, 0, 6180, 7623, 0, 0, 12941,
    31038, 0, 0, 14370, 29147, 0, 0, 166, 10994, 0, 0, 8397, 354, 0, 0,
    26973, 17435, 0, 0, 21128, 5064, 0, 0, 20453, 30343, 0, 0, 25679,
    6503, 0, 0, 28364, 4924, 0, 0, 6341, 11339, 0, 0, 10624, 17028, 0, 0,
    28001, 192, 0, 0, 27052, 28470, 0, 0, 10034, 12162, 0, 0, 25236, 6842,
    0, 0, 19334, 7593, 0, 0, 29159, 26694, 0, 0, 14719, 26199, 0, 0, 9382,
    25741, 0, 0, 31174, 8440, 0, 0, 4218, 17722, 0, 0, 11574, 8090, 0, 0,
    9938, 4982, 0, 0, 22701, 6931, 0, 0, 29372, 29088, 0, 0, 28, 24186, 0,
    0, 23852, 7521, 0, 0, 18508, 9883, 0, 0, 2392, 30611, 0, 0, 26103,
    11926, 0, 0, 27676, 24118, 0, 0, 7495, 15441, 0, 0, 12681, 20030, 0,
    0, 8106, 3260, 0, 0, 25626, 24143, 0, 0, 17471, 9620, 0, 0, 24315,
    24991, 0, 0, 25966, 12488, 0, 0, 9839, 25896, 0, 0, 14932, 27863, 0,
    0, 20652, 11663, 0, 0, 1779, 5540, 0, 0, 2066, 24758, 0, 0, 23522,
    20408, 0, 0, 26357, 2423, 0, 0, 14671, 18184, 0, 0, 16904, 264, 0, 0,
    27879, 30997, 0, 0, 1506, 23000, 0, 0, 1236, 13243, 0, 0, 24917, 2346,
    0, 0, 2397, 14849, 0, 0, 21298, 15702, 0, 0, 9627, 8103, 0, 0, 26185,
    6738, 0, 0, 2476, 21350, 0, 0, 7578, 9859, 0, 0, 19157, 5329, 0, 0,
    401, 3223, 0, 0, 30337, 21258, 0, 0, 25707, 14593, 0, 0, 8611, 15496,
    0, 0, 12225, 1505, 0, 0, 23534, 16704, 0, 0, 13230, 19926, 0, 0,
    12220, 19104, 0, 0, 6130, 12697, 0, 0, 19881, 6864, 0, 0, 25044, 7436,
    0, 0, 29137, 14435, 0, 0, 20628, 26292, 0, 0, 30933, 8606, 0, 0,
    28752, 23603, 0, 0, 11633, 21739, 0, 0, 22592, 18621, 0, 0, 20125,
    1595, 0, 0, 25306, 15723, 0, 0, 1643, 16444, 0, 0, 11825, 27858, 0, 0,
    13835, 4948, 0, 0, 29003, 12298, 0, 0, 16479, 12920, 0, 0, 25238,
    15558, 0, 0, 4102, 16578, 0, 0, 10456, 546, 0, 0, 3738, 28968, 0, 0,
    20266, 8035, 0, 0, 9779, 25941, 0, 0, 5911, 20127, 0, 0, 18291, 28982,
    0, 0, 18831, 29846, 0, 0, 21272, 11493, 0, 0, 24427, 28574, 0, 0,
    22664, 14849, 0, 0, 30445, 20996, 0, 0, 13612, 18604, 0, 0, 19880,
    19227, 0, 0, 6230, 17398, 0, 0, 29192, 29933, 0, 0, 25046, 14031, 0,
    0, 24100, 463, 0, 0, 19254, 17038, 0, 0, 19912, 10339, 0, 0, 12475,
    24909, 0, 0, 3292, 18482, 0, 0, 1542, 24603, 0, 0, 24850, 2332, 0, 0,
    22262, 14510, 0, 0, 16526, 18396, 0, 0, 30544, 14396, 0, 0, 2788,
    18461, 0, 0, 17619, 14749, 0, 0, 29832, 24740, 0, 0, 19298, 20355, 0,
    0, 538, 18003, 0, 0, 4225, 4401, 0, 0, 28095, 1371, 0, 0, 2724, 353,
    0, 0, 8917, 943, 0, 0, 4998, 2384, 0, 0, 14677, 9528, 0, 0, 7351,
    13960, 0, 0, 4467, 8153, 0, 0, 16711, 8606, 0, 0, 24590, 1061, 0, 0,
    7678, 5647, 0, 0, 27940, 3563, 0, 0, 11137, 29574, 0, 0, 15623, 29261,
    0, 0, 15422, 3266, 0, 0, 26284, 9039, 0, 0, 9616, 21817, 0, 0, 2661,
    8479, 0, 0, 19882, 5965, 0, 0, 13459, 2231, 0, 0, 10839, 22085, 0, 0,
    23581, 14883, 0, 0, 7516, 20147, 0, 0, 7072, 16192, 0, 0, 26870,
    29168, 0, 0, 5724, 19808, 0, 0, 25458, 21905, 0, 0, 28942, 29586, 0,
    0, 13256, 6359, 0, 0, 15267, 8726, 0, 0, 29008, 24734, 0, 0, 2786,
    716, 0, 0, 29798, 3021, 0, 0, 1388, 28017, 0, 0, 26001, 11541, 0, 0,
    19326, 2630, 0, 0, 5326, 1153, 0, 0, 12367, 12089, 0, 0, 15701, 7816,
    0, 0, 28096, 13980, 0, 0, 15683, 16412, 0, 0, 29609, 21907, 0, 0,
    2266, 30126, 0, 0, 26922, 29563, 0, 0, 27024, 11031, 0, 0, 28714,
    4013, 0, 0, 20446, 30395, 0, 0, 1980, 26546, 0, 0, 26413, 13346, 0, 0,
    4083, 837, 0, 0, 7134, 251, 0, 0, 29540, 6521, 0, 0, 18515, 2436, 0,
    0, 21198, 24291, 0, 0, 26451, 28377, 0, 0, 18689, 15300, 0, 0, 8527,
    27153, 0, 0, 18586, 30431, 0, 0, 28981, 13795, 0, 0, 11286, 26207, 0,
    0, 11870, 2833, 0, 0, 6996, 19018, 0, 0, 22638, 18958, 0, 0, 10333,
    12804, 0, 0, 13618, 8212, 0, 0, 30926, 11947, 0, 0, 6681
};

static const int hilbert_poly_values_2[] = {
    -2000, 13490,
    -1999, 10172,
    -4000, 2042,
    -3999, 16478,
    -8000, 14919,
    -7999, 15365,
    -16000, 8190,
    -15999, 30928,
    -32000, 10486,
    -31999, 2808,
    -64000, 26310,
    -63999, 23551,
/* slow
    -128000, 30815,
    -127999, 2144,
    -256000, 5257,
    -255999, 18151,
    -512000, 31288,
    -511999, 30227,
    -1024000, 14241,
    -1023999, 16711,
*/
    0, 0,
};

int main()
{
    flint_printf("hilbert_class_poly....");
    fflush(stdout);

    {
        slong i;
        mp_limb_t c;
        fmpz_poly_t hd;
        nmod_poly_t hdp;

        fmpz_poly_init(hd);
        nmod_poly_init(hdp, 31337);

        for (i = 0; i < 1000; i++)
        {
            acb_modular_hilbert_class_poly(hd, -i);
            fmpz_poly_get_nmod_poly(hdp, hd);
            c = nmod_poly_evaluate_nmod(hdp, 2015);

            if (c != hilbert_poly_values[i])
            {
                flint_printf("FAIL!\n");
                flint_printf("D = %wd\n", -i);
                flint_printf("c = %wu\n", c);
                flint_printf("expected = %d\n", (int) hilbert_poly_values[i]);
                abort();
            }
        }

        for (i = 0; hilbert_poly_values_2[2 * i] != 0; i++)
        {
            acb_modular_hilbert_class_poly(hd, hilbert_poly_values_2[2 * i]);
            fmpz_poly_get_nmod_poly(hdp, hd);
            c = nmod_poly_evaluate_nmod(hdp, 2015);

            if (c != hilbert_poly_values_2[2 * i + 1])
            {
                flint_printf("FAIL!\n");
                flint_printf("D = %wd\n", -i);
                flint_printf("c = %wu\n", c);
                flint_printf("expected = %d\n", (int) hilbert_poly_values_2[2*i+1]);
                abort();
            }
        }

        fmpz_poly_clear(hd);
        nmod_poly_clear(hdp);
    }

    flint_cleanup();
    flint_printf("PASS\n");
    return EXIT_SUCCESS;
}

