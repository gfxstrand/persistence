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
    mpz_tdiv_qr_ui(in, out, in, 10);

    while (mpz_cmp_ui(in, 10) >= 0) {
        unsigned r = mpz_tdiv_q_ui(in, in, 10);
        mpz_mul_ui(out, out, r);
    }

    mpz_mul(out, out, in);
}

static void
mul_digits2(mpz_t out, mpz_t in)
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

    //if (hist[5] && (hist[2] || hist[4] || hist[6] || hist[8])) {
    //    mpz_set_ui(out, 0);
    //    return;
    //}

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

void (*freefunc) (void *, size_t);

static unsigned
mul_digits3(mpz_t out, mpz_t in)
{
    mpz_set_ui(out, 1);

    char *str = mpz_get_str(NULL, 10, in);
    unsigned i;
    for (i = 0; str[i]; i++)
        mpz_mul_ui(out, out, str[i] - '0');

    freefunc(str, i + 1);
}

static unsigned
mpz_persistence(mpz_t in)
{
    mpz_t tmp;
    mpz_init(tmp);

    unsigned count;
    for (count = 0; mpz_cmp_ui(in, 10) > 0; count++) {
#if 0
        mpz_t in1, in2, out1, out2;
        mpz_init_set(in1, in);
        mpz_init_set(in2, in);
        mpz_init(out1);
        mpz_init(out2);
        mul_digits2(out1, in1);
        mul_digits2(out2, in2);
        if (mpz_cmp(out1, out2) != 0) {
            printf("%s  %s  %s\n",
                   mpz_get_str(NULL, 10, in),
                   mpz_get_str(NULL, 10, out1),
                   mpz_get_str(NULL, 10, out2));
            abort();
        }
        mpz_clear(in1);
        mpz_clear(in2);
        mpz_clear(out1);
        mpz_clear(out2);
#endif

        mul_digits2(tmp, in);
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
    mp_get_memory_functions (NULL, NULL, &freefunc);

#if 0
    mpz_t tmp_in, tmp_out;
    mpz_init(tmp_out);
    mpz_init_set_str(tmp_in, "108", 10);
    mul_digits(tmp_out, tmp_in);
    printf("%s\n", mpz_get_str(NULL, 10, tmp_out));
    return 0;
#endif

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
