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
    ngx_str_t            name;
    ngx_regex_compile_t  regex;
    ngx_str_t            device_default;
    ngx_array_t         *models;
} ngx_http_d14n_brand_t;

typedef struct {
    ngx_str_t            model;
    ngx_regex_compile_t  regex;
    ngx_str_t            device;
} ngx_http_d14n_model_t;

typedef struct {
    ngx_flag_t    loaded;
    ngx_str_t     yaml;
    ngx_array_t  *brands;
} ngx_http_d14n_conf_t;


static ngx_int_t ngx_http_d14n_loaded_variable(ngx_http_request_t *r, ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_http_d14n_type_variable(ngx_http_request_t *r, ngx_http_variable_value_t *v, uintptr_t data);

static ngx_int_t ngx_http_d14n_add_variables(ngx_conf_t *cf);
static void *ngx_http_d14n_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_d14n_merge_loc_conf(ngx_conf_t *cf,void *parent, void *child);
static char *ngx_http_d14n_yaml_value(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_int_t ngx_http_d14n_yaml_load(ngx_conf_t *cf, ngx_http_d14n_conf_t *lcf);
static ngx_int_t ngx_http_d14n_yaml_count_brands(ngx_conf_t *cf, ngx_http_d14n_conf_t *lcf);
static ngx_int_t ngx_http_d14n_yaml_parse_brands(ngx_conf_t *cf, ngx_http_d14n_conf_t *lcf);
static ngx_int_t ngx_http_d14n_yaml_count_models(ngx_conf_t *cf, ngx_http_d14n_conf_t *lcf, ngx_http_d14n_brand_t *brand);
static ngx_int_t ngx_http_d14n_yaml_parse_models(ngx_conf_t *cf, ngx_http_d14n_conf_t *lcf, ngx_http_d14n_brand_t *brand);


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
    ngx_http_d14n_conf_t *lcf;
    size_t                len;
    const char           *val;

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
    ngx_http_d14n_conf_t *lcf;
    ngx_uint_t            brand;
    size_t                len;
    char                 *val = "undetected";

    ngx_http_d14n_brand_t **brands;

    lcf = ngx_http_get_module_loc_conf(r, ngx_http_device_detection_module);

    if (1 == lcf->loaded && r->headers_in.user_agent->value.len && lcf->brands->nelts) {
        ngx_log_error(
            NGX_LOG_DEBUG, r->connection->log, 0,
            "matching '%s' againts %d brands",
            r->headers_in.user_agent->value.data, lcf->brands->nelts
        );

        brands = lcf->brands->elts;

        for (brand = 0; brand < lcf->brands->nelts; ++brand) {
            if (!brands[brand]->regex.pattern.len) {
                continue;
            }

            if (NGX_REGEX_NO_MATCHED == ngx_regex_exec(brands[brand]->regex.regex, &r->headers_in.user_agent->value, NULL, 0)) {
                continue;
            }

            ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0, "matched brand: %s", brands[brand]->name.data);

            if (brands[brand]->device_default.len) {
                val = brands[brand]->device_default.data;
            }

            break;
        }
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
ngx_http_d14n_add_variables(ngx_conf_t *cf)
{
    ngx_http_variable_t  *var, *v;

    for (v = ngx_http_d14n_vars; v->name.len; ++v) {
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
    conf->brands = ngx_array_create(cf->pool, 0, sizeof(ngx_http_d14n_brand_t));

    return conf;
}


static char *
ngx_http_d14n_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_d14n_conf_t *prev = parent;
    ngx_http_d14n_conf_t *conf = child;

    ngx_conf_merge_off_value(conf->loaded, prev->loaded, 0);
    ngx_conf_merge_str_value(conf->yaml, prev->yaml, "");

    if (prev->brands->nelts) {
        conf->brands = prev->brands;
    }

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
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "failed to load yaml file: '%s'", (char *) lcf->yaml.data);
        return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_d14n_yaml_load(ngx_conf_t *cf, ngx_http_d14n_conf_t *lcf)
{
    ngx_conf_log_error(NGX_LOG_DEBUG, cf, 0, "device detection yaml loading: '%s': ", (char *) lcf->yaml.data);

    if (NGX_OK != ngx_http_d14n_yaml_count_brands(cf, lcf)) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "failed to parse brand count from yaml file!");
        return NGX_ERROR;
    }

    if (NGX_OK != ngx_http_d14n_yaml_parse_brands(cf, lcf)) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "failed to parse brands count from yaml file!");
        return NGX_ERROR;
    }

    return NGX_OK;
}


