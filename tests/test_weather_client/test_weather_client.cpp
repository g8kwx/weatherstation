#include <unity.h>
#include <string>
#include "weather_client.h"
#include "../mocks/mocks.cpp"
#include "../../src/time_utils.cpp"
#include "../../src/weather_client.cpp"

void setUp(void) {
    // set up before tests
}

void tearDown(void) {
    // clean up after tests
}

void test_weather_client_initialization(void) {
    settings.setZipCode(""); settings.setLatitude("37.7749"); settings.setLongitude("-122.4194"); WeatherClient client;
    // Assert it builds and sets up without crashing
    TEST_ASSERT_TRUE(true);
}

void test_weather_client_get_weather_desc(void) {
    TEST_ASSERT_EQUAL_STRING("Clear sky", WeatherClient::getWeatherDesc(0).c_str());
    TEST_ASSERT_EQUAL_STRING("Mainly clear", WeatherClient::getWeatherDesc(1).c_str());
    TEST_ASSERT_EQUAL_STRING("Partly cloudy", WeatherClient::getWeatherDesc(2).c_str());
    TEST_ASSERT_EQUAL_STRING("Overcast", WeatherClient::getWeatherDesc(3).c_str());
    TEST_ASSERT_EQUAL_STRING("Foggy", WeatherClient::getWeatherDesc(45).c_str());
    TEST_ASSERT_EQUAL_STRING("Unknown", WeatherClient::getWeatherDesc(999).c_str());
}

void test_weather_client_fetch_weather_native_mock(void) {
    settings.setZipCode(""); settings.setLatitude("37.7749"); settings.setLongitude("-122.4194"); WeatherClient client;
    WeatherData data = client.fetchWeather();
    
    TEST_ASSERT_TRUE(data.valid);
    TEST_ASSERT_EQUAL_FLOAT(21.5f, data.temperature);
    TEST_ASSERT_EQUAL(60, data.humidity);
    TEST_ASSERT_EQUAL_STRING("Clear sky", data.status.c_str());
    TEST_ASSERT_EQUAL(0, data.weatherCode);
    TEST_ASSERT_EQUAL_FLOAT(8.8f, data.windSpeed);

    // Verify 3-day forecast data from the mock payload
    TEST_ASSERT_EQUAL_FLOAT(25.5f, data.forecast[0].tempMax);
    TEST_ASSERT_EQUAL_FLOAT(15.0f, data.forecast[0].tempMin);
    TEST_ASSERT_EQUAL_STRING("Today", data.forecast[0].dayName.c_str());
    TEST_ASSERT_EQUAL_STRING("Clear sky", data.forecast[0].status.c_str());

    TEST_ASSERT_EQUAL_FLOAT(26.0f, data.forecast[1].tempMax);
    TEST_ASSERT_EQUAL_FLOAT(14.8f, data.forecast[1].tempMin);
    TEST_ASSERT_EQUAL_STRING("Tmrw", data.forecast[1].dayName.c_str());
    TEST_ASSERT_EQUAL_STRING("Mainly clear", data.forecast[1].status.c_str());

    TEST_ASSERT_EQUAL_FLOAT(24.8f, data.forecast[2].tempMax);
    TEST_ASSERT_EQUAL_FLOAT(15.2f, data.forecast[2].tempMin);
    TEST_ASSERT_EQUAL_STRING("Sat", data.forecast[2].dayName.c_str());
    TEST_ASSERT_EQUAL_STRING("Partly cloudy", data.forecast[2].status.c_str());
}

void test_weather_client_zip_code_initialization(void) {
    settings.setZipCode("90210"); WeatherClient client;
    WeatherData data = client.fetchWeather();
    
    TEST_ASSERT_TRUE(data.valid);
    TEST_ASSERT_EQUAL_FLOAT(21.5f, data.temperature);
    TEST_ASSERT_EQUAL(60, data.humidity);
    TEST_ASSERT_EQUAL_STRING("Clear sky", data.status.c_str());
    TEST_ASSERT_EQUAL(0, data.weatherCode);
    TEST_ASSERT_EQUAL_FLOAT(8.8f, data.windSpeed);
}

void test_weather_client_parse_weather_json_custom(void) {
    WeatherData data = { 0.0f, 0, "Unknown", false, -1, 0.0f, 0, "", {} };
    const char* customJson = "{"
        "\"current\":{"
            "\"temperature_2m\":15.5,"
            "\"relative_humidity_2m\":80,"
            "\"weather_code\":3,"
            "\"wind_speed_10m\":12.4"
        "},"
        "\"daily\":{"
            "\"time\":[\"2026-07-09\",\"2026-07-10\",\"2026-07-11\"],"
            "\"weather_code\":[3,45,95],"
            "\"temperature_2m_max\":[18.5,16.0,22.0],"
            "\"temperature_2m_min\":[12.0,11.0,14.5]"
        "}"
    "}";
    
    bool parsed = WeatherClient::parseWeatherJson(customJson, data);
    TEST_ASSERT_TRUE(parsed);
    TEST_ASSERT_TRUE(data.valid);
    TEST_ASSERT_EQUAL_FLOAT(15.5f, data.temperature);
    TEST_ASSERT_EQUAL(80, data.humidity);
    TEST_ASSERT_EQUAL_STRING("Overcast", data.status.c_str());
    TEST_ASSERT_EQUAL(3, data.weatherCode);
    TEST_ASSERT_EQUAL_FLOAT(12.4f, data.windSpeed);

    TEST_ASSERT_EQUAL_FLOAT(18.5f, data.forecast[0].tempMax);
    TEST_ASSERT_EQUAL_FLOAT(12.0f, data.forecast[0].tempMin);
    TEST_ASSERT_EQUAL_STRING("Today", data.forecast[0].dayName.c_str());
    TEST_ASSERT_EQUAL_STRING("Overcast", data.forecast[0].status.c_str());

    TEST_ASSERT_EQUAL_FLOAT(16.0f, data.forecast[1].tempMax);
    TEST_ASSERT_EQUAL_FLOAT(11.0f, data.forecast[1].tempMin);
    TEST_ASSERT_EQUAL_STRING("Tmrw", data.forecast[1].dayName.c_str());
    TEST_ASSERT_EQUAL_STRING("Foggy", data.forecast[1].status.c_str());

    TEST_ASSERT_EQUAL_FLOAT(22.0f, data.forecast[2].tempMax);
    TEST_ASSERT_EQUAL_FLOAT(14.5f, data.forecast[2].tempMin);
    TEST_ASSERT_EQUAL_STRING("Sat", data.forecast[2].dayName.c_str());
    TEST_ASSERT_EQUAL_STRING("Thunderstorm", data.forecast[2].status.c_str());
}

void test_weather_client_parse_metoffice_hourly_json(void) {
    settings.setUnitSystem(UNIT_METRIC);
    WeatherData data = {};
    const char* hourlyJson = "{"
        "\"features\":[{\"properties\":{"
            "\"location\":{\"name\":\"Shamley Green\"},"
            "\"timeSeries\":["
                "{\"time\":\"2026-10-17T12:00Z\",\"screenTemperature\":15.5,\"screenRelativeHumidity\":80,\"windSpeed10m\":4.0,\"windDirectionFrom10m\":180,\"significantWeatherCode\":3,\"probOfPrecipitation\":10},"
                "{\"time\":\"2026-10-17T13:00Z\",\"screenTemperature\":16.5,\"screenRelativeHumidity\":75,\"windSpeed10m\":5.0,\"windDirectionFrom10m\":190,\"significantWeatherCode\":1,\"probOfPrecipitation\":20}"
            "]"
        "}}]}";
    time_t now = parseUtcDtTxt("2026-10-17 12:30:00");

    TEST_ASSERT_TRUE(WeatherClient::parseMetOfficeHourlyJson(hourlyJson, data, now));
    TEST_ASSERT_TRUE(data.valid);
    TEST_ASSERT_EQUAL_STRING("Shamley Green", data.cityName.c_str());
    TEST_ASSERT_EQUAL_FLOAT(15.5f, data.temperature);
    TEST_ASSERT_EQUAL(80, data.humidity);
    TEST_ASSERT_EQUAL(2, data.weatherCode);
    TEST_ASSERT_EQUAL_STRING("Partly cloudy", data.status.c_str());
    TEST_ASSERT_EQUAL_FLOAT(14.4f, data.windSpeed);
    TEST_ASSERT_EQUAL(180, data.windDirection);
    TEST_ASSERT_EQUAL_FLOAT(15.5f, data.hourly[0].temperature);
    TEST_ASSERT_EQUAL(10, data.hourly[0].precipitationProbability);
    TEST_ASSERT_EQUAL_FLOAT(16.5f, data.hourly[1].temperature);
    TEST_ASSERT_EQUAL(20, data.hourly[1].precipitationProbability);

    settings.setUnitSystem(UNIT_IMPERIAL);
    WeatherData imperialData = {};
    TEST_ASSERT_TRUE(WeatherClient::parseMetOfficeHourlyJson(hourlyJson, imperialData, now));
    TEST_ASSERT_EQUAL_FLOAT(59.9f, imperialData.temperature);
    TEST_ASSERT_EQUAL_FLOAT(8.947745f, imperialData.windSpeed);
    TEST_ASSERT_EQUAL_FLOAT(61.7f, imperialData.hourly[1].temperature);
    settings.setUnitSystem(UNIT_METRIC);
}

