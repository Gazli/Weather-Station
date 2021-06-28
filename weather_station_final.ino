#include <arduino-timer.h>
#include <Arduino_MKRIoTCarrier.h>
#include "arduino_secrets.h"
#include <ArduinoHttpClient.h>
#include <Arduino_JSON.h>
#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

MKRIoTCarrier carrier;

WiFiConnectionHandler ArduinoIoTPreferredConnection(SECRET_SSID, SECRET_PASS);

float pressure, humidity, temperature, temperatureRaw;
const char THING_ID[] = "5746c121-b1a4-4f9d-91cb-b388dfc2eb9b";
char serverAddress[] = "www.7timer.info";
String response;
int tempMin, tempMax, tempMin2, tempMax2;
int port = 80;
String weather, weather2, weatherType, weatherType2;

int windSpeed, windSpeed2;

WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);

auto timer = timer_create_default();

//LOCAL MEASUREMENT

void initProperties() {

    ArduinoCloud.setThingId(THING_ID);
    ArduinoCloud.addProperty(temperature, READ, ON_CHANGE, NULL);
    ArduinoCloud.addProperty(humidity, READ, ON_CHANGE, NULL);
    ArduinoCloud.addProperty(pressure, READ, ON_CHANGE, NULL);
}

bool weatherCheck(void *) {
    Serial.println("weather check...");
    pressure = carrier.Pressure.readPressure();
    temperatureRaw = carrier.Env.readTemperature();
    humidity = carrier.Env.readHumidity();
    if (humidity > 100.0) {
        humidity = 100.0;
    }
    Serial.println("weather updated");
}



//INTERNET SEARCH
//complete link: www.7timer.info/bin/civillight.php?lon=8.403&lat=49.006&ac=0&unit=metric&output=json&tzshift=0

bool internetSearch(void *) {
    client.get("/bin/civillight.php?lon=8.403&lat=49.006&ac=0&unit=metric&output=json&tzshift=0");

    int statusCode = client.responseStatusCode();
    String response = client.responseBody();
    JSONVar responseParsed = JSON.parse(response);
    tempMin = responseParsed["dataseries"][0]["temp2m"]["min"];
    tempMax = responseParsed["dataseries"][0]["temp2m"]["max"];
    weather = responseParsed["dataseries"][0]["weather"];
    windSpeed = responseParsed["dataseries"][0]["wind10m_max"];
    tempMin2 = responseParsed["dataseries"][1]["temp2m"]["min"];
    tempMax2 = responseParsed["dataseries"][1]["temp2m"]["max"];
    weather2 = responseParsed["dataseries"][1]["weather"];
    windSpeed2 = responseParsed["dataseries"][1]["wind10m_max"];

    if (weather == "clear") weatherType = "Clear";
    else if (weather == "pcloudy") weatherType = "Part.Cloudy";
    else if (weather == "mcloudy") weatherType = "Most.Cloudy";
    else if (weather == "cloudy") weatherType = "Cloudy";
    else if (weather == "humid") weatherType = "Humid";
    else if (weather == "lightrain") weatherType = "Light Rain";
    else if (weather == "oshower") weatherType = "Showers";
    else if (weather == "ishower") weatherType = "Few showers";
    else if (weather == "rain") weatherType = "Rain";
    else if (weather == "lightsnow") weatherType = "Light Snow";
    else if (weather == "snow") weatherType = "Snow";
    else if (weather == "rainsnow") weatherType = "Rain/Snow";
    else if (weather == "ts") weatherType = "Thunderst.";
    else weatherType = weather;

    if (weather2 == "clear") weatherType2 = "Clear";
    else if (weather2 == "pcloudy") weatherType2 = "Part.Cloudy";
    else if (weather2 == "mcloudy") weatherType2 = "Most.Cloudy";
    else if (weather2 == "cloudy") weatherType2 = "Cloudy";
    else if (weather2 == "humid") weatherType2 = "Humid";
    else if (weather2 == "lightrain") weatherType2 = "Light Rain";
    else if (weather2 == "oshower") weatherType2 = "Showers";
    else if (weather2 == "ishower") weatherType2 = "Few showers";
    else if (weather2 == "rain") weatherType2 = "Rain";
    else if (weather2 == "lightsnow") weatherType2 = "Light Snow";
    else if (weather2 == "snow") weatherType2 = "Snow";
    else if (weather2 == "rainsnow") weatherType2 = "Rain/Snow";
    else if (weather2 == "ts") weatherType2 = "Thunderst.";
    else weatherType2 = weather2;
    return true;
}

