#include <assert.h>
#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <pthread.h>

#define MAX_DIGITS 100000

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

#define NUM_PREFIXES 5
struct prefix prefixes[NUM_PREFIXES] = {
    { "26", 2,  12  },
    { "2",  1,  2   },
    { "3",  1,  3   },
    { "4",  1,  4   },
    { "6",  1,  6   },
};

int
main()
{
    pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
    unsigned max = 1;

    #pragma omp parallel for schedule(dynamic)
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
                        pthread_mutex_lock(&mtx);
                        if (persistence <= max) {
                            pthread_mutex_unlock(&mtx);
                            continue;
                        }
                        printf("%02u:  %s", persistence, prefix->str);
                        for (unsigned i = 0; i < num7s; i++) printf("7");
                        for (unsigned i = 0; i < num8s; i++) printf("8");
                        for (unsigned i = 0; i < num9s; i++) printf("9");
                        printf("\n");
                        max = persistence;
                        pthread_mutex_unlock(&mtx);
                    }
                }
            }
        }

        mpz_clear(num);
        mpz_clear(pow);
    }

    return 0;
}
