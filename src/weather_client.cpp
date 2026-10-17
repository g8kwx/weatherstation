#include "weather_client.h"
#include "time_utils.h"
#include <ArduinoJson.h>
#include "config/config.h"
#include "settings_manager.h"
#include <time.h>
#include <math.h>
#include <stdlib.h>

#ifndef METOFFICE_API_KEY
#define METOFFICE_API_KEY ""
#endif

extern SettingsManager settings;

#ifndef NATIVE_TEST
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#endif

namespace {

const char* const METOFFICE_ROOT_CA =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n"
    "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"
    "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n"
    "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n"
    "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n"
    "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n"
    "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n"
    "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n"
    "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n"
    "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n"
    "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n"
    "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n"
    "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n"
    "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n"
    "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n"
    "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n"
    "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n"
    "rqXRfboQnoZsG4q5WTP468SQvvG5\n"
    "-----END CERTIFICATE-----\n";

const char* const THINGSPEAK_ROOT_CA =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh\n"
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
    "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n"
    "MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT\n"
    "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n"
    "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG\n"
    "9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI\n"
    "2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx\n"
    "1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ\n"
    "q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz\n"
    "tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ\n"
    "vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP\n"
    "BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV\n"
    "5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY\n"
    "1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4\n"
    "NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG\n"
    "Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91\n"
    "8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe\n"
    "pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl\n"
    "MrY=\n"
    "-----END CERTIFICATE-----\n";

bool waitForSystemTime() {
    for (int attempt = 0; attempt < 10 && time(nullptr) < 1577836800; attempt++) {
        delay(500);
    }
    return time(nullptr) >= 1577836800;
}

time_t parseMetOfficeTime(const char* value) {
    if (!value || strlen(value) < 16 ||
        value[4] != '-' || value[7] != '-' ||
        (value[10] != 'T' && value[10] != ' ' ) ||
        value[13] != ':') {
        return 0;
    }

    char utcTime[20];
    snprintf(utcTime, sizeof(utcTime), "%.10s %.5s:00", value, value + 11);
    return parseUtcDtTxt(utcTime);
}

float metOfficeTemperature(float celsius) {
    return settings.getUnitSystem() == UNIT_IMPERIAL
        ? celsius * 1.8f + 32.0f
        : celsius;
}

float metOfficeWindSpeed(float metresPerSecond) {
    return metresPerSecond * (settings.getUnitSystem() == UNIT_IMPERIAL
        ? 2.2369363f
        : 3.6f);
}

bool parseMetOfficeHumidity(JsonVariantConst value, int& humidity) {
    if (value.isNull()) {
        return false;
    }

    float parsedValue;
    if (value.is<const char*>()) {
        const char* text = value.as<const char*>();
        char* end = nullptr;
        parsedValue = strtof(text, &end);
        if (end == text || *end != '\0') {
            return false;
        }
    } else if (value.is<float>() || value.is<int>()) {
        parsedValue = value.as<float>();
    } else {
        return false;
    }

    if (!isfinite(parsedValue) || parsedValue < 0.0f || parsedValue > 100.0f) {
        return false;
    }
    humidity = static_cast<int>(parsedValue + 0.5f);
    return true;
}

String shortDayName(const char* date) {
    int year = (date[0] - '0') * 1000 + (date[1] - '0') * 100 +
               (date[2] - '0') * 10 + (date[3] - '0');
    int month = (date[5] - '0') * 10 + (date[6] - '0');
    int day = (date[8] - '0') * 10 + (date[9] - '0');
    static const char* days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    static const int offsets[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    if (month < 1 || month > 12) {
        return date;
    }
    if (month < 3) year--;
    int index = (year + year / 4 - year / 100 + year / 400 +
                 offsets[month - 1] + day) % 7;
    return days[index];
}

#ifndef NATIVE_TEST
bool requestMetOfficeForecast(const char* frequency, const char* latitude,
                              const char* longitude, String& payload) {
    if (strlen(METOFFICE_API_KEY) == 0) {
        Serial.println("[Weather] Met Office API key is not configured.");
        return false;
    }
    if (!waitForSystemTime()) {
        Serial.println("[Weather] System clock is not synchronized for HTTPS validation.");
        return false;
    }

    String url = "https://data.hub.api.metoffice.gov.uk/sitespecific/v0/point/";
    url += frequency;
    url += "?latitude=";
    url += latitude;
    url += "&longitude=";
    url += longitude;
    url += "&excludeParameterMetadata=true&includeLocationName=true";

    WiFiClientSecure client;
    client.setCACert(METOFFICE_ROOT_CA);
    HTTPClient http;
    http.setTimeout(20000);
    if (!http.begin(client, url)) {
        Serial.printf("[Weather] Unable to connect to Met Office %s forecast.\n", frequency);
        return false;
    }

    http.addHeader("Accept", "application/json");
    http.addHeader("apikey", METOFFICE_API_KEY);
    int httpCode = http.GET();
    bool success = false;
    if (httpCode == HTTP_CODE_OK) {
        payload = http.getString();
        success = true;
    } else {
        Serial.printf("[Weather] Met Office %s forecast failed: HTTP %d\n", frequency, httpCode);
    }
    http.end();
    return success;
}
#endif

} // namespace

WeatherClient::WeatherClient()
    : _useZip(false), _zipResolved(false), _cityName("") {}

bool WeatherClient::parseThingSpeakReadingsJson(const char* json, float& temperatureC,
                                                bool& temperatureValid, float& voltageV,
                                                bool& voltageValid) {
    temperatureValid = false;
    voltageValid = false;

    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        Serial.print("[ThingSpeak] JSON parse failed: ");
        Serial.println(error.c_str());
        return false;
    }

