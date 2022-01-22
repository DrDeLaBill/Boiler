#include "DisplayManager.h"

//TODO: extern
//extern BoilerConfig BoilerCfg; //######################################################################
//
//extern uint8_t current_temp;            // текущая температура для отображения и ПИД
//extern uint32_t t_newPage;
//extern uint8_t user_boiler_mode;
//extern uint8_t user_error;
//extern bool connected_to_server;
//extern uint8_t radio_connected;

DisplayManager::DisplayManager() {
  this->u8g2 = new U8G2_PCD8544_84X48_F_4W_HW_SPI(U8G2_R0, /* cs=*/ 25, /* dc=*/ 26, /* reset=*/ U8X8_PIN_NONE);
  DisplayManager::brightness = 255;
  this->menu_item = 0;              
  this->t_page_save_settings = 0;        
  this->t_newPage = 0;
  this->display_init();
}

void DisplayManager::display_init() {
  this->u8g2->begin();
  this->u8g2->setContrast(70);
  this->u8g2->enableUTF8Print();
  this->u8g2->setFontDirection(0);
  DisplayManager::page_name = pageTemp;
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  this->t_newPage = millis();                         //TODO: откуда?
}

void DisplayManager::display_on() {
  // включаем дисплей и подсветку. Переходим на основную страницу.
  DisplayManager::page_name = pageTemp;
  this->menu_item = 0;
  this->u8g2->setPowerSave(false);
  digitalWrite(LED_PIN, HIGH);
  this->t_newPage = millis();                         //TODO: откуда?
}

void DisplayManager::display_off() {
  // выключаем дисплей и подсветку
  this->u8g2->setPowerSave(true);
  digitalWrite(LED_PIN, LOW);
}

void DisplayManager::display_lightning() {
  // плавно выключаем подсветку при отсутствии активности
  if (this->t_newPage != 0) {                         
    //TODO: вся функция из extern
    
    // если недавно была активность то ждем время до выключения
    if (millis() - t_newPage >= TIMEOUT_LIGHTNING) {                         //TODO: откуда?
      ledcWrite(0, brightness--);
      if (this->brightness == 0) {
        ledcWrite(0, 0);
        t_newPage = 0;
      }
    } else {
      // если время не подходит, то обновляем данные.
      this->brightness = 255;
      ledcWrite(0, 255);
    }
  }
}

void DisplayManager::check_page() {
  // проверяем время неактивности. Если в течении 5с не было активности возвращаемся на начальный экран.
  // время экрана "сохранено" - 1с

  if (DisplayManager::page_name != pageTemp && DisplayManager::page_name != pageError) {

    if (DisplayManager::page_name == this->t_page_save_settings || DisplayManager::page_name == this->t_page_save_settings) {
      if (millis() - this->t_page_save_settings >= SAVE_TIMEOUT) {
        DisplayManager::page_name = pageTemp; //TODO: повторяющийся код
        this->menu_item = 0;
        return;
      }
    } else if (millis() - this->t_newPage >= CANCEL_TIMEOUT) { //TODO: t_newPage
      DisplayManager::page_name = pageTemp;
      this->menu_item = 0;
    }
  }
}

