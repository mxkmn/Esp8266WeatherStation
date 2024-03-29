# Погодная станция на ESP8266 от mxkmn

### [English](README.md) | Русский

Погодная станция создана с нуля и использует дисплей LCD1602 для отображения следующей информации:
* Температура с двух датчиков DS18B20: за окном и в помещении
* Дата и время
* Текущая погода: температура настоящая и ощущаемая, скорость ветра, влажность, давление (над уровнем моря)
* Прогноз почасовой (до 48 часов): температура настоящая и ощущаемая, статус\*
* Прогноз дневной (до 8 часов): минимальная и максимальная температура за день, статус\*

\* Под статусом подразумевается подпись, в которой пишется об осадках, ситуации на небе или экстремальных погодных условиях (Дождь со снегом, Ясно, Пасмурно, Торнадо...)


Для навигации по вкладкам используется две кнопки. Имеющиеся вкладки:
* Основная вкладка: температура за окном/в городе\*\*, ощущаемая температура, дата и время (сменяется раз в 5 секунд), иконка осадков и получения данных\*\*\*\*\*
* Дополнительная вкладка: скорость ветра, влажность, давление (над уровнем моря), температура в помещении/в городе\*\*\*
* Дебаг-вкладка: температура в городе или инициалы создателя\*\*\*\*, версия прошивки, время с последней перезагрузки станции
* Вкладки с прогнозом на час: температура настоящая и ощущаемая, статус
* Вкладки с прогнозом на день: минимальная и максимальная температура за день, статус

\*\* Температура в городе пишется, если внешний датчик не подключён
\*\*\* Температура в городе пишется, если внутренний датчик не подключён и внешний датчик подключён (то есть если на главном экране пишется температура с внешнего датчика и для полной картины нужно написать температуру от OWM)
\*\*\*\* Температура в городе пишется, если оба датчика подключены и, соответственно, температура в городе больше нигде не пишется. Если она уже где-то выведена (один или оба датчика отключены), то пишем инициалы разработчика (ибо почему нет :) )
\*\*\*\*\* Иконка в углу экрана видна не всегда: показывается капля, если в скором времени будет дождь, снежинка если будет снег, часы во время синхронизации погоды/времени

Схема вкладок:
```
--- MainPage ⇄ AdditionalPage ?⇄? DebugPage
|   ↑
|   DayPage0 ⇄ DayPage1 ⇄ DayPage2 ⇄ ... ⇄ DayPage7
|   ↑
--> HourPage0 ⇄ HourPage1 ⇄ HourPage2 ⇄ ... ⇄ HourPage47
```

Фото вкладок:
<p float="left">
  <img src="/ReadmeFiles/Ru/MainPage.jpg" width="33%" />
  <img src="/ReadmeFiles/Ru/AdditionalPage.jpg" width="33%" /> 
  <img src="/ReadmeFiles/Ru/DebugPage.jpg" width="33%" />
</p>
<p float="left">
  <img src="/ReadmeFiles/Ru/DayPage0.jpg" width="33%" />
  <img src="/ReadmeFiles/Ru/DayPage1.jpg" width="33%" /> 
  <img src="/ReadmeFiles/Ru/DayPage2.jpg" width="33%" />
</p>
<p float="left">
  <img src="/ReadmeFiles/Ru/HourPage0.jpg" width="33%" />
  <img src="/ReadmeFiles/Ru/HourPage5.jpg" width="33%" /> 
  <img src="/ReadmeFiles/Ru/HourPage6.jpg" width="33%" />
</p>

***
Настройка станции происходит с помощью веб-интерфейса, в который можно войти при включении станции. Для этого необходимо подключиться к точке доступа и перейти по адресу 192.168.4.1:
![Настройки](/ReadmeFiles/Ru/Settings.png)
Остальные настройки (их немного, при этом их достаточно настроить лишь один раз) доступны в начале скетча.
***
Станция использует несколько сторонних библиотек. Их необходимо установить перед использованием:
* [OWM_for_ESP](https://github.com/mxkmn/OWM_for_ESP) - получение погоды от OWM
* [JSON_Decoder](https://github.com/Bodmer/JSON_Decoder) - необходима для OWM_for_ESP
* [TimeLib](https://github.com/PaulStoffregen/Time) - работа с временем
* [DallasTemperature](https://github.com/milesburton/Arduino-Temperature-Control-Library) - получение температуры с датчиков DS18B20
* [OneWire](https://github.com/PaulStoffregen/OneWire) - необходима для DallasTemperature
* [LiquidCrystalRus](https://github.com/mxkmn/LiquidCrystalRus) - библиотека для печати русских символов на ЖК-дисплеях

Не забудьте установить увеличенную скорость CPU 160MHz. Это важно для ускорения получения данных из интернета, при этом не увеличивает энергопотребление и не ухудшает стабильность чипа.
***
Схема подключения:
![Scheme](/ReadmeFiles/Scheme.png)
Транзистор BC337 используется для регулировки яркости дисплея. Можно использовать аналогичные транзисторы взамен этого.
Код рассчитан только на 2 датчика DS18B20, но при необходимости можно добавить большее количество - просто подключите их к пину D1.
Рекомендуется использовать сенсорные кнопки на чипе TTP223, однако при необходимости их можно заменить классическими кнопками (используйте в паре с pull-down резистором).

При отсутствии некоторых (или всех, не считая ESP8266) деталей Вы можете использовать урезанный функционал прошивки - код готов к работе с 0 или 1 датчиком, без кнопок (управление будет недоступно), без дисплея (всё логируется в Serial при включении соответствующей настройки) и без транзистора (яркость меняться не будет). Однако резистор обязателен при подключении датчиков DS18B20 - без него могут возникнуть проблемы.
***
Корпуса для 3D-принтера пока что нет, он в разработке.
***
Я попытался сделать код простым для изменения, поэтому я надеюсь, что Вы сможете переделать его под другие типы дисплеев/навигации/датчики/языки. Если ваша реализация полностью закончена, Вы можете отправить мне в Issues ссылку на неё и я добавлю её ниже.
* Пока что тут пусто
