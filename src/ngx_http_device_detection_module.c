#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#include <yaml.h>


#define NGX_HTTP_D14N_YAML_LEVEL_BRANDS 0
#define NGX_HTTP_D14N_YAML_LEVEL_BRAND  1
#define NGX_HTTP_D14N_YAML_LEVEL_MODELS 2
#define NGX_HTTP_D14N_YAML_LEVEL_MODEL  3


typedef struct {
    ngx_flag_t  loaded;
    ngx_str_t   yaml;
} ngx_http_d14n_conf_t;

typedef struct {
    ngx_uint_t  brands;
    ngx_uint_t  models;
} ngx_http_d14n_regex_counts_t;


static ngx_int_t ngx_http_d14n_loaded_variable(ngx_http_request_t *r, ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_http_d14n_type_variable(ngx_http_request_t *r, ngx_http_variable_value_t *v, uintptr_t data);

static ngx_int_t ngx_http_d14n_add_variables(ngx_conf_t *cf);
static void *ngx_http_d14n_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_d14n_merge_loc_conf(ngx_conf_t *cf,void *parent, void *child);
static char *ngx_http_d14n_yaml_value(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_int_t ngx_http_d14n_yaml_load(ngx_conf_t *cf, ngx_http_d14n_conf_t *lcf);
static ngx_int_t ngx_http_d14n_yaml_count_regexes(char *yaml_file, ngx_http_d14n_regex_counts_t *counts);


static ngx_command_t  ngx_http_d14n_commands[] = {
    { ngx_string("device_detection_yaml"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_http_d14n_yaml_value,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_d14n_conf_t, yaml),
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

    { ngx_string("device_detection_type"),
      NULL,
      ngx_http_d14n_type_variable,
      0, 0, 0 },

    { ngx_null_string, NULL, NULL, 0, 0, 0 }
};


static ngx_int_t
ngx_http_d14n_loaded_variable(ngx_http_request_t *r, ngx_http_variable_value_t *v, uintptr_t data)
{
    size_t                            len;
    const char                       *val;
    ngx_http_d14n_conf_t *lcf;

    lcf = ngx_http_get_module_loc_conf(r, ngx_http_device_detection_module);

    if (1 == lcf->loaded) {
        val = "on";
    } else {
        val = "off";
    }

    len     = ngx_strlen(val);
    v->data = ngx_pnalloc(r->pool, len);

    if (NULL == v->data) {
        return NGX_ERROR;
    }

    ngx_memcpy(v->data, val, len);

    v->len          = len;
    v->valid        = 1;
    v->no_cacheable = 0;
    v->not_found    = 0;

    return NGX_OK;
}


static ngx_int_t
ngx_http_d14n_type_variable(ngx_http_request_t *r, ngx_http_variable_value_t *v, uintptr_t data)
{
    const char *val = "undetected";
    size_t      len = ngx_strlen(val);

    v->data = ngx_pnalloc(r->pool, len);

    if (NULL == v->data) {
        return NGX_ERROR;
    }

    ngx_memcpy(v->data, val, len);

    v->len          = len;
    v->valid        = 1;
    v->no_cacheable = 0;
    v->not_found    = 0;

    return NGX_OK;
}


static ngx_int_t
ngx_http_d14n_add_variables(ngx_conf_t *cf)
{
    ngx_http_variable_t  *var, *v;

    for (v = ngx_http_d14n_vars; v->name.len; v++) {
        var = ngx_http_add_variable(cf, &v->name, v->flags);

        if (NULL == var) {
            return NGX_ERROR;
        }

        var->get_handler = v->get_handler;
        var->data        = v->data;
    }

    return NGX_OK;
}


static void *
ngx_http_d14n_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_d14n_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_d14n_conf_t));

    if (NULL == conf) {
        return NULL;
    }

    conf->loaded = NGX_CONF_UNSET;

    return conf;
}


static char *
ngx_http_d14n_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_d14n_conf_t *prev = parent;
    ngx_http_d14n_conf_t *conf = child;

    ngx_conf_merge_off_value(conf->loaded, prev->loaded, 0);
    ngx_conf_merge_str_value(conf->yaml, prev->yaml, "");

    return NGX_CONF_OK;
}


static char *
ngx_http_d14n_yaml_value(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_str_t *value;

    ngx_http_d14n_conf_t *lcf = conf;

    if (lcf->yaml.data) {
        return "is duplicate";
    }

    value       = cf->args->elts;
    lcf->yaml   = value[1];
    lcf->loaded = (NGX_OK == ngx_http_d14n_yaml_load(cf, lcf));

    if (!lcf->loaded) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "Failed to load yaml file: '%s'", (char *) lcf->yaml.data);
        return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_d14n_yaml_load(ngx_conf_t *cf, ngx_http_d14n_conf_t *lcf)
{
    ngx_http_d14n_regex_counts_t *counts = ngx_pnalloc(cf->pool, sizeof(ngx_http_d14n_regex_counts_t));

    ngx_conf_log_error(NGX_LOG_DEBUG, cf, 0, "device detection yaml loading: '%s': ", (char *) lcf->yaml.data);

    if (NGX_OK != ngx_http_d14n_yaml_count_regexes((char *) lcf->yaml.data, counts)) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "failed to parse counts from yaml file!");
        return NGX_ERROR;
    }

    ngx_conf_log_error(NGX_LOG_DEBUG, cf, 0, "found %d brands with %d models", counts->brands, counts->models);

    return NGX_OK;
}

static ngx_int_t
ngx_http_d14n_yaml_count_regexes(char *yaml_file, ngx_http_d14n_regex_counts_t *counts)
{
    int error        = 0;
    int parser_level = NGX_HTTP_D14N_YAML_LEVEL_BRANDS;

    FILE          *file;
    yaml_event_t   event;
    yaml_parser_t  parser;

    file = fopen(yaml_file, "rb");

    if (!file) {
        return NGX_ERROR;
    }

    counts->brands = 0;
    counts->models = 0;

    yaml_parser_initialize(&parser);
    yaml_parser_set_input_file(&parser, file);

    do {
        if (!yaml_parser_parse(&parser, &event)) {
            error = 1;
            break;
        }

        switch (event.type) {

            // level modification (and counting)
            case YAML_SEQUENCE_START_EVENT:
            case YAML_MAPPING_START_EVENT:
                if (NGX_HTTP_D14N_YAML_LEVEL_BRAND == parser_level) {
                    counts->brands++;
                }

                parser_level++;
                break;

            case YAML_SEQUENCE_END_EVENT:
            case YAML_MAPPING_END_EVENT:
                parser_level--;
                break;

            case YAML_SCALAR_EVENT:
                if (0 == ngx_strcmp(event.data.scalar.value, "model")) {
                    counts->models++;
                }
                break;

            // ignore events
            case YAML_ALIAS_EVENT:
            case YAML_DOCUMENT_END_EVENT:
            case YAML_DOCUMENT_START_EVENT:
            case YAML_NO_EVENT:
            case YAML_STREAM_END_EVENT:
            case YAML_STREAM_START_EVENT:
            default:
                break;
        }

        if (YAML_STREAM_END_EVENT != event.type) {
            yaml_event_delete(&event);
        }

        if (0 > parser_level) {
            error = 1;
            break;
        }
    } while (YAML_STREAM_END_EVENT != event.type);

    yaml_event_delete(&event);
    yaml_parser_delete(&parser);
    fclose(file);

    if (1 == error) {
        return NGX_ERROR;
    }

    return NGX_OK;
}