    const char* fieldNames[] = {"field1", "field3"};
    float* values[] = {&temperatureC, &voltageV};
    bool* validity[] = {&temperatureValid, &voltageValid};
    for (size_t i = 0; i < 2; i++) {
        const char* field = doc[fieldNames[i]];
        if (field == nullptr || field[0] == '\0') {
            Serial.printf("[ThingSpeak] Response is missing %s.\n", fieldNames[i]);
            continue;
        }

        char* end = nullptr;
        float value = strtof(field, &end);
        if (end == field || *end != '\0' || !isfinite(value)) {
            Serial.printf("[ThingSpeak] %s is not a valid reading.\n", fieldNames[i]);
            continue;
        }
        *values[i] = value;
        *validity[i] = true;
    }
    return temperatureValid || voltageValid;
}

bool WeatherClient::fetchThingSpeakReadings(float& temperatureC, bool& temperatureValid,
                                            float& voltageV, bool& voltageValid) {
#ifdef NATIVE_TEST
    return parseThingSpeakReadingsJson(
        "{\"field1\":\"22.30\",\"field3\":\"12.45\"}", temperatureC,
        temperatureValid, voltageV, voltageValid);
#else
    temperatureValid = false;
    voltageValid = false;
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[ThingSpeak] WiFi not connected; cannot fetch channel readings.");
        return false;
    }
    if (!waitForSystemTime()) {
        Serial.println("[ThingSpeak] System clock is not synchronized for HTTPS validation.");
        return false;
    }

    WiFiClientSecure client;
    client.setCACert(THINGSPEAK_ROOT_CA);
    HTTPClient http;
    http.setTimeout(15000);
    const char* url = "https://api.thingspeak.com/channels/1997679/feeds/last.json";
    if (!http.begin(client, url)) {
        Serial.println("[ThingSpeak] Unable to connect to public channel.");
        return false;
    }

    int httpCode = http.GET();
    bool success = false;
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        success = parseThingSpeakReadingsJson(payload.c_str(), temperatureC,
                                              temperatureValid, voltageV, voltageValid);
    } else {
        Serial.printf("[ThingSpeak] Channel request failed: HTTP %d\n", httpCode);
    }
    http.end();
    return success;
#endif
}

bool WeatherClient::geocodeZip() {
#ifdef NATIVE_TEST
    _resolvedLat = "34.0736";
    _resolvedLng = "-118.4004";
    _zipResolved = true;
    return true;
#else
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[Weather] WiFi not connected. Cannot geocode zip code.");
        return false;
    }

    WiFiClient client;
    HTTPClient http;

    String url = "http://geocoding-api.open-meteo.com/v1/search?name=";
    url += _zipCode;
    url += "&count=1&language=en&format=json";

    Serial.print("[Weather] Geocoding Zip Code: ");
    Serial.println(_zipCode);

    if (http.begin(client, url)) {
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            StaticJsonDocument<1024> doc;
            DeserializationError error = deserializeJson(doc, payload);

            if (!error && doc["results"].is<JsonArray>() && doc["results"].size() > 0) {
                float lat = doc["results"][0]["latitude"] | 0.0f;
                float lng = doc["results"][0]["longitude"] | 0.0f;
                const char* name = doc["results"][0]["name"] | "Unknown";

                _resolvedLat = String(lat, 5);
                _resolvedLng = String(lng, 5);
                _cityName    = String(name);
                _zipResolved = true;

                Serial.printf("[Weather] Zip %s resolved to %s (%s, %s)\n",
                    _zipCode.c_str(), name, _resolvedLat.c_str(), _resolvedLng.c_str());
                http.end();
                return true;
            } else {
                Serial.print("[Weather] Geocoding JSON parsing failed or empty: ");
                Serial.println(error.c_str());
            }
        } else {
            Serial.printf("[Weather] Geocoding HTTP GET failed: %d\n", httpCode);
        }
        http.end();
    }
    return false;
#endif
}

