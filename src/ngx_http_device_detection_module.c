#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


static ngx_int_t ngx_http_device_detection_init(ngx_conf_t *cf);


static ngx_command_t  ngx_http_device_detection_commands[] = {
    ngx_null_command
};


static ngx_http_module_t  ngx_http_device_detection_module_ctx = {
    NULL,                            /* preconfiguration */
    ngx_http_device_detection_init,  /* postconfiguration */

    NULL,                            /* create main configuration */
    NULL,                            /* init main configuration */

    NULL,                            /* create server configuration */
    NULL,                            /* merge server configuration */

    NULL,                            /* create location configuration */
    NULL                             /* merge location configuration */
};


ngx_module_t  ngx_http_device_detection_module = {
    NGX_MODULE_V1,
    &ngx_http_device_detection_module_ctx,  /* module context */
    ngx_http_device_detection_commands,     /* module directives */
    NGX_HTTP_MODULE,                        /* module type */

    NULL,                                   /* init master */
    NULL,                                   /* init module */
    NULL,                                   /* init process */
    NULL,                                   /* init thread */

    NULL,                                   /* exit thread */
    NULL,                                   /* exit process */
    NULL,                                   /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_int_t
ngx_http_device_detection_init(ngx_conf_t *cf)
{
    return NGX_OK;
}
