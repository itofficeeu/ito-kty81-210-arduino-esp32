/*
 * This program reads continous the temperature determined by a KTY81-210
 * and outputs it in the Arduino monitor, alias terminal.
 * 
 * The software is written with adaptability in mind,
 * and shows the calculations behind the temperature output
 * in a way where it is possible to understand and modify these.
 * 
 * The precision is dependent on your own calibration.
 * 
 * 
 * SOFTWARE
 * 
 * C/C++ in the Arduino IDE
 * 
 * Author: Andreas Chr. Dyhrberg @itoffice.eu
 * 
 * Licens: This software may be distributed and modified under the terms of the 
 * GNU General Public License version 2 (GPL2) as published by the Free Software 
 * Foundation and appearing in the file GPL2.TXT included in the packaging of 
 * this file. Please note that GPL2 Section 2[b] requires that all works based 
 * on this software must also be made publicly available under the terms of the 
 * GPL2 ("Copyleft").
 * 
 * 
 * HARDWARE
 * 
 * A KTY81-210 mounted in the Sensor Adapter Switchboard from itoffice.eu 
 * 
 * The Sensor Adapter Switchboard is mounted on the Minimal-X Extension Board from itoffice.eu
 * 
 * ESP32 in a Heltec WiFi Lora 32 that runs on top of the Minimal-X Extension Board.
 */

/* The sensor module "Sensor Adapter Switchboard", 
 * alias the circuit around the KTY81. Setup as a voltage divider like this:
 * 
 *  (Vcc 5+)--+-->3k--+-->kty81-210---->(GND)
 *                    |
 *                    +-----> ADC (Analog Port of the microcontroller)
 */
int sensor_pin = 36;
float sensor_vdd = 5.2; /* Volt */
float sensor_r1 = 3000; /* Ohm - Can be between 2.7 and 5.4 K Ohm. (Keep the current below 1mA, if posible) */
float sensor_r2_kty81210_rating_at_25_degrees = 2000; /* Ohm - Resistor value of KTY81-210 at 25 degrees Celcius */
float sensor_correction = 88;
  
/* Limitations of the microcontroller */
float mc_analog_volt_limit  = 3.3;  /* Volt - Maximal voltage that the pin can work with */
float mc_analog_read_limit  = 4095; /* Esp32: 4095, Arduino: 1023 */
float mc_correction = 0;

/* The circuit on the Minimal-X Extension Board where the sensor is inserted. */
float voltage_divider_r1 = 26850; /* Ohm */
float voltage_divider_r2 = 26850; /* Ohm */
float voltage_divider_ratio = voltage_divider_r2 / (voltage_divider_r1+voltage_divider_r2); /* In this case is the result 0.5 */

/* Implications of the microcontroller and the extension board combined. */
float voltage_divider_volt_limit = mc_analog_volt_limit / voltage_divider_ratio;
/* In the actual case is the result 6.6V and the sensor module has to return values within that limit. */

void setup()
{
  Serial.begin(115200);

  /* Print a header */
  /* - Using columns make it easy to copy-paste into a spreadsheet. */
  Serial.print("Resistance/Ohms ;");
  Serial.print(" Temp./Celcius ;");
  Serial.print(" Temp./Fahrenheit");
  Serial.println("");
}

void loop()
{
  /* Read the raw integer value from the sensor. */
  float analog_read_value = analogRead(sensor_pin);

  /* Convert the raw integer value to a voltage. */
  float analog_read_volt = (analog_read_value / mc_analog_read_limit ) * voltage_divider_volt_limit;

  /* Correct for the microcontroller isn't totally accurate. */
  analog_read_volt = McCorrection(analog_read_volt);

  /*Calculate the resistance of the KTY81 based on the voltage. */
  float sensor_r2_kty81210_rating_at_25_degrees_resistance = ( analog_read_volt * (sensor_r1+sensor_r2_kty81210_rating_at_25_degrees) ) / sensor_vdd;

  /* Correct for the KTY81 being off ideal values. */
  float sensor_r2_kty81210_resistance = sensor_r2_kty81210_rating_at_25_degrees_resistance + sensor_correction;

  /* Calculate the temperature in Celcius based on the resistance of the KTY81. */
  /* - Celcuis: */
  float kty81210_temperature_celcius = temperatureForResistance(sensor_r2_kty81210_resistance);
  /* - Fahrenheit: */
  float kty81210_temperature_fahrenheit = (kty81210_temperature_celcius * 9.0)/ 5.0 + 32.0; 

  /* The value kty81210_temperature will still fluctuate if you haven't attached a capacitor to avoid noise. */
  /* A running mean could maybe also help. */

  Serial.print("           ");
  Serial.print(sensor_r2_kty81210_resistance,0);

  Serial.print(" ;          ");
  Serial.print(kty81210_temperature_celcius,1);

  Serial.print(" ;             ");
  Serial.print(kty81210_temperature_fahrenheit,1);
  Serial.println("");
  delay (1000);
}

