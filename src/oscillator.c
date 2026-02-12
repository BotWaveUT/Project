#include "oscillator.h"
#include "timer.h"
#include "irq.h"
#include "mini_uart.h"
#include "utils.h"
#include "i2s.h"
#include "math.h"
#include "dma.h"
#include "input.h"

float tableauDesNotes[NB_NOTES_GrandTableau] = {
    32.703f,   36.708f,   41.203f,   43.654f,   48.999f,   55.000f,   61.735f,
    65.406f,   73.416f,   82.407f,   87.307f,   97.999f,   110.000f,  123.47f,
    130.81f,   146.83f,   164.814f,  174.614f,  195.998f,  220.000f,  246.94f,
    261.63f,   293.66f,   329.63f,   349.23f,   392.00f,   440.000f,  493.88f,
    523.25f,   587.330f,  659.255f,  689.456f,  783.991f,  880.000f,  987.77f,
    1046.502f, 1174.659f, 1318.510f, 1396.913f, 1567.982f, 1760.000f, 1975.533f,
    2093.005f, 2349.318f, 2637.020f, 2793.826f, 3135.963f, 3520.000f, 3951.1f
};

static float phase_inc_table[NB_NOTES_GrandTableau];
static int octaveSelect = 3;
volatile unsigned long value_keyboard;
Oscillator oscillators[N_OSCILLATORS];
volatile unsigned char first_half_empty=0;
volatile unsigned char second_half_empty=0;
static int amplitude_dyn = 10000;



void init_phase_inc_table(void)
{
    for (int i = 0; i < NB_NOTES_GrandTableau; i++)
    {
        phase_inc_table[i] = 2.0f * PI * tableauDesNotes[i] / F_SAMPLE;
    }
}

void enveloppe_oscillator(float *buffer_out, Oscillator *osc){
	// fade in
	if(osc->state == ON){
		for(int i = 0; i < DMA_BUFFER_SIZE_HALF; i +=1){
			if(osc->amplitude < 1){
				osc->amplitude += RISING_AMPLITUDE_NOTE;
			}
			else{
				osc->amplitude = 1;
			}
			buffer_out[i] *= osc->amplitude;
		}
	// fade out
	} else {
		for(int i = 0; i < DMA_BUFFER_SIZE_HALF; i +=1){
			if(osc->amplitude > 0){
				osc->amplitude -= DEC_AMPLITUDE_NOTE;
			}
			else{
				osc->amplitude = 0;
			}
			buffer_out[i] *= osc->amplitude;
		}
	}
}

void process_oscillator(float *buffer_out, Oscillator *osc) {
/* Génère le signal audio d’un oscillateur,
   applique le fade in/out et écrit
   le résultat dans buffer_out */

	for (int i = 0; i < DMA_BUFFER_SIZE_HALF; i++){
		osc->phase += osc->phase_inc;
			if (osc->phase >= 2.0f * PI)  osc->phase -= 2.0f * PI;
		float sample =  sinf(osc->phase);
		buffer_out[i] = sample;
	}
}

void process_output(unsigned *buffer_out) {
    /* Génère le signal audio complet */

	float target_grain;
	float output_tmp[DMA_BUFFER_SIZE_HALF]; // output_tmp : the temporary output buffer that will recieve the added amplified sin signals
	int active_voices = 0;

    static float current_grain = 1.0f;


	for(int i = 0; i < DMA_BUFFER_SIZE_HALF; i++){
		buffer_out[i] = 0;
		output_tmp[i] = 0;
	}

    for (int i = 0; i < N_OSCILLATORS; i++) {
        if (oscillators[i].state == ON || oscillators[i].amplitude > 0) {

        	float tmp_osc[DMA_BUFFER_SIZE_HALF];

        	// write in tmp buffer
            process_oscillator(&tmp_osc[0], &oscillators[i]);
            enveloppe_oscillator(&tmp_osc[0], &oscillators[i]);


            /* additionne le tmp au buffer_out...*/
            for (int i = 0; i < DMA_BUFFER_SIZE_HALF; i++){
            	output_tmp[i] += tmp_osc[i]* amplitude_dyn;
            }
            active_voices += 1;

        }
    }
    // divide global amplitude depending on the number of oscillators and write buffer_out

    if (active_voices > 0 ){
    	target_grain = 1.0f / (float)active_voices;
    }
    else{
    	target_grain = 1.0f;
    }


   for (int i = 0; i < DMA_BUFFER_SIZE_HALF; i++) {

	   current_grain += (target_grain - current_grain) / DMA_BUFFER_SIZE_HALF;

	   float s = output_tmp[i] * current_grain;
	   buffer_out[i] = ((signed short)s << 16) | (signed short)s;
   }
}

void change_octaves(){
	for (int i = 0; i < N_OSCILLATORS; i++) {
		(&oscillators[i])->phase_inc = phase_inc_table[i+7*octaveSelect];
	}
}

void reduce_octave(){
	if(octaveSelect > 0){
		octaveSelect -= 1;
		change_octaves();
	}
}

void increase_octave(){
	if(octaveSelect < NB_OCTAVE_MAX-1){
		octaveSelect += 1;
		change_octaves();
	}
}

void synth_init(void) {
    /* Initialise les oscillateurs, les buffers audio
    et les paramètres globaux du synthé */

	for (int i = 0; i < N_OSCILLATORS; i++) {
			Oscillator o;
			o.phase = 0.0f;
			o.amplitude = 0.0f;
			o.state = OFF;
			o.phase_inc = phase_inc_table[i+7*octaveSelect];
			oscillators[i] = o;

	}
	oscillators[0].button = BUTT_DO;
	oscillators[1].button = BUTT_RE;
	oscillators[2].button = BUTT_MI;
	oscillators[3].button = BUTT_FA;
	oscillators[4].button = BUTT_SOL;
	oscillators[5].button = BUTT_LA;
	oscillators[6].button = BUTT_SI;

	// init first DMA buffer
	//process_output(&dma_buffer[0]);
	//process_output(&dma_buffer[DMA_BUFFER_SIZE_HALF]);
}

unsigned long reverse35(unsigned long v)
{
    unsigned long r = 0;

    for (int i = 0; i < 40; i++)
    {
        if (v & (1ULL << i))
            r |= (1ULL << (34 - i));
    }

    return r;
}