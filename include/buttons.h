/*
 * buttons.h
 *
 *  Created on: Jan 26, 2026
 *      Author: nono
 */

#ifndef BUTTONS_H_
#define BUTTONS_H_

#define BUTT_DO 7
#define BUTT_RE 1
#define BUTT_MI 2
#define BUTT_FA 3
#define BUTT_SOL 4
#define BUTT_LA 5
#define BUTT_SI 6

#define BUTT_OCT_INC 8
#define BUTT_OCT_DEC 9


void Init_butts (void);

char Butt_pushed(int butt_id);
char Inc_Oct_Butt_Clicked();
char Dec_Oct_Butt_Clicked();


int read_input_pot();


#endif /* BUTTONS_H_ */