void test_weather_client_metoffice_descriptions_use_api_codes(void) {
    const int metOfficeCodes[] = {13, 14, 15};
    const char* expectedDescriptions[] = {
        "Heavy rain",
        "Heavy rain shower (night)",
        "Heavy rain shower (day)"
    };
    const int expectedWmoCodes[] = {65, 82, 82};
    const time_t now = parseUtcDtTxt("2026-10-17 12:30:00");

    for (size_t i = 0; i < sizeof(metOfficeCodes) / sizeof(metOfficeCodes[0]); i++) {
        char hourlyJson[384];
        snprintf(hourlyJson, sizeof(hourlyJson),
            "{\"features\":[{\"properties\":{\"timeSeries\":["
            "{\"time\":\"2026-10-17T12:00Z\",\"screenTemperature\":15.5,"
            "\"screenRelativeHumidity\":80,\"significantWeatherCode\":%d}"
            "]}}]}",
            metOfficeCodes[i]);

        WeatherData data = {};
        TEST_ASSERT_TRUE(WeatherClient::parseMetOfficeHourlyJson(hourlyJson, data, now));
        TEST_ASSERT_EQUAL(expectedWmoCodes[i], data.weatherCode);
        TEST_ASSERT_EQUAL_STRING(expectedDescriptions[i], data.status.c_str());
    }

    String originalTimezone = settings.getTimezone();
    settings.setTimezone("UTC0");
    const char* dailyJson = "{"
        "\"features\":[{\"properties\":{\"timeSeries\":["
            "{\"time\":\"2026-10-17T00:00Z\",\"dayMaxScreenTemperature\":18.0,"
            "\"nightMinScreenTemperature\":11.0,\"daySignificantWeatherCode\":14}"
        "]}}]}";
    WeatherData dailyData = {};
    TEST_ASSERT_TRUE(WeatherClient::parseMetOfficeDailyJson(dailyJson, dailyData, now));
    TEST_ASSERT_EQUAL_STRING("Heavy rain shower (night)", dailyData.forecast[0].status.c_str());
    settings.setTimezone(originalTimezone);
}

void test_weather_client_parse_bpf_coverage_json(void) {
    settings.setUnitSystem(UNIT_METRIC);
    String originalTimezone = settings.getTimezone();
    settings.setTimezone("UTC0");
    const char* percentileJson = R"json({
        "coverages":[
            {"id":"airTemperature1p5m","domain":{"axes":{
                "t":{"values":["2026-01-01T00:00Z","2026-01-01T01:00Z","2026-01-02T00:00Z","2026-01-03T00:00Z"]},
                "percentiles":{"values":[10,50,90]}
            }},"ranges":{"airTemperature1p5m":{
                "axisNames":["percentiles","t"],"shape":[3,4],
                "values":[280,281,282,283,290,291,292,293,300,301,302,303]
            }}},
            {"id":"relativeHumidity1p5m","domain":{"axes":{
                "t":{"values":["2026-01-01T00:00Z","2026-01-01T01:00Z","2026-01-02T00:00Z","2026-01-03T00:00Z"]}
            }},"ranges":{"relativeHumidity1p5m":{
                "axisNames":["t"],"shape":[4],"values":[75,74,73,72]
            }}},
            {"id":"weatherCodePt01h","domain":{"axes":{
                "t":{"values":["2026-01-01T00:00Z","2026-01-01T01:00Z","2026-01-02T00:00Z","2026-01-03T00:00Z"]}
            }},"ranges":{"weatherCodePt01h":{
                "axisNames":["t"],"shape":[4],"values":[14,13,3,1]
            }}},
            {"id":"windSpeed10m","domain":{"axes":{
                "t":{"values":["2026-01-01T00:00Z","2026-01-01T01:00Z","2026-01-02T00:00Z","2026-01-03T00:00Z"]}
            }},"ranges":{"windSpeed10m":{
                "axisNames":["t"],"shape":[4],"values":[3.6,4,5,6]
            }}},
            {"id":"windFromDirection10mMean","domain":{"axes":{
                "t":{"values":["2026-01-01T00:00Z","2026-01-01T01:00Z","2026-01-02T00:00Z","2026-01-03T00:00Z"]}
            }},"ranges":{"windFromDirection10mMean":{
                "axisNames":["t"],"shape":[4],"values":[225,230,235,240]
            }}}
        ]
    })json";
    const char* probabilityJson = R"json({
        "coverages":[{
            "id":"probabilityOfLweThicknessOfPrecipitationAmountAboveThresholdSumPt01h",
            "domain":{"axes":{
                "threshold":{"values":[0.1,1]},
                "t":{"values":[
                    "2026-01-01T00:00Z","2026-01-01T01:00Z","2026-01-02T00:00Z","2026-01-03T00:00Z"
                ]}
            }},
            "ranges":{"probabilityOfLweThicknessOfPrecipitationAmountAboveThresholdSumPt01h":{
                "axisNames":["threshold","t"],"shape":[2,4],
                "values":[0.1,0.25,0.5,0.75,0.01,0.02,0.03,0.04]
            }}
        }]
    })json";
    WeatherData data = {};
    time_t now = parseUtcDtTxt("2026-01-01 00:30:00");
    std::string expandedPercentileJson(percentileJson);
    expandedPercentileJson.pop_back();
    expandedPercentileJson += ",\"ignored\":[";
    for (size_t i = 0; i < 8000; i++) {
        expandedPercentileJson += "0,";
    }
    expandedPercentileJson += "0]}";

    TEST_ASSERT_TRUE(WeatherClient::parseBlendedProbabilisticJson(
        expandedPercentileJson.c_str(), probabilityJson, data, now));
    TEST_ASSERT_TRUE(data.valid);
    TEST_ASSERT_EQUAL_FLOAT(16.85f, data.temperature);
    TEST_ASSERT_EQUAL(75, data.humidity);
    TEST_ASSERT_EQUAL_STRING("Heavy rain shower (night)", data.status.c_str());
    TEST_ASSERT_EQUAL(82, data.weatherCode);
    TEST_ASSERT_EQUAL_FLOAT(12.96f, data.windSpeed);
    TEST_ASSERT_EQUAL(225, data.windDirection);
    TEST_ASSERT_EQUAL_FLOAT(16.85f, data.hourly[0].temperature);
    TEST_ASSERT_EQUAL(10, data.hourly[0].precipitationProbability);
    TEST_ASSERT_EQUAL_FLOAT(17.85f, data.hourly[1].temperature);
    TEST_ASSERT_EQUAL(25, data.hourly[1].precipitationProbability);
    TEST_ASSERT_EQUAL_STRING("Today", data.forecast[0].dayName.c_str());
    TEST_ASSERT_EQUAL_FLOAT(16.85f, data.forecast[0].tempMin);
    TEST_ASSERT_EQUAL_FLOAT(17.85f, data.forecast[0].tempMax);
    TEST_ASSERT_EQUAL_STRING("Tmrw", data.forecast[1].dayName.c_str());
    TEST_ASSERT_EQUAL_STRING("Sat", data.forecast[2].dayName.c_str());
    settings.setTimezone(originalTimezone);
}

