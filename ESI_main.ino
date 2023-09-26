void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:
  curr_temp = get_temp();
  is_day = get_light();
  buzz_LED(co2_ppm);

  LCD_out(curr_temp);
}

float get_temp() {

}

bool get_light() {

}

void buzz_LED(float co2_ppm) {
  // Linear map of CO2 concentration in the air
  // LED green blinking for good air, then to orange for mid air, red for poor air
  // Buzzer will only buzz if air is mid or worse, buzz faster and higher pitch for poorer air
}