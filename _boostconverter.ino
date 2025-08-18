//#include <Arduino.h>
#include <avr/io.h>

// We use Timer2 and OC2B for Fast PWM output to the switching circuit.
// This takes up Port D3 (Arduino pin 3).
// We'll use ADC7 (Arduino pin A7) for ADC Boost Converter voltage feedback (thru resistor divider)
// a 1M and a 100k resistor should scale 12V to 1.09V nicely


// 20V feedback ADC target: 0.9523809523809523 V
// 12V feedback ADC target: 0.5714285714285714 V
// 9V feedback ADC target: 0.42857142857142855 V


#define TOP_RESISTOR (100000.0)
#define BOTTOM_RESISTOR (4950.0)
#define VOLT_SCALAR (TOP_RESISTOR + BOTTOM_RESISTOR)
#define VOLT_FACTOR (BOTTOM_RESISTOR / VOLT_SCALAR)

#define TARGET_VOLTAGE (12)
#define ADC_REF_VOLTAGE (1.1f)
#define TARGET_ADC_VOLTAGE TARGET_VOLTAGE* VOLT_FACTOR
#define TARGET_ADC_RESULT ((TARGET_ADC_VOLTAGE * 1024.0f / ADC_REF_VOLTAGE) + 1L)

#define DUTYCYCLE OCR2B // Port D3, Arduino pin D3



void start_boost_converter() {
  // Reset Duty Cycle to low-voltage.
  // Set enable to high. This should also disenage the PMOSFET connecting the LV reset pin.
  // Start PWM output to switch.
  
  // TODO: open Q1 to drain load to ground until stabilised so we don't overshoot voltage straight into MCU

  TCCR2B = (0 << CS22) | (1 << CS21) | (0 << CS20);  // Start timer with div8 prescale
  TCCR2A |= (1 << COM2B1); // enable PWM output on PD3

  // We'll hang here on the ADC waiting for the voltage to stabilise close to the target by adjusting DC
  // then go into ADC free-running with interrupts to monitor and adjust duty cycle

  const uint16_t adc = TARGET_ADC_RESULT;
  const uint8_t ADC_MARGIN = 5;
  const uint16_t adc_result_lower_bound = TARGET_ADC_RESULT - ADC_MARGIN;
  const uint16_t adc_result_upper_bound = TARGET_ADC_RESULT + ADC_MARGIN;
  //const float closeenough_low = ((((TARGET_ADC_RESULT - ADC_MARGIN) - 1L) * 1.1f) / 1024.0f)/VOLT_FACTOR;
  //const float closeenough_hi = ((((TARGET_ADC_RESULT + ADC_MARGIN) - 1L) * 1.1f) / 1024.0f)/VOLT_FACTOR;



  //do {
  //  ADCSRA |= (1 << ADSC);
  //  while (ADCSRA & (1 << ADSC))
  //    ;
  //  boostconverter_feedback_adjust();
//  } while ((ADC < adc_result_lower_bound) || (ADC > adc_result_upper_bound ));
    set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_enable();


  // Enable ADC, enable auto-trigger: freerunning mode, enable interrupts
  ADCSRA |= (1 << ADEN) | (1 << ADATE) | (1 << ADIE);
  // Start first ADC conversion - kicks-off free running
  ADCSRA |= (1 << ADSC);

  // fuck it just do it 255 times
  for (uint8_t i = 0x0; i < 0xff; ++i) {
        sleep_cpu();

   // boostconverter_feedback_adjust();
  }

}

void stop_boost_converter() {
  // Reset Duty Cycle to low-voltage.
  // Set enable to low. 
  // Stop PWM output to switch.

  TCCR2B = (0 << CS22) | (0 << CS21) | (0 << CS20);  // Stop timer

  // Disable ADC
  ADCSRA = 0;

  TCCR2A &= ~(1 << COM2B1); // disable PWM output on PD3
  PORTD &= ~(1 << PORTD3); // set PD3 to low - opens switch, HV pin should be 5V
}

void setup_boost_converter() {
  // Set up ADC for BC feedback operation

  // Select ADC7 as input
  // Set reference to internal 1.1V source
  ADMUX = (1 << REFS1) | (1 << REFS0) | (0 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0);

  // Enable ADC, enable auto-trigger:freerunning mode, enable interrupts
  // ADCSRA |= (1 << ADEN) | (1 << ADATE) | (1 << ADIE);

  DDRD |= (1 << DDD3);  // Set Timer2 CompareB pin (Port D3) to output

  // We want a 1MHz fast PWM clock on OC2B/

  TCCR2A = (1 << COM2B1) | (0 << COM2B0) | (1 << WGM21) | (1 << WGM20);  // Normal mode, PWM output enabled on OC2B; Fast PWM waveform gen mode

  DUTYCYCLE = 1;
  //OCR2B = dutyCycle; // set duty cycle - lower means lower volts (27 gave 12.1V open-circuit)

  //TCCR2B = (0 << CS22) | (1 << CS21) | (0 << CS20);  // Start timer with div8 prescale



  // Start first ADC conversion - kicks-off free running
  //ADCSRA |= (1 << ADSC);
}

void boostconverter_feedback_adjust() {
      // Read ADC conversion
      // Compare to (scaled) target
      // adjust duty cycle up or down to meet target (binary search...?)
      // something something hysteresis

      // Remember: ADCResult = Vin * 1024 / Vref
      //uint16_t adc = ADC;


      if (ADC > (uint16_t)TARGET_ADC_RESULT) {
        // adc voltage too high, reduce DC (but not below zero to 0xff!)
        if (DUTYCYCLE)
          --DUTYCYCLE;
      } else if (ADC < (uint16_t)TARGET_ADC_RESULT) {
        // ADC duty cycle too low, increase DC (but not above, say, 35 for now)
        if (DUTYCYCLE < 128)
          ++DUTYCYCLE;
      }
}


ISR(ADC_vect) {
  boostconverter_feedback_adjust();
}