/* Corrects the voltage running out of the voltage divider into the microcontroller (Mc) */
float McCorrection(float volt)
{
  /*
  //float mc_correction = 0.63; // @ 5V
  //float mc_correction = 0.48; // @ 3.3V
  
  Determine a and b in y = a*x + b
  
  Coefficient (a):
  5 - 3.3 = 1.7
  0.63 - 0.48 = 0.15
  0.15 / 1.7 = 0,088235294
  
  X=0-value (b):
  0.48 - (3.3 * 0,088235294)
  0.48 - 0,291176471 = 0,188823529
  
  => mc_correction = 0,188823529 + (volt*0,088235294)

  Swapping the inpput of the voltage divider between two known voltages (3.3V and 5.2V).
  Measured the output of the voltage divider.
  Then manually adjusted the formula while shifting between the two known voltages and measuring.
  Ended up with the below:
  */

  return volt + (0.25 + (volt*0.085)); /* Note: Determined at 28 degrees Celcius room temperature */
}

/* Source: https://www.eevblog.com/forum/beginners/kty81-220-temp-sensor-meh!/msg1371075/?PHPSESSID=jm8heeqc4easvca9f7gbur4ai7#msg1371075 */
struct resistance_row {
  int temp;
  int r2;
};

/* Source: Page 7 of 15 of the datasheet on https://www.nxp.com/docs/en/data-sheet/KTY81_SER.pdf
 *  
 * No, not the "smartest" way in the sense of "cool",
 * but the smartest in the sense of the simplest and most adaptable way,
 * as the datasheet only delivers data in this way.
 * More on:  https://en.wikipedia.org/wiki/KISS_principle
 */
struct resistance_row kty81210_resistance_table[] = {
  {0, 1630},
  {10, 1772},
  {20, 1922},
  {25, 2000},
  {30, 2080},
  {40, 2245},
  {50, 2417},
  {60, 2597},
  {70, 2785},
  {80, 2980},
  {90, 3182},
  {100, 3392},
  {110, 3607},
  {120, 3817},
  {130, 3915},
  {-1,-1}
};

/* Source: https://www.eevblog.com/forum/beginners/kty81-220-temp-sensor-meh!/msg1371075/?PHPSESSID=jm8heeqc4easvca9f7gbur4ai7#msg1371075 */
float temperatureForResistance( float resistance ) {
  int index = -1;
  boolean notFound=true;
 
  while(notFound)  {
    index+=1;
    if( kty81210_resistance_table[index].temp == -1 ) {
      return -1.0; // ERROR
    }
    if( kty81210_resistance_table[index].r2 > resistance ) {
      notFound = false;     
    }
  }
  if( index < 1 ) {
    return -1.0; // ERROR
  }
  int lowerBandT = kty81210_resistance_table[index-1].temp;
  int upperBandT = kty81210_resistance_table[index].temp;
 
  int lowerBandR = kty81210_resistance_table[index-1].r2;
  int upperBandR = kty81210_resistance_table[index].r2;

  int bandwidthT = upperBandT - lowerBandT;
  int bandwidthR = upperBandR - lowerBandR;
  int rDelta = resistance - lowerBandR;
  float temp = lowerBandT + (bandwidthT/(float)bandwidthR)*rDelta;
 
  return temp;
}
