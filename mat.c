#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define TAU_M 5.0 // time constant [ms]
#define R_RESIST 50.0 // membrane resistance [MOhm]
#define L_NUM 2 // the number of threshold time constants
#define OMEGA_REST 19.0 // resting value [mV]
#define REFRACTORY 2.0 // absolute resractory period [ms]
#define A_SCALE 0.1 // scale factor (0.1 -- 1.2)
#define I_EXC 0.1 // exc amplitudes [nA]

#define DT 0.001 // step size [ms]
#define MAX_T 100 // max time [ms]

typedef struct {
    int spikeNumber;
    double spikeTime;
} SpikeInfo;

double calc_CurrentInjection(double time){
    double gtau_exc, gtau_inh;
}

double dvdt(double time, double current, double voltage){
    return (-voltage + R_RESIST * current) / TAU_M;
}

int main(){
    double threshold_time_constant[L_NUM];
    double spike_time;

    double v_membrane, i_input, time, theta;
    int maxtime_count;
    maxtime_count = (int)MAX_T / DT;

    for(int count = 0; count < maxtime_count; count++){

    }

    
    return 0;
}
