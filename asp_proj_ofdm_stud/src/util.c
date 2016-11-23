#include "util.h"
#include "backend/printfn/printfn.h"
#include "backend/hw/board.h"

void print_vector_f(char * name, float * vals, int_fast32_t len){
	printf("%s = [", name);
	int_fast32_t i;
	for(i = 0; i < len; i++){
		if(i != 0){
			//Suppress a tab character on the first output
			printf("\t");
		}

		printf("%e", *(vals + i));
		if(i == len - 1){
			//On the last call, add a ']' symbol to indicate the end of the vector
			printf("]");
		}
		printf(";\n");
	}
}

void print_cplx_vector_f(char * prefix, float * vals, int_fast32_t len){
	printf("%s [", prefix);
	int_fast32_t i;
	for(i = 0; i < len/2; i++){
		if(i != 0){
			//Suppress a tab character on the first output
			printf("\t");
		}

		printf("%e", *(vals + 2*i));
		printf(" + %e*i", *(vals + 2*i + 1));
		if(i == (len/2 - 1)){
			//On the last call, add a ']' symbol to indicate the end of the vector
			printf("]");
		}
		printf(";\n");
	}
}

void print_vector_i(char * prefix, int32_t * vals, int_fast32_t len){
	printf("%s [", prefix);
	int_fast32_t i;
	for(i = 0; i < len; i++){
		if(i != 0){
			//Suppress a tab character on the first output
			printf("\t");
		}

		printf("%d", *(vals + i));
		if(i == len - 1){
			//On the last call, add a ']' symbol to indicate the end of the vector
			printf("]");
		}
		printf(";\n");
	}
}

void vector_f2i(float * vals_f, int32_t * vals_i, int_fast32_t len, float scale){
	int_fast32_t i;
	for(i = 0; i < len; i++){
		*(vals_i + i) = (int32_t) (*(vals_f + i) * scale);
	}
}

float vector_mean(float * vals, int_fast32_t len){
	float retval = 0;
	int_fast32_t i;
	for(i = 0; i < len; i++){
		retval += *(vals + i);
	}
	retval /= len;
	return retval;
}

void halt_error(char * errmsg){
	printf("%s", errmsg);
	board_set_led(board_led_red, false);
	board_set_led(board_led_orange, false);
	board_set_led(board_led_green, false);
	board_set_led(board_led_blue, false);
	for(;;){
		//Blink all status LED's
		volatile uint_fast32_t i;
		for(i = 0; i < 10e6; i++){};	//Crude delay function
		board_toggle_led(board_led_red);
		board_toggle_led(board_led_orange);
		board_toggle_led(board_led_green);
		board_toggle_led(board_led_blue);
	}
}
