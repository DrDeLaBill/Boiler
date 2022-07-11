#include "RelayTemperature.h"

uint16_t RelayTemperature::ssr_temp_buf[SSR_TEMP_BUF_SIZE] = {};
uint32_t RelayTemperature::ssr_temp_summ = 0;
uint32_t RelayTemperature::ssr_broken_last_time = 0;

RelayTemperature::RelayTemperature() {
	pinMode(SSR_TEMP_PIN, INPUT);
	// загружаем "дефолтные" значения в массив
	for (uint8_t i = 0; i < SSR_TEMP_BUF_SIZE; i++){
    uint16_t analog_value = analogRead(SSR_TEMP_PIN);
    RelayTemperature::ssr_temp_buf[i] = analog_value;
    RelayTemperature::ssr_temp_summ += analog_value;
    delay(1);
	}
  Serial.println(F("Relay temperature service start"));
}


uint8_t RelayTemperature::check_ssr_temp(){
	// вычислим скользящее среднее по аналоговому значению напряжения.
	RelayTemperature::ssr_temp_summ -= RelayTemperature::ssr_temp_buf[0];

	uint16_t temp_analog = analogRead(SSR_TEMP_PIN);
	for (uint8_t i = 0; i < SSR_TEMP_BUF_SIZE-1; i++){
		RelayTemperature::ssr_temp_buf[i] = RelayTemperature::ssr_temp_buf[i+1];
	}

	RelayTemperature::ssr_temp_summ += temp_analog;
	RelayTemperature::ssr_temp_buf[SSR_TEMP_BUF_SIZE-1] = temp_analog;

	uint16_t current_ssr_temp = (uint16_t)(RelayTemperature::ssr_temp_summ / SSR_TEMP_BUF_SIZE);
//  Serial.println(current_ssr_temp);
  int8_t ssr_temp_celc = (SSR_AN_VAL_0 - current_ssr_temp) / SSR_TEMP_KOEF;

  // TODO: написать отключение нагревателя при превышении температуры ТТР
  if (ssr_temp_celc >= SSR_TEMP_UPPER_LIM && ErrorService::is_set_error(ERROR_SSRBROKEN)){
    Serial.println("SSR is dangerously hot.");
    RelayTemperature::ssr_broken_last_time = millis();
  } else if (ssr_temp_celc <= SSR_TEMP_LOWER_LIM && ErrorService::is_set_error(ERROR_SSRBROKEN)){
    ErrorService::remove_error(ERROR_SSRBROKEN);
  } else if (ssr_temp_celc >= SSR_TEMP_UPPER_LIM && ErrorService::is_set_error(ERROR_SSRBROKEN)){
    // если за некоторое время ТТР не остыли, то значит все плохо. Выключаем расцепитель. 
    if (millis() - RelayTemperature::ssr_broken_last_time >= SSR_BROKEN_TIMEOUT){
      Serial.println("Error! SSR overheated!");
      RelayTemperature::ssr_broken_last_time == millis();
      ErrorService::add_error(ERROR_SSRBROKEN);
      DisplayManager::set_page_name(pageError);
      digitalWrite(CRASH_OUT_PIN, HIGH);
    }
  }

  static uint32_t last_time_ssr_temp = 0;
  if (millis() - last_time_ssr_temp >= 5000){
    last_time_ssr_temp = millis();
//    Serial.print("ssr_temp: ");
//    Serial.print(ssr_temp_celc);
//    Serial.println("*C");
  }
}

bool RelayTemperature::is_heating_on() {
  return TemperatureSensor::is_heating_on;
}
