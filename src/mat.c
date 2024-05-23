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
#define A_SCALE 1.2 // scale factor (0.1 -- 1.2)

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

void update_sums_and_currents(double time_ms, bool rand_material, double *sum1_e, double *sum2_e, double *sum1_i, double *sum2_i, double *g_exc, double *g_inh, double *i_curr) {
    double old_sum1_e = *sum1_e;
    double old_sum2_e = *sum2_e;
    double old_sum1_i = *sum1_i;
    double old_sum2_i = *sum2_i;

    *sum1_e = Sum1(time_ms, rand_material, TAU_EXC, old_sum1_e);
    *sum2_e = Sum2(time_ms, rand_material, TAU_EXC, old_sum2_e);
    *sum1_i = Sum1(time_ms, rand_material, TAU_INH, old_sum1_i);
    *sum2_i = Sum2(time_ms, rand_material, TAU_INH, old_sum2_i);

    *g_exc = g_calculate(time_ms, TAU_EXC, *sum1_e, *sum2_e);
    *g_inh = g_calculate(time_ms, TAU_INH, *sum1_i, *sum2_i);

    *i_curr = i_current(*g_exc, *g_inh);
}

void update_threshold_and_spike(double *threshold, double tau_theta, double threshold_0, double v_memb, double b_volt, bool *spike) {
    double old_threshold = *threshold;
    *threshold = exp(-DT / tau_theta) * old_threshold + (1.0 - exp(-DT / tau_theta)) * threshold_0;
    *spike = (v_memb >= *threshold) ? 1 : 0;
    *threshold = *threshold + *spike * b_volt;
}

void update_membrane_voltage(double *v_memb, double i_curr) {
    *v_memb += DT * dvdt(*v_memb, i_curr);
}

int main(){
    double sum1_e, sum2_e, sum1_i, sum2_i, i_curr, g_exc, g_inh, frec, v_memb, threshold, b_volt, tau_theta, threshold_0;
    sum1_e = 0;
    sum2_e = 0;
    sum1_i = 0;
    sum2_i = 0;
    v_memb = 0;
    threshold = 0;
    threshold_0 = -1;
    b_volt = 19; // <=> 発火時の上がり幅
    tau_theta = 200; // <=> expの下がり具合
    frec = 6.88;

    int maxtime_count = MAX_T / DT;      
    bool spike = 0; 

    FILE *v_memb_file;
    char *v_filename = "output/v_memb.dat";
    v_memb_file = fopen(v_filename, "w");
    
    FILE *threshold_file;
    char *t_filename = "output/threshold.dat";
    threshold_file = fopen(t_filename, "w");
    
    FILE *current_file;
    char *c_filename = "output/i_current.dat";
    current_file = fopen(c_filename, "w");
    
        
    uint32_t seed = (uint32_t)time(NULL);
    sfmt_t rng;
    sfmt_init_gen_rand(&rng, seed); 


    for (int time_count = 0; time_count < maxtime_count; time_count++){
        double time_ms = time_count * DT;
        double rand = sfmt_genrand_real2(&rng);
        bool rand_material = (rand < frec * DT);

        update_sums_and_currents(time_ms, rand_material, &sum1_e, &sum2_e, &sum1_i, &sum2_i, &g_exc, &g_inh, &i_curr);
        update_membrane_voltage(&v_memb, i_curr);
        update_threshold_and_spike(&threshold, tau_theta, threshold_0, v_memb, b_volt, &spike);

        if(spike){
            fprintf(v_memb_file, "%lf %lf\n", time_ms, v_memb + 50);
        }

        fprintf(v_memb_file, "%lf %lf\n", time_ms, v_memb);
        fprintf(threshold_file, "%lf %lf\n", time_ms, threshold);
        fprintf(current_file, "%lf %lf\n", time_ms, i_curr);

    }

    fclose(v_memb_file);
    fclose(threshold_file);
    fclose(current_file);

    return 0;
}
