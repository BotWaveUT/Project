#include "printf.h"
#include "timer.h"
#include "irq.h"
#include "mini_uart.h"
#include "utils.h"
#include "i2s.h"
#include "math.h"
#include "buttons.h"
#include "keyboard.h"

#define N_OSCILLATORS 7
#define NB_DATA_DMA 1024
#define AUDIO_BUFFER_SIZE 128
#define DEC_AMPLITUDE_NOTE 0.0002
#define RISING_AMPLITUDE_NOTE 0.0005
#define PI 			3.14159f
#define DEFAULT_AMPLITUDE   20000
#define F_SAMPLE	    44100.0f //60000.0f

#define NB_OCTAVE_MAX 7

#define NB_NOTES_GrandTableau 49  // jusqu'à 3951.1 Hz

float tableauDesNotes[NB_NOTES_GrandTableau] = {
    32.703f,   36.708f,   41.203f,   43.654f,   48.999f,   55.000f,   61.735f,
    65.406f,   73.416f,   82.407f,   87.307f,   97.999f,   110.000f,  123.47f,
    130.81f,   146.83f,   164.814f,  174.614f,  195.998f,  220.000f,  246.94f,
    261.63f,   293.66f,   329.63f,   349.23f,   392.00f,   440.000f,  493.88f,
    523.25f,   587.330f,  659.255f,  689.456f,  783.991f,  880.000f,  987.77f,
    1046.502f, 1174.659f, 1318.510f, 1396.913f, 1567.982f, 1760.000f, 1975.533f,
    2093.005f, 2349.318f, 2637.020f, 2793.826f, 3135.963f, 3520.000f, 3951.1f
};

float phase_inc_table[NB_NOTES_GrandTableau];
int octaveSelect = 3;

void init_phase_inc_table(void)
{
    for (int i = 0; i < NB_NOTES_GrandTableau; i++)
    {
        phase_inc_table[i] = 2.0f * PI * tableauDesNotes[i] / F_SAMPLE;
    }
}

volatile uint64_t value_keyboard;


typedef enum { ON, OFF } OscillatorState;

typedef struct
{
    int button;
    OscillatorState state;
    float phase;
	float phase_inc;
    float amplitude;
} Oscillator;

static Oscillator oscillators[N_OSCILLATORS];
int16_t dataI2S[NB_DATA_DMA];

int HALF_BUFFER_SIZE = NB_DATA_DMA/2;
volatile uint8_t first_half_empty=0;
volatile uint8_t second_half_empty=0;
int amplitude_dyn = 10000;


void enveloppe_oscillator(float *buffer_out, Oscillator *osc){
	// fade in
	if(osc->state == ON){
		for(int i = 0; i < HALF_BUFFER_SIZE; i +=1){
			if(osc->amplitude < 1){
				osc->amplitude += RISING_AMPLITUDE_NOTE;
			}
			else{
				osc->amplitude = 1;
			}
			buffer_out[i] *= osc->amplitude;
			buffer_out[i+1] *= osc->amplitude;
		}
	// fade out
	} else {
		for(int i = 0; i < HALF_BUFFER_SIZE; i +=1){
			if(osc->amplitude > 0){
				osc->amplitude -= DEC_AMPLITUDE_NOTE;
			}
			else{
				osc->amplitude = 0;
			}
			buffer_out[i] *= osc->amplitude;
			buffer_out[i+1] *= osc->amplitude;
		}
	}
}

void process_oscillator(float *buffer_out, Oscillator *osc) {
/* Génère le signal audio d’un oscillateur,
   applique le fade in/out et écrit
   le résultat dans buffer_out */

	for (int i = 0; i < HALF_BUFFER_SIZE; i = i+2){
		osc->phase += osc->phase_inc;
			if (osc->phase >= 2.0f * PI)  osc->phase -= 2.0f * PI;
		float sample =  sinf(osc->phase);
		buffer_out[i] = sample;
		buffer_out[i+1] = sample;
	}
}


void process_output(int16_t *buffer_out) {
    /* Génère le signal audio complet */

	float target_grain;
	float output_tmp[HALF_BUFFER_SIZE]; // output_tmp : the temporary output buffer that will recieve the added amplified sin signals
	int active_voices = 0;

    static float current_grain = 1.0f;


	for(int i = 0; i < HALF_BUFFER_SIZE; i++){
		buffer_out[i] = 0;
		output_tmp[i] = 0;
	}

    for (int i = 0; i < N_OSCILLATORS; i++) {
        if (oscillators[i].state == ON || oscillators[i].amplitude > 0) {

        	float tmp_osc[HALF_BUFFER_SIZE];

        	// write in tmp buffer
            process_oscillator(&tmp_osc[0], &oscillators[i]);
            enveloppe_oscillator(&tmp_osc[0], &oscillators[i]);


            /* additionne le tmp au buffer_out...*/
            for (int i = 0; i < HALF_BUFFER_SIZE; i++){
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


   for (int i = 0; i < HALF_BUFFER_SIZE; i++) {

	   current_grain += (target_grain - current_grain) / HALF_BUFFER_SIZE;

	   float s = output_tmp[i] * current_grain;
	   buffer_out[i] = (int16_t)s;
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

void read_buttons() {
/* Lit l’état des boutons physiques et met à jour
   les champs des oscillateurs */
	for (int i = 0; i < N_OSCILLATORS; i++){
		if(Butt_pushed(oscillators[i].button)){
			(&oscillators[i])->state = ON;
		}
		else {
			(&oscillators[i])->state = OFF;
		}
	}
	int pot_input = read_input_pot();
	amplitude_dyn = pot_input*6;

	if (Dec_Oct_Butt_Clicked()){
		reduce_octave();
	}
	if (Inc_Oct_Butt_Clicked()){
		increase_octave();
	}
}
//void read_buttons(){
//
//}

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
	process_output(&dataI2S[0]);
	process_output(&dataI2S[HALF_BUFFER_SIZE]);

	// start the DMA with its first buffer
	I2S_SendData_DMA(dataI2S, NB_DATA_DMA);
}

uint64_t reverse35(uint64_t v)
{
    uint64_t r = 0;

    for (int i = 0; i < 40; i++)
    {
        if (v & (1ULL << i))
            r |= (1ULL << (34 - i));
    }

    return r;
}

void kernel_main(void)
{
	//uart_init();
	init_printf(0, putc);

	irq_vector_init();
	generic_timer_init();
	enable_interrupt_controller();
	disable_irq();
	init_I2S();
	Init_butts();
	Init_Pin_Keyboard();

	init_phase_inc_table();

	printf("initialization finished\r\n");

    /* Initialisation globale du synthétiseur */
    synth_init();

    /* Boucle principale */
    while (1)
    {
    	value_keyboard = reading_inputs();

		if(first_half_empty){

			process_output(&dataI2S[0]);
			first_half_empty = 0;

		}
		if(second_half_empty){

			process_output(&dataI2S[HALF_BUFFER_SIZE]);
			second_half_empty = 0;

		}
        /* Lecture des boutons et mise à jour des oscillateurs */
        read_buttons();

    }
		
}


// audio_fill_buffer(&buffer[0], BUFFER_SIZE / 2);
void SPI3_DMA_send_half_1(void)
{
	first_half_empty = 1;
}

// audio_fill_buffer(&buffer[BUFFER_SIZE / 2], BUFFER_SIZE / 2);
void SPI3_DMA_send_half_2(void)
{
	second_half_empty = 1;
}
