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

#define I_EXC 0.1 // exc amplitude [nA]
#define I_INH 0.033 // inh amplitude [nA]
#define TAU_EXC 1.0 // exc decay-time constant [ms]
#define TAU_INH 3.0 // inh decay-time constant [ms]

#define DT 0.001 // step size [ms]
#define MAX_T 100.0 // max time [ms]

// double g_Conductance(double tau, double time){
//     return (time >= 0) ? (time * exp(- time / tau) / tau) : 0.0;
// }

// double CurrentInjection(double time, int maxtime_count, SpikeInfo *exc_spikes, SpikeInfo *inh_spikes){
//     // eq(7)
//     double sum_exc, sum_inh, gtau_exc, gtau_inh, t;
//     sum_exc = 0;
//     sum_inh = 0;
//     for (int k = 0; k < maxtime_count; k++){
//         sum_exc += I_EXC * g_Conductance(TAU_EXC, time - exc_spikes[k].spikedTime);
//     }
//     for (int j = 0; j < maxtime_count; j++){
//         sum_inh += I_INH * g_Conductance(TAU_INH, time - inh_spikes[j].spikedTime);
//     }
//     return A_SCALE * (sum_exc + sum_inh);
// }

// double dvdt(double time, double current, double voltage){
//     return (-voltage + R_RESIST * current) / TAU_M;
// }

// double* readFile(char *file_name){
//     FILE *file = fopen(file_name, "r");
//     if (file == NULL) {
//         perror("file cannot open\n");
//         return 1;
//     }
//     // 行数をカウント
//     int line_count = 0;
//     char buffer[100]; // 行ごとの入力を一時的に格納するバッファ
//     while (fgets(buffer, sizeof(buffer), file) != NULL) {
//         line_count++;
//     }
//     // ファイルの先頭に戻る
//     rewind(file);

//     // 配列を動的に確保
//     double *array = (double *)malloc(line_count * sizeof(double));
//     if (array == NULL) {
//         perror("malloc error\n");
//         return 1;
//     }

//     // ファイルから値を読み取り、配列に格納
//     double index, value;
//     int i = 0;
//     while (fscanf(file, "%lf %lf", &index, &value) == 2 && i < line_count) {
//         array[i] = value;
//         i++;
//     }

//     // ファイルを閉じる
//     fclose(file);

//     // 配列を返す
//     return array;

// }

void generateSpike(int frec, sfmt_t rng, FILE *file){
    int maxtime_count = (int)MAX_T / DT;
    int spikeNumber_count = 0;
    for(int count = 0; count < maxtime_count; count++){
       double time_ms = count * DT;
       double rand = sfmt_genrand_real2(&rng);
       bool spike = (rand < frec * DT);
       if(spike){
            fprintf(file, "%d %f\n", spikeNumber_count, time_ms);
            spikeNumber_count++;
        }
    }
}

int main(){    
    // parameter setup
    int maxtime_count = (int)MAX_T / DT;
    int spikeNumber_count = 0;
    // double spikedTime_count = 0;
    double exc_frec = 6.88;

    // random number setup
    uint32_t seed = (uint32_t)time(NULL);
    sfmt_t rng;
    sfmt_init_gen_rand (&rng, seed);

    // file setup (debug menu)
    FILE *exc_spike_file;
    char *e_filename = "exc_spike.dat";
    exc_spike_file = fopen(e_filename, "w");

    // generate random spike
    generateSpike(exc_frec, rng, exc_spike_file);

    // for(int count = 0; count < maxtime_count; count++){
    //    double time_ms = count * DT;
    //    double rand = sfmt_genrand_real2(&rng);
    //    bool spike = (rand < exc_frec * DT);
    //    if(spike){
    //         fprintf(exc_spike_file, "%d %f\n", spikeNumber_count, time_ms);
    //         spikeNumber_count++;
    //     }
    // }


    fclose(exc_spike_file);


    return 0;
}
