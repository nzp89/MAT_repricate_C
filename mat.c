#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <SFMT.h>
#include <time.h>

#define TAU_M 5.0 // time constant [ms]
#define R_RESIST 50.0 // membrane resistance [MOhm]
#define L_NUM 2 // the number of threshold time constants
#define REFRACTORY 2.0 // absolute resractory period [ms]
#define A_SCALE 0.1 // scale factor (0.1 -- 1.2)

#define I_EXC 0.1 // exc amplitude [nA]
#define I_INH 0.033 // inh amplitude [nA]
#define TAU_EXC 1.0 // exc decay-time constant [ms]
#define TAU_INH 3.0 // inh decay-time constant [ms]

#define DT 0.001 // step size [ms]
#define MAX_T 1000.0 // max time [ms]

double Sum1(double time, bool spike, double tau, double old_sum1){
    return exp(-DT / tau) * old_sum1 + spike;
}

double Sum2(double time, bool spike, double tau, double old_sum2){
    return exp(-DT / tau) * old_sum2 + spike * time;
}

double g_calculate(double time, double tau, double sum1, double sum2){
    return ((time + DT) * sum1 - sum2) / tau;
}


double i_current(double g_exc, double g_inh){
    return A_SCALE * I_EXC * g_exc + A_SCALE * I_INH * g_inh;
}

double dvdt(double v_memb, double i_current){
    return ( -v_memb + R_RESIST * i_current) / TAU_M;
}

int main(){
    double sum1_e, sum2_e, sum1_i, sum2_i;
    sum1_e = 0;
    sum2_e = 0;
    sum1_i = 0;
    sum2_i = 0;
    double i_curr, g_exc, g_inh, frec, v_memb, threshold, b_volt, tau_theta, threshold_0;
    v_memb = 0;
    threshold = 0;
    threshold_0 = -1;

    b_volt = 1; // <=> omega
    tau_theta = 5000; // <=> H tau

    frec = 6.88;

    int maxtime_count = MAX_T / DT;       

    FILE *mat_all_file;
    char *m_filename = "mat_all.dat";
    mat_all_file = fopen(m_filename, "w");
        
    uint32_t seed = (uint32_t)time(NULL);
    sfmt_t rng;
    sfmt_init_gen_rand(&rng, seed); 


    for (int time_count = 0; time_count < maxtime_count; time_count++){
        double old_threshold = threshold;
        double time_ms = time_count * DT;
        double rand = sfmt_genrand_real2(&rng);
        bool spike = (rand < frec * DT);
        double old_sum1_e = sum1_e;
        double old_sum2_e = sum2_e;
        double old_sum1_i = sum1_i;
        double old_sum2_i = sum2_i;

        sum1_e = Sum1(time_ms, spike, TAU_EXC, old_sum1_e);
        sum2_e = Sum2(time_ms, spike, TAU_EXC, old_sum2_e);
        sum1_i = Sum1(time_ms, spike, TAU_INH, old_sum1_i);
        sum2_i = Sum2(time_ms, spike, TAU_INH, old_sum2_i);

        g_exc = g_calculate(time_ms, TAU_EXC, sum1_e, sum2_e);
        g_inh = g_calculate(time_ms, TAU_INH, sum1_i, sum2_i);

        i_curr = i_current(g_exc, g_inh);

        v_memb += DT * dvdt(v_memb, i_curr);
        threshold = exp(-DT / tau_theta) * old_threshold + (1.0 - exp(-DT / tau_theta)) * threshold_0;
        spike = (v_memb >= threshold) ? 1: 0;
        threshold = threshold + spike * b_volt;
        if(spike){
            fprintf(mat_all_file, "%lf %lf %lf %lf\n", time_ms, i_curr, v_memb + 10, threshold);
        }

        fprintf(mat_all_file, "%lf %lf %lf %lf\n", time_ms, i_curr, v_memb, threshold);

    }

    fclose(mat_all_file);

    return 0;
}