WeatherData WeatherClient::fetchWeather() {

    _zipCode = settings.getZipCode();
    _latitude = settings.getLatitude();
    _longitude = settings.getLongitude();
    _useZip = (_zipCode.length() > 0 && _zipCode != "YOUR_ZIP_CODE");
    WeatherData data = { 0.0f, 0, "Unknown", false, -1, 0.0f, 0, "", {} };

    // Resolve zip code if using zip code and not yet resolved
    if (_useZip && !_zipResolved) {
        if (!geocodeZip()) {
            Serial.println("[Weather] Failed to resolve zip code. Cannot fetch weather.");
            return data;
        }
    }

    const char* lat = _useZip ? _resolvedLat.c_str() : ((_latitude.length() > 0) ? _latitude.c_str() : _resolvedLat.c_str());
    const char* lng = _useZip ? _resolvedLng.c_str() : ((_longitude.length() > 0) ? _longitude.c_str() : _resolvedLng.c_str());

#ifdef NATIVE_TEST
    // Mock response for native unit tests
    const char* mockJson = "{"
        "\"current\":{"
            "\"temperature_2m\":21.5,"
            "\"relative_humidity_2m\":60,"
            "\"weather_code\":0,"
            "\"wind_speed_10m\":8.8"
        "},"
        "\"daily\":{"
            "\"time\":[\"2026-07-09\",\"2026-07-10\",\"2026-07-11\"],"
            "\"weather_code\":[0,1,2],"
            "\"temperature_2m_max\":[25.5,26.0,24.8],"
            "\"temperature_2m_min\":[15.0,14.8,15.2]"
        "}"
    "}";
    _cityName = "Mock City";
    parseWeatherJson(mockJson, data);
    data.cityName = _cityName;
    return data;
#else
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[Weather] WiFi not connected. Cannot fetch weather.");
        return data;
    }

    String hourlyPayload;
    if (!requestMetOfficeForecast("hourly", lat, lng, hourlyPayload) ||
        !parseMetOfficeHourlyJson(hourlyPayload.c_str(), data)) {
        data.valid = false;
        return data;
    }

    String dailyPayload;
    if (!requestMetOfficeForecast("daily", lat, lng, dailyPayload) ||
        !parseMetOfficeDailyJson(dailyPayload.c_str(), data)) {
        data.valid = false;
        return data;
    }

    _cityName = data.cityName;
    Serial.printf("[Weather] Met Office success. Temp: %.1f, Hum: %d %%, Status: %s, Wind: %.1f, City: %s\n",
        data.temperature, data.humidity, data.status.c_str(), data.windSpeed, data.cityName.c_str());
    return data;
#endif
}

bool WeatherClient::parseWeatherJson(const char* json, WeatherData& data, time_t ref_time) {
    DynamicJsonDocument doc(8192);
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        Serial.print("[Weather] JSON Deserialization failed: ");
        Serial.println(error.c_str());
        return false;
    }

    // Parse current weather
    if (doc.containsKey("current")) {
        data.temperature = doc["current"]["temperature_2m"].as<float>();
        data.humidity = doc["current"]["relative_humidity_2m"].as<int>();
        int code = doc["current"]["weather_code"].is<int>() ? doc["current"]["weather_code"].as<int>() : -1;
        data.status = getWeatherDesc(code);
        data.windSpeed = doc["current"]["wind_speed_10m"].as<float>();
        data.windDirection = doc["current"]["wind_direction_10m"].is<int>() ? doc["current"]["wind_direction_10m"].as<int>() : 0;
        data.valid = true;
        data.weatherCode = code;
    }

    // Parse hourly forecast
    if (doc.containsKey("hourly")) {
        JsonArray hourly_temp = doc["hourly"]["temperature_2m"];
        JsonArray hourly_precip = doc["hourly"]["precipitation_probability"];

        int start_idx = 0;
        if (doc.containsKey("hourly") && doc["hourly"].containsKey("time")) {
            const char* current_time = nullptr;
            if (doc.containsKey("current") && doc["current"].containsKey("time")) {
                current_time = doc["current"]["time"];
            }
#ifndef NATIVE_TEST
            char now_buf[14] = "";
            if (current_time == nullptr) {
                struct tm tm_now;
                if (getLocalTimeWrapper(&tm_now)) {
                    snprintf(now_buf, sizeof(now_buf), "%04d-%02d-%02dT%02d",
                             tm_now.tm_year + 1900, tm_now.tm_mon + 1, tm_now.tm_mday, tm_now.tm_hour);
                    current_time = now_buf;
                }
            }
#endif
            if (current_time != nullptr) {
                JsonArray hourly_time = doc["hourly"]["time"];
                for (size_t k = 0; k < hourly_time.size(); k++) {
                    const char* ht = hourly_time[k];
                    if (ht != nullptr) {
                        if (strncmp(ht, current_time, 13) == 0 || strcmp(ht, current_time) == 0) {
                            start_idx = (int)k;
                            break;
                        }
                    }
                }
            }
        }

        for (int i = 0; i < 24; i++) {
            int src_idx = start_idx + i;
            if (src_idx < (int)hourly_temp.size()) {
                data.hourly[i].temperature = hourly_temp[src_idx].as<float>();
            } else {
                data.hourly[i].temperature = 0.0f;
            }
            if (src_idx < (int)hourly_precip.size()) {
                data.hourly[i].precipitationProbability = hourly_precip[src_idx].as<int>();
            } else {
                data.hourly[i].precipitationProbability = 0;
            }
        }
    }

    // Parse daily forecast
    if (doc.containsKey("daily")) {
        JsonArray daily_time = doc["daily"]["time"];
        JsonArray daily_max = doc["daily"]["temperature_2m_max"];
        JsonArray daily_min = doc["daily"]["temperature_2m_min"];
        JsonArray daily_code = doc["daily"]["weather_code"];

        // Compute today's and tomorrow's date in the user's configured timezone
        // so that "Today" / "Tomorrow" labels are correct regardless of UTC rollover.
        // Falls back to index-based labelling if NTP hasn't synced yet.
        char today_str[11]    = "";
        char tomorrow_str[11] = "";
        const bool time_valid = getLocalDateStrings(today_str, sizeof(today_str),
                                                    tomorrow_str, sizeof(tomorrow_str),
                                                    ref_time);

        for (int i = 0; i < 3; i++) {
            if (i < (int)daily_time.size()) {
                data.forecast[i].tempMax = daily_max[i].as<float>();
                data.forecast[i].tempMin = daily_min[i].as<float>();
                int code = daily_code[i].as<int>();
                data.forecast[i].weatherCode = code;
                data.forecast[i].status = getWeatherDesc(code);

                String dateStr = daily_time[i].as<String>();

                // Determine the human-readable label for this date.
                // Forecast slots are fixed: Slot 0 = Today, Slot 1 = Tomorrow, Slot 2 = Day After Tomorrow
                if (i == 0) {
                    data.forecast[i].dayName = "Today";
                } else if (i == 1) {
                    data.forecast[i].dayName = "Tmrw";
                } else if (dateStr.length() >= 10) {
                    data.forecast[i].dayName = shortDayName(dateStr.c_str());
                } else {
                    data.forecast[i].dayName = dateStr;
                }
            } else {
                data.forecast[i].tempMax = 0.0f;
                data.forecast[i].tempMin = 0.0f;
                data.forecast[i].weatherCode = -1;
                data.forecast[i].status = "Unknown";
                data.forecast[i].dayName = "N/A";
            }
        }
    }

    return data.valid;
}

