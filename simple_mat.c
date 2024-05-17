// A simple model of long-ter, spike train regularization
#define _USE_MATH_DEFINES_
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define T_MAX 1000 // ms
#define DT 1 // ms
#define C_RESIST 1.0 // MOhm

// ゼロ平均、指定された分散のガウシアンノイズを生成する関数
double generateGaussianNoise(double sigma) {
    static int hasSpare = 0;
    static double spare;
    double u, v, s;

    if (hasSpare) {
        hasSpare = 0;
        return spare * sigma;
    }

    hasSpare = 1;
    do {
        u = (rand() / ((double) RAND_MAX)) * 2.0 - 1.0;
        v = (rand() / ((double) RAND_MAX)) * 2.0 - 1.0;
        s = u * u + v * v;
    } while (s >= 1.0 || s == 0.0);

    s = sqrt(-2.0 * log(s) / s);
    spare = v * s;
    return u * s * sigma;
}

int main(){
    double v_memb, threshold, i_current, w_noise, a_ms, b_volt;
    double sigma = 0.2;
    int max_count = (int) T_MAX / DT;
    int spike = 0;

    threshold = 0;
    i_current = 0;
    a_ms = 5.0;
    b_volt = 1.0;

    FILE *simple_mat_all_file;
    char *s_filename = "simple_mat_all.dat";
    simple_mat_all_file = fopen(s_filename, "w");


    for (int i = 0; i < max_count; i++){
        double old_threshold = threshold;
        double time_ms = i * DT;
        w_noise = generateGaussianNoise(sigma);
        i_current = sin(2 * M_PI * i / 100);

        v_memb = C_RESIST * i_current + w_noise;
        threshold = old_threshold - (b_volt / a_ms);
        spike = (v_memb >= threshold) ? 1: 0;
        threshold = threshold + spike * b_volt;

        fprintf(simple_mat_all_file, "%lf %lf %lf %lf %lf\n", time_ms, v_memb, threshold, (double)spike, i_current);
    }

    fclose(simple_mat_all_file);
    return 0;
}