void test_weather_client_metoffice_humidity_falls_back_to_nearest_hour(void) {
    WeatherData data = {};
    const char* hourlyJson = "{"
        "\"features\":[{\"properties\":{"
            "\"timeSeries\":["
                "{\"time\":\"2026-10-17T12:00Z\",\"screenTemperature\":15.5,\"windSpeed10m\":4.0,\"windDirectionFrom10m\":180,\"significantWeatherCode\":3},"
                "{\"time\":\"2026-10-17T13:00Z\",\"screenTemperature\":16.5,\"screenRelativeHumidity\":75,\"windSpeed10m\":5.0,\"windDirectionFrom10m\":190,\"significantWeatherCode\":1}"
            "]"
        "}}]}";
    time_t now = parseUtcDtTxt("2026-10-17 12:30:00");

    TEST_ASSERT_TRUE(WeatherClient::parseMetOfficeHourlyJson(hourlyJson, data, now));
    TEST_ASSERT_EQUAL_FLOAT(15.5f, data.temperature);
    TEST_ASSERT_EQUAL(75, data.humidity);
}

void test_weather_client_parse_metoffice_daily_json(void) {
    settings.setUnitSystem(UNIT_METRIC);
    String originalTimezone = settings.getTimezone();
    settings.setTimezone("UTC0");
    WeatherData data = {};
    const char* dailyJson = "{"
        "\"features\":[{\"properties\":{\"timeSeries\":["
            "{\"time\":\"2026-10-17T00:00Z\",\"dayMaxScreenTemperature\":18.0,\"nightMinScreenTemperature\":11.0,\"daySignificantWeatherCode\":1},"
            "{\"time\":\"2026-10-18T00:00Z\",\"dayMaxScreenTemperature\":17.0,\"nightMinScreenTemperature\":10.0,\"daySignificantWeatherCode\":7},"
            "{\"time\":\"2026-10-19T00:00Z\",\"dayMaxScreenTemperature\":16.0,\"nightMinScreenTemperature\":9.0,\"daySignificantWeatherCode\":12}"
        "]}}]}";
    time_t now = parseUtcDtTxt("2026-10-17 12:30:00");

    TEST_ASSERT_TRUE(WeatherClient::parseMetOfficeDailyJson(dailyJson, data, now));
    TEST_ASSERT_EQUAL_STRING("Today", data.forecast[0].dayName.c_str());
    TEST_ASSERT_EQUAL_FLOAT(18.0f, data.forecast[0].tempMax);
    TEST_ASSERT_EQUAL_FLOAT(11.0f, data.forecast[0].tempMin);
    TEST_ASSERT_EQUAL_STRING("Clear sky", data.forecast[0].status.c_str());
    TEST_ASSERT_EQUAL_STRING("Tmrw", data.forecast[1].dayName.c_str());
    TEST_ASSERT_EQUAL_STRING("Overcast", data.forecast[1].status.c_str());
    TEST_ASSERT_EQUAL_STRING("Mon", data.forecast[2].dayName.c_str());
    TEST_ASSERT_EQUAL_STRING("Slight rain", data.forecast[2].status.c_str());
    settings.setTimezone(originalTimezone);
}

void test_weather_client_parse_thingspeak_readings_json(void) {
    float temperature = 0.0f;
    float voltage = 0.0f;
    bool temperatureValid = false;
    bool voltageValid = false;
    TEST_ASSERT_TRUE(WeatherClient::parseThingSpeakReadingsJson(
        "{\"field1\":\"22.30\",\"field3\":\"12.45\"}", temperature,
        temperatureValid, voltage, voltageValid));
    TEST_ASSERT_EQUAL_FLOAT(22.3f, temperature);
    TEST_ASSERT_EQUAL_FLOAT(12.45f, voltage);
    TEST_ASSERT_TRUE(temperatureValid);
    TEST_ASSERT_TRUE(voltageValid);

    TEST_ASSERT_TRUE(WeatherClient::parseThingSpeakReadingsJson(
        "{\"field1\":\"\",\"field3\":\"13.2\"}", temperature,
        temperatureValid, voltage, voltageValid));
    TEST_ASSERT_FALSE(temperatureValid);
    TEST_ASSERT_TRUE(voltageValid);
    TEST_ASSERT_EQUAL_FLOAT(13.2f, voltage);

    TEST_ASSERT_FALSE(WeatherClient::parseThingSpeakReadingsJson(
        "{\"field1\":\"not-a-temperature\",\"field3\":\"invalid\"}", temperature,
        temperatureValid, voltage, voltageValid));
    TEST_ASSERT_FALSE(temperatureValid);
    TEST_ASSERT_FALSE(voltageValid);

    TEST_ASSERT_FALSE(WeatherClient::parseThingSpeakReadingsJson(
        "{ invalid", temperature, temperatureValid, voltage, voltageValid));
}

void test_weather_client_parse_owm_json(void) {
    settings.setZipCode(""); settings.setLatitude("33.60002"); settings.setLongitude("-117.67200"); WeatherClient client;
    WeatherData data = { 0.0f, 0, "Unknown", false, -1, 0.0f, 0, "", {} };

    const char* owmJson = "{"
        "\"list\":["
            "{"
                "\"dt_txt\":\"2026-07-11 12:00:00\","
                "\"main\":{\"temp\":72.5,\"humidity\":50,\"temp_min\":68.0,\"temp_max\":75.0},"
                "\"wind\":{\"speed\":5.5,\"deg\":180},"
                "\"weather\":[{\"id\":800,\"description\":\"clear sky\"}]"
            "}"
        "],"
        "\"city\":{\"name\":\"Orange County\"}"
    "}";

    bool parsed = client.parseOwmJson(owmJson, data);
    TEST_ASSERT_TRUE(parsed);
    TEST_ASSERT_TRUE(data.valid);
    TEST_ASSERT_EQUAL_FLOAT(72.5f, data.temperature);
    TEST_ASSERT_EQUAL(50, data.humidity);
    TEST_ASSERT_EQUAL_FLOAT(5.5f, data.windSpeed);
    TEST_ASSERT_EQUAL(180, data.windDirection);
    TEST_ASSERT_EQUAL_STRING("Orange County", data.cityName.c_str());
    TEST_ASSERT_EQUAL_STRING("Clear sky", data.status.c_str());
}

void test_weather_client_parse_ip_location_json(void) {
    const char* ipLocationJson = "{"
        "\"status\":\"success\","
        "\"city\":\"San Francisco\","
        "\"lat\":37.7749,"
        "\"lon\":-122.4194"
    "}";
    float lat = 0.0f;
    float lon = 0.0f;
    String city = "";
    bool parsed = WeatherClient::parseIpLocationJson(ipLocationJson, lat, lon, city);
    TEST_ASSERT_TRUE(parsed);
    TEST_ASSERT_EQUAL_FLOAT(37.7749f, lat);
    TEST_ASSERT_EQUAL_FLOAT(-122.4194f, lon);
    TEST_ASSERT_EQUAL_STRING("San Francisco", city.c_str());
}

void test_weather_client_parse_ip_location_json_fail(void) {
    const char* ipLocationJson = "{"
        "\"status\":\"fail\","
        "\"message\":\"invalid query\""
    "}";
    float lat = 0.0f;
    float lon = 0.0f;
    String city = "";
    bool parsed = WeatherClient::parseIpLocationJson(ipLocationJson, lat, lon, city);
    TEST_ASSERT_FALSE(parsed);
}

void test_weather_client_is_location_empty(void) {
    settings.setZipCode(""); settings.setLatitude(""); settings.setLongitude(""); WeatherClient client1;
    TEST_ASSERT_TRUE(client1.isLocationEmpty());

    settings.setZipCode("YOUR_ZIP_CODE"); WeatherClient client2;
    TEST_ASSERT_TRUE(client2.isLocationEmpty());

    settings.setZipCode("90210"); WeatherClient client3;
    TEST_ASSERT_FALSE(client3.isLocationEmpty());

    settings.setZipCode(""); settings.setLatitude(""); settings.setLongitude(""); WeatherClient client4;
    TEST_ASSERT_TRUE(client4.isLocationEmpty());

    settings.setZipCode(""); settings.setLatitude("YOUR_LATITUDE"); settings.setLongitude("-122.4194"); WeatherClient client5;
    TEST_ASSERT_TRUE(client5.isLocationEmpty());

    settings.setZipCode(""); settings.setLatitude("37.7749"); settings.setLongitude("-122.4194"); WeatherClient client6;
    TEST_ASSERT_FALSE(client6.isLocationEmpty());
}

