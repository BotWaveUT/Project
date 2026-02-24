#ifndef __OSCILLATOR_H__
#define __OSCILLATOR_H__

#define N_OSCILLATORS 10

#define DEC_AMPLITUDE_NOTE 0.0002
#define RISING_AMPLITUDE_NOTE 0.0005
#define PI 			3.141592653589f
#define DEFAULT_AMPLITUDE   20000
#define F_SAMPLE	    44100.0f //60000.0f

#define NB_OCTAVE_MAX 7



#define NB_NOTES_GrandTableau 49  // jusqu'à 3951.1 Hz

typedef enum { ON, OFF, RELEASE } OscillatorState;

typedef struct
{
    int button;
    OscillatorState state;
    float phase;
	float phase_inc;
    float amplitude;
} Oscillator;

extern volatile unsigned long value_keyboard;
extern volatile unsigned long old_keyboard;

extern volatile unsigned char first_half_empty;
extern volatile unsigned char second_half_empty;
extern Oscillator oscillators[N_OSCILLATORS];

void process_output(unsigned *buffer_out);
void init_phase_inc_table(void);
void synth_init(void);
void read_buttons();

#endif