String WeatherClient::getWeatherDesc(int code) {
    switch (code) {
        case 0: return "Clear sky";
        case 1: return "Mainly clear";
        case 2: return "Partly cloudy";
        case 3: return "Overcast";
        case 45: return "Foggy";
        case 48: return "Depositing rime fog";
        case 51: return "Light drizzle";
        case 53: return "Moderate drizzle";
        case 55: return "Dense drizzle";
        case 56: return "Light freezing drizzle";
        case 57: return "Dense freezing drizzle";
        case 61: return "Slight rain";
        case 63: return "Moderate rain";
        case 65: return "Heavy rain";
        case 66: return "Light freezing rain";
        case 67: return "Heavy freezing rain";
        case 71: return "Slight snow fall";
        case 73: return "Moderate snow fall";
        case 75: return "Heavy snow fall";
        case 77: return "Snow grains";
        case 80: return "Slight rain showers";
        case 81: return "Moderate rain showers";
        case 82: return "Violent rain showers";
        case 85: return "Slight snow showers";
        case 86: return "Heavy snow showers";
        case 95: return "Thunderstorm";
        case 96: return "Thunderstorm with slight hail";
        case 99: return "Thunderstorm with heavy hail";
        default: return "Unknown";
    }
}

int WeatherClient::metOfficeToWmoCode(int code) {
    switch (code) {
        case 0:
        case 1: return 0;
        case 2:
        case 3: return 2;
        case 5:
        case 6: return 45;
        case 7:
        case 8: return 3;
        case 9:
        case 10: return 80;
        case 11: return 51;
        case 12: return 61;
        case 13:
        case 14: return 82;
        case 15: return 65;
        case 16:
        case 17: return 85;
        case 18: return 67;
        case 19:
        case 20:
        case 21: return 96;
        case 22:
        case 23: return 85;
        case 24: return 71;
        case 25:
        case 26: return 86;
        case 27: return 75;
        case 28:
        case 29:
        case 30: return 95;
        default: return -1;
    }
}

bool WeatherClient::parseMetOfficeHourlyJson(const char* json, WeatherData& data, time_t ref_time) {
    StaticJsonDocument<1024> filter;
    filter["features"][0]["properties"]["location"]["name"] = true;
    filter["features"][0]["properties"]["timeSeries"][0]["time"] = true;
    filter["features"][0]["properties"]["timeSeries"][0]["screenTemperature"] = true;
    filter["features"][0]["properties"]["timeSeries"][0]["screenRelativeHumidity"] = true;
    filter["features"][0]["properties"]["timeSeries"][0]["windSpeed10m"] = true;
    filter["features"][0]["properties"]["timeSeries"][0]["windDirectionFrom10m"] = true;
    filter["features"][0]["properties"]["timeSeries"][0]["significantWeatherCode"] = true;
    filter["features"][0]["properties"]["timeSeries"][0]["probOfPrecipitation"] = true;

    if (filter.overflowed()) {
        Serial.println("[Weather] Met Office hourly JSON filter overflow.");
        return false;
    }

    DynamicJsonDocument doc(16384);
    DeserializationError error = deserializeJson(doc, json, DeserializationOption::Filter(filter));
    if (error) {
        Serial.print("[Weather] Met Office hourly JSON error: ");
        Serial.println(error.c_str());
        return false;
    }

    JsonArray timeSeries = doc["features"][0]["properties"]["timeSeries"].as<JsonArray>();
    if (timeSeries.isNull() || timeSeries.size() == 0) {
        Serial.println("[Weather] Met Office hourly response has no time series.");
        return false;
    }

    JsonObject properties = doc["features"][0]["properties"];
    if (properties["location"]["name"].is<const char*>()) {
        data.cityName = properties["location"]["name"].as<String>();
    }

    time_t now = (ref_time == 0) ? time(nullptr) : ref_time;
    int currentIndex = 0;
    time_t latestPastTime = 0;
    for (size_t i = 0; i < timeSeries.size(); i++) {
        time_t forecastTime = parseMetOfficeTime(timeSeries[i]["time"] | "");
        if (forecastTime > latestPastTime && forecastTime <= now) {
            latestPastTime = forecastTime;
            currentIndex = static_cast<int>(i);
        }
    }

    JsonObject current = timeSeries[currentIndex];
    if (!current.containsKey("screenTemperature") ||
        !current.containsKey("significantWeatherCode")) {
        Serial.println("[Weather] Met Office hourly response is missing current conditions.");
        return false;
    }

    data.temperature = metOfficeTemperature(current["screenTemperature"].as<float>());
    if (!parseMetOfficeHumidity(current["screenRelativeHumidity"], data.humidity)) {
        bool humidityFound = false;
        for (size_t distance = 1; distance < timeSeries.size() && !humidityFound; distance++) {
            if (currentIndex >= static_cast<int>(distance)) {
                humidityFound = parseMetOfficeHumidity(
                    timeSeries[currentIndex - distance]["screenRelativeHumidity"],
                    data.humidity);
            }
            size_t futureIndex = static_cast<size_t>(currentIndex) + distance;
            if (!humidityFound && futureIndex < timeSeries.size()) {
                humidityFound = parseMetOfficeHumidity(
                    timeSeries[futureIndex]["screenRelativeHumidity"], data.humidity);
            }
        }
        if (!humidityFound) {
            data.humidity = 0;
            Serial.println("[Weather] Met Office hourly response has no valid humidity values.");
        } else {
            Serial.println("[Weather] Current Met Office hour has no humidity; using nearest hourly value.");
        }
    }
    data.weatherCode = metOfficeToWmoCode(current["significantWeatherCode"].as<int>());
    data.status = getWeatherDesc(data.weatherCode);
    data.windSpeed = metOfficeWindSpeed(current["windSpeed10m"] | 0.0f);
    data.windDirection = current["windDirectionFrom10m"] | 0;
    data.valid = true;

    for (int i = 0; i < 24; i++) {
        size_t sourceIndex = static_cast<size_t>(currentIndex + i);
        if (sourceIndex >= timeSeries.size()) {
            data.hourly[i] = {0.0f, 0};
            continue;
        }
        JsonObject hour = timeSeries[sourceIndex];
        data.hourly[i].temperature = metOfficeTemperature(hour["screenTemperature"] | 0.0f);
        data.hourly[i].precipitationProbability = hour["probOfPrecipitation"] | 0;
    }
    return true;
}

