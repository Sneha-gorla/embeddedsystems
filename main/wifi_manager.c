#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_http_server.h"

#define WIFI1_SSID "G16 ES"
#define WIFI1_PASS "nointernet"

#define WIFI2_SSID "Kasu"
#define WIFI2_PASS "kameswari"

static const char *TAG = "WIFI";

// Connect Function
void connect_wifi(const char *ssid, const char *pass)
{
    wifi_config_t wifi_config = {0};

    strcpy((char *)wifi_config.sta.ssid, ssid);
    strcpy((char *)wifi_config.sta.password, pass);

    esp_wifi_disconnect();
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_connect();

    ESP_LOGI(TAG, "Switching to %s", ssid);
}

// Handlers
static esp_err_t root_get_handler(httpd_req_t *req)
{
    const char *resp =
        "<html><body>"
        "<h1>ESP32 WiFi Manager</h1>"
        "<button onclick=\"location.href='/wifi1'\">Connect WiFi 1</button><br><br>"
        "<button onclick=\"location.href='/wifi2'\">Connect WiFi 2</button>"
        "</body></html>";

    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t wifi1_handler(httpd_req_t *req)
{
    connect_wifi(WIFI1_SSID, WIFI1_PASS);
    httpd_resp_send(req, "Switching to WiFi 1", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t wifi2_handler(httpd_req_t *req)
{
    connect_wifi(WIFI2_SSID, WIFI2_PASS);
    httpd_resp_send(req, "Switching to WiFi 2", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Web Server
httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_uri_t root = {
            .uri = "/", .method = HTTP_GET, .handler = root_get_handler};
        httpd_register_uri_handler(server, &root);

        httpd_uri_t wifi1 = {
            .uri = "/wifi1", .method = HTTP_GET, .handler = wifi1_handler};
        httpd_register_uri_handler(server, &wifi1);

        httpd_uri_t wifi2 = {
            .uri = "/wifi2", .method = HTTP_GET, .handler = wifi2_handler};
        httpd_register_uri_handler(server, &wifi2);
    }

    return server;
}

// WiFi Init
void wifi_init(void)
{
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
}

// Main
void app_main(void)
{
    nvs_flash_init();
    wifi_init();

    connect_wifi(WIFI1_SSID, WIFI1_PASS);

    vTaskDelay(5000 / portTICK_PERIOD_MS);

    start_webserver();
    ESP_LOGI(TAG, "Web server started");
}