#ifndef PARAMETERS_H
#define PARAMETERS_H

struct Neuron_constant{
    double tau_m; // time constant [ms]
    double r_resist; // membrane resistance [MOhm]
    double r_ahp;
    double v_rest; // rest potential [mV]
    double refractory; // absolute refractory period [ms]
};

struct Current_constant{
    double i_exc; // excitatory post synaptic current's amplitude [nA]
    double i_inh; // inhibitory post synaptic current's amplitude [nA]
    double tau_exc; // excitatory decay-time constant [ms]
    double tau_inh; // inhibitory decay-time constant [ms]
    double tau_ahp; // ahp time constant [ms]
    double frec; // statistical rates 
    double a_scale; // 0.1--1.2
};

struct Synapse_constant{
    double tau_syn; // synapse time constant [ms]
    double r_syn; // syynapse resistance [MOhm]
    double weight; // synapse weight
};

struct Neuron_variable{
    double sum1_alpha1; // use Sum1 for alpha1
    double sum1_alpha2; // use Sum1 for alpha2
    double v_membrane; // membrane potential [mV]
    double threshold; // threshold [mV]
    int refractory_flag; // is refractory?
    int refractory_counter; // count refractory time
    bool spike; // is spike?
};

struct Current_variable{
    double i_current;
    double i_synapse;
    double i_ahp;
    double sum1_exc;
    double sum2_exc;
    double sum1_inh;
    double sum2_inh;
    double g_exc;
    double g_inh;
};

const struct Neuron_constant Neuron_cons = {
    .tau_m = 5.0,
    .r_resist = 50.0,
    .r_ahp = 2.0,
    .v_rest = -85.0,
    .refractory = 15.0
};

const struct Current_constant Current_cons = {
    .i_exc = 0.1,
    .i_inh = 0.033,
    .tau_exc = 1.0,
    .tau_inh = 3.0,
    .tau_ahp = 400.0,
    .frec = 6.33,
    .a_scale = 1.2
};

const struct Synapse_constant Synapse_cons = {
    .tau_syn = 5.0,
    .r_syn = 5.0,
    .weight = -40.0
};

#endif