void setup() {

    Serial.begin(9600);
    Serial.println("Setup started");
    initProperties();

    ArduinoCloud.begin(ArduinoIoTPreferredConnection);

    setDebugMessageLevel(2);
    ArduinoCloud.printDebugInfo();

    while (ArduinoCloud.connected() != 1) {
        ArduinoCloud.update();
        delay(500);
    }
    delay(500);

    CARRIER_CASE = true;
    carrier.begin();

    carrier.Buttons.updateConfig(3);
    carrier.display.setRotation(0);
    carrier.display.setTextSize(3);
    delay(1500);

    wifi = (WiFiClient) ArduinoIoTPreferredConnection.getClient();
    client = HttpClient(wifi, serverAddress, port);

    weatherCheck(&timer);
    internetSearch(&timer);

    timer.every(600000, internetSearch);
    timer.every(60000, weatherCheck);

    Serial.println("Setup done");
}

void loop() {
    ArduinoCloud.update();
    carrier.Buttons.update();

    temperature = temperatureRaw - 4;

    //LOCAL
    if (carrier.Buttons.onTouchUp(TOUCH0)) {

        carrier.display.fillScreen(ST77XX_BLACK);
        carrier.display.setTextColor(ST77XX_GREEN);

        carrier.display.enableSleep(false);

        carrier.display.setCursor(28, 50);
        carrier.display.print("LOCAL");

        carrier.display.setCursor(28, 75);
        carrier.display.print(temperature);

        carrier.display.setCursor(118, 75);
        carrier.display.print("C");

        carrier.display.setCursor(28, 125);
        carrier.display.print(pressure * 10.0F);

        if (pressure < 100.00) {
            Serial.println("pressure < 1000");
            carrier.display.setCursor(136, 125);
        } else {
            carrier.display.setCursor(154, 125);
        }
        carrier.display.print("hPA");

        carrier.display.setCursor(28, 100);
        carrier.display.print(humidity);

        carrier.display.setCursor(118, 100);
        carrier.display.print("%");
    }

    //TODAY
    if (carrier.Buttons.onTouchUp(TOUCH1)) {

        carrier.display.fillScreen(ST77XX_BLACK);
        carrier.display.setTextColor(ST77XX_RED);

        carrier.display.enableSleep(false);

        carrier.display.setCursor(28, 50);
        carrier.display.print("TODAY");

        carrier.display.setCursor(28, 75);
        carrier.display.print(tempMin);

        carrier.display.setCursor(64, 75);
        carrier.display.print("-");

        carrier.display.setCursor(82, 75);
        carrier.display.print(tempMax);

        carrier.display.setCursor(118, 75);
        carrier.display.print("C");

        carrier.display.setCursor(28, 100);
        carrier.display.print("Wind:");

        carrier.display.setCursor(118, 100);
        carrier.display.print(windSpeed);

        carrier.display.setCursor(28, 125);
        carrier.display.print(weatherType);

        if (windSpeed > 9) {
            carrier.display.setCursor(154, 100);
        } else {
            carrier.display.setCursor(136, 100);
        }
        carrier.display.print("m/s");
    }

    //TOMORROW
    if (carrier.Buttons.onTouchUp(TOUCH2)) {

        carrier.display.fillScreen(ST77XX_BLACK);
        carrier.display.setTextColor(ST77XX_CYAN);

        carrier.display.enableSleep(false);

        carrier.display.setCursor(28, 50);
        carrier.display.print("TOMORROW");

        carrier.display.setCursor(28, 75);
        carrier.display.print(tempMin2);

        carrier.display.setCursor(64, 75);
        carrier.display.print("-");

        carrier.display.setCursor(82, 75);
        carrier.display.print(tempMax2);

        carrier.display.setCursor(118, 75);
        carrier.display.print("C");

        carrier.display.setCursor(28, 100);
        carrier.display.print("Wind:");

        carrier.display.setCursor(118, 100);
        carrier.display.print(windSpeed2);

        carrier.display.setCursor(28, 125);
        carrier.display.print(weatherType2);

        if (windSpeed2 > 9) {
            carrier.display.setCursor(154, 100);
        } else {
            carrier.display.setCursor(136, 100);
        }
        carrier.display.print("m/s");
    }

    if (carrier.Buttons.onTouchUp(TOUCH4)) {
        carrier.display.fillScreen(ST77XX_BLACK);
        carrier.display.enableSleep(true);
    }

    delay(20);

    timer.tick();
} 
