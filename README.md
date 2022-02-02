# Boiler
 

---

###Источники:

-Использование html, css, js файлов внутри ESP32, создание файловой системы https://wikihandbk.com/wiki/ESP32:Примеры/Создание_веб-сервера_на_базе_ESP32_при_помощи_файлов_из_файловой_системы_(SPIFFS)
-Установка загрузчика файлов данных в ESP32 через Arduino IDE https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/
-Библиотека U8g2 модифицирована, необходимо закинуть её в libreries

##Пометки
-network в BoilerProfile и Server (current_ssid)
-display и pump в ErorService
-BoilerProfile в CommandManager.h
-set_ext_sensor в BoilerProfile
-хранение ошибок в ОП
-объяснение, как ользоваться командами
-описание классов и докстринги для них:
	-CommandsManager
-везде, где есть add_error добавить Serial.print об обноруженных ошибках
	
	
6
9
1.5
2
2
4
2
3
3
3
5
1
2