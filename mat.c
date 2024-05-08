#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <SFMT.h>

#define TAU_M 5.0 // time constant [ms]
#define R_RESIST 50.0 // membrane resistance [MOhm]
#define L_NUM 2 // the number of threshold time constants
#define OMEGA_REST 19.0 // resting value [mV]
#define REFRACTORY 2.0 // absolute resractory period [ms]
#define A_SCALE 0.1 // scale factor (0.1 -- 1.2)
#define I_EXC 0.1 // exc amplitudes [nA]

#define DT 0.001 // step size [ms]
#define MAX_T 100.0 // max time [ms]

typedef struct {
    bool isSpiked;
    int spikeNumber;
    double spikedTime;
} SpikeInfo;

double CurrentInjection(double time, int maxtime_count, SpikeInfo *spikes){
    // WIP
    double sum_exc, sum_inh, gtau_exc, gtau_inh, t;
    for (int k = 0; k < maxtime_count; k++){
        
    }
}

void PoissonSpikes(sfmt_t rng, int count, SpikeInfo *spikes, double frec){
    // generate poisson spikes
    int spikeNumber_count;
    double spikedTime_count;
    double rand = sfmt_genrand_real2(&rng); // generate random number (maybe)
        bool spike = (rand < frec * DT); // is it spiked?
        if(spike){ // when spiked
            spikeNumber_count++;
            spikedTime_count = (double)count;
        }
        spikes[count].isSpiked = spike;
        spikes[count].spikeNumber = spikeNumber_count;
        spikes[count].spikedTime = spikedTime_count;
}

double dvdt(double time, double current, double voltage){
    return (-voltage + R_RESIST * current) / TAU_M;
}

int main(){    
    // time parameter setup
    int maxtime_count = (int)MAX_T / DT;

    SpikeInfo exc_spikes[maxtime_count], inh_spikes[maxtime_count]; 
    double exc_frec, inh_frec;
    exc_frec = 6.88;
    inh_frec = 2.88;

    // random number setup
    uint32_t seed = (uint32_t)time(NULL);
    sfmt_t rng;
    sfmt_init_gen_rand (&rng, seed);

    // file setup (debug menu)
    FILE *spike_file;
    char *s_filename = "spikes.dat";
    spike_file = fopen(s_filename, "w");

    // generate random spike
    for(int count = 0; count < maxtime_count; count++){
        PoissonSpikes(rng, count, exc_spikes, exc_frec);
        PoissonSpikes(rng, count, inh_spikes, inh_frec);
    }

    
    

    fclose(spike_file);

    return 0;
}
