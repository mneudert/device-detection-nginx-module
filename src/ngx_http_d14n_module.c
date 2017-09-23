#include "./ngx_http_d14n_module.h"


static ngx_int_t
ngx_http_d14n_loaded_variable(ngx_http_request_t *r,
                              ngx_http_variable_value_t *v,
                              uintptr_t data) {
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
ngx_http_d14n_model_variable(ngx_http_request_t *r,
                             ngx_http_variable_value_t *v,
                             uintptr_t data) {
  ngx_http_d14n_conf_t *lcf;
  ngx_uint_t            brand, model;
  size_t                len;
  int                   re_result;
  char                 *val = "undetected";

  ngx_http_d14n_brand_t **brands;
  ngx_http_d14n_model_t **models;

  lcf = ngx_http_get_module_loc_conf(r, ngx_http_device_detection_module);

  if (1 == lcf->loaded
        && r->headers_in.user_agent->value.len
        && lcf->brands->nelts
  ) {
    ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
                  "matching '%s' against %d brands",
                  r->headers_in.user_agent->value.data, lcf->brands->nelts);

    brands = lcf->brands->elts;

    for (brand = 0; brand < lcf->brands->nelts; ++brand) {
      re_result = ngx_regex_exec(brands[brand]->regex,
                                 &r->headers_in.user_agent->value,
                                 NULL, 0);

      if (NGX_REGEX_NO_MATCHED == re_result) {
        continue;
      }

      ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
                    "matched brand: %s",
                    brands[brand]->name.data);

      if (brands[brand]->model_default.len) {
        val = (char *) brands[brand]->model_default.data;
      }

      if (!brands[brand]->models->nelts) {
        break;
      }

      models = brands[brand]->models->elts;

      for (model = 0; model < brands[brand]->models->nelts; ++model) {
        re_result = ngx_regex_exec(models[model]->regex,
                                   &r->headers_in.user_agent->value,
                                   NULL, 0);

        if (NGX_REGEX_NO_MATCHED == re_result) {
          continue;
        }

        ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
                      "matched model: %s",
                      models[model]->model.data);

        if (models[model]->model.len) {
          val = (char *) models[model]->model.data;
        }
      }

      break;
    }
  }

  if (0 == ngx_strcmp(val, "undetected") && lcf->miss_log) {
    ngx_write_fd(lcf->miss_log->fd,
                 r->headers_in.user_agent->value.data,
                 r->headers_in.user_agent->value.len);
    ngx_write_fd(lcf->miss_log->fd, "\n", 1);
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
ngx_http_d14n_type_variable(ngx_http_request_t *r,
                            ngx_http_variable_value_t *v,
                            uintptr_t data) {
  ngx_http_d14n_conf_t *lcf;
  ngx_uint_t            brand, model;
  size_t                len;
  int                   re_result;
  char                 *val = "undetected";

  ngx_http_d14n_brand_t **brands;
  ngx_http_d14n_model_t **models;

  lcf = ngx_http_get_module_loc_conf(r, ngx_http_device_detection_module);

  if (1 == lcf->loaded
        && r->headers_in.user_agent->value.len
        && lcf->brands->nelts
  ) {
    ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
                  "matching '%s' against %d brands",
                  r->headers_in.user_agent->value.data,
                  lcf->brands->nelts);

    brands = lcf->brands->elts;

    for (brand = 0; brand < lcf->brands->nelts; ++brand) {
      re_result = ngx_regex_exec(brands[brand]->regex,
                                 &r->headers_in.user_agent->value,
                                 NULL, 0);

      if (NGX_REGEX_NO_MATCHED == re_result) {
        continue;
      }

      ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
                    "matched brand: %s",
                    brands[brand]->name.data);

      if (brands[brand]->device_default.len) {
        val = (char *) brands[brand]->device_default.data;
      }

      if (!brands[brand]->models->nelts) {
        break;
      }

      models = brands[brand]->models->elts;

      for (model = 0; model < brands[brand]->models->nelts; ++model) {
        re_result = ngx_regex_exec(models[model]->regex,
                                   &r->headers_in.user_agent->value,
                                   NULL, 0);

        if (NGX_REGEX_NO_MATCHED == re_result) {
          continue;
        }

        ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
                      "matched model: %s",
                      models[model]->model.data);

        if (models[model]->device.len) {
          val = (char *) models[model]->device.data;
        }
      }

      break;
    }
  }

  if (0 == ngx_strcmp(val, "undetected") && lcf->miss_log) {
    ngx_write_fd(lcf->miss_log->fd,
                 r->headers_in.user_agent->value.data,
                 r->headers_in.user_agent->value.len);
    ngx_write_fd(lcf->miss_log->fd, "\n", 1);
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
ngx_http_d14n_add_variables(ngx_conf_t *cf) {
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
ngx_http_d14n_create_loc_conf(ngx_conf_t *cf) {
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
ngx_http_d14n_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child) {
  ngx_http_d14n_conf_t *prev = parent;
  ngx_http_d14n_conf_t *conf = child;

  ngx_conf_merge_off_value(conf->loaded, prev->loaded, 0);

  if (prev->miss_log) {
    conf->miss_log = prev->miss_log;
  }

  if (prev->brands->nelts) {
    conf->brands = prev->brands;
  }

  return NGX_CONF_OK;
}


static char *
ngx_http_d14n_miss_log_value(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
  ngx_str_t            *value;
  ngx_http_d14n_conf_t *lcf = conf;

  if (lcf->miss_log) {
    if (NGX_FILE_ERROR == ngx_close_file(lcf->miss_log->fd)) {
      ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                         "failed to close miss_log file!");
      return NGX_CONF_ERROR;
    }
  }

  value         = cf->args->elts;
  lcf->miss_log = ngx_pcalloc(cf->pool, sizeof(ngx_open_file_t));
  lcf->miss_log = ngx_conf_open_file(cf->cycle, &value[1]);

  if (!lcf->miss_log->fd) {
    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "failed to open miss_log file!");
    return NGX_CONF_ERROR;
  }

  return NGX_CONF_OK;
}


