// how to work
// 1. make mat
// 2. ./mat (alpha1) (alpha2) (omega) (i_const)
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <SFMT.h>
#include <time.h>
#include "mat_parameter.h"

#define DT 0.01 // step size [ms]
#define MAX_T 1000.0 // max time [ms]
#define NUM 2 // the number of neurons

double Sum1(double time, bool spike, double tau, double old_sum1){
    return exp(-DT / tau) * old_sum1 + spike;
}

double Sum2(double time, bool spike, double tau, double old_sum2){
    return exp(-DT / tau) * old_sum2 + spike * time;
}

double g_calculate(double time, double tau, double sum1, double sum2){
    return ((time + DT) * sum1 - sum2) / tau;
}

double i_current(double g_exc, double g_inh, const struct Current_constant *current){
    return (current->a_scale * current->i_exc * g_exc) + (current->a_scale * current->i_inh * g_inh);
}

double dvdt(double v_memb, struct Current_variable *current, const struct Neuron_constant *neuron, const struct Synapse_constant *synapse){
    return ( -(v_memb - neuron->v_rest) + neuron->r_resist * current->i_current - neuron->r_ahp * current->i_ahp + synapse->r_syn * current->i_synapse ) / neuron->tau_m;
}

void update_sums_and_currents(double time_ms, bool rand_material, struct Current_variable *current, const struct Current_constant *current_cons) {
    double old_sum1_e = current -> sum1_exc;
    double old_sum2_e = current -> sum2_exc;
    double old_sum1_i = current -> sum1_inh;
    double old_sum2_i = current -> sum2_inh;

    current -> sum1_exc = Sum1(time_ms, rand_material, current_cons->tau_exc, old_sum1_e);
    current -> sum2_exc = Sum2(time_ms, rand_material, current_cons->tau_exc, old_sum2_e);
    current -> sum1_inh = Sum1(time_ms, rand_material, current_cons->tau_inh, old_sum1_i);
    current -> sum2_inh = Sum2(time_ms, rand_material, current_cons->tau_inh, old_sum2_i);

    current -> g_exc = g_calculate(time_ms, current_cons->tau_exc, current -> sum1_exc, current -> sum2_exc);
    current -> g_inh = g_calculate(time_ms, current_cons->tau_inh, current -> sum1_inh, current -> sum2_inh);

    current -> i_current = i_current(current -> g_exc, current -> g_inh, current_cons);
}

void update_threshold_and_spike(struct Neuron_variable *neuron, const struct Neuron_constant *neuron_cons,  double time_ms, double alpha1, double alpha2, double omega) {
    double tau1 = 10;
    double tau2 = 200;
    double old_sum1_1 = neuron -> sum1_alpha1;
    double old_sum1_2 = neuron -> sum1_alpha2;

    neuron -> sum1_alpha1 = Sum1(time_ms, neuron -> spike, tau1, old_sum1_1);
    neuron -> sum1_alpha2 = Sum1(time_ms, neuron -> spike, tau2, old_sum1_2);

    neuron -> threshold = alpha1 * neuron -> sum1_alpha1 + alpha2 * neuron -> sum1_alpha2 + omega + neuron_cons -> v_rest;
    neuron -> spike = (!neuron -> refractory_flag && neuron -> v_membrane >= neuron -> threshold) ? 1 : 0;
}

void constant_currents(struct Current_variable *current){
    current -> i_current = 0.6;
}

void update_synapse_currents(struct Current_variable *current, const struct Synapse_constant *synapse, int spike){
    current -> i_synapse = exp( -DT / synapse -> tau_syn) * current -> i_synapse + synapse -> weight * spike;
}

void update_ahp_currents(struct Current_variable *current,const struct Current_constant *current_cons, int spike){
    current -> i_ahp = exp( -DT / current_cons -> tau_ahp ) * current -> i_ahp + spike;
}

void update_membrane_voltage(struct Neuron_variable *neuron_val, struct Current_variable *current,  const struct Neuron_constant *neuron_cons, const struct Synapse_constant *synapse) {
    neuron_val -> v_membrane += DT * dvdt(neuron_val -> v_membrane, current, neuron_cons, synapse);
}

void initialize_neuron(struct Neuron_variable *neuron, double v_membrane, double threshold) {
    neuron->sum1_alpha1 = 0.0;
    neuron->sum1_alpha2 = 0.0;
    neuron->v_membrane = v_membrane;
    neuron->threshold = threshold;
    neuron->refractory_flag = 0;
    neuron->refractory_counter = 0;
    neuron->spike = false;
}

void initialize_current(struct Current_variable *current){
    current->sum1_exc = 0;
    current->sum2_exc = 0;
    current->sum1_inh = 0;
    current->sum2_inh = 0;
    current->i_synapse = 0.0;
}

