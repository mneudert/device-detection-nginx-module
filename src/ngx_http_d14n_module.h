#ifndef NGX_HTTP_D14N_MODULE_H_
#define NGX_HTTP_D14N_MODULE_H_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#include <yaml.h>


#define NGX_HTTP_D14N_YAML_LEVEL_ROOT   0
#define NGX_HTTP_D14N_YAML_LEVEL_BRANDS 1
#define NGX_HTTP_D14N_YAML_LEVEL_BRAND  2
#define NGX_HTTP_D14N_YAML_LEVEL_MODELS 3
#define NGX_HTTP_D14N_YAML_LEVEL_MODEL  4

#define NGX_HTTP_D14N_YAML_KEY_NONE   0
#define NGX_HTTP_D14N_YAML_KEY_DEVICE 1
#define NGX_HTTP_D14N_YAML_KEY_MODEL  2
#define NGX_HTTP_D14N_YAML_KEY_REGEX  3


typedef struct {
  ngx_str_t      name;
  ngx_regex_compile_t  regex;
  ngx_str_t      device_default;
  ngx_str_t      model_default;
  ngx_array_t     *models;
} ngx_http_d14n_brand_t;

typedef struct {
  ngx_str_t      model;
  ngx_regex_compile_t  regex;
  ngx_str_t      device;
} ngx_http_d14n_model_t;

typedef struct {
  ngx_flag_t     loaded;
  ngx_open_file_t *miss_log;
  ngx_array_t   *brands;
} ngx_http_d14n_conf_t;


static ngx_int_t ngx_http_d14n_loaded_variable(
  ngx_http_request_t *r, ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_http_d14n_model_variable(
  ngx_http_request_t *r, ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_http_d14n_type_variable(
  ngx_http_request_t *r, ngx_http_variable_value_t *v, uintptr_t data);

static ngx_int_t ngx_http_d14n_add_variables(ngx_conf_t *cf);
static void *ngx_http_d14n_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_d14n_merge_loc_conf(
  ngx_conf_t *cf, void *parent, void *child);
static char *ngx_http_d14n_miss_log_value(
  ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *ngx_http_d14n_yaml_value(
  ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_int_t ngx_http_d14n_yaml_load(
  ngx_conf_t *cf, ngx_http_d14n_conf_t *lcf, ngx_str_t *yaml);
static ngx_int_t ngx_http_d14n_yaml_count_brands(
  ngx_conf_t *cf, ngx_http_d14n_conf_t *lcf, ngx_str_t *yaml);
static ngx_int_t ngx_http_d14n_yaml_parse_brands(
  ngx_conf_t *cf, ngx_http_d14n_conf_t *lcf, ngx_str_t *yaml);
static ngx_int_t ngx_http_d14n_yaml_count_models(
  ngx_conf_t *cf, ngx_http_d14n_conf_t *lcf,
  ngx_http_d14n_brand_t *brand, ngx_str_t *yaml);
static ngx_int_t ngx_http_d14n_yaml_parse_models(
  ngx_conf_t *cf, ngx_http_d14n_conf_t *lcf,
  ngx_http_d14n_brand_t *brand, ngx_str_t *yaml);


static ngx_command_t  ngx_http_d14n_commands[] = {
  { ngx_string("device_detection_miss_log"),
    NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
    ngx_http_d14n_miss_log_value,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_d14n_conf_t, miss_log),
    NULL },

  { ngx_string("device_detection_yaml"),
    NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
    ngx_http_d14n_yaml_value,
    NGX_HTTP_LOC_CONF_OFFSET,
    0,
    NULL },

  ngx_null_command
};


static ngx_http_module_t  ngx_http_d14n_module_ctx = {
  ngx_http_d14n_add_variables,    /* preconfiguration */
  NULL,                           /* postconfiguration */

  NULL,                           /* create main configuration */
  NULL,                           /* init main configuration */

  NULL,                           /* create server configuration */
  NULL,                           /* merge server configuration */

  ngx_http_d14n_create_loc_conf,  /* create location configuration */
  ngx_http_d14n_merge_loc_conf    /* merge location configuration */
};


ngx_module_t  ngx_http_device_detection_module = {
  NGX_MODULE_V1,
  &ngx_http_d14n_module_ctx,      /* module context */
  ngx_http_d14n_commands,         /* module directives */
  NGX_HTTP_MODULE,                /* module type */

  NULL,                           /* init master */
  NULL,                           /* init module */
  NULL,                           /* init process */
  NULL,                           /* init thread */

  NULL,                           /* exit thread */
  NULL,                           /* exit process */
  NULL,                           /* exit master */
  NGX_MODULE_V1_PADDING
};


static ngx_http_variable_t  ngx_http_d14n_vars[] = {
  { ngx_string("device_detection_loaded"),
    NULL,
    ngx_http_d14n_loaded_variable,
    0, 0, 0 },

  { ngx_string("device_detection_model"),
    NULL,
    ngx_http_d14n_model_variable,
    0, 0, 0 },

  { ngx_string("device_detection_type"),
    NULL,
    ngx_http_d14n_type_variable,
    0, 0, 0 },

  { ngx_null_string, NULL, NULL, 0, 0, 0 }
};


#endif  // NGX_HTTP_D14N_MODULE_H_