void DisplayManager::paint() {
  this->check_page(); //TODO: check_page()
  //display_lightning();

  this->u8g2->clearBuffer();

  switch (DisplayManager::page_name) { //TODO: остановился тут
    case pageTemp:
      // основной экран. Отображение текущей и уставной температуры.

      // если котел подключен к WiFi, то показываем значок вайфай
      if (this->display_data_config.is_wifi_connect){
        this->u8g2->drawXBM(0, 0, 12, 9, wifi);
      }

      //TODO: старый код в комментах
      // если у нас сейчас "работает" нагрев, то показываем это.
      if (this->display_data_config.is_heating_on){ //digitalRead(SSR1_OUT_PIN) вот он)
        this->u8g2->drawXBM(14, 0, 9, 9, heat);
      }

      // если есть связь с сервером - то показываем это
      if (this->display_data_config.is_connected_to_server){
        this->u8g2->drawXBM(26, 0, 8, 9, inet);
      }

      
      // рисование текущего датчика температуры
      if (this->display_data_config.is_external_sensor){ //user_boiler_mode == MODE_AIR || user_boiler_mode == MODE_PROFILE
        // если выбран внешний датчик температуры или термопрофиль
        this->u8g2->drawXBM(65, 20, 16, 16, pict_air);
      } else if (this->display_data_config.is_internal_sensor){ //user_boiler_mode == MODE_WATER
        // иначе если выбран датчик температуры теплоносителя
        this->u8g2->drawXBM(65, 20, 16, 16, pict_water);
      }

      // если есть радиосвязь с датчиком, то рисуем термометр
      if (this->display_data_config.is_radio_connected){ //radio_connected == RADIO_ON
        this->u8g2->drawXBM(50, 20, 9, 16, pict_therm); //TODO: pict_therm
      }

      // текущая температура
      //u8g2->setFont(u8g2_font_inb30_mr);
      this->u8g2->setFont(u8g2_font_inb30_mn);
      this->u8g2->setCursor(0, 40);
      this->u8g2->print(this->display_data_config.current_temperature);

      // установленная температура
      this->u8g2->setFont(u8g2_font_luBS12_tr);
      this->u8g2->setCursor(43, 12);
      this->u8g2->print(this->display_data_config.target_temperature); // get_target_temp()
      this->u8g2->setFont(u8g_font_5x8);
      this->u8g2->setCursor(67, 5);
      this->u8g2->print("o");
      this->u8g2->setFont(u8g2_font_ncenR12_tf);
      this->u8g2->setCursor(71, 12);
      this->u8g2->print("C");

      // отрисовка текущей даты и времени
      this->u8g2->setFont(u8g_font_5x8);
      this->u8g2->setCursor(0, 48);
      this->u8g2->print(this->display_data_config.current_day); //clock_get_time("d/m/Y")

      this->u8g2->setFont(u8g2_font_t0_12b_mr);
      this->u8g2->drawStr(54, 48, this->display_data_config.current_time); // clock_get_time("H:i")

      break;

    case pageTempSet:
      // страница настройки установленной температуры

      this->u8g2->setFont(u8g2_font_5x8_t_cyrillic);
      this->u8g2->setCursor(0, 6);
      this->u8g2->print("Требуемая");
      this->u8g2->setCursor(0, 13);
      this->u8g2->print("температура:");

      this->u8g2->setCursor(54, 30);
      this->u8g2->print("o");

      this->u8g2->setFont(u8g2_font_luBS12_tr);
      this->u8g2->setCursor(58, 38);
      this->u8g2->print("C");

      this->u8g2->setFont(u8g2_font_luBS12_tr);
      this->u8g2->setCursor(29, 38);
      this->u8g2->print(this->temporary_target_temp);
      break;

    case pageSaveSettings:
      // страница сохранения новой установленной температуры

      this->u8g2->setFont(u8g2_font_5x8_t_cyrillic);
      this->u8g2->setCursor(18, 22);
      this->u8g2->print("Сохранено!");  //Страницы на экране
      break;

    case pageResetSettings:
      // страница сохранения новой установленной температуры

      this->u8g2->setFont(u8g2_font_5x8_t_cyrillic);
      this->u8g2->setCursor(10, 8);
      this->u8g2->print("Настройки");
      this->u8g2->setCursor(10, 20);
      this->u8g2->print("возвращены к");
      this->u8g2->setCursor(10, 32);
      this->u8g2->print("заводским");
      this->u8g2->setCursor(10, 44);
      this->u8g2->print("установкам !");

      break;

    case pageSettings:
      // страница настроек котла.
      
      this->u8g2->setFont(u8g2_font_5x8_t_cyrillic);
      this->u8g2->setCursor(5, 8);
      this->u8g2->print("Режим работы");
      this->u8g2->setCursor(5, 20);
      this->u8g2->print("Сброс настроек");
      

      if (this->menu_item == 0) {
        // без выбора
      } else if (this->menu_item == 1) {
        this->u8g2->drawRFrame(0,  1, 78, 10, 4);
      } else if (this->menu_item == 2) {
        this->u8g2->drawRFrame(0, 12, 78, 10, 4);
      }
      break;

    case pageSetMode:
      // страница выбора режима работы по уставке теплоносителя, воздуха или по термопрофилю

      this->u8g2->setFont(u8g2_font_5x8_t_cyrillic);
      this->u8g2->setCursor(5, 8);
      this->u8g2->print("термопрофиль");
      this->u8g2->setCursor(5, 20);
      this->u8g2->print("темп. вода");
      this->u8g2->setCursor(5, 32);
      this->u8g2->print("темп. воздух");

      if (this->menu_item == 1) {
        this->u8g2->drawRFrame(0, 1, 78, 10, 4);
      } else if (this->menu_item == 2) {
        this->u8g2->drawRFrame(0, 12, 78, 10, 4);
      } else if (this->menu_item == 3){
        this->u8g2->drawRFrame(0, 23, 78, 10, 4);
      }
      break;

    case pageError:
      // страница отображения критических ошибок

      this->u8g2->setFont(u8g2_font_6x12_t_cyrillic);
      this->u8g2->setCursor(18, 12);
      this->u8g2->print("АВАРИЯ!");

      //TODO: ещё комменты
      if (this->display_data_config.is_overheat) { // user_error == OVERHEAT || user_error == WATEROVERHEAT
        // перегрев
        this->u8g2->setCursor(10, 32);
        this->u8g2->print("Перегрев!");
      } else if (this->display_data_config.is_pumpbroken) { // user_error == PUMPBROKEN
        // датчик протока сигнализирует об отсутствии потока
        this->u8g2->setCursor(10, 32);
        this->u8g2->print("Нет");
        this->u8g2->setCursor(10, 44);
        this->u8g2->print("протока!");
      } else if (this->display_data_config.is_ssrbroken) { // user_error == SSRBROKEN
        // твердотельные реле неисправны
        this->u8g2->setCursor(10, 32);
        this->u8g2->print("Перегрев");
        this->u8g2->setCursor(10, 44);
        this->u8g2->print("ТТ реле!");
      } else if (this->display_data_config.is_tempsensbroken) { // user_error == TEMPSENSBROKEN
        // датчик температуры теплоносителя неисправен
        this->u8g2->setCursor(10, 32);
        this->u8g2->print("Датчик темп.");
        this->u8g2->setCursor(10, 44);
        this->u8g2->print("неисправен!");
      } else if (this->display_data_config.is_nopower){ // user_error == NOPOWER
        // нет напряжения после ТТР
        this->u8g2->setCursor(10, 32);
        this->u8g2->print("Нет");
        this->u8g2->setCursor(10, 44);
        this->u8g2->print("нагрева!");
      }
      break;

    default:
      break;
  }

  this->u8g2->sendBuffer();
}