bool WeatherClient::parseMetOfficeDailyJson(const char* json, WeatherData& data, time_t ref_time) {
    StaticJsonDocument<512> filter;
    filter["features"][0]["properties"]["timeSeries"][0]["time"] = true;
    filter["features"][0]["properties"]["timeSeries"][0]["dayMaxScreenTemperature"] = true;
    filter["features"][0]["properties"]["timeSeries"][0]["nightMinScreenTemperature"] = true;
    filter["features"][0]["properties"]["timeSeries"][0]["daySignificantWeatherCode"] = true;

    if (filter.overflowed()) {
        Serial.println("[Weather] Met Office daily JSON filter overflow.");
        return false;
    }

    DynamicJsonDocument doc(8192);
    DeserializationError error = deserializeJson(doc, json, DeserializationOption::Filter(filter));
    if (error) {
        Serial.print("[Weather] Met Office daily JSON error: ");
        Serial.println(error.c_str());
        return false;
    }

    JsonArray timeSeries = doc["features"][0]["properties"]["timeSeries"].as<JsonArray>();
    if (timeSeries.isNull() || timeSeries.size() == 0) {
        Serial.println("[Weather] Met Office daily response has no time series.");
        return false;
    }

    char today[11] = "";
    char tomorrow[11] = "";
    bool haveLocalDates = getLocalDateStrings(today, sizeof(today), tomorrow, sizeof(tomorrow), ref_time);
    bool populated[3] = {false, false, false};
    for (size_t i = 0; i < timeSeries.size(); i++) {
        JsonObject day = timeSeries[i];
        if (!day.containsKey("dayMaxScreenTemperature") ||
            !day.containsKey("nightMinScreenTemperature") ||
            !day.containsKey("daySignificantWeatherCode")) {
            continue;
        }

        char date[11] = "";
        int slot = -1;
        time_t forecastTime = parseMetOfficeTime(day["time"] | "");
        if (haveLocalDates && getLocalDateStringFromTimestamp(forecastTime, date, sizeof(date))) {
            if (strcmp(date, today) == 0) {
                slot = 0;
            } else if (strcmp(date, tomorrow) == 0) {
                slot = 1;
            } else if (strcmp(date, tomorrow) > 0 && !populated[2]) {
                slot = 2;
            }
        } else if (i < 3) {
            slot = static_cast<int>(i);
        }
        if (slot < 0 || slot > 2 || populated[slot]) {
            continue;
        }

        int code = metOfficeToWmoCode(day["daySignificantWeatherCode"].as<int>());
        ForecastDay& forecast = data.forecast[slot];
        forecast.tempMax = metOfficeTemperature(day["dayMaxScreenTemperature"].as<float>());
        forecast.tempMin = metOfficeTemperature(day["nightMinScreenTemperature"].as<float>());
        forecast.weatherCode = code;
        forecast.status = getWeatherDesc(code);
        if (slot == 0) {
            forecast.dayName = "Today";
        } else if (slot == 1) {
            forecast.dayName = "Tmrw";
        } else if (date[0] != '\0') {
            forecast.dayName = shortDayName(date);
        }
        populated[slot] = true;
    }

    if (!populated[0] || !populated[1] || !populated[2]) {
        Serial.println("[Weather] Met Office daily response does not cover the next three local days.");
        return false;
    }
    return true;
}

