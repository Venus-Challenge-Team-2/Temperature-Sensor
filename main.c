#include <stdio.h>
#include <math.h>
#include <libpynq.h>
#include <platform.h>

#define VCC 3.3
#define R_FIXED 10000.0

#define R0 10000.0
#define T0 298.15
#define BETA 4050.0

#define CAL_VOUT 0.643
#define CAL_RNTC 9600.0

static double parallel_from_voltage(double v_out) {
    return R_FIXED * v_out / (VCC - v_out);
}

static double calculate_r_load(double v_cal, double r_ntc_cal) {
    double r_parallel = parallel_from_voltage(v_cal);
    double denom = (1.0 / r_parallel) - (1.0 / r_ntc_cal);

    if (denom <= 0.0) {
        return -1.0;
    }

    return 1.0 / denom;
}

static double ntc_from_parallel(double r_parallel, double r_load) {
    double denom = (1.0 / r_parallel) - (1.0 / r_load);

    if (denom <= 0.0) {
        return -1.0;
    }

    return 1.0 / denom;
}

static double resistance_to_temp_c(double r_ntc) {
    double temp_k;
    temp_k = 1.0 / ((1.0 / T0) + (log(r_ntc / R0) / BETA));
    return temp_k - 273.15;
}

int main(void) {
    pynq_init();
    adc_init();

    double r_load = calculate_r_load(CAL_VOUT, CAL_RNTC);

    printf("Calculated R_load = %.2f ohm\n", r_load);

    while (1) {
        double v_out = adc_read_channel(ADC0);
        double r_parallel = parallel_from_voltage(v_out);
        double r_ntc = ntc_from_parallel(r_parallel, r_load);

        if (r_ntc <= 0.0) {
            printf("V_out: %.3f V | invalid\n", v_out);
            sleep_msec(1000);
            continue;
        }

        double temp_c = resistance_to_temp_c(r_ntc);

        printf("V_out: %.3f V | R_ntc: %.2f ohm | Temp: %.2f C\n",
               v_out, r_ntc, temp_c);

        sleep_msec(1000);
    }

    adc_destroy();
    pynq_destroy();
    return 0;
}