void test_weather_client_parse_weather_json_hourly(void) {
    WeatherData data = { 0.0f, 0, "Unknown", false, -1, 0.0f, 0, "", {}, {} };
    const char* customJson = "{"
        "\"current\":{"
            "\"temperature_2m\":15.5,"
            "\"relative_humidity_2m\":80,"
            "\"weather_code\":3,"
            "\"wind_speed_10m\":12.4"
        "},"
        "\"daily\":{"
            "\"time\":[\"2026-07-09\",\"2026-07-10\",\"2026-07-11\"],"
            "\"weather_code\":[3,45,95],"
            "\"temperature_2m_max\":[18.5,16.0,22.0],"
            "\"temperature_2m_min\":[12.0,11.0,14.5]"
        "},"
        "\"hourly\":{"
            "\"temperature_2m\":[10.0,11.0,12.0,13.0,14.0,15.0,16.0,17.0,18.0,19.0,20.0,21.0,22.0,23.0,24.0,25.0,26.0,27.0,28.0,29.0,30.0,31.0,32.0,33.0],"
            "\"precipitation_probability\":[0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100,5,10,15]"
        "}"
    "}";
    
    bool parsed = WeatherClient::parseWeatherJson(customJson, data);
    TEST_ASSERT_TRUE(parsed);
    TEST_ASSERT_TRUE(data.valid);
    
    // Check hourly data
    TEST_ASSERT_EQUAL_FLOAT(10.0f, data.hourly[0].temperature);
    TEST_ASSERT_EQUAL(0, data.hourly[0].precipitationProbability);
    TEST_ASSERT_EQUAL_FLOAT(21.0f, data.hourly[11].temperature);
    TEST_ASSERT_EQUAL(55, data.hourly[11].precipitationProbability);
    TEST_ASSERT_EQUAL_FLOAT(33.0f, data.hourly[23].temperature);
    TEST_ASSERT_EQUAL(15, data.hourly[23].precipitationProbability);
}

void test_weather_client_parse_owm_json_hourly(void) {
    settings.setZipCode(""); settings.setLatitude("33.60002"); settings.setLongitude("-117.67200"); WeatherClient client;
    WeatherData data = { 0.0f, 0, "Unknown", false, -1, 0.0f, 0, "", {}, {} };

    // OWM payload with at least 8 elements (24 hours at 3-hour intervals)
    const char* owmJson = "{"
        "\"list\":["
            "{\"dt_txt\":\"2026-07-11 12:00:00\",\"main\":{\"temp\":70.0,\"humidity\":50},\"wind\":{\"speed\":5.0,\"deg\":180},\"weather\":[{\"id\":800,\"description\":\"clear sky\"}],\"pop\":0.10},"
            "{\"dt_txt\":\"2026-07-11 15:00:00\",\"main\":{\"temp\":71.0},\"weather\":[{\"id\":800}],\"pop\":0.20},"
            "{\"dt_txt\":\"2026-07-11 18:00:00\",\"main\":{\"temp\":72.0},\"weather\":[{\"id\":800}],\"pop\":0.30},"
            "{\"dt_txt\":\"2026-07-11 21:00:00\",\"main\":{\"temp\":73.0},\"weather\":[{\"id\":800}],\"pop\":0.40},"
            "{\"dt_txt\":\"2026-07-12 00:00:00\",\"main\":{\"temp\":74.0},\"weather\":[{\"id\":800}],\"pop\":0.50},"
            "{\"dt_txt\":\"2026-07-12 03:00:00\",\"main\":{\"temp\":75.0},\"weather\":[{\"id\":800}],\"pop\":0.60},"
            "{\"dt_txt\":\"2026-07-12 06:00:00\",\"main\":{\"temp\":76.0},\"weather\":[{\"id\":800}],\"pop\":0.70},"
            "{\"dt_txt\":\"2026-07-12 09:00:00\",\"main\":{\"temp\":77.0},\"weather\":[{\"id\":800}],\"pop\":0.80}"
        "],"
        "\"city\":{\"name\":\"Orange County\"}"
    "}";

    bool parsed = client.parseOwmJson(owmJson, data);
    TEST_ASSERT_TRUE(parsed);
    TEST_ASSERT_TRUE(data.valid);
    
    // Check hourly mapped from the 3-hour list
    TEST_ASSERT_EQUAL_FLOAT(70.0f, data.hourly[0].temperature);
    TEST_ASSERT_EQUAL(10, data.hourly[0].precipitationProbability);
    TEST_ASSERT_EQUAL_FLOAT(70.0f, data.hourly[1].temperature); // Mapped
    TEST_ASSERT_EQUAL_FLOAT(71.0f, data.hourly[3].temperature);
    TEST_ASSERT_EQUAL(20, data.hourly[3].precipitationProbability);
    TEST_ASSERT_EQUAL_FLOAT(77.0f, data.hourly[23].temperature);
    TEST_ASSERT_EQUAL(80, data.hourly[23].precipitationProbability);
}

void test_weather_client_parse_owm_json_city_override(void) {
    settings.setZipCode(""); settings.setLatitude("33.60002"); settings.setLongitude("-117.67200"); WeatherClient client;
    
    // Set _cityName via mock fetchIpLocation (sets it to "San Francisco")
    String latStr, lonStr, city;
    client.fetchIpLocation(latStr, lonStr, city);

    WeatherData data = { 0.0f, 0, "Unknown", false, -1, 0.0f, 0, "", {} };
    const char* owmJson = "{"
        "\"list\":["
            "{"
                "\"dt_txt\":\"2026-07-11 12:00:00\","
                "\"main\":{\"temp\":72.5,\"humidity\":50,\"temp_min\":68.0,\"temp_max\":75.0},"
                "\"wind\":{\"speed\":5.5,\"deg\":180},"
                "\"weather\":[{\"id\":800,\"description\":\"clear sky\"}]"
            "}"
        "],"
        "\"city\":{\"name\":\"London\"}"
    "}";

    bool parsed = client.parseOwmJson(owmJson, data);
    TEST_ASSERT_TRUE(parsed);
    TEST_ASSERT_TRUE(data.valid);
    TEST_ASSERT_EQUAL_STRING("London", data.cityName.c_str());
}