void update_refractory(struct Neuron_variable *neuron, double refractory, int count_per_ms) {
    if (neuron->refractory_flag) {
        neuron->refractory_counter++;
        if (neuron->refractory_counter >= refractory * count_per_ms) {
            neuron->refractory_flag = 0;
            neuron->refractory_counter = 0;
        }
    }
}

void print_and_spike(struct Neuron_variable *neuron, struct Current_variable *current, FILE *files[], double time_ms){
    if (neuron -> spike) {
            neuron -> refractory_flag = 1;
            neuron -> refractory_counter = 0;
            fprintf(files[0], "%lf %lf\n", time_ms, neuron -> v_membrane + 50);
            fprintf(files[4], "%lf %d\n", time_ms, neuron -> spike);
        }
    fprintf(files[0], "%lf %lf\n", time_ms, neuron -> v_membrane);
    fprintf(files[1], "%lf %lf\n", time_ms, neuron -> threshold);
    fprintf(files[2], "%lf %lf %lf %lf\n", time_ms, current -> i_current, current -> i_ahp, current -> i_synapse);
    fprintf(files[3], "%lf %lf %lf\n", time_ms, neuron -> sum1_alpha1, neuron -> sum1_alpha2);
}

int main(int argc, char *argv[]){
    double v_memb_0[NUM] = {-70.0, -75.0};
    double threshold_0[NUM] = {-50.0, -50.0};
    struct Neuron_variable Neuron[NUM];
    struct Current_variable Current[NUM];
    
    if (argc != 5) {
        printf("Error: no input or wrong input. -> ./mat (alpha1) (alpha2) (omega) (i_const(0/1))\n");
        return 1;
    }
    double alpha1 = atof(argv[1]);
    double alpha2 = atof(argv[2]);
    double omega = atof(argv[3]);
    bool i_const = atoi(argv[4]);

    for (int i = 0; i < NUM; i++){
        initialize_neuron(&Neuron[i], v_memb_0[i], threshold_0[i]);
        initialize_current(&Current[i]);
    }
    
    int maxtime_count = MAX_T / DT;
    int count_per_ms = (int)(1.0 / DT);  // use refractory checking

    const char *filenames_neuron1[] = {
        "output/v_memb_1.dat",
        "output/threshold_1.dat",
        "output/i_current_1.dat",
        "output/sum_1.dat",
        "output/spike_1.dat"
    };

    const char *filenames_neuron2[] = {
        "output/v_memb_2.dat",
        "output/threshold_2.dat",
        "output/i_current_2.dat",
        "output/sum_2.dat",
        "output/spike_2.dat"
    };

    
    FILE *files_neuron1[5];
    FILE *files_neuron2[5];

    for (int i = 0; i < 5; i++) {
        files_neuron1[i] = fopen(filenames_neuron1[i], "w");
        files_neuron2[i] = fopen(filenames_neuron2[i], "w");
        if (files_neuron1[i] == NULL) {
            fprintf(stderr, "Error opening file: %s\n", filenames_neuron1[i]);
            return 1;
        }
        else if(files_neuron2[i] == NULL){
            fprintf(stderr, "Error opening file: %s\n", filenames_neuron2[i]);
            return 1;
        }
    }

    FILE **files[NUM] = {files_neuron1, files_neuron2};

    uint32_t seed = (uint32_t)time(NULL);
    sfmt_t rng;
    sfmt_init_gen_rand(&rng, seed); 

    for (int time_count = 0; time_count < maxtime_count; time_count++){
        double time_ms = time_count * DT;
        
        // use when current is not const.
        double rand = sfmt_genrand_real2(&rng); 
        bool rand_material = (rand < Current_cons.frec * DT);

        // calculate ext currents
        if(i_const){
            for (int i = 0; i < NUM; i++){
                constant_currents(&Current[i]);
            }
        }else{
            for (int i = 0; i < NUM; i++){
                update_sums_and_currents(time_ms, rand_material, &Current[i], &Current_cons);
            }
        }
        // calculate synapse currents
        for (int i = 0; i < NUM; i++){
            update_synapse_currents(&Current[i], &Synapse_cons, Neuron[(i + 1) % 2].spike);
            update_ahp_currents(&Current[i], &Current_cons, Neuron[i].spike);
        }
        // check refractory
        for (int i = 0; i < NUM; i++){
            update_refractory(&Neuron[i], Neuron_cons.refractory, count_per_ms);
        }
        // calculate membrane voltage
        // update threshold and spike
        for (int i = 0; i < NUM; i++){
            update_membrane_voltage(&Neuron[i], &Current[i], &Neuron_cons, &Synapse_cons);
            update_threshold_and_spike(&Neuron[i], &Neuron_cons, time_ms, alpha1, alpha2, omega);
            print_and_spike(&Neuron[i], &Current[i], files[i], time_ms);
        }
    }

    for (int i = 0; i < 5; i++) {
        fclose(files_neuron1[i]);
        fclose(files_neuron2[i]);
    }
    return 0;
}
