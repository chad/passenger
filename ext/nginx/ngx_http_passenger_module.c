/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) 2007 Manlio Perillo (manlio.perillo@gmail.com)
 * Copyright (C) 2008 Phusion
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <nginx.h>

#include "ngx_http_passenger_module.h"
#include "Configuration.h"
#include "ContentHandler.h"


static ngx_str_t  ngx_http_scgi_script_name =
    ngx_string("scgi_script_name");


static ngx_int_t
ngx_http_passenger_post_config_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt        *h;
    ngx_http_core_main_conf_t  *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    /* Register ngx_http_passenger_handler as a default content handler. */

    h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    *h = ngx_http_passenger_handler;
    
    return NGX_OK;
}

static ngx_int_t
ngx_http_scgi_script_name_variable(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    u_char                         *p;
    ngx_http_passenger_loc_conf_t  *slcf;

    if (r->uri.len) {
        v->valid = 1;
        v->no_cacheable = 0;
        v->not_found = 0;

        slcf = ngx_http_get_module_loc_conf(r, ngx_http_passenger_module);

        if (r->uri.data[r->uri.len - 1] != '/') {
            v->len = r->uri.len;
            v->data = r->uri.data;
            return NGX_OK;
        }

        v->len = r->uri.len + slcf->index.len;

        v->data = ngx_palloc(r->pool, v->len);
        if (v->data == NULL) {
            return NGX_ERROR;
        }

        p = ngx_copy(v->data, r->uri.data, r->uri.len);
        ngx_memcpy(p, slcf->index.data, slcf->index.len);

    } else {
        v->len = 0;
        v->valid = 1;
        v->no_cacheable = 0;
        v->not_found = 0;
        v->data = NULL;

        return NGX_OK;
    }

    return NGX_OK;
}

static ngx_int_t
ngx_http_passenger_add_variables(ngx_conf_t *cf)
{
    ngx_http_variable_t  *var;

    var = ngx_http_add_variable(cf, &ngx_http_scgi_script_name,
                                NGX_HTTP_VAR_NOHASH|NGX_HTTP_VAR_NOCACHEABLE);
    if (var == NULL) {
        return NGX_ERROR;
    }

    var->get_handler = ngx_http_scgi_script_name_variable;

    return NGX_OK;
}


static ngx_http_module_t  ngx_http_passenger_module_ctx = {
    ngx_http_passenger_add_variables,    /* preconfiguration */
    ngx_http_passenger_post_config_init, /* postconfiguration */

    NULL,                                /* create main configuration */
    NULL,                                /* init main configuration */

    NULL,                                /* create server configuration */
    NULL,                                /* merge server configuration */

    ngx_http_passenger_create_loc_conf,  /* create location configuration */
    ngx_http_passenger_merge_loc_conf    /* merge location configuration */
};


ngx_module_t  ngx_http_passenger_module = {
    NGX_MODULE_V1,
    &ngx_http_passenger_module_ctx,                  /* module context */
    (ngx_command_t *) ngx_http_passenger_commands,   /* module directives */
    NGX_HTTP_MODULE,                                 /* module type */
    NULL,                                            /* init master */
    NULL,                                            /* init module */
    NULL,                                            /* init process */
    NULL,                                            /* init thread */
    NULL,                                            /* exit thread */
    NULL,                                            /* exit process */
    NULL,                                            /* exit master */
    NGX_MODULE_V1_PADDING
};