void test_weather_client_all_desc_codes(void) {
    TEST_ASSERT_EQUAL_STRING("Depositing rime fog", WeatherClient::getWeatherDesc(48).c_str());
    TEST_ASSERT_EQUAL_STRING("Light drizzle", WeatherClient::getWeatherDesc(51).c_str());
    TEST_ASSERT_EQUAL_STRING("Moderate drizzle", WeatherClient::getWeatherDesc(53).c_str());
    TEST_ASSERT_EQUAL_STRING("Dense drizzle", WeatherClient::getWeatherDesc(55).c_str());
    TEST_ASSERT_EQUAL_STRING("Light freezing drizzle", WeatherClient::getWeatherDesc(56).c_str());
    TEST_ASSERT_EQUAL_STRING("Dense freezing drizzle", WeatherClient::getWeatherDesc(57).c_str());
    TEST_ASSERT_EQUAL_STRING("Slight rain", WeatherClient::getWeatherDesc(61).c_str());
    TEST_ASSERT_EQUAL_STRING("Moderate rain", WeatherClient::getWeatherDesc(63).c_str());
    TEST_ASSERT_EQUAL_STRING("Heavy rain", WeatherClient::getWeatherDesc(65).c_str());
    TEST_ASSERT_EQUAL_STRING("Light freezing rain", WeatherClient::getWeatherDesc(66).c_str());
    TEST_ASSERT_EQUAL_STRING("Heavy freezing rain", WeatherClient::getWeatherDesc(67).c_str());
    TEST_ASSERT_EQUAL_STRING("Slight snow fall", WeatherClient::getWeatherDesc(71).c_str());
    TEST_ASSERT_EQUAL_STRING("Moderate snow fall", WeatherClient::getWeatherDesc(73).c_str());
    TEST_ASSERT_EQUAL_STRING("Heavy snow fall", WeatherClient::getWeatherDesc(75).c_str());
    TEST_ASSERT_EQUAL_STRING("Snow grains", WeatherClient::getWeatherDesc(77).c_str());
    TEST_ASSERT_EQUAL_STRING("Slight rain showers", WeatherClient::getWeatherDesc(80).c_str());
    TEST_ASSERT_EQUAL_STRING("Moderate rain showers", WeatherClient::getWeatherDesc(81).c_str());
    TEST_ASSERT_EQUAL_STRING("Violent rain showers", WeatherClient::getWeatherDesc(82).c_str());
    TEST_ASSERT_EQUAL_STRING("Slight snow showers", WeatherClient::getWeatherDesc(85).c_str());
    TEST_ASSERT_EQUAL_STRING("Heavy snow showers", WeatherClient::getWeatherDesc(86).c_str());
    TEST_ASSERT_EQUAL_STRING("Thunderstorm", WeatherClient::getWeatherDesc(95).c_str());
    TEST_ASSERT_EQUAL_STRING("Thunderstorm with slight hail", WeatherClient::getWeatherDesc(96).c_str());
    TEST_ASSERT_EQUAL_STRING("Thunderstorm with heavy hail", WeatherClient::getWeatherDesc(99).c_str());
}

void test_weather_client_owm_to_wmo_code_via_json(void) {
    WeatherClient client;
    WeatherData data;

    int testOwmCodes[] = { 205, 310, 511, 500, 600, 741, 800, 801, 802, 804, 999 };
    int expectedWmo[] = { 95, 51, 66, 61, 71, 45, 0, 1, 2, 3, -1 };

    for (size_t i = 0; i < sizeof(testOwmCodes) / sizeof(testOwmCodes[0]); i++) {
        String json = "{\"list\":[{\"dt_txt\":\"2026-07-11 12:00:00\",\"main\":{\"temp\":70.0,\"humidity\":50},\"wind\":{\"speed\":5.0,\"deg\":180},\"weather\":[{\"id\":" + std::to_string(testOwmCodes[i]) + ",\"description\":\"test\"}]}],\"city\":{\"name\":\"TestCity\"}}";
        TEST_ASSERT_TRUE(client.parseOwmJson(json.c_str(), data));
        TEST_ASSERT_EQUAL(expectedWmo[i], data.weatherCode);
    }
}

void test_weather_client_parsing_errors(void) {
    WeatherData data;
    // 1. Invalid JSON
    TEST_ASSERT_FALSE(WeatherClient::parseWeatherJson("{ invalid", data));
    
    // 2. Invalid OWM JSON
    WeatherClient client;
    TEST_ASSERT_FALSE(client.parseOwmJson("{ invalid", data));
    TEST_ASSERT_FALSE(client.parseOwmJson("{\"list\":[]}", data));

    // 3. Deserialization error
    TEST_ASSERT_FALSE(WeatherClient::deserializeWeatherData("", data));
    TEST_ASSERT_FALSE(WeatherClient::deserializeWeatherData("{ invalid", data));

    // 4. IP location invalid JSON
    float lat, lon;
    String city;
    TEST_ASSERT_FALSE(WeatherClient::parseIpLocationJson("{ invalid", lat, lon, city));
}

void test_weather_client_parse_weather_json_hourly_sliding_window(void) {
    WeatherData data;
    // Construct JSON where hourly data starts at 00:00 (midnight), but current time is 14:00 (index 14)
    String json = "{\"current\":{\"time\":\"2026-10-05T14:00\",\"temperature_2m\":24.5,\"relative_humidity_2m\":50,\"weather_code\":0,\"wind_speed_10m\":5.0},"
                  "\"hourly\":{\"time\":[";
    for (int i = 0; i < 48; i++) {
        char tBuf[32];
        snprintf(tBuf, sizeof(tBuf), "\"2026-10-%02dT%02d:00\"%s", (i < 24 ? 5 : 6), i % 24, (i < 47 ? "," : ""));
        json += tBuf;
    }
    json += "],\"temperature_2m\":[";
    for (int i = 0; i < 48; i++) {
        char vBuf[16];
        snprintf(vBuf, sizeof(vBuf), "%.1f%s", (float)(10.0 + i), (i < 47 ? "," : ""));
        json += vBuf;
    }
    json += "],\"precipitation_probability\":[";
    for (int i = 0; i < 48; i++) {
        char pBuf[16];
        snprintf(pBuf, sizeof(pBuf), "%d%s", i, (i < 47 ? "," : ""));
        json += pBuf;
    }
    json += "]}}";

    bool parsed = WeatherClient::parseWeatherJson(json.c_str(), data);
    TEST_ASSERT_TRUE(parsed);
    TEST_ASSERT_TRUE(data.valid);
    // data.hourly[0] should match index 14 (14:00 = 24.0°C)
    TEST_ASSERT_EQUAL_FLOAT(24.0f, data.hourly[0].temperature);
    TEST_ASSERT_EQUAL(14, data.hourly[0].precipitationProbability);
    // data.hourly[1] should match index 15 (15:00 = 25.0°C)
    TEST_ASSERT_EQUAL_FLOAT(25.0f, data.hourly[1].temperature);
    TEST_ASSERT_EQUAL(15, data.hourly[1].precipitationProbability);
    // data.hourly[23] should match index 14 + 23 = 37 (10.0 + 37 = 47.0°C)
    TEST_ASSERT_EQUAL_FLOAT(47.0f, data.hourly[23].temperature);
    TEST_ASSERT_EQUAL(37, data.hourly[23].precipitationProbability);
}

void test_weather_client_parse_weather_json_hourly_subhour_current_time(void) {
    WeatherData data;
    // current.time has minutes (14:35), should still match 14:00 in hourly.time
    String json = "{\"current\":{\"time\":\"2026-10-05T14:35\",\"temperature_2m\":24.5,\"relative_humidity_2m\":50,\"weather_code\":0,\"wind_speed_10m\":5.0},"
                  "\"hourly\":{\"time\":[\"2026-10-05T12:00\",\"2026-10-05T13:00\",\"2026-10-05T14:00\",\"2026-10-05T15:00\"],"
                  "\"temperature_2m\":[20.0,22.0,24.0,26.0],"
                  "\"precipitation_probability\":[10,20,30,40]}}";

    bool parsed = WeatherClient::parseWeatherJson(json.c_str(), data);
    TEST_ASSERT_TRUE(parsed);
    TEST_ASSERT_TRUE(data.valid);
    // data.hourly[0] matches 14:00 (index 2)
    TEST_ASSERT_EQUAL_FLOAT(24.0f, data.hourly[0].temperature);
    TEST_ASSERT_EQUAL(30, data.hourly[0].precipitationProbability);
    TEST_ASSERT_EQUAL_FLOAT(26.0f, data.hourly[1].temperature);
    TEST_ASSERT_EQUAL(40, data.hourly[1].precipitationProbability);
}

void test_weather_client_parse_weather_json_forecast_hours_24(void) {
    WeatherData data;
    // When forecast_hours=24 is used, hourly.time[0] is already the current hour
    const char* json = "{\"current\":{\"time\":\"2026-10-05T20:00\",\"temperature_2m\":18.5},"
                       "\"hourly\":{\"time\":[\"2026-10-05T20:00\",\"2026-10-05T21:00\"],"
                       "\"temperature_2m\":[18.5,17.2],"
                       "\"precipitation_probability\":[5,10]}}";

    bool parsed = WeatherClient::parseWeatherJson(json, data);
    TEST_ASSERT_TRUE(parsed);
    TEST_ASSERT_EQUAL_FLOAT(18.5f, data.hourly[0].temperature);
    TEST_ASSERT_EQUAL(5, data.hourly[0].precipitationProbability);
    TEST_ASSERT_EQUAL_FLOAT(17.2f, data.hourly[1].temperature);
    TEST_ASSERT_EQUAL(10, data.hourly[1].precipitationProbability);
}

