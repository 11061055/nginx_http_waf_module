
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#define WAF_LEVEL0 0 // not set      未设置
#define WAF_LEVEL1 1 // normal level 正常水平
#define WAF_LEVEL2 2 // urgent level 紧急水平

typedef struct {

    ngx_str_t  waf_target;

} ngx_http_waf_conf_t;

typedef struct {

    ngx_uint_t                waf_level;
    ngx_http_request_t        *sub_req;

} ngx_http_waf_ctx_t;

// waf init
static ngx_int_t ngx_http_waf_init(ngx_conf_t *cf);

// waf create conf
static void *ngx_http_waf_create_conf(ngx_conf_t *cf);

// waf merge conf
static char *ngx_http_waf_merge_conf(ngx_conf_t *cf, void *parent, void *child);

// waf start handler
static ngx_int_t ngx_http_waf_start_handler(ngx_http_request_t *r);

// waf done handler
static ngx_int_t ngx_http_waf_done_handler(ngx_http_request_t *r, void *data, ngx_int_t rc);

// waf target setter
static char *ngx_http_waf_target_setter(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_command_t  ngx_http_waf_commands[] = {

    {
      ngx_string("waf_target"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_http_waf_target_setter,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL
    },

    ngx_null_command
};


static ngx_http_module_t  ngx_http_waf_module_ctx = {
    NULL,                                  /* preconfiguration */
    ngx_http_waf_init,                     /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    ngx_http_waf_create_conf,              /* create location configuration */
    ngx_http_waf_merge_conf                /* merge location configuration */
};


ngx_module_t  ngx_http_waf_module = {
    NGX_MODULE_V1,
    &ngx_http_waf_module_ctx,              /* module context */
    ngx_http_waf_commands,                 /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};

// waf start handler
static ngx_int_t ngx_http_waf_start_handler(ngx_http_request_t *r)
{
    ngx_table_elt_t               *h, *ho;
    ngx_http_request_t            *sr;
    ngx_http_post_subrequest_t    *ps;

    ngx_http_waf_conf_t *waf_conf = ngx_http_get_module_loc_conf(r, ngx_http_waf_module);

    // there is no waf_target
    if (waf_conf->waf_target.len == 0) {
        return NGX_DECLINED;
    }

    ngx_http_waf_ctx_t *ctx = ngx_http_get_module_ctx(r, ngx_http_waf_module);

    if (ctx != NULL) {

        // waf_level not set yet
        if (WAF_LEVEL0 == ctx->waf_level) {
            return NGX_AGAIN;
        }

        // normal level
        if (WAF_LEVEL1 == ctx->waf_level) {
            return NGX_OK;
        }

        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                      " [waf urgent uri]");

        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    ctx = ngx_pcalloc(r->pool, sizeof(ngx_http_waf_ctx_t));
    if (ctx == NULL) {
        return NGX_ERROR;
    }

    ngx_http_post_subrequest_t* sub_req = ngx_palloc(r->pool, sizeof(ngx_http_post_subrequest_t));
    if (sub_req == NULL) {
        return NGX_ERROR;
    }

    sub_req->handler = ngx_http_waf_done_handler;
    sub_req->data    = ctx;

    ngx_http_request_t *req;

    if (ngx_http_subrequest(r, &waf_conf->waf_target, NULL, &req, sub_req,
                            NGX_HTTP_SUBREQUEST_WAITED)
        != NGX_OK)
    {
        return NGX_ERROR;
    }

    req->header_only = 1;

    ctx->sub_req = req;

    ngx_http_set_ctx(r, ctx, ngx_http_waf_module);

    return NGX_AGAIN;
}

static ngx_int_t ngx_http_waf_done_handler(ngx_http_request_t *r, void *data, ngx_int_t rc)
{
    ngx_http_waf_ctx_t   *ctx = data;

    // http 200 indicates normal level
    if (200 == r->headers_out.status)
    {
        ctx->waf_level = WAF_LEVEL1;
    }
    else
    {
        ctx->waf_level = WAF_LEVEL2;
    }

    return rc;
}

static void * ngx_http_waf_create_conf(ngx_conf_t *conf)
{
    ngx_http_waf_conf_t  *waf_conf;

    waf_conf = ngx_pcalloc(conf->pool, sizeof(ngx_http_waf_conf_t));

    if (waf_conf == NULL) {
        return NULL;
    }

    return waf_conf;
}

static char * ngx_http_waf_merge_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_waf_conf_t *waf_parent_conf = parent;
    ngx_http_waf_conf_t *waf_child_conf  = child;

    ngx_conf_merge_str_value(waf_child_conf->waf_target, waf_parent_conf->waf_target, "");

    return NGX_CONF_OK;
}

static ngx_int_t ngx_http_waf_init(ngx_conf_t *conf)
{
    ngx_http_core_main_conf_t* core_main_conf = ngx_http_conf_get_module_main_conf(conf, ngx_http_core_module);

    ngx_http_handler_pt* handler = ngx_array_push(&core_main_conf->phases[NGX_HTTP_ACCESS_PHASE].handlers);
    if (handler == NULL) {
        return NGX_ERROR;
    }

    *handler = ngx_http_waf_start_handler;

    return NGX_OK;
}

static char * ngx_http_waf_target_setter(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_waf_conf_t *waf_conf = conf;

    ngx_str_t* value = cf->args->elts;

    if (ngx_strcmp(value[1].data, "off") == 0) {

        waf_conf->waf_target.len = 0;
        waf_conf->waf_target.data = (u_char *) "";

        return NGX_CONF_OK;
    }

    waf_conf->waf_target = value[1];

    return NGX_CONF_OK;
}
