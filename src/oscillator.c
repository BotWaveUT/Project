#include "oscillator.h"
#include "dma.h"
#include "i2s.h"
#include "input.h"
#include "irq.h"
#include "math.h"
#include "mini_uart.h"
#include "timer.h"
#include "utils.h"
#include <stdint.h>

#define NB_BUTTONS 35

float tableauDesNotes[NB_NOTES_GrandTableau] = {
    65.406f,   73.416f,   82.407f,   87.307f,   97.999f,   110.000f,  123.47f,
    130.81f,   146.83f,   164.814f,  174.614f,  195.998f,  220.000f,  246.94f,
    261.63f,   293.66f,   329.63f,   349.23f,   392.00f,   440.000f,  493.88f,
    523.25f,   587.330f,  659.255f,  689.456f,  783.991f,  880.000f,  987.77f,
    1046.502f, 1174.659f, 1318.510f, 1396.913f, 1567.982f, 1760.000f, 1975.533f,
    2093.005f, 2349.318f, 2637.020f, 2793.826f, 3135.963f, 3520.000f, 3951.1f};

static float phase_inc_table[NB_NOTES_GrandTableau];
static int octaveSelect = 3;
volatile unsigned long value_keyboard;
volatile unsigned long old_keyboard;
Oscillator oscillators[N_OSCILLATORS];
volatile unsigned char first_half_empty = 0;
volatile unsigned char second_half_empty = 0;
static int amplitude_dyn = 10000;

void init_phase_inc_table(void) {
    for (int i = 0; i < NB_NOTES_GrandTableau; i++) {
        phase_inc_table[i] = tableauDesNotes[i] / F_SAMPLE;
    }
}

void enveloppe_oscillator(float *buffer_out, Oscillator *osc) {
    // fade in
    if (osc->state == ON) {
        for (int i = 0; i < DMA_BUFFER_SIZE_HALF; i += 1) {
            if (osc->amplitude < 1) {
                osc->amplitude += RISING_AMPLITUDE_NOTE;
            } else {
                osc->amplitude = 1;
            }
            buffer_out[i] *= osc->amplitude;
        }
        // fade out
    } else if (osc->state == RELEASE) {
        for (int i = 0; i < DMA_BUFFER_SIZE_HALF; i += 1) {
            osc->amplitude -= DEC_AMPLITUDE_NOTE;

            if (osc->amplitude <= 0.0f) {
                osc->amplitude = 0.0f;
                osc->state = OFF;
            }

            buffer_out[i] *= osc->amplitude;
        }
    }
}

void process_oscillator(float *buffer_out, Oscillator *osc) {
    /* Génère le signal audio d’un oscillateur,
       applique le fade in/out et écrit
       le résultat dans buffer_out */

    for (int i = 0; i < DMA_BUFFER_SIZE_HALF; i++) {
        osc->phase += osc->phase_inc;
        if (osc->phase >= 1.0f)
            osc->phase -= 1.0f;
        float sample = sinf(osc->phase);
        buffer_out[i] = sample;
    }
}
char Butt_pushed(int butt_id, uint64_t keyboard) {

    return ((keyboard >> (butt_id - 1)) & 1ULL) != 0;
}
/* Vérifie si une note est associée à un oscillo*/
int has_oscilloscope(int butt_id) {
    for (int i = 0; i < N_OSCILLATORS; i++) {
        if (oscillators[i].button == butt_id)
            return i;
    }
    return -1;
}

/*Si une note possède oscill et n'est plus appuyée alors on la passe en release */
void put_release(int butt_id) {
    int i = 0, found = 0;
    while (!found && (i < N_OSCILLATORS)) {
        if (oscillators[i].button == butt_id) {
            // oscillators[i].state = RELEASE;
            oscillators[i].state = OFF;
            found = 1;
        }
        i++;
    }
}
/* Cherche les notes qui ne sont plus appuyées et on met en release*/
void search_released(uint64_t v) {
    for (int i = 0; i < NB_BUTTONS; i++) {
        if ((v >> i) & 1ULL)
            put_release(i + 1);
    }
}
/*Renvoie le premier indice d'un oscilator en mode off*/
int find_off() {
    for (int i = 0; i < N_OSCILLATORS; i++) {
        if (oscillators[i].state == OFF) {
            return i;
        }
    }
    return -1;
}
/*met l'oscil en mode ON*/
void set_oscillator(int note, int i) {
    oscillators[i].button = note;
    oscillators[i].phase = 0.0f;
    oscillators[i].amplitude = 0.0f;
    oscillators[i].state = ON;
    oscillators[i].phase_inc = phase_inc_table[note - 1];
}

