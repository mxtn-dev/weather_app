#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

// Размер буфера для хранения ответа сервера
#define BUFFER_SIZE 65536
#define MAX_CITY_NAME_LENGTH 128

// Функция обратного вызова для записи данных от сервера
size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    if (!userdata || !ptr) return 0;
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
    if (!city || strlen(city) == 0) {
        fprintf(stderr, "City name is empty or invalid.\n");
        return -1;
    }

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize libcurl\n");
        return -1;
    }

    // Проверка формирования URL
    char url[256];
    if (snprintf(url, sizeof(url), "http://wttr.in/%s?format=j1", city) >= sizeof(url)) {
        fprintf(stderr, "URL is too long.\n");
        curl_easy_cleanup(curl);
        return -1;
    }

    // Настройка libcurl
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); // Таймаут в 10 секунд
    curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, buffer_size);

    // Выполнение запроса
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "Failed to fetch weather data for %s: %s\n", city, curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return -1;
    }

    curl_easy_cleanup(curl);
    return 0;
}

// Функция для обработки JSON-ответа
void parse_weather_data(const char *city, const char *json_data) {
    if (!json_data || json_data[0] == '\0') {
        printf("No weather data available for %s.\n", city);
        return;
    }

    cJSON *root = cJSON_Parse(json_data);
    if (!root) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr) {
            fprintf(stderr, "Error parsing JSON for %s: %s\n", city, error_ptr);
        } else {
            printf("Invalid JSON response for %s.\n", city);
        }
        return;
    }

    cJSON *current_condition = cJSON_GetObjectItemCaseSensitive(root, "current_condition");
    if (!cJSON_IsArray(current_condition)) {
        printf("No current weather data available for %s.\n", city);
        cJSON_Delete(root);
        return;
    }

    cJSON *condition = cJSON_GetArrayItem(current_condition, 0);
    if (!condition) {
        printf("Failed to retrieve current weather condition for %s.\n", city);
        cJSON_Delete(root);
        return;
    }

    // Извлечение данных из JSON
    const char *weather_desc = cJSON_GetObjectItemCaseSensitive(condition, "weatherDesc")->valuestring;
    const char *wind_dir = cJSON_GetObjectItemCaseSensitive(condition, "winddir16Point")->valuestring;
    const char *wind_speed = cJSON_GetObjectItemCaseSensitive(condition, "windspeedKmph")->valuestring;
    const char *temp_c = cJSON_GetObjectItemCaseSensitive(condition, "temp_C")->valuestring;

    // Вывод данных
    printf("\nWeather in %s:\n", city);
    printf("  Weather Description: %s\n", weather_desc ? weather_desc : "N/A");
    printf("  Wind Direction: %s\n", wind_dir ? wind_dir : "N/A");
    printf("  Wind Speed: %s km/h\n", wind_speed ? wind_speed : "N/A");
    printf("  Temperature: %s°C\n", temp_c ? temp_c : "N/A");

    cJSON_Delete(root);
}

// Главная функция
int main() {
    char buffer[BUFFER_SIZE];
    char city_input[MAX_CITY_NAME_LENGTH];

    // Запрос названия города у пользователя
    printf("Погода в каком городе вас интересует? ");
    if (fgets(city_input, sizeof(city_input), stdin) == NULL) {
        fprintf(stderr, "Ошибка чтения ввода.\n");
        return EXIT_FAILURE;
    }

    // Удаление символа новой строки (\n) из ввода
    city_input[strcspn(city_input, "\n")] = '\0';

    // Проверка длины названия города
    if (strlen(city_input) == 0 || strlen(city_input) >= MAX_CITY_NAME_LENGTH) {
        fprintf(stderr, "Название города должно быть от 1 до %d символов.\n", MAX_CITY_NAME_LENGTH - 1);
        return EXIT_FAILURE;
    }

    // Загрузка данных о погоде
    if (fetch_weather_data(city_input, buffer, BUFFER_SIZE) == 0) {
        parse_weather_data(city_input, buffer);
    } else {
        printf("Failed to fetch weather data for %s.\n", city_input);
    }

    return EXIT_SUCCESS;
}