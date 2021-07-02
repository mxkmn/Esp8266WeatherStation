const char settingsPage1[] PROGMEM = R"=====(<!DOCTYPE html>
<html lang='ru'>
  <head>
    <meta charset='utf-8'>
    <title>Настройки</title>
    <style>
      html {
        font-family: Tahoma;
      }
      body {
        margin: 0 auto;
        width: 90%;
        min-width: 200px;
      }
      @media (min-aspect-ratio: 3/4) {
        body {
          width: 50%;
          min-width: 67.5vh;
        }
      }
      h4 {
        margin-top: 40px;
      }
      dt {
        float: left;
        width: 200px;
        text-align: right;
        padding-right: 5px;
        min-height: 1px;
      }
      dd {
        position: relative;
        top: -1px;
        margin-bottom: 10px;
      }
      .formFields {
        text-align: center;
        display: flex;
        justify-content: space-around;
        flex-wrap: wrap;
      }
      .formField {
        margin: 10px 15px;
      }
    </style>

    <script>
      var )=====";

const char settingsPage2[] PROGMEM = R"=====(

      window.onload = function() {
        if (wifi === '') {
          document.getElementsByName('ssid')[0].required = true;
          document.getElementsByName('pass')[0].required = true;
          document.getElementsByName('key')[0].required = true;
          document.getElementsByName('lati')[0].required = true;
          document.getElementsByName('long')[0].required = true;
        }
        else {
          document.getElementsByName('ssid')[0].placeholder='Введено, можно изменить';
          document.getElementsByName('ssid')[0].value=wifi;
          document.getElementsByName('pass')[0].placeholder='Введено, можно изменить';
          document.getElementsByName('key')[0].placeholder='Введено, можно изменить';

          document.getElementsByName('lati')[0].placeholder='Введено, можно изменить';
          document.getElementsByName('lati')[0].value=lati;
          document.getElementsByName('long')[0].placeholder='Введено, можно изменить';
          document.getElementsByName('long')[0].value=long;
          
          document.querySelectorAll('select')[0].getElementsByTagName('option')[uni].selected = true;
          document.querySelectorAll('select')[1].getElementsByTagName('option')[deb].selected = true;
          document.querySelectorAll('select')[2].getElementsByTagName('option')[tem].selected = true;
          document.querySelectorAll('select')[3].getElementsByTagName('option')[one].selected = true;
          if (cou == 100) document.querySelectorAll('select')[4].getElementsByTagName('option')[0].selected = true;
          else document.querySelectorAll('select')[4].getElementsByTagName('option')[cou+1].selected = true;
          document.querySelectorAll('select')[5].getElementsByTagName('option')[mLog].selected = true;
          document.querySelectorAll('select')[6].getElementsByTagName('option')[wLog].selected = true;
          document.querySelectorAll('select')[7].getElementsByTagName('option')[set].selected = true;
        }
      }
    </script>
  </head>

  <body>
    <h1>Настройки погодной станции</h1>
    <form action='/senddata' method='post' onsubmit='alert("Сейчас произойдёт применение настроек и перезагрузка метеостанции...");' id="filters">
      <div class='formFields'>
        <div class='formField'><label>Имя сети WiFi</label><br>
        <input placeholder='Необходимо ввести' name='ssid'></div>

        <div class='formField'><label>Пароль WiFi</label><br>
        <input placeholder='Необходимо ввести' type='password' name='pass'></div>

        <div class='formField'><label>Ключ OpenWeatherMap</label><br>
        <input placeholder='Необходимо ввести' name='key' size='30px'></div>

        <div class='formField'><label>Широта</label><br>
        <input type="number" step="0.001" placeholder='Необходимо ввести' name='lati'></div>

        <div class='formField'><label>Высота</label><br>
        <input type="number" step="0.001" placeholder='Необходимо ввести' name='long'></div>

        <div class='formField'><label>Величины</label><br>
        <select name='units'>
          <option value='0'>METRIC</option>
          <option selected value='1'>RUS</option>
          <option value='2'>IMPERIAL</option>
        </select></div>

        <div class='formField'><label>Debug-вкладка</label><br>
        <select name='debug'>
          <option selected value='0'>Деактивирована</option>
          <option value='1'>Активирована</option>
        </select></div>

        <div class='formField'><label>Темп. датчики</label><br>
        <select name='temp'>
          <option selected value='0'>По умолчанию</option>
          <option value='1'>Перевернуть</option>
        </select></div>

        <div class='formField'><label>Если подключён один датчик</label><br>
        <select name='selSensor'>
          <option selected value='0'>Настроить как комнатный</option>
          <option value='1'>Настроить как внешний</option>
        </select></div>

        <div class='formField'><label>Проверка осадков</label><br>
        <select name='hours'>
          <option value='100'>Отключить</option>
          <option value='0'>На этот час</option>
          <option value='1'>На следующий час</option>
          <option value='2'>На следующие 2 часа</option>
          <option value='3'>На следующие 3 часа</option>
          <option value='4'>На следующие 4 часа</option>
          <option value='5'>На следующие 5 часов</option>
          <option value='6'>На следующие 6 часов</option>
          <option value='7'>На следующие 7 часов</option>
          <option selected value='8'>На следующие 8 часов</option>
          <option value='9'>На следующие 9 часов</option>
          <option value='10'>На следующие 10 часов</option>
        </select></div>

        <div class='formField'><label>Осн. логирование</label><br>
        <select name='mLogging'>
          <option selected value='0'>Отключить</option>
        <option value='1'>Включить</option>
        </select></div>

        <div class='formField'><label>Лог. статуса WiFi</label><br>
        <select name='wLogging'>
          <option selected value='0'>Отключить</option>
          <option value='1'>Включить</option>
        </select></div>

        <div class='formField'><label>Настр. при включ.</label><br>
        <select name='setup'>
          <option value='0'>Не предлагать</option>
          <option selected value='1'>Предлагать</option>
        </select></div>

        <div class='formField'><input type='submit' value='Сохранить'></div>
      </div>
    </form>
    <article>
      <h2>Объяснения:</h2>
      <p>Ключ OpenWeatherMap можно получить <a href='https://home.openweathermap.org/api_keys'>тут</a>. Не забудьте подключиться к настоящему интернету.</p>
      <p>Широта и высота - ваше местоположение. Необходимая точность - 2 цифры после точки, желательная - 3 или более цифры после точки. Эти координаты можно получить через <a href='https://www.google.ru/maps'>Google Maps</a>, кликнув на пустую землю рядом с домом.</p>
      <p>Величины - выбор среди RUS (гр. Цельсия, метры/сек. и мм. рт. ст.), METRIC (гр. Цельсия, метры/сек. и hPa) и IMPERIAL (гр. Фаренгейта, мили/час и hPa).</p>
      <p>Debug-вкладка - вкладка правее той, в которой указана влажность, скорость ветра и всё такое. В ней можно отслеживать температуру, полученную от OWM и время с запуска станции.</p>
      <p>Темп. датчики нужно разворачивать в том случае, если температура 'за окном' на самом деле 'в помещении' (и наоборот).</p>
      <p>Проверка осадков на следующие часы дает возможность показывать иконку дождя/погоды в краю главной вкладки.</p>
      <p>Основное логирование - вывод большинства полученной информации в Serial.</p>
      <p>Логирование статуса WiFI - вывод информации о изменениях статуса WiFi в Serial.</p>
      <p>Настройка при включении - предложение, выводящееся в течение трёх секунд после включения, перенастройки метеостанции.
        При отключении этой функции устройство можно будет перенастроить только в случае, если оно не поймает Wifi
        (для этого придётся отойти на большое расстояние от точки доступа, временно отключить её, ну или случайно ввести неправильный логин/пароль).</p>
    </article>
    <article>
      <h4>Версия прошивки: )=====";

const char settingsPage3[] PROGMEM = R"=====(</h4>
      <h4>Страница проекта: <a href='https://github.com/mxkmn/esp8266_weather_station'>github</a>. Разработал я, <a href='https://mxkmn.github.io'>mxkmn</a>.</h4>
    </article>
  </body>
</html>)=====";
