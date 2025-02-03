#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

// Размер буфера для хранения ответа сервера
#define BUFFER_SIZE 65536

// Функция обратного вызова для записи данных от сервера
size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    char *buffer = (char *)userdata;
    size_t len = size * nmemb;
    if (len >= BUFFER_SIZE) {
        fprintf(stderr, "Response too large to handle.\n");
        return 0;
    }
    strncpy(buffer, ptr, len);
    buffer[len] = '\0';
    return len;
}

// Функция для загрузки данных с помощью libcurl
int fetch_weather_data(const char *city, char *buffer, size_t buffer_size) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize libcurl\n");
        return -1;
    }

    char url[256];
    int url_len = snprintf(url, sizeof(url), "http://wttr.in/%s?format=j1", city);
    if (url_len >= sizeof(url)) {
        fprintf(stderr, "URL is too long.\n");
        curl_easy_cleanup(curl);
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        fprintf(stderr, "Failed to fetch weather data for %s: %s\n", city, curl_easy_strerror(res));
        return -1;
    }

    return 0;
}

// Функция для обработки JSON-ответа
int parse_weather_data(const char *city, const char *json_data) {
    if (!json_data || json_data[0] == '\0') {
        printf("No weather data available for %s.\n", city);
        return -1;
    }

    cJSON *root = cJSON_Parse(json_data);
    if (!root) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr) {
            fprintf(stderr, "Error parsing JSON for %s: %s\n", city, error_ptr);
        } else {
            printf("Invalid JSON response for %s.\n", city);
        }
        return -1;
    }

    cJSON *current_condition = cJSON_GetObjectItemCaseSensitive(root, "current_condition");
    if (!cJSON_IsArray(current_condition)) {
        printf("No current weather data available for %s.\n", city);
        cJSON_Delete(root);
        return -1;
    }

    cJSON *condition = cJSON_GetArrayItem(current_condition, 0);
    if (!condition) {
        printf("Failed to retrieve current weather condition for %s.\n", city);
        cJSON_Delete(root);
        return -1;
    }

    const char *weather_desc = cJSON_GetObjectItemCaseSensitive(condition, "weatherDesc")->valuestring;
    const char *wind_dir = cJSON_GetObjectItemCaseSensitive(condition, "winddir16Point")->valuestring;
    const char *wind_speed = cJSON_GetObjectItemCaseSensitive(condition, "windspeedKmph")->valuestring;
    const char *temp_c = cJSON_GetObjectItemCaseSensitive(condition, "temp_C")->valuestring;

    printf("\nWeather in %s:\n", city);
    printf("  Weather Description: %s\n", weather_desc ? weather_desc : "N/A");
    printf("  Wind Direction: %s\n", wind_dir ? wind_dir : "N/A");
    printf("  Wind Speed: %s km/h\n", wind_speed ? wind_speed : "N/A");
    printf("  Temperature: %s°C\n", temp_c ? temp_c : "N/A");

    cJSON_Delete(root);
    return 0;
}

// Главная функция
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <city>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char buffer[BUFFER_SIZE];
    const char *city = argv[1];
    size_t city_len = strlen(city);

    if (city_len == 0) {
        fprintf(stderr, "City name cannot be empty.\n");
        return EXIT_FAILURE;
    }

    if (fetch_weather_data(city, buffer, BUFFER_SIZE) != 0) {
        return EXIT_FAILURE;
    }

    if (parse_weather_data(city, buffer) != 0) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}