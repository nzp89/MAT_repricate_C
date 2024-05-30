#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <SFMT.h>
#include <time.h>

#define TAU_M 5.0 // time constant [ms]
#define R_RESIST 50.0 // membrane resistance [MOhm]
#define L_NUM 2 // the number of threshold time constants
#define REFRACTORY 2.0 // absolute refractory period [ms]
#define A_SCALE 1.2 // scale factor (0.1 -- 1.2)

#define I_EXC 0.1 // exc amplitude [nA]
#define I_INH 0.033 // inh amplitude [nA]
#define TAU_EXC 1.0 // exc decay-time constant [ms]
#define TAU_INH 3.0 // inh decay-time constant [ms]

#define DT 0.01 // step size [ms]
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

void update_threshold_and_spike(double *time_ms, double *threshold, double v_memb, double *sum1_1, double *sum1_2, bool *spike, bool refractory_flag) {
    double alpha1 = -1.0;
    double alpha2 = 0.4;
    double omega = 26.0;
    double tau1 = 10;
    double tau2 = 200;
    double old_sum1_1 = *sum1_1;
    double old_sum1_2 = *sum1_2;

    *sum1_1 = Sum1(*time_ms, *spike, tau1, old_sum1_1);
    *sum1_2 = Sum1(*time_ms, *spike, tau2, old_sum1_2);

    *threshold = alpha1 * *sum1_1 + alpha2 * *sum1_2 + omega;
    *spike = (!refractory_flag && v_memb >= *threshold) ? 1 : 0;
}

void constant_currents(double *i_curr){
    *i_curr = 0.6;
}

void update_membrane_voltage(double *v_memb, double i_curr, bool refractory_flag, double threshold) {
    *v_memb += DT * dvdt(*v_memb, i_curr);
}

int main(){
    double sum1_e, sum2_e, sum1_i, sum2_i, i_curr, g_exc, g_inh, frec, v_memb, threshold, sum1_1, sum1_2;
    sum1_e = 0;
    sum2_e = 0;
    sum1_i = 0;
    sum2_i = 0;
    sum1_1 = 0;
    sum1_2 = 0;
    v_memb = 0;
    threshold = 0;
    frec = 6.33;

    int maxtime_count = MAX_T / DT;
    int count_per_ms = (int)(1.0 / DT);  
    int refractory_flag = 0; 
    int refractory_counter = 0; 
    bool spike = 0;
    bool i_const = 1;

    FILE *v_memb_file;
    char *v_filename = "output/v_memb.dat";
    v_memb_file = fopen(v_filename, "w");
    
    FILE *threshold_file;
    char *t_filename = "output/threshold.dat";
    threshold_file = fopen(t_filename, "w");
    
    FILE *current_file;
    char *c_filename = "output/i_current.dat";
    current_file = fopen(c_filename, "w");

    FILE *sum_file;
    char *sum_filename = "output/sum.dat";
    sum_file = fopen(sum_filename, "w");

    FILE *spike_file;
    char *spike_filename = "output/spike.dat";
    spike_file = fopen(spike_filename, "w");
    
    uint32_t seed = (uint32_t)time(NULL);
    sfmt_t rng;
    sfmt_init_gen_rand(&rng, seed); 

    for (int time_count = 0; time_count < maxtime_count; time_count++){
        double time_ms = time_count * DT;
        
        // use when current is not const.
        double rand = sfmt_genrand_real2(&rng); 
        bool rand_material = (rand < frec * DT);

        if(i_const){
            constant_currents(&i_curr);
        }else{
            update_sums_and_currents(time_ms, rand_material, &sum1_e, &sum2_e, &sum1_i, &sum2_i, &g_exc, &g_inh, &i_curr);
        }

        if (refractory_flag) {
            refractory_counter++;
            if (refractory_counter >= REFRACTORY * count_per_ms) {
                refractory_flag = 0;
                refractory_counter = 0;
            }
        }

        update_membrane_voltage(&v_memb, i_curr, refractory_flag, threshold);
        update_threshold_and_spike(&time_ms, &threshold, v_memb, &sum1_1, &sum1_2, &spike, refractory_flag);

        if (spike) {
            refractory_flag = 1;
            refractory_counter = 0;
            fprintf(v_memb_file, "%lf %lf\n", time_ms, v_memb + 50);
            fprintf(spike_file, "%lf %d\n", time_ms, spike);
        }

        fprintf(v_memb_file, "%lf %lf\n", time_ms, v_memb);
        fprintf(threshold_file, "%lf %lf\n", time_ms, threshold);
        fprintf(current_file, "%lf %lf\n", time_ms, i_curr);
        fprintf(sum_file, "%lf %lf %lf\n", time_ms, sum1_1, sum1_2);
    }

    fclose(v_memb_file);
    fclose(threshold_file);
    fclose(current_file);
    fclose(sum_file);
    fclose(spike_file);

    return 0;
}