void test_weather_client_forecast_day_labels_utc_rollover_issue_45(void) {
    // Regression test for Issue #45:
    // When local time is Wednesday evening in America/Los_Angeles (e.g. 2026-10-07 20:00 PDT),
    // UTC is already Thursday morning (2026-10-08 03:00 UTC).
    // The 3 daily forecast boxes must show "Today" (Wed), "Tmrw" (Thu), and "Fri".
    // Previously, UTC rollover caused it to display "Wed", "Today", "Tmrw".
    settings.setTimezone("America/Los_Angeles");
    time_t wednesday_evening_pdt = 1791428400; // 2026-10-08 03:00:00 UTC = 2026-10-07 20:00 PDT

    const char* json = "{"
        "\"current\":{\"temperature_2m\":20.0,\"relative_humidity_2m\":50,\"weather_code\":0,\"wind_speed_10m\":5.0},"
        "\"daily\":{"
            "\"time\":[\"2026-10-07\",\"2026-10-08\",\"2026-10-09\"],"
            "\"weather_code\":[0,1,2],"
            "\"temperature_2m_max\":[75.0,76.0,77.0],"
            "\"temperature_2m_min\":[55.0,56.0,57.0]"
        "}"
    "}";

    WeatherData data = {};
    bool parsed = WeatherClient::parseWeatherJson(json, data, wednesday_evening_pdt);
    TEST_ASSERT_TRUE(parsed);
    TEST_ASSERT_EQUAL_STRING("Today", data.forecast[0].dayName.c_str());
    TEST_ASSERT_EQUAL_STRING("Tmrw", data.forecast[1].dayName.c_str());
    TEST_ASSERT_EQUAL_STRING("Fri", data.forecast[2].dayName.c_str());
}

void test_weather_client_forecast_day_labels_owm_utc_rollover_issue_45(void) {
    // Same UTC rollover condition for OpenWeatherMap parser
    settings.setTimezone("America/Los_Angeles");
    time_t wednesday_evening_pdt = 1791428400; // 2026-10-07 20:00 PDT

    const char* owmJson = "{"
        "\"list\":["
            "{\"dt_txt\":\"2026-10-07 12:00:00\",\"main\":{\"temp\":70.0,\"temp_min\":55.0,\"temp_max\":75.0,\"humidity\":50},\"weather\":[{\"id\":800,\"description\":\"clear sky\"}],\"wind\":{\"speed\":5.0,\"deg\":180}},"
            "{\"dt_txt\":\"2026-10-08 12:00:00\",\"main\":{\"temp\":71.0,\"temp_min\":56.0,\"temp_max\":76.0,\"humidity\":50},\"weather\":[{\"id\":800,\"description\":\"clear sky\"}],\"wind\":{\"speed\":5.0,\"deg\":180}},"
            "{\"dt_txt\":\"2026-10-09 12:00:00\",\"main\":{\"temp\":72.0,\"temp_min\":57.0,\"temp_max\":77.0,\"humidity\":50},\"weather\":[{\"id\":800,\"description\":\"clear sky\"}],\"wind\":{\"speed\":5.0,\"deg\":180}}"
        "],"
        "\"city\":{\"name\":\"Los Angeles\"}"
    "}";

    WeatherClient client;
    WeatherData data = {};
    bool parsed = client.parseOwmJson(owmJson, data, wednesday_evening_pdt);
    TEST_ASSERT_TRUE(parsed);
    TEST_ASSERT_EQUAL_STRING("Today", data.forecast[0].dayName.c_str());
    TEST_ASSERT_EQUAL_STRING("Tmrw", data.forecast[1].dayName.c_str());
    TEST_ASSERT_EQUAL_STRING("Fri", data.forecast[2].dayName.c_str());
}

void test_weather_client_forecast_day_labels_ahead_of_utc(void) {
    // Timezone ahead of UTC (Asia/Tokyo, UTC+9):
    // 2026-10-07 16:00:00 UTC = 2026-10-08 01:00:00 JST (Thursday)
    settings.setTimezone("Asia/Tokyo");
    time_t thursday_early_jst = 1791388800;

    const char* json = "{"
        "\"current\":{\"temperature_2m\":20.0,\"relative_humidity_2m\":50,\"weather_code\":0,\"wind_speed_10m\":5.0},"
        "\"daily\":{"
            "\"time\":[\"2026-10-08\",\"2026-10-09\",\"2026-10-10\"],"
            "\"weather_code\":[0,1,2],"
            "\"temperature_2m_max\":[22.0,23.0,24.0],"
            "\"temperature_2m_min\":[14.0,15.0,16.0]"
        "}"
    "}";

    WeatherData data = {};
    bool parsed = WeatherClient::parseWeatherJson(json, data, thursday_early_jst);
    TEST_ASSERT_TRUE(parsed);
    TEST_ASSERT_EQUAL_STRING("Today", data.forecast[0].dayName.c_str());
    TEST_ASSERT_EQUAL_STRING("Tmrw", data.forecast[1].dayName.c_str());
    TEST_ASSERT_EQUAL_STRING("Sat", data.forecast[2].dayName.c_str());
}

void test_weather_client_forecast_day_labels_uninitialized_ntp_fallback(void) {
    // When NTP has not synced yet (time < year 2000), fall back to index-based labeling
    settings.setTimezone("America/Los_Angeles");
    time_t unsynced_time = 500; // near unix epoch 1970

    const char* json = "{"
        "\"current\":{\"temperature_2m\":20.0,\"relative_humidity_2m\":50,\"weather_code\":0,\"wind_speed_10m\":5.0},"
        "\"daily\":{"
            "\"time\":[\"2026-10-07\",\"2026-10-08\",\"2026-10-09\"],"
            "\"weather_code\":[0,1,2],"
            "\"temperature_2m_max\":[75.0,76.0,77.0],"
            "\"temperature_2m_min\":[55.0,56.0,57.0]"
        "}"
    "}";

    WeatherData data = {};
    bool parsed = WeatherClient::parseWeatherJson(json, data, unsynced_time);
    TEST_ASSERT_TRUE(parsed);
    TEST_ASSERT_EQUAL_STRING("Today", data.forecast[0].dayName.c_str());
    TEST_ASSERT_EQUAL_STRING("Tmrw", data.forecast[1].dayName.c_str());
    TEST_ASSERT_EQUAL_STRING("Fri", data.forecast[2].dayName.c_str());
}

void test_get_local_date_strings_direct(void) {
    char today[11] = "";
    char tomorrow[11] = "";

    // Test America/Los_Angeles across UTC rollover
    settings.setTimezone("America/Los_Angeles");
    bool ok = getLocalDateStrings(today, sizeof(today), tomorrow, sizeof(tomorrow), 1791428400);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_STRING("2026-10-07", today);
    TEST_ASSERT_EQUAL_STRING("2026-10-08", tomorrow);

    // Test Asia/Tokyo across UTC boundary
    settings.setTimezone("Asia/Tokyo");
    ok = getLocalDateStrings(today, sizeof(today), tomorrow, sizeof(tomorrow), 1791388800);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_STRING("2026-10-08", today);
    TEST_ASSERT_EQUAL_STRING("2026-10-09", tomorrow);

    // Test invalid / unsynced time
    ok = getLocalDateStrings(today, sizeof(today), tomorrow, sizeof(tomorrow), 1000);
    TEST_ASSERT_FALSE(ok);
}