static ngx_int_t
ngx_http_d14n_yaml_count_brands(ngx_conf_t *cf, ngx_http_d14n_conf_t *lcf)
{
    FILE          *file;
    yaml_event_t   event;
    yaml_parser_t  parser;

    int        error        = 0;
    int        parser_level = NGX_HTTP_D14N_YAML_LEVEL_BRANDS;
    ngx_uint_t brands       = 0;

    file = fopen((char *) lcf->yaml.data, "rb");

    if (!file) {
        return NGX_ERROR;
    }

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
                    brands++;
                }

                parser_level++;
                break;

            case YAML_SEQUENCE_END_EVENT:
            case YAML_MAPPING_END_EVENT:
                parser_level--;
                break;

            // ignore events
            case YAML_ALIAS_EVENT:
            case YAML_DOCUMENT_END_EVENT:
            case YAML_DOCUMENT_START_EVENT:
            case YAML_NO_EVENT:
            case YAML_SCALAR_EVENT:
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

    ngx_conf_log_error(NGX_LOG_DEBUG, cf, 0, "found %d brands", brands);
    ngx_array_push_n(lcf->brands, brands);

    return NGX_OK;
}


static ngx_int_t
ngx_http_d14n_yaml_parse_brands(ngx_conf_t *cf, ngx_http_d14n_conf_t *lcf)
{
    FILE          *file;
    yaml_event_t   event;
    yaml_parser_t  parser;

    int        error        = 0;
    int        parser_level = NGX_HTTP_D14N_YAML_LEVEL_ROOT;
    int        current_key  = NGX_HTTP_D14N_YAML_KEY_NONE;
    ngx_uint_t brand        = 0;

    ngx_http_d14n_brand_t **brands = lcf->brands->elts;

    file = fopen((char *) lcf->yaml.data, "rb");

    if (!file) {
        return NGX_ERROR;
    }

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
                parser_level++;
                break;

            case YAML_SEQUENCE_END_EVENT:
            case YAML_MAPPING_END_EVENT:
                parser_level--;

                if (NGX_HTTP_D14N_YAML_LEVEL_BRANDS == parser_level) {
                    brand++;
                }

                break;

            // ignore events
            case YAML_SCALAR_EVENT:
            case YAML_ALIAS_EVENT:
            case YAML_DOCUMENT_END_EVENT:
            case YAML_DOCUMENT_START_EVENT:
            case YAML_NO_EVENT:
            case YAML_STREAM_END_EVENT:
            case YAML_STREAM_START_EVENT:
            default:
                break;
        }

        if (YAML_SCALAR_EVENT == event.type && NGX_HTTP_D14N_YAML_LEVEL_BRANDS == parser_level) {
            if (NULL == brands[brand]) {
                brands[brand]            = ngx_pcalloc(cf->pool, sizeof(ngx_http_d14n_brand_t) + 1);
                brands[brand]->name.len  = event.data.scalar.length + 1;
                brands[brand]->name.data = ngx_pcalloc(cf->pool, event.data.scalar.length + 1);
                brands[brand]->models    = ngx_array_create(cf->pool, 0, sizeof(ngx_http_d14n_model_t));

                ngx_memcpy(brands[brand]->name.data, event.data.scalar.value, event.data.scalar.length);
                ngx_conf_log_error(NGX_LOG_DEBUG, cf, 0, "parsing brand: '%s'", brands[brand]->name.data);

                if (NGX_OK != ngx_http_d14n_yaml_count_models(cf, lcf, brands[brand])) {
                    error = 1;
                    break;
                }

                if (NGX_OK != ngx_http_d14n_yaml_parse_models(cf, lcf, brands[brand])) {
                    error = 1;
                    break;
                }

                continue;
            }
        }

        if (YAML_SCALAR_EVENT == event.type && NGX_HTTP_D14N_YAML_LEVEL_BRAND == parser_level) {
            switch (current_key) {
                case NGX_HTTP_D14N_YAML_KEY_NONE:
                    if (0 == ngx_strcmp(event.data.scalar.value, "device")) {
                        current_key = NGX_HTTP_D14N_YAML_KEY_DEVICE;
                    }
                    if (0 == ngx_strcmp(event.data.scalar.value, "regex")) {
                        current_key = NGX_HTTP_D14N_YAML_KEY_REGEX;
                    }
                    break;

                case NGX_HTTP_D14N_YAML_KEY_DEVICE:
                    current_key                        = NGX_HTTP_D14N_YAML_KEY_NONE;
                    brands[brand]->device_default.len  = event.data.scalar.length + 1;
                    brands[brand]->device_default.data = ngx_pcalloc(cf->pool, event.data.scalar.length + 1);

                    ngx_memcpy(brands[brand]->device_default.data, event.data.scalar.value, event.data.scalar.length);
                    break;

                case NGX_HTTP_D14N_YAML_KEY_REGEX:
                    current_key                       = NGX_HTTP_D14N_YAML_KEY_NONE;
                    brands[brand]->regex.pool         = cf->pool;
                    brands[brand]->regex.err.len      = NGX_MAX_CONF_ERRSTR;
                    brands[brand]->regex.err.data     = ngx_pcalloc(cf->pool, NGX_MAX_CONF_ERRSTR);
                    brands[brand]->regex.pattern.len  = event.data.scalar.length + 1;
                    brands[brand]->regex.pattern.data = ngx_pcalloc(cf->pool, event.data.scalar.length + 1);

                    ngx_memcpy(brands[brand]->regex.pattern.data, event.data.scalar.value, event.data.scalar.length);

                    if (NGX_OK != ngx_regex_compile(&brands[brand]->regex)) {
                        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "brand regex error: %V", &brands[brand]->regex.err);

                        error = 1;
                    }
                    break;

                default: break;
            }
        }

        if (YAML_STREAM_END_EVENT != event.type) {
            yaml_event_delete(&event);
        }

        if (0 > parser_level) {
            error = 1;
            break;
        }

        if (1 == error) {
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


static ngx_int_t
ngx_http_d14n_yaml_count_models(ngx_conf_t *cf, ngx_http_d14n_conf_t *lcf, ngx_http_d14n_brand_t *brand)
{
    FILE          *file;
    yaml_event_t   event;
    yaml_parser_t  parser;

    int        error        = 0;
    int        in_brand     = 0;
    int        parser_level = NGX_HTTP_D14N_YAML_LEVEL_BRANDS;
    ngx_uint_t models       = 0;

    file = fopen((char *) lcf->yaml.data, "rb");

    if (!file) {
        return NGX_ERROR;
    }

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
                parser_level++;
                break;

            case YAML_SEQUENCE_END_EVENT:
            case YAML_MAPPING_END_EVENT:
                parser_level--;
                break;

            // ignore events
            case YAML_ALIAS_EVENT:
            case YAML_DOCUMENT_END_EVENT:
            case YAML_DOCUMENT_START_EVENT:
            case YAML_NO_EVENT:
            case YAML_SCALAR_EVENT:
            case YAML_STREAM_END_EVENT:
            case YAML_STREAM_START_EVENT:
            default:
                break;
        }

        if (YAML_SCALAR_EVENT == event.type && NGX_HTTP_D14N_YAML_LEVEL_BRAND == parser_level) {
            if (0 == ngx_strcmp(event.data.scalar.value, brand->name.data)) {
                in_brand = 1;
            } else {
                in_brand = 0;
            }
        }

        if (in_brand && YAML_MAPPING_END_EVENT == event.type && NGX_HTTP_D14N_YAML_LEVEL_MODEL == parser_level) {
            models++;
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

    ngx_conf_log_error(NGX_LOG_DEBUG, cf, 0, "found %d models", models);
    ngx_array_push_n(brand->models, models);

    return NGX_OK;
}


static ngx_int_t
ngx_http_d14n_yaml_parse_models(ngx_conf_t *cf, ngx_http_d14n_conf_t *lcf, ngx_http_d14n_brand_t *brand)
{
    if (!brand->models->nelts) {
        // no models!
        return NGX_OK;
    }

    FILE          *file;
    yaml_event_t   event;
    yaml_parser_t  parser;

    int        error        = 0;
    int        in_brand     = 0;
    int        parser_level = NGX_HTTP_D14N_YAML_LEVEL_ROOT;
    int        current_key  = NGX_HTTP_D14N_YAML_KEY_NONE;
    ngx_uint_t model        = 0;

    ngx_http_d14n_model_t **models = brand->models->elts;

    file = fopen((char *) lcf->yaml.data, "rb");

    if (!file) {
        return NGX_ERROR;
    }

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
                parser_level++;
                break;

            case YAML_SEQUENCE_END_EVENT:
            case YAML_MAPPING_END_EVENT:
                parser_level--;

                if (in_brand && NGX_HTTP_D14N_YAML_LEVEL_MODELS == parser_level) {
                    model++;
                }

                break;

            // ignore events
            case YAML_SCALAR_EVENT:
            case YAML_ALIAS_EVENT:
            case YAML_DOCUMENT_END_EVENT:
            case YAML_DOCUMENT_START_EVENT:
            case YAML_NO_EVENT:
            case YAML_STREAM_END_EVENT:
            case YAML_STREAM_START_EVENT:
            default:
                break;
        }

        if (YAML_SCALAR_EVENT == event.type && NGX_HTTP_D14N_YAML_LEVEL_BRANDS == parser_level) {
            if (0 == ngx_strcmp(event.data.scalar.value, brand->name.data)) {
                in_brand = 1;
            } else {
                in_brand = 0;
            }

            continue;
        }

        if (in_brand && YAML_SCALAR_EVENT == event.type && NGX_HTTP_D14N_YAML_LEVEL_MODEL == parser_level) {
            if (NULL == models[model]) {
                models[model] = ngx_pcalloc(cf->pool, sizeof(ngx_http_d14n_brand_t) + 1);
            }

            switch (current_key) {
                case NGX_HTTP_D14N_YAML_KEY_NONE:
                    if (0 == ngx_strcmp(event.data.scalar.value, "device")) {
                        current_key = NGX_HTTP_D14N_YAML_KEY_DEVICE;
                    }
                    if (0 == ngx_strcmp(event.data.scalar.value, "model")) {
                        current_key = NGX_HTTP_D14N_YAML_KEY_MODEL;
                    }
                    if (0 == ngx_strcmp(event.data.scalar.value, "regex")) {
                        current_key = NGX_HTTP_D14N_YAML_KEY_REGEX;
                    }
                    break;

                case NGX_HTTP_D14N_YAML_KEY_DEVICE:
                    current_key                = NGX_HTTP_D14N_YAML_KEY_NONE;
                    models[model]->device.len  = event.data.scalar.length + 1;
                    models[model]->device.data = ngx_pcalloc(cf->pool, event.data.scalar.length + 1);

                    ngx_memcpy(models[model]->device.data, event.data.scalar.value, event.data.scalar.length);
                    break;

                case NGX_HTTP_D14N_YAML_KEY_MODEL:
                    current_key               = NGX_HTTP_D14N_YAML_KEY_NONE;
                    models[model]->model.len  = event.data.scalar.length + 1;
                    models[model]->model.data = ngx_pcalloc(cf->pool, event.data.scalar.length + 1);

                    ngx_memcpy(models[model]->model.data, event.data.scalar.value, event.data.scalar.length);
                    break;

                case NGX_HTTP_D14N_YAML_KEY_REGEX:
                    current_key                       = NGX_HTTP_D14N_YAML_KEY_NONE;
                    models[model]->regex.pool         = cf->pool;
                    models[model]->regex.err.len      = NGX_MAX_CONF_ERRSTR;
                    models[model]->regex.err.data     = ngx_pcalloc(cf->pool, NGX_MAX_CONF_ERRSTR);
                    models[model]->regex.pattern.len  = event.data.scalar.length + 1;
                    models[model]->regex.pattern.data = ngx_pcalloc(cf->pool, event.data.scalar.length + 1);

                    ngx_memcpy(models[model]->regex.pattern.data, event.data.scalar.value, event.data.scalar.length);

                    if (NGX_OK != ngx_regex_compile(&models[model]->regex)) {
                        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "model regex error: %V", &models[model]->regex.err);

                        error = 1;
                    }
                    break;

                default: break;
            }
        }

        if (YAML_STREAM_END_EVENT != event.type) {
            yaml_event_delete(&event);
        }

        if (0 > parser_level) {
            error = 1;
            break;
        }

        if (1 == error) {
            break;
        }
    } while (YAML_STREAM_END_EVENT != event.type);

    yaml_event_delete(&event);
    yaml_parser_delete(&parser);
    fclose(file);

    if (1 == error) {
        return NGX_ERROR;
    }

    models = brand->models->elts;

    for (model = 0; model < brand->models->nelts; ++model) {
        if (models[model]->device.len) {
            ngx_conf_log_error(
                NGX_LOG_DEBUG, cf, 0,
                "parsed %s model: '%s' ( %s )",
                models[model]->device.data,
                models[model]->model.data,
                models[model]->regex.pattern.data
            );
        } else {
            ngx_conf_log_error(
                NGX_LOG_DEBUG, cf, 0,
                "parsed unknown model: '%s' ( %s )",
                models[model]->model.data,
                models[model]->regex.pattern.data
            );
        }
    }

    return NGX_OK;
}
