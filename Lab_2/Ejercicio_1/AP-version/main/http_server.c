#include "http_server.h"
#include "control_motores.h"
#include "esp_log.h"

#include "esp_http_server.h"
static const char *TAG = "HTTP_SERVER";
static httpd_handle_t server = NULL;

// HTML embebido
extern const char index_html_start[] asm("_binary_index_html_start");
extern const char index_html_end[]   asm("_binary_index_html_end");

esp_err_t root_handler(httpd_req_t *req)
{
    size_t len = index_html_end - index_html_start;
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, index_html_start, len);
    return ESP_OK;
}

esp_err_t forward_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "FORWARD command received");
    move_forward();
    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
}

esp_err_t right_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "RIGHT command received");
    move_right();
    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
}

esp_err_t left_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "LEFT command received");
    move_left();
    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
}

esp_err_t backward_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "BACKWARD command received");
    move_backward();
    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
}


esp_err_t stop_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "STOP command received"); 
    stop();
    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
}

httpd_uri_t root = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = root_handler
};

httpd_uri_t forward = {
    .uri = "/forward",
    .method = HTTP_GET,
    .handler = forward_handler
};

httpd_uri_t right = {
    .uri = "/right",
    .method = HTTP_GET,
    .handler = right_handler
};

httpd_uri_t left = {
    .uri = "/left",
    .method = HTTP_GET,
    .handler = left_handler
};

httpd_uri_t backward = {
    .uri = "/backward",
    .method = HTTP_GET,
    .handler = backward_handler
};

httpd_uri_t stop_uri = {
    .uri = "/stop",
    .method = HTTP_GET,
    .handler = stop_handler
};

void start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &root);
        httpd_register_uri_handler(server, &forward);
        httpd_register_uri_handler(server, &stop_uri);
        httpd_register_uri_handler(server, &right);
        httpd_register_uri_handler(server, &left);
        httpd_register_uri_handler(server, &backward);
    }
}