void test_weather_client_owm_ahead_of_utc_morning_issue_45(void) {
    // Regression test for user feedback / Issue #45:
    // Location: Asia/Tokyo (UTC+9). Local time: Thursday 2026-10-08 06:00:00 JST.
    // UTC time: Wednesday 2026-10-07 21:00:00.
    // OpenWeatherMap dt_txt in UTC starts with "2026-10-07 21:00:00".
    // Previously, dt_txt's raw UTC date "2026-10-07" was parsed as Box 0,
    // which caused Box 0 to display "Thu" (or "Wed"), Box 1 "Today", Box 2 "Tmrw".
    // With timezone-aware parsing, 1791406800 maps to 2026-10-08 (local Today).
    settings.setTimezone("Asia/Tokyo");
    time_t thursday_morning_jst = 1791406800; // 2026-10-08 06:00:00 JST = 2026-10-07 21:00:00 UTC

    const char* owmJson = "{"
        "\"list\":["
            "{\"dt\":1791406800,\"dt_txt\":\"2026-10-07 21:00:00\",\"main\":{\"temp\":18.0,\"temp_min\":15.0,\"temp_max\":22.0,\"humidity\":60},\"weather\":[{\"id\":800,\"description\":\"clear sky\"}],\"wind\":{\"speed\":3.0,\"deg\":90}},"
            "{\"dt\":1791428400,\"dt_txt\":\"2026-10-08 03:00:00\",\"main\":{\"temp\":21.0,\"temp_min\":15.0,\"temp_max\":22.0,\"humidity\":50},\"weather\":[{\"id\":800,\"description\":\"clear sky\"}],\"wind\":{\"speed\":4.0,\"deg\":90}},"
            "{\"dt\":1791514800,\"dt_txt\":\"2026-10-09 03:00:00\",\"main\":{\"temp\":20.0,\"temp_min\":14.0,\"temp_max\":21.0,\"humidity\":55},\"weather\":[{\"id\":801,\"description\":\"few clouds\"}],\"wind\":{\"speed\":3.5,\"deg\":100}},"
            "{\"dt\":1791601200,\"dt_txt\":\"2026-10-10 03:00:00\",\"main\":{\"temp\":19.0,\"temp_min\":13.0,\"temp_max\":20.0,\"humidity\":50},\"weather\":[{\"id\":802,\"description\":\"scattered clouds\"}],\"wind\":{\"speed\":3.0,\"deg\":110}}"
        "],"
        "\"city\":{\"name\":\"Tokyo\",\"timezone\":32400}"
    "}";

    WeatherClient client;
    WeatherData data = {};
    bool parsed = client.parseOwmJson(owmJson, data, thursday_morning_jst);
    TEST_ASSERT_TRUE(parsed);
    TEST_ASSERT_EQUAL_STRING("Today", data.forecast[0].dayName.c_str());
    TEST_ASSERT_EQUAL_STRING("Tmrw", data.forecast[1].dayName.c_str());
    TEST_ASSERT_EQUAL_STRING("Sat", data.forecast[2].dayName.c_str());
}

void test_weather_client_owm_behind_utc_evening_issue_45(void) {
    // Location: America/Los_Angeles (PDT, UTC-7). Local time: Wednesday 2026-10-07 20:00:00 PDT.
    // UTC time: Thursday 2026-10-08 03:00:00.
    // dt: 1791428400 (Wed 20:00 PDT), dt_txt: "2026-10-08 03:00:00" (Thu in UTC).
    // Box 0 must be "Today", Box 1 "Tmrw", Box 2 "Fri".
    settings.setTimezone("America/Los_Angeles");
    time_t wednesday_evening_pdt = 1791428400; // 2026-10-07 20:00 PDT

    const char* owmJson = "{"
        "\"list\":["
            "{\"dt\":1791428400,\"dt_txt\":\"2026-10-08 03:00:00\",\"main\":{\"temp\":68.0,\"temp_min\":55.0,\"temp_max\":75.0,\"humidity\":50},\"weather\":[{\"id\":800,\"description\":\"clear sky\"}],\"wind\":{\"speed\":5.0,\"deg\":180}},"
            "{\"dt\":1791514800,\"dt_txt\":\"2026-10-09 03:00:00\",\"main\":{\"temp\":70.0,\"temp_min\":56.0,\"temp_max\":76.0,\"humidity\":50},\"weather\":[{\"id\":800,\"description\":\"clear sky\"}],\"wind\":{\"speed\":5.0,\"deg\":180}},"
            "{\"dt\":1791601200,\"dt_txt\":\"2026-10-10 03:00:00\",\"main\":{\"temp\":72.0,\"temp_min\":57.0,\"temp_max\":77.0,\"humidity\":50},\"weather\":[{\"id\":800,\"description\":\"clear sky\"}],\"wind\":{\"speed\":5.0,\"deg\":180}}"
        "],"
        "\"city\":{\"name\":\"Los Angeles\",\"timezone\":-25200}"
    "}";

    WeatherClient client;
    WeatherData data = {};
    bool parsed = client.parseOwmJson(owmJson, data, wednesday_evening_pdt);
    TEST_ASSERT_TRUE(parsed);
    TEST_ASSERT_EQUAL_STRING("Today", data.forecast[0].dayName.c_str());
    TEST_ASSERT_EQUAL_STRING("Tmrw", data.forecast[1].dayName.c_str());
    TEST_ASSERT_EQUAL_STRING("Fri", data.forecast[2].dayName.c_str());
}

void test_weather_client_owm_yesterday_entry_skipped(void) {
    // If the list starts with an entry from yesterday (e.g. at 01:00 AM local,
    // current 3-hour period started at 22:00 yesterday), Box 0 must skip yesterday
    // and anchor to Today.
    settings.setTimezone("America/Los_Angeles");
    time_t thursday_01am_pdt = 1791446400; // 2026-10-08 01:00 PDT = 2026-10-08 08:00 UTC

    const char* owmJson = "{"
        "\"list\":["
            "{\"dt\":1791435600,\"dt_txt\":\"2026-10-08 05:00:00\",\"main\":{\"temp\":58.0,\"temp_min\":55.0,\"temp_max\":60.0,\"humidity\":70},\"weather\":[{\"id\":800,\"description\":\"clear sky\"}],\"wind\":{\"speed\":3.0,\"deg\":180}}," // 2026-10-07 22:00 PDT (yesterday)
            "{\"dt\":1791446400,\"dt_txt\":\"2026-10-08 08:00:00\",\"main\":{\"temp\":65.0,\"temp_min\":55.0,\"temp_max\":75.0,\"humidity\":60},\"weather\":[{\"id\":800,\"description\":\"clear sky\"}],\"wind\":{\"speed\":4.0,\"deg\":180}}," // 2026-10-08 01:00 PDT (today)
            "{\"dt\":1791532800,\"dt_txt\":\"2026-10-09 08:00:00\",\"main\":{\"temp\":68.0,\"temp_min\":56.0,\"temp_max\":76.0,\"humidity\":55},\"weather\":[{\"id\":800,\"description\":\"clear sky\"}],\"wind\":{\"speed\":4.0,\"deg\":180}}," // 2026-10-09 01:00 PDT (tomorrow)
            "{\"dt\":1791619200,\"dt_txt\":\"2026-10-10 08:00:00\",\"main\":{\"temp\":70.0,\"temp_min\":57.0,\"temp_max\":77.0,\"humidity\":50},\"weather\":[{\"id\":800,\"description\":\"clear sky\"}],\"wind\":{\"speed\":4.0,\"deg\":180}}"  // 2026-10-10 01:00 PDT (Sat)
        "],"
        "\"city\":{\"name\":\"Los Angeles\",\"timezone\":-25200}"
    "}";

    WeatherClient client;
    WeatherData data = {};
    bool parsed = client.parseOwmJson(owmJson, data, thursday_01am_pdt);
    TEST_ASSERT_TRUE(parsed);
    TEST_ASSERT_EQUAL_STRING("Today", data.forecast[0].dayName.c_str());
    TEST_ASSERT_EQUAL_STRING("Tmrw", data.forecast[1].dayName.c_str());
    TEST_ASSERT_EQUAL_STRING("Sat", data.forecast[2].dayName.c_str());
}

void test_get_local_date_string_from_timestamp_direct(void) {
    char date_str[11] = "";
    int local_hour = -1;

    // 1791428400 = 2026-10-08 03:00:00 UTC
    // America/Los_Angeles (PDT, UTC-7) -> 2026-10-07 20:00:00
    settings.setTimezone("America/Los_Angeles");
    bool ok = getLocalDateStringFromTimestamp(1791428400, date_str, sizeof(date_str), &local_hour);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_STRING("2026-10-07", date_str);
    TEST_ASSERT_EQUAL_INT(20, local_hour);

    // Asia/Tokyo (JST, UTC+9) -> 2026-10-08 12:00:00
    settings.setTimezone("Asia/Tokyo");
    ok = getLocalDateStringFromTimestamp(1791428400, date_str, sizeof(date_str), &local_hour);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_STRING("2026-10-08", date_str);
    TEST_ASSERT_EQUAL_INT(12, local_hour);
}