bool WeatherClient::parseOwmJson(const char* json, WeatherData& data, time_t ref_time) {
    StaticJsonDocument<1024> filter;
    filter["list"][0]["dt"] = true;
    filter["list"][0]["dt_txt"] = true;
    filter["list"][0]["main"]["temp"] = true;
    filter["list"][0]["main"]["humidity"] = true;
    filter["list"][0]["main"]["temp_min"] = true;
    filter["list"][0]["main"]["temp_max"] = true;
    filter["list"][0]["wind"]["speed"] = true;
    filter["list"][0]["wind"]["deg"] = true;
    filter["list"][0]["weather"][0]["id"] = true;
    filter["list"][0]["weather"][0]["description"] = true;
    filter["list"][0]["pop"] = true;
    filter["city"]["name"] = true;
    filter["city"]["timezone"] = true;

    DynamicJsonDocument doc(12288);
    DeserializationError error = deserializeJson(doc, json, DeserializationOption::Filter(filter));

    if (error) {
        Serial.print("[Weather] OWM JSON Deserialization failed: ");
        Serial.println(error.c_str());
        return false;
    }

    if (!doc.containsKey("list") || doc["list"].size() == 0) {
        Serial.println("[Weather] Invalid OWM JSON payload");
        return false;
    }

    JsonArray list = doc["list"];

    // 1. Parse current weather from list[0]
    JsonObject current = list[0];
    data.temperature = current["main"]["temp"].as<float>();
    data.humidity = current["main"]["humidity"].as<int>();
    data.windSpeed = current["wind"]["speed"].as<float>();
    data.windDirection = current["wind"]["deg"].is<int>() ? current["wind"]["deg"].as<int>() : 0;
    // Convert metric wind speed from m/s to km/h (if metric)
    if (settings.getUnitSystem() == UNIT_METRIC) {
        data.windSpeed *= 3.6f;
    }
    
    int owmCode = current["weather"][0]["id"].as<int>();
    data.weatherCode = owmToWmoCode(owmCode);
    data.status = current["weather"][0]["description"].as<String>();
    // Capitalize first letter of status description for better aesthetics
    if (data.status.length() > 0) {
        data.status[0] = toupper(data.status[0]);
    }
    data.valid = true;

    // Resolve city name from OWM city object
    if (doc.containsKey("city")) {
        const char* name = doc["city"]["name"] | "";
        if (name && name[0] != '\0') {
            _cityName = String(name);
        }
    }
    data.cityName = _cityName;

    // 2. Parse 3-day forecast (Today, Tomorrow, Day after)
    // OpenWeatherMap returns city.timezone (UTC offset in seconds).
    long city_tz_offset = 0;
    bool has_city_tz = false;
    if (doc.containsKey("city") && doc["city"].containsKey("timezone")) {
        city_tz_offset = doc["city"]["timezone"].as<long>();
        has_city_tz = true;
    }

    // Determine reference time in UTC
    time_t now_utc = ref_time;
    if (now_utc == 0) {
        now_utc = time(nullptr);
    }
    // If NTP is not yet synced on ESP32 or test mock data differs significantly from system clock:
    if (list.size() > 0) {
        time_t first_entry_ts = 0;
        if (list[0].containsKey("dt")) {
            first_entry_ts = list[0]["dt"].as<time_t>();
        } else if (list[0].containsKey("dt_txt")) {
            first_entry_ts = parseUtcDtTxt(list[0]["dt_txt"].as<const char*>());
        }
        if (first_entry_ts > 0 && (now_utc < 946684800L || labs(now_utc - first_entry_ts) > 7 * 86400L)) {
            now_utc = first_entry_ts;
        }
    }

    // Fallback to configured device timezone if city.timezone was omitted
    if (!has_city_tz) {
        city_tz_offset = getTimezoneOffsetSeconds(now_utc);
    }

    // Compute local calendar dates for Today, Tomorrow, and Day 2 in city timezone
    time_t city_now = now_utc + city_tz_offset;
    struct tm tm_today;
    gmtime_r(&city_now, &tm_today);
    char today_str[11];
    snprintf(today_str, sizeof(today_str), "%04d-%02d-%02d",
             tm_today.tm_year + 1900, tm_today.tm_mon + 1, tm_today.tm_mday);

    time_t city_tomorrow = city_now + 86400;
    struct tm tm_tomorrow;
    gmtime_r(&city_tomorrow, &tm_tomorrow);
    char tomorrow_str[11];
    snprintf(tomorrow_str, sizeof(tomorrow_str), "%04d-%02d-%02d",
             tm_tomorrow.tm_year + 1900, tm_tomorrow.tm_mon + 1, tm_tomorrow.tm_mday);

    time_t city_day2 = city_now + 172800;
    struct tm tm_day2;
    gmtime_r(&city_day2, &tm_day2);
    char day2_str[11];
    snprintf(day2_str, sizeof(day2_str), "%04d-%02d-%02d",
             tm_day2.tm_year + 1900, tm_day2.tm_mon + 1, tm_day2.tm_mday);

    static const char* days[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
    const char* day2_name = (tm_day2.tm_wday >= 0 && tm_day2.tm_wday < 7) ? days[tm_day2.tm_wday] : "";

    struct TargetDayForecast {
        float tempMin = 999.0f;
        float tempMax = -999.0f;
        int weatherCode = -1;
        String status = "";
        int bestMiddayDist = 999;
        bool hasData = false;
    };

    TargetDayForecast targetDays[3];

    for (JsonObject entry : list) {
        time_t utc_ts = 0;
        if (entry.containsKey("dt")) {
            utc_ts = entry["dt"].as<time_t>();
        } else if (entry.containsKey("dt_txt")) {
            utc_ts = parseUtcDtTxt(entry["dt_txt"].as<const char*>());
        }
        if (utc_ts == 0) continue;

        time_t entry_city_ts = utc_ts + city_tz_offset;
        struct tm tm_entry;
        gmtime_r(&entry_city_ts, &tm_entry);
        char dateBuf[11];
        snprintf(dateBuf, sizeof(dateBuf), "%04d-%02d-%02d",
                 tm_entry.tm_year + 1900, tm_entry.tm_mon + 1, tm_entry.tm_mday);

        int targetSlot = -1;
        if (strcmp(dateBuf, today_str) == 0) targetSlot = 0;
        else if (strcmp(dateBuf, tomorrow_str) == 0) targetSlot = 1;
        else if (strcmp(dateBuf, day2_str) == 0) targetSlot = 2;

        if (targetSlot >= 0) {
            targetDays[targetSlot].hasData = true;
            float tMin = entry["main"].containsKey("temp_min") ? entry["main"]["temp_min"].as<float>() : entry["main"]["temp"].as<float>();
            float tMax = entry["main"].containsKey("temp_max") ? entry["main"]["temp_max"].as<float>() : entry["main"]["temp"].as<float>();
            if (tMin < targetDays[targetSlot].tempMin) targetDays[targetSlot].tempMin = tMin;
            if (tMax > targetDays[targetSlot].tempMax) targetDays[targetSlot].tempMax = tMax;

            int local_hour = tm_entry.tm_hour;
            int dist = abs(local_hour - 12);
            if (dist < targetDays[targetSlot].bestMiddayDist || targetDays[targetSlot].weatherCode == -1) {
                targetDays[targetSlot].bestMiddayDist = dist;
                int code = entry["weather"][0]["id"].as<int>();
                targetDays[targetSlot].weatherCode = owmToWmoCode(code);
                targetDays[targetSlot].status = entry["weather"][0]["description"].as<String>();
                if (targetDays[targetSlot].status.length() > 0) {
                    targetDays[targetSlot].status[0] = toupper(targetDays[targetSlot].status[0]);
                }
            }
        }
    }

    // If today's remaining 3-hourly intervals have passed (e.g. late evening), use current conditions
    if (!targetDays[0].hasData) {
        targetDays[0].tempMin = data.temperature;
        targetDays[0].tempMax = data.temperature;
        targetDays[0].weatherCode = data.weatherCode;
        targetDays[0].status = data.status;
        targetDays[0].hasData = true;
    }

    // Populate the 3 forecast slots: Slot 0 = Today, Slot 1 = Tomorrow, Slot 2 = Day 2
    data.forecast[0].tempMax = targetDays[0].tempMax;
    data.forecast[0].tempMin = targetDays[0].tempMin;
    data.forecast[0].weatherCode = targetDays[0].weatherCode;
    data.forecast[0].status = targetDays[0].status;
    data.forecast[0].dayName = "Today";

    if (targetDays[1].hasData) {
        data.forecast[1].tempMax = targetDays[1].tempMax;
        data.forecast[1].tempMin = targetDays[1].tempMin;
        data.forecast[1].weatherCode = targetDays[1].weatherCode;
        data.forecast[1].status = targetDays[1].status;
    } else {
        data.forecast[1].tempMax = targetDays[0].tempMax;
        data.forecast[1].tempMin = targetDays[0].tempMin;
        data.forecast[1].weatherCode = targetDays[0].weatherCode;
        data.forecast[1].status = targetDays[0].status;
    }
    data.forecast[1].dayName = "Tmrw";

    if (targetDays[2].hasData) {
        data.forecast[2].tempMax = targetDays[2].tempMax;
        data.forecast[2].tempMin = targetDays[2].tempMin;
        data.forecast[2].weatherCode = targetDays[2].weatherCode;
        data.forecast[2].status = targetDays[2].status;
    } else {
        data.forecast[2].tempMax = data.forecast[1].tempMax;
        data.forecast[2].tempMin = data.forecast[1].tempMin;
        data.forecast[2].weatherCode = data.forecast[1].weatherCode;
        data.forecast[2].status = data.forecast[1].status;
    }
    data.forecast[2].dayName = day2_name;

    // 3. Parse hourly forecast (24 hours at 3-hour intervals)
    // OWM list array contains 3-hourly entries. We map them to 24 slots.
    for (int i = 0; i < 8; i++) {
        float tempVal = 0.0f;
        int precipProb = 0;
        if (i < (int)list.size()) {
            tempVal = list[i]["main"]["temp"].as<float>();
            precipProb = (int)(list[i]["pop"].as<float>() * 100.0f + 0.5f);
        }
        for (int h = 0; h < 3; h++) {
            int targetIdx = i * 3 + h;
            data.hourly[targetIdx].temperature = tempVal;
            data.hourly[targetIdx].precipitationProbability = precipProb;
        }
    }

    return true;
}

int WeatherClient::owmToWmoCode(int owmCode) {
    if (owmCode >= 200 && owmCode < 300) return 95; // Thunderstorm
    if (owmCode >= 300 && owmCode < 400) return 51; // Drizzle
    if (owmCode >= 500 && owmCode < 600) {
        if (owmCode == 511) return 66; // Freezing rain
        return 61; // Rain
    }
    if (owmCode >= 600 && owmCode < 700) return 71; // Snow
    if (owmCode >= 700 && owmCode < 800) return 45; // Fog
    if (owmCode == 800) return 0; // Clear
    if (owmCode == 801) return 1; // Mainly clear
    if (owmCode == 802) return 2; // Partly cloudy
    if (owmCode >= 803 && owmCode < 900) return 3; // Overcast
    return -1;
}

String WeatherClient::serializeWeatherData(const WeatherData& data) {
    StaticJsonDocument<4096> doc;
    doc["temp"] = data.temperature;
    doc["hum"] = data.humidity;
    doc["status"] = data.status;
    doc["valid"] = data.valid;
    doc["code"] = data.weatherCode;
    doc["wind_spd"] = data.windSpeed;
    doc["wind_dir"] = data.windDirection;
    doc["city"] = data.cityName;

    JsonArray forecast = doc.createNestedArray("forecast");
    for (int i = 0; i < 3; i++) {
        JsonObject fDay = forecast.createNestedObject();
        fDay["temp_max"] = data.forecast[i].tempMax;
        fDay["temp_min"] = data.forecast[i].tempMin;
        fDay["code"] = data.forecast[i].weatherCode;
        fDay["status"] = data.forecast[i].status;
        fDay["day"] = data.forecast[i].dayName;
    }

    JsonArray hourly = doc.createNestedArray("hourly");
    for (int i = 0; i < 24; i++) {
        JsonObject hHour = hourly.createNestedObject();
        hHour["temp"] = data.hourly[i].temperature;
        hHour["precip"] = data.hourly[i].precipitationProbability;
    }

    String output;
    serializeJson(doc, output);
    return output;
}

bool WeatherClient::deserializeWeatherData(const String& json, WeatherData& data) {
    if (json.length() == 0) return false;
    StaticJsonDocument<4096> doc;
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        return false;
    }

    data.temperature = doc["temp"] | 0.0f;
    data.humidity = doc["hum"] | 0;
    data.status = doc["status"].as<String>();
    data.valid = doc["valid"] | false;
    data.weatherCode = doc["code"] | 0;
    data.windSpeed = doc["wind_spd"] | 0.0f;
    data.windDirection = doc["wind_dir"] | 0;
    data.cityName = doc["city"].as<String>();

    JsonArray forecast = doc["forecast"];
    if (!forecast.isNull()) {
        for (int i = 0; i < 3 && i < (int)forecast.size(); i++) {
            JsonObject fDay = forecast[i];
            data.forecast[i].tempMax = fDay["temp_max"] | 0.0f;
            data.forecast[i].tempMin = fDay["temp_min"] | 0.0f;
            data.forecast[i].weatherCode = fDay["code"] | 0;
            data.forecast[i].status = fDay["status"].as<String>();
            data.forecast[i].dayName = fDay["day"].as<String>();
        }
    }

    JsonArray hourly = doc["hourly"];
    if (!hourly.isNull()) {
        for (int i = 0; i < 24 && i < (int)hourly.size(); i++) {
            JsonObject hHour = hourly[i];
            data.hourly[i].temperature = hHour["temp"] | 0.0f;
            data.hourly[i].precipitationProbability = hHour["precip"] | 0;
        }
    }
    return true;
}