/* Pour chaque nouvelle note appuyer essayer de l'ajouter à */
void adress_pressed_buttons(uint64_t v) {
    for (int i = 0; i < NB_BUTTONS; i++) {
        if ((v >> i) & 1ULL) {
            // there is a newly pressed button
            if (has_oscilloscope(i + 1) == -1) {

                int place = find_off();

                if (place != -1) {

                    set_oscillator(i + 1, place);

                } else {

                    // there is no off button for this so no need to try again with other buttons
                    break;
                }
            } else {
                // a un oscillator
                oscillators[has_oscilloscope(i + 1)].state = ON;
            }
        }
    }
}

void read_buttons() {
    value_keyboard = reading_inputs();
    // if (value_keyboard != old_keyboard) {
    unsigned long newly_pressed = ~old_keyboard & value_keyboard;
    unsigned long newly_released = old_keyboard & ~value_keyboard;

    search_released(newly_released);
    adress_pressed_buttons(newly_pressed);

    old_keyboard = value_keyboard;
    // }
}

void process_output(unsigned *buffer_out) {
    // Génère le signal audio complet

    // float target_grain;
    float output_tmp[DMA_BUFFER_SIZE_HALF]; // output_tmp : the temporary output buffer that will
                                            // recieve the added amplified sin signals
    // int active_voices = 0;

    // static float current_grain = 1.0f;

    for (int i = 0; i < DMA_BUFFER_SIZE_HALF; i++) {
        buffer_out[i] = 0;
        output_tmp[i] = 0;
    }

    for (int i = 0; i < N_OSCILLATORS; i++) {
        if (oscillators[i].state == ON || oscillators[i].amplitude > 0) {

            float tmp_osc[DMA_BUFFER_SIZE_HALF];

            // write in tmp buffer
            process_oscillator(tmp_osc, &oscillators[i]);
            // enveloppe_oscillator(tmp_osc, &oscillators[i]);

            // additionne le tmp au buffer_out...
            for (int i = 0; i < DMA_BUFFER_SIZE_HALF; i++) {
                output_tmp[i] += tmp_osc[i];
            }
            // active_voices += 1;
        }
    }
    // divide global amplitude depending on the number of oscillators and write buffer_out
    /*    if (active_voices > 0 ){
            target_grain = 1.0f / (float)active_voices;
        }
        else{
            target_grain = 1.0f;
        }

       for (int i = 0; i < DMA_BUFFER_SIZE_HALF; i++) {

               current_grain += (target_grain - current_grain) / DMA_BUFFER_SIZE_HALF;

               float s = output_tmp[i] * current_grain;
               buffer_out[i] = (unsigned)((((unsigned)s) << 16) | (unsigned)s);
       }*/

    for (int i = 0; i < DMA_BUFFER_SIZE_HALF; i++) {

        output_tmp[i] /= 10.;

        // soft clipping
        if (output_tmp[i] >= 1.0)
            output_tmp[i] = (2.0f / 3.0f);
        else if (output_tmp[i] <= -1.0)
            output_tmp[i] = -(2.0f / 3.0f);
        else
            output_tmp[i] =
                output_tmp[i] - ((output_tmp[i] * output_tmp[i] * output_tmp[i]) / 3.0f);

        unsigned short s = (signed short)(output_tmp[i] * 30000);

        /* if (s < 0) {
        buffer_out[i] = 0;
   } else*/
        buffer_out[i] = (((unsigned)s << 16) | (unsigned)s);
    }
}

void change_octaves() {
    for (int i = 0; i < N_OSCILLATORS; i++) {
        (&oscillators[i])->phase_inc = phase_inc_table[i + 7 * octaveSelect];
    }
}

void reduce_octave() {
    if (octaveSelect > 0) {
        octaveSelect -= 1;
        change_octaves();
    }
}

void increase_octave() {
    if (octaveSelect < NB_OCTAVE_MAX - 1) {
        octaveSelect += 1;
        change_octaves();
    }
}

void synth_init(void) {
    /* Initialise les oscillateurs, les buffers audio
    et les paramètres globaux du synthé */

    for (int i = 0; i < N_OSCILLATORS; i++) {
        oscillators[i].phase = 0.0f;
        oscillators[i].amplitude = 0.0f;
        oscillators[i].state = OFF;
        oscillators[i].phase_inc = phase_inc_table[i];
        // oscillators[i].button = i + 1;
        oscillators[i].button = -1;
    }

    // oscillators[0].button = BUTT_DO;
    // oscillators[1].button = BUTT_RE;
    // oscillators[2].button = BUTT_MI;
    // oscillators[3].button = BUTT_FA;
    // oscillators[4].button = BUTT_SOL;
    // oscillators[5].button = BUTT_LA;
    // oscillators[6].button = BUTT_SI;

    // init first DMA buffer
    // process_output(&dma_buffer[0]);
    // process_output(&dma_buffer[DMA_BUFFER_SIZE_HALF]);
}