void test_weather_client_owm_late_evening_today_passed(void) {
    // Wednesday 2026-10-07 23:30 PDT (Thursday 2026-10-08 06:30 UTC).
    // Local time in LA is Wednesday night, 30 min before midnight.
    // OWM forecast intervals for Wednesday have all passed; list starts at Thursday 08:00 UTC (01:00 PDT).
    settings.setTimezone("America/Los_Angeles");
    time_t wednesday_late_pdt = 1791441000; // 2026-10-08 06:30 UTC = 2026-10-07 23:30 PDT

    const char* owmJson = "{"
        "\"list\":["
            "{\"dt\":1791446400,\"dt_txt\":\"2026-10-08 08:00:00\",\"main\":{\"temp\":62.0,\"temp_min\":58.0,\"temp_max\":72.0,\"humidity\":65},\"weather\":[{\"id\":800,\"description\":\"clear sky\"}],\"wind\":{\"speed\":3.0,\"deg\":180}}," // 2026-10-08 01:00 PDT (Thursday / Tomorrow)
            "{\"dt\":1791532800,\"dt_txt\":\"2026-10-09 08:00:00\",\"main\":{\"temp\":65.0,\"temp_min\":59.0,\"temp_max\":75.0,\"humidity\":60},\"weather\":[{\"id\":800,\"description\":\"clear sky\"}],\"wind\":{\"speed\":4.0,\"deg\":180}}," // 2026-10-09 01:00 PDT (Friday / Day 2)
            "{\"dt\":1791619200,\"dt_txt\":\"2026-10-10 08:00:00\",\"main\":{\"temp\":68.0,\"temp_min\":60.0,\"temp_max\":77.0,\"humidity\":55},\"weather\":[{\"id\":800,\"description\":\"clear sky\"}],\"wind\":{\"speed\":4.0,\"deg\":180}}"  // 2026-10-10 01:00 PDT (Saturday)
        "],"
        "\"city\":{\"name\":\"Los Angeles\",\"timezone\":-25200}"
    "}";

    WeatherClient client;
    WeatherData data = {};
    bool parsed = client.parseOwmJson(owmJson, data, wednesday_late_pdt);
    TEST_ASSERT_TRUE(parsed);
    TEST_ASSERT_EQUAL_STRING("Today", data.forecast[0].dayName.c_str());
    TEST_ASSERT_EQUAL_STRING("Tmrw", data.forecast[1].dayName.c_str());
    TEST_ASSERT_EQUAL_STRING("Fri", data.forecast[2].dayName.c_str());
    TEST_ASSERT_EQUAL_FLOAT(72.0f, data.forecast[1].tempMax);
    TEST_ASSERT_EQUAL_FLOAT(58.0f, data.forecast[1].tempMin);
}

void test_weather_client_owm_device_tz_utc_with_city_tz_offset(void) {
    // Device settings timezone is default "UTC", but city is Los Angeles (PDT, -25200).
    // UTC time is Thursday 2026-10-08 03:00 UTC.
    // In LA, local time is Wednesday 2026-10-07 20:00 PDT.
    // Box 0 must be "Today" (Wednesday in LA), Box 1 "Tmrw" (Thursday), Box 2 "Fri".
    settings.setTimezone("UTC");
    time_t wednesday_evening_pdt_in_utc = 1791428400; // 2026-10-08 03:00 UTC = 2026-10-07 20:00 PDT

    const char* owmJson = "{"
        "\"list\":["
            "{\"dt\":1791428400,\"dt_txt\":\"2026-10-08 03:00:00\",\"main\":{\"temp\":68.0,\"temp_min\":55.0,\"temp_max\":75.0,\"humidity\":50},\"weather\":[{\"id\":800,\"description\":\"clear sky\"}],\"wind\":{\"speed\":5.0,\"deg\":180}}," // Wed 20:00 PDT (Today)
            "{\"dt\":1791514800,\"dt_txt\":\"2026-10-09 03:00:00\",\"main\":{\"temp\":70.0,\"temp_min\":56.0,\"temp_max\":76.0,\"humidity\":50},\"weather\":[{\"id\":800,\"description\":\"clear sky\"}],\"wind\":{\"speed\":5.0,\"deg\":180}}," // Thu 20:00 PDT (Tomorrow)
            "{\"dt\":1791601200,\"dt_txt\":\"2026-10-10 03:00:00\",\"main\":{\"temp\":72.0,\"temp_min\":57.0,\"temp_max\":77.0,\"humidity\":50},\"weather\":[{\"id\":800,\"description\":\"clear sky\"}],\"wind\":{\"speed\":5.0,\"deg\":180}}"  // Fri 20:00 PDT (Day 2)
        "],"
        "\"city\":{\"name\":\"Los Angeles\",\"timezone\":-25200}"
    "}";

    WeatherClient client;
    WeatherData data = {};
    bool parsed = client.parseOwmJson(owmJson, data, wednesday_evening_pdt_in_utc);
    TEST_ASSERT_TRUE(parsed);
    TEST_ASSERT_EQUAL_STRING("Today", data.forecast[0].dayName.c_str());
    TEST_ASSERT_EQUAL_STRING("Tmrw", data.forecast[1].dayName.c_str());
    TEST_ASSERT_EQUAL_STRING("Fri", data.forecast[2].dayName.c_str());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_weather_client_initialization);
    RUN_TEST(test_weather_client_get_weather_desc);
    RUN_TEST(test_weather_client_fetch_weather_native_mock);
    RUN_TEST(test_weather_client_zip_code_initialization);
    RUN_TEST(test_weather_client_parse_weather_json_custom);
    RUN_TEST(test_weather_client_parse_metoffice_hourly_json);
    RUN_TEST(test_weather_client_metoffice_descriptions_use_api_codes);
    RUN_TEST(test_weather_client_parse_bpf_coverage_json);
    RUN_TEST(test_weather_client_metoffice_humidity_falls_back_to_nearest_hour);
    RUN_TEST(test_weather_client_parse_metoffice_daily_json);
    RUN_TEST(test_weather_client_parse_thingspeak_temperature_json);
    RUN_TEST(test_weather_client_parse_owm_json);
    RUN_TEST(test_weather_client_parse_ip_location_json);
    RUN_TEST(test_weather_client_parse_ip_location_json_fail);
    RUN_TEST(test_weather_client_is_location_empty);
    RUN_TEST(test_weather_client_parse_weather_json_hourly);
    RUN_TEST(test_weather_client_parse_weather_json_hourly_sliding_window);
    RUN_TEST(test_weather_client_parse_weather_json_hourly_subhour_current_time);
    RUN_TEST(test_weather_client_parse_weather_json_forecast_hours_24);
    RUN_TEST(test_weather_client_parse_owm_json_hourly);
    RUN_TEST(test_weather_client_parse_owm_json_city_override);
    RUN_TEST(test_weather_client_all_desc_codes);
    RUN_TEST(test_weather_client_owm_to_wmo_code_via_json);
    RUN_TEST(test_weather_client_parsing_errors);
    RUN_TEST(test_weather_client_forecast_day_labels_utc_rollover_issue_45);
    RUN_TEST(test_weather_client_forecast_day_labels_owm_utc_rollover_issue_45);
    RUN_TEST(test_weather_client_forecast_day_labels_ahead_of_utc);
    RUN_TEST(test_weather_client_forecast_day_labels_uninitialized_ntp_fallback);
    RUN_TEST(test_get_local_date_strings_direct);
    RUN_TEST(test_weather_client_owm_ahead_of_utc_morning_issue_45);
    RUN_TEST(test_weather_client_owm_behind_utc_evening_issue_45);
    RUN_TEST(test_weather_client_owm_yesterday_entry_skipped);
    RUN_TEST(test_get_local_date_string_from_timestamp_direct);
    RUN_TEST(test_weather_client_owm_late_evening_today_passed);
    RUN_TEST(test_weather_client_owm_device_tz_utc_with_city_tz_offset);
    return UNITY_END();
}