bool WeatherClient::isLocationEmpty() {
    _zipCode = settings.getZipCode();
    _latitude = settings.getLatitude();
    _longitude = settings.getLongitude();
    _useZip = (_zipCode.length() > 0 && _zipCode != "YOUR_ZIP_CODE");

    if (_resolvedLat.length() > 0 && _resolvedLng.length() > 0) {
        return false;
    }
    if (_useZip) {
        return _zipCode.length() == 0 || _zipCode == "YOUR_ZIP_CODE";
    } else {
        return _latitude.length() == 0 || _longitude.length() == 0 || _latitude == "YOUR_LATITUDE";
    }
}

bool WeatherClient::fetchIpLocation(String& latStr, String& lonStr, String& city) {
#ifdef NATIVE_TEST
    latStr = "37.7749";
    lonStr = "-122.4194";
    city = "San Francisco";
    _resolvedLat = latStr;
    _resolvedLng = lonStr;
    _cityName = city;
    _useZip = false;
    _zipResolved = true;
    return true;
#else
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[Weather] WiFi not connected. Cannot fetch IP location.");
        return false;
    }

    WiFiClient client;
    HTTPClient http;

    String url = "http://ip-api.com/json/";
    Serial.print("[Weather] Fetching IP Geolocation URL: ");
    Serial.println(url);

    if (http.begin(client, url)) {
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            float lat = 0.0f;
            float lon = 0.0f;
            if (parseIpLocationJson(payload.c_str(), lat, lon, city)) {
                latStr = String(lat, 5);
                lonStr = String(lon, 5);
                _resolvedLat = latStr;
                _resolvedLng = lonStr;
                _cityName = city;
                _useZip = false;
                _zipResolved = true;
                http.end();
                return true;
            }
        } else {
            Serial.printf("[Weather] Geolocation HTTP GET failed: %d\n", httpCode);
        }
        http.end();
    }
    return false;
#endif
}

bool WeatherClient::parseIpLocationJson(const char* json, float& lat, float& lon, String& city) {
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        Serial.print("[Weather] Geolocation JSON Deserialization failed: ");
        Serial.println(error.c_str());
        return false;
    }

    const char* status = doc["status"] | "fail";
    if (strcmp(status, "success") != 0) {
        Serial.print("[Weather] Geolocation failed status: ");
        Serial.println(status);
        return false;
    }

    lat = doc["lat"] | 0.0f;
    lon = doc["lon"] | 0.0f;
    const char* city_c = doc["city"] | "Unknown";
    city = String(city_c);

    return true;
}