void DisplayManager::rotary_encoder_action(uint8_t rotary_state, uint8_t session_boiler_mode) {
  if (rotary_state == BUTTON_ROTARY_RIGHT) {
    this->_rotary_right(session_boiler_mode);
  } else if (rotary_state == BUTTON_ROTARY_LEFT) {
    this->_rotary_left(session_boiler_mode);
  }
}

void DisplayManager::_rotary_right(uint8_t session_boiler_mode) {
  // обработаем вращение энкодера вправо
  this->t_newPage = millis();

  switch (DisplayManager::page_name) {
    case pageTemp:
      // страница с настройками
      DisplayManager::page_name = pageSettings;
      break;

    case pageTempSet:
      // страница настройки устанавливаемой температуры

      // TODO: user_boiler_mode
      if (session_boiler_mode == MODE_WATER){
        if (this->temporary_target_temp < WATER_TEMP_MAX) {
          this->temporary_target_temp++;
        }
      } else {
        if (this->temporary_target_temp < AIR_TEMP_MAX) {
          this->temporary_target_temp++;
        }
      }
      break;

    case pageSettings:
      // страница настроек

      if (this->menu_item != 0 && this->menu_item < 3) {
        this->menu_item++;
      }
      break;

    case pageSetMode:
      // страница выбора режима работы

      if (this->menu_item < 3) {
        this->menu_item++;
      }
      break;

    default:
      break;
  }
}

