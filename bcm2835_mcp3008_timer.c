#include <stdio.h>
#include <stdlib.h>
#include <bcm2835.h>
#include <inttypes.h>

#define ADC_MAX 8 //number of ADC inputs
#define SAMPLE_MAX 100 //size of struct sample array 

//prototypes
int bcm_spi_init();
int readadc(int);

//struct to hold the adc result and the value at in the timer register
typedef struct {
	int adc[ADC_MAX]; //array of ints for each adc input
	uint64_t timer_uS[ADC_MAX];
	uint64_t tdiff_uS[ADC_MAX];
} adcStruct;


int main(int argc, char **argv) 
{
	//initialize struct array
	adcStruct adcbuf[SAMPLE_MAX];
	//parse command line arguments
	//ex: (numSamples)
	//intialize spi
	if(bcm_spi_init()) return 1; //failed to open 
	//variables to hold timer values
	uint64_t timer_s, timer_e;
	//take timer value at acd read start
	timer_s = bcm2835_st_read();
	printf("Timer Start: %ld\n", timer_s);
	//read each every adc input and store it in adcbuf
	printf("mcp3008 ADC readings\n");
	printf("Pass\tInput\tValue\tTimer Register\tuS Difference\n");
	int i, j=0;
	for(i=0; i<SAMPLE_MAX; i++) //for SAMPLE_MAX times
 	{
		for(j=0; j<ADC_MAX; j++) 
		{ 	
			//for ADC_MAX times
			//take adc value and store it in struct adcbuf[i].adc[j]
			adcbuf[i].adc[j] = readadc(j);
			//store current value of timer register in struct
			adcbuf[i].timer_uS[j] = bcm2835_st_read();
			//store difference between timer values in struct
			if(i==0&&j==0) 
			{ 	
				//if this is the very first sample 
				//then store the timer value as the difference between time now and timer_s
				adcbuf[i].tdiff_uS[j] = (bcm2835_st_read() - timer_s); 
			}
			else 
			{
				if(i!=0&&j==0) 
				{ 
					//first sample of a new pass
					adcbuf[i].tdiff_uS[j] = (bcm2835_st_read() - adcbuf[i-1].timer_uS[7]); 
				}
					else
					{
						//store timer_uS as the difference between the current timer 
						//and that of the previous element
						adcbuf[i].tdiff_uS[j] = (bcm2835_st_read() - adcbuf[i].timer_uS[j-1]); 
					} 
			}
			printf("[%d]\t[%d]\t%d\t%lld\t%lld\n", i, j, adcbuf[i].adc[j], adcbuf[i].timer_uS[j], adcbuf[i].tdiff_uS[j]); 
		}
	}
	//take end timer value
	timer_e = bcm2835_st_read();
	printf("Read took %lld microseconds\n", (timer_e-timer_s));
	bcm2835_spi_end();
	bcm2835_close();
	return 0; 
}	

int bcm_spi_init() {
	//bcm2835_set_debug(1); 
	if (!bcm2835_init()) return 1; 
	bcm2835_spi_begin();
	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST); //default
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0); //default
	bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_2048); //122khz ~(250mhz/2048)
	bcm2835_spi_chipSelect(BCM2835_SPI_CS0); 
	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);
	return 0;
}

int readadc(int channel) {
	if(channel>ADC_MAX||channel<0) { 
		printf("ERROR: Invalid Channel. Valid Channels are %d through %d\n", 0, ADC_MAX); 
		return -1;  
	}
	uint8_t buf[] = { 1, ((8+channel)<<4), 0 }; //see mcp3008 datasheet on spi
	bcm2835_spi_transfern(buf, sizeof(buf));
	int adcout = (((buf[1]&3) << 8) + buf[2]); 
	return adcout;
}