static char *
ngx_http_d14n_yaml_value(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
  ngx_str_t            *value;
  ngx_http_d14n_conf_t *lcf = conf;

  value       = cf->args->elts;
  lcf->loaded = (NGX_OK == ngx_http_d14n_yaml_load(cf, lcf, &value[1]));

  if (!lcf->loaded) {
    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                       "failed to load yaml file: '%s'",
                       value[1].data);
    return NGX_CONF_ERROR;
  }

  return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_d14n_yaml_load(ngx_conf_t *cf,
                        ngx_http_d14n_conf_t *lcf,
                        ngx_str_t *yaml) {
  ngx_conf_log_error(NGX_LOG_DEBUG, cf, 0,
                     "loading device detection yaml file: '%s'",
                     yaml->data);

  if (NGX_OK != ngx_http_d14n_yaml_count_brands(cf, lcf, yaml)) {
    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                       "failed to parse brand count from yaml file!");
    return NGX_ERROR;
  }

  if (NGX_OK != ngx_http_d14n_yaml_parse_brands(cf, lcf, yaml)) {
    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                       "failed to parse brands from yaml file!");
    return NGX_ERROR;
  }

  return NGX_OK;
}


static ngx_int_t
ngx_http_d14n_yaml_count_brands(ngx_conf_t *cf,
                                ngx_http_d14n_conf_t *lcf,
                                ngx_str_t *yaml) {
  FILE          *file;
  yaml_event_t   event;
  yaml_parser_t  parser;

  ngx_uint_t brands       = 0;
  int        error        = 0;
  int        parser_level = NGX_HTTP_D14N_YAML_LEVEL_BRANDS;

  file = fopen((char *) yaml->data, "rb");

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

      // ignore other types
      default: break;
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
ngx_http_d14n_yaml_parse_brands(ngx_conf_t *cf,
                                ngx_http_d14n_conf_t *lcf,
                                ngx_str_t *yaml) {
  FILE          *file;
  yaml_event_t   event;
  yaml_parser_t  parser;

  int error        = 0;
  int parser_level = NGX_HTTP_D14N_YAML_LEVEL_ROOT;
  int current_key  = NGX_HTTP_D14N_YAML_KEY_NONE;

  u_char                  regex_err[NGX_MAX_CONF_ERRSTR];
  ngx_regex_compile_t     regex_c;
  ngx_uint_t              brand;
  ngx_http_d14n_brand_t **brands = lcf->brands->elts;

  for (brand = 0;
       brand < lcf->brands->nelts && NULL != brands[brand];
       ++brand) {}

  file = fopen((char *) yaml->data, "rb");

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

      // ignore other types
      default: break;
    }

    if (YAML_SCALAR_EVENT == event.type
          && NGX_HTTP_D14N_YAML_LEVEL_BRANDS == parser_level
          && NULL == brands[brand]
    ) {
      brands[brand] = ngx_pcalloc(cf->pool, sizeof(ngx_http_d14n_brand_t));

      brands[brand]->name.len  = event.data.scalar.length;
      brands[brand]->name.data = ngx_pcalloc(cf->pool,
                                             event.data.scalar.length + 1);

      brands[brand]->models = ngx_array_create(cf->pool, 0,
                                               sizeof(ngx_http_d14n_model_t));

      ngx_memcpy(brands[brand]->name.data,
                 event.data.scalar.value,
                 event.data.scalar.length);
      ngx_conf_log_error(NGX_LOG_DEBUG, cf, 0,
                         "parsing brand: '%s'",
                         brands[brand]->name.data);

      if (NGX_OK != ngx_http_d14n_yaml_count_models(cf, lcf,
                                                    brands[brand],
                                                    yaml)) {
        error = 1;
        break;
      }

      if (NGX_OK != ngx_http_d14n_yaml_parse_models(cf, lcf,
                                                    brands[brand],
                                                    yaml)) {
        error = 1;
        break;
      }

      continue;
    }

    if (YAML_SCALAR_EVENT == event.type
          && NGX_HTTP_D14N_YAML_LEVEL_BRAND == parser_level
    ) {
      switch (current_key) {
        case NGX_HTTP_D14N_YAML_KEY_DEVICE:
          brands[brand]->device_default.len  = event.data.scalar.length;
          brands[brand]->device_default.data = ngx_pcalloc(
            cf->pool, event.data.scalar.length + 1);

          ngx_memcpy(brands[brand]->device_default.data,
                     event.data.scalar.value,
                     event.data.scalar.length);
          break;

        case NGX_HTTP_D14N_YAML_KEY_MODEL:
          brands[brand]->model_default.len  = event.data.scalar.length + 1;
          brands[brand]->model_default.data = ngx_pcalloc(
            cf->pool, event.data.scalar.length + 1);

          ngx_memcpy(brands[brand]->model_default.data,
                     event.data.scalar.value,
                     event.data.scalar.length);
          break;

        case NGX_HTTP_D14N_YAML_KEY_REGEX:
          ngx_memzero(&regex_c, sizeof(ngx_regex_compile_t));

          regex_c.pool         = cf->pool;
          regex_c.err.len      = NGX_MAX_CONF_ERRSTR;
          regex_c.err.data     = regex_err;
          regex_c.pattern.len  = event.data.scalar.length;
          regex_c.pattern.data = event.data.scalar.value;

          if (NGX_OK != ngx_regex_compile(&regex_c)) {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "brand regex error: %V",
                               regex_err);
            error = 1;
          } else {
            brands[brand]->regex = regex_c.regex;
          }

          break;
      }

      current_key = ngx_http_d14n_yaml_key((char *) event.data.scalar.value);
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
ngx_http_d14n_yaml_count_models(ngx_conf_t *cf,
                                ngx_http_d14n_conf_t *lcf,
                                ngx_http_d14n_brand_t *brand,
                                ngx_str_t *yaml) {
  FILE          *file;
  yaml_event_t   event;
  yaml_parser_t  parser;

  int        error        = 0;
  int        in_brand     = 0;
  int        parser_level = NGX_HTTP_D14N_YAML_LEVEL_BRANDS;
  ngx_uint_t models       = 0;

  file = fopen((char *) yaml->data, "rb");

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
      // level modification
      case YAML_SEQUENCE_START_EVENT:
      case YAML_MAPPING_START_EVENT:
        parser_level++;
        break;

      case YAML_MAPPING_END_EVENT:
      case YAML_SEQUENCE_END_EVENT:
        parser_level--;
        break;

      // ignore other types
      default: break;
    }

    if (in_brand
          && YAML_MAPPING_END_EVENT == event.type
          && NGX_HTTP_D14N_YAML_LEVEL_MODEL == parser_level
    ) {
      models++;
    }

    if (YAML_SCALAR_EVENT == event.type
          && NGX_HTTP_D14N_YAML_LEVEL_BRAND == parser_level
    ) {
      in_brand = (0 == ngx_strcmp(event.data.scalar.value, brand->name.data));
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
ngx_http_d14n_yaml_parse_models(ngx_conf_t *cf,
                                ngx_http_d14n_conf_t *lcf,
                                ngx_http_d14n_brand_t *brand,
                                ngx_str_t *yaml) {
  if (!brand->models->nelts) {
    // no models!
    return NGX_OK;
  }

  FILE          *file;
  yaml_event_t   event;
  yaml_parser_t  parser;

  int    error        = 0;
  int    in_brand     = 0;
  int    parser_level = NGX_HTTP_D14N_YAML_LEVEL_ROOT;
  int    current_key  = NGX_HTTP_D14N_YAML_KEY_NONE;
  ngx_uint_t model    = 0;

  u_char                  regex_err[NGX_MAX_CONF_ERRSTR];
  ngx_regex_compile_t     regex_c;
  ngx_http_d14n_model_t **models = brand->models->elts;

  file = fopen((char *) yaml->data, "rb");

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

      // ignore other types
      default: break;
    }

    if (in_brand
          && YAML_MAPPING_END_EVENT == event.type
          && NGX_HTTP_D14N_YAML_LEVEL_MODELS == parser_level
    ) {
      model++;
    }

    if (YAML_SCALAR_EVENT == event.type
          && NGX_HTTP_D14N_YAML_LEVEL_BRANDS == parser_level) {
      in_brand = (0 == ngx_strcmp(event.data.scalar.value, brand->name.data));
      continue;
    }

    if (in_brand && YAML_SCALAR_EVENT == event.type
          && NGX_HTTP_D14N_YAML_LEVEL_MODEL == parser_level
    ) {
      if (NULL == models[model]) {
        models[model] = ngx_pcalloc(cf->pool, sizeof(ngx_http_d14n_model_t));
      }

      switch (current_key) {
        case NGX_HTTP_D14N_YAML_KEY_DEVICE:
          models[model]->device.len  = event.data.scalar.length;
          models[model]->device.data = ngx_pcalloc(
            cf->pool, event.data.scalar.length + 1);

          ngx_memcpy(models[model]->device.data,
                     event.data.scalar.value,
                     event.data.scalar.length);
          break;

        case NGX_HTTP_D14N_YAML_KEY_MODEL:
          models[model]->model.len  = event.data.scalar.length;
          models[model]->model.data = ngx_pcalloc(
            cf->pool, event.data.scalar.length + 1);

          ngx_memcpy(models[model]->model.data,
                     event.data.scalar.value,
                     event.data.scalar.length);
          break;

        case NGX_HTTP_D14N_YAML_KEY_REGEX:
          ngx_memzero(&regex_c, sizeof(ngx_regex_compile_t));

          regex_c.pool         = cf->pool;
          regex_c.err.len      = NGX_MAX_CONF_ERRSTR;
          regex_c.err.data     = regex_err;
          regex_c.pattern.len  = event.data.scalar.length;
          regex_c.pattern.data = event.data.scalar.value;

          if (NGX_OK != ngx_regex_compile(&regex_c)) {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "model regex error: %V",
                               regex_err);
            error = 1;
          } else {
            models[model]->regex = regex_c.regex;
          }

          break;
      }

      current_key = ngx_http_d14n_yaml_key((char *) event.data.scalar.value);
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


static int
ngx_http_d14n_yaml_key(char *key_value) {
  if (0 == ngx_strcmp(key_value, "device")) {
    return NGX_HTTP_D14N_YAML_KEY_DEVICE;
  }

  if (0 == ngx_strcmp(key_value, "model")) {
    return NGX_HTTP_D14N_YAML_KEY_MODEL;
  }

  if (0 == ngx_strcmp(key_value, "regex")) {
    return NGX_HTTP_D14N_YAML_KEY_REGEX;
  }

  return NGX_HTTP_D14N_YAML_KEY_NONE;
}