void DisplayManager::_rotary_left(uint8_t session_boiler_mode) {
  // обработаем вращение энкодера влево
  this->t_newPage = millis();

  switch (DisplayManager::page_name) {
    case pageTempSet:
      // страница настройки устанавливаемой температуры

      // TODO: user_boiler_mode
      if (session_boiler_mode == MODE_WATER){
        if (this->temporary_target_temp > WATER_TEMP_MIN) {
          this->temporary_target_temp--;
        }
      } else {
        if (this->temporary_target_temp > AIR_TEMP_MIN) {
          this->temporary_target_temp--;
        }
      }
      break;

    case pageSettings:
      // страница настроек

      if (this->menu_item == 0) {
        DisplayManager::page_name = pageTemp;
      } else {
        if (this->menu_item > 1) {
          this->menu_item--;
        }
      }
      break;

    case pageSetMode:
      // страница выбора режима работы

      if (this->menu_item > 1) {
        this->menu_item--;
      }
      break;

    default:
      break;
  }
}


// Какая красота
void DisplayManager::set_display_data_config(DisplayDataConfig display_data_config) {
  this->display_data_config.is_wifi_connect = display_data_config.is_wifi_connect;
  this->display_data_config.is_heating_on = display_data_config.is_heating_on;
  this->display_data_config.is_connected_to_server = display_data_config.is_connected_to_server;
  this->display_data_config.is_external_sensor = display_data_config.is_external_sensor;
  this->display_data_config.is_internal_sensor = display_data_config.is_internal_sensor;
  this->display_data_config.is_radio_connected = display_data_config.is_radio_connected;
  this->display_data_config.is_overheat = display_data_config.is_overheat;
  this->display_data_config.is_pumpbroken = display_data_config.is_pumpbroken;
  this->display_data_config.is_ssrbroken = display_data_config.is_ssrbroken;
  this->display_data_config.is_tempsensbroken = display_data_config.is_tempsensbroken;
  this->display_data_config.is_nopower = display_data_config.is_nopower;
  this->display_data_config.current_temperature = display_data_config.current_temperature;
  this->display_data_config.target_temperature = display_data_config.target_temperature;
  strncpy(this->display_data_config.current_day, display_data_config.current_day, sizeof(display_data_config.current_day));
  strncpy(this->display_data_config.current_time, display_data_config.current_time, sizeof(display_data_config.current_time));
}

void DisplayManager::set_t_newPage(int value) {
  this->t_newPage = value;
}

DisplayPages DisplayManager::get_page_name() {
  return DisplayManager::page_name;
}

// TODO: проверка на существование
void DisplayManager::set_page_name(DisplayPages page_name) {
  DisplayManager::page_name = page_name;
}

uint8_t DisplayManager::get_menu_item() {
  return this->menu_item;
}

// TODO: проверка на существование
void DisplayManager::set_menu_item(uint8_t menu_item) {
  this->menu_item = menu_item;
}

void DisplayManager::set_temporary_target_temp(uint8_t temporary_target_temp) {
  this->temporary_target_temp = temporary_target_temp;
}

void DisplayManager::set_t_page_save_settings(int value) {
  this->t_page_save_settings = value;
}

uint8_t DisplayManager::get_temporary_target_temp() {
  return this->temporary_target_temp;
}
