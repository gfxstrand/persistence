/*
 * Copyright © 2019 Jason Ekstrand
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <assert.h>
#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <pthread.h>

#ifndef MAX_DIGITS
#define MAX_DIGITS 100
#endif

/* Destroys in */
static void
mul_digits(mpz_t out, mpz_t in)
{
    unsigned hist[10] = { 0, };

    while (mpz_cmp_ui(in, 0) > 0) {
        unsigned r = mpz_tdiv_q_ui(in, in, 10);
        if (r == 0) {
            mpz_set_ui(out, 0);
            return;
        }
        hist[r]++;
    }
    unsigned r = mpz_get_ui(in);
    hist[r]++;

    hist[2] += (hist[4] * 2) + hist[6] + (hist[8] * 3);
    hist[3] += hist[6] + (hist[9] * 2);

    mpz_ui_pow_ui(out, 2, hist[2]);

    mpz_t pow;
    mpz_init(pow);

    if (hist[3]) {
        mpz_ui_pow_ui(pow, 3, hist[3]);
        mpz_mul(out, out, pow);
    }
    if (hist[5]) {
        mpz_ui_pow_ui(pow, 5, hist[5]);
        mpz_mul(out, out, pow);
    }
    if (hist[7]) {
        mpz_ui_pow_ui(pow, 7, hist[7]);
        mpz_mul(out, out, pow);
    }

    mpz_clear(pow);
}

static unsigned
mpz_persistence(mpz_t in)
{
    mpz_t tmp;
    mpz_init(tmp);

    unsigned count;
    for (count = 0; mpz_cmp_ui(in, 10) > 0; count++) {
        mul_digits(tmp, in);
        mpz_swap(tmp, in);
    }

    mpz_clear(tmp);

    return count;
}

struct prefix {
    const char *str;
    unsigned digits;
    unsigned prod;
};

/** Unique prefixes which do not contain 7, 8, or 9
 *
 * Given any number, we shrink it as far as possible by combining digits so
 * as to get 7s, 8s, and 8s on the right-hand side and one of the six unique
 * prefixes below on the left-hand side.  For instance, given the number
 * 7236, we can split the digits it into primes and re-combine and get 479
 * which is the smallest number whose digets multiply to the same value as
 * 7236.  Using this scheme, and reforming all numbers as <prefix>777888999
 * where the number of 7s, 8s, and 8s varies, we can get all unique products
 * of digits with the smallest possible number.  This also gives us a very
 * nice way to generate them.  I cannot take credit for this; it was Matt
 * Parker's idea: https://www.youtube.com/watch?v=Wim9WJeDTHQ
 *
 * When we do this reduction, we are left with six unique prefixes that can
 * end up at the front of the 7s, 8s, and 9s which I have in the list below
 * sorted smallest to largest.  Even though 26 looks like the largest, the
 * next 2-digit number will be a 2 followed by something that's at least a
 * 7 so it really does make sense.
 *
 * I've left 5 in the list but commented it out because once you have a 5,
 * you will always have a 5 and the moment you have both a 5 and an even
 * number, you hit a multiple of 10 which then goes to 0.  Therefore, while
 * it's theoretically possible that a persistent number exists which contains
 * a 5, I've considered it so incredibly unlikely as to not be worth even
 * bothering to search.  Searching all numbers up to 100 digits, cutting out
 * the 5 cases saves about 10% of the runtime.
 */
#define NUM_PREFIXES 5
struct prefix prefixes[NUM_PREFIXES] = {
    { "26", 2,  12  },
    { "2",  1,  2   },
    { "3",  1,  3   },
    { "4",  1,  4   },
/*  { "5",  1,  4   }, */
    { "6",  1,  6   },
};

int
main()
{
    unsigned max = 1;

#ifdef USE_OPENMP
    pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
    #pragma omp parallel for schedule(dynamic)
#endif
    for (unsigned digits = 2; digits <= MAX_DIGITS; digits++) {
        mpz_t num, pow;
        mpz_init(num);
        mpz_init(pow);

        for (unsigned p = 0; p < NUM_PREFIXES; p++) {
            struct prefix *prefix = &prefixes[p];
            if (digits < prefix->digits)
                continue;

            unsigned num789s = digits - prefix->digits;
            for (unsigned num89s = 0; num89s <= num789s; num89s++) {
                unsigned num7s = num789s - num89s;
                for (unsigned num9s = 0; num9s <= num89s; num9s++) {
                    unsigned num8s = num89s - num9s;

                    /* Compute the first step */
                    mpz_ui_pow_ui(num, 7, num7s);
                    mpz_ui_pow_ui(pow, 8, num8s);
                    mpz_mul(num, num, pow);
                    mpz_ui_pow_ui(pow, 9, num9s);
                    mpz_mul(num, num, pow);
                    mpz_mul_ui(num, num, prefix->prod);

                    unsigned persistence = 1 + mpz_persistence(num);
                    if (persistence > max) {
#ifdef USE_OPENMP
                        pthread_mutex_lock(&mtx);
                        if (persistence <= max) {
                            pthread_mutex_unlock(&mtx);
                            continue;
                        }
#endif
                        printf("%02u:  %s", persistence, prefix->str);
                        for (unsigned i = 0; i < num7s; i++) printf("7");
                        for (unsigned i = 0; i < num8s; i++) printf("8");
                        for (unsigned i = 0; i < num9s; i++) printf("9");
                        printf("\n");
                        max = persistence;
#ifdef USE_OPENMP
                        pthread_mutex_unlock(&mtx);
#endif
                    }
                }
            }
        }

        mpz_clear(num);
        mpz_clear(pow);
    }

    return 0;
}
