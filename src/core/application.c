/**
 * Copyright (C) 2017 Quenza Inc.
 * All Rights Reserved
 *
 * This file is part of the Ayahesa core.
 *
 * Content can not be copied and/or distributed without the express
 * permission of the author.
 */

#include "../include/ayahesa.h"
#include "util.h"
#include "ini.h"

#include <assert.h>
#include <time.h>

void application_create(app_t **);
void application_config(app_t *, const char *);
void application_env(app_t *);
void application_bootstrap(app_t *);
void application_release(app_t *);
void application_prelude(struct connection *);
void application_postproc(struct connection *c);

void database_init(void);

struct {
    time_t boot;
    unsigned int req_count;
    unsigned int conn_active;
} internal_counter;

extern struct aya_providers providers[];
static FILE *log_fp;

/*
 * Bootstrap the application
 */
void
application_create(app_t **app)
{
    assert(*app == NULL);

    /* Seed random pool */
    srand(time(NULL));

    /* Reset counters */
    internal_counter.boot = time(NULL);
    internal_counter.req_count = 0;
    internal_counter.conn_active = 0;

    /* Setup application root tree */
    app_t *root = (app_t *)aya_calloc(1, sizeof(app_t));
    root->value.str = generate_instance_id();
    root->type = T_STRING;
    root->flags = 0;
    root->key = NULL;
    tree_new_root(root);

    /* Config tree */
    root->child.ptr[TREE_CONFIG] = (app_t *)aya_calloc(1, sizeof(app_t));
    root->child.ptr[TREE_CONFIG]->flags = T_FLAG_READONLY;
    root->child.ptr[TREE_CONFIG]->type = T_NULL;
    root->child.ptr[TREE_CONFIG]->key = NULL;
    tree_new_root(root->child.ptr[TREE_CONFIG]);

    /* Cache tree */
    root->child.ptr[TREE_CACHE] = (app_t *)aya_calloc(1, sizeof(app_t));
    root->child.ptr[TREE_CACHE]->flags = 0;
    root->child.ptr[TREE_CACHE]->type = T_NULL;
    root->child.ptr[TREE_CACHE]->key = NULL;
    tree_new_root(root->child.ptr[TREE_CACHE]);

    kore_log(LOG_NOTICE, "application instance %s", root->value.str);

    *app = root; 
}

/*
 * Prepare for new connection
 */
void
application_prelude(struct connection *c)
{
    internal_counter.req_count++;
    internal_counter.conn_active++;
}

/*
 * Post process connection
 */
void
application_postproc(struct connection *c)
{
    internal_counter.conn_active--;
}

/*
 * Parse INI file and load key,value into config tree
 */
static int //TODO: handle empty section
load_config_tree(void *user, const char *section, const char *name, const char *value, int lineno)
{
    app_t *app = (app_t *)user;

    /* Config item must have string value */
    if (!strcmp(section, "app")) {
        if (!strlen(value)) {
            printf("Config item in '[app]' cannot be empty (line:%d)\n", lineno);
            abort();
        }
    }

    /* Redefine debug */
    if (!strcmp(name, "debug")) {
        if (!strcmp(value, "true") || !strcmp(value, "1")) {
            config_put_int(app, "debug", 1);
            kore_log(LOG_NOTICE, "application in debug mode");
        } else {
            config_put_int(app, "debug", 0);
        }
        return 1;
    }

    /* Redefine session time */
    if (!strcmp(section, "app") && !strcmp(name, "session")) {
        int err;
        int time = kore_strtonum(value, 10, 60, 86400, &err);
        config_put_int(app, "app.session", time);
        return 1;
    }

    /* Set instance name */
    if (!strcmp(name, "instance")) {
        aya_free(root_app->value.str);
        root_app->value.str = kore_strdup(value);
        kore_log(LOG_NOTICE, "application instance %s", root_app->value.str);
        return 1;
    }

    /* Prefix section */
    size_t szc = strlen(section);
    if (szc) {
        size_t keylen = szc + 1 + strlen(name) + 1;
        char *key = (char *)aya_malloc(keylen);
        aya_strlcpy(key, section, keylen);
        aya_strlcat(key, ".", keylen);
        aya_strlcat(key, name, keylen);

        config_put_str(app, key, value);
        aya_free(key);
        return 1;
    }

    /* Append to tree */
    config_put_str(app, name, value);
    return 1;
}

static void
validate_config(app_t *app)
{
    int has_name = tree_contains(app->child.ptr[TREE_CONFIG], "app.name");
    int has_env = tree_contains(app->child.ptr[TREE_CONFIG], "app.env");
    int has_key = tree_contains(app->child.ptr[TREE_CONFIG], "app.key");
    int has_debug = tree_contains(app->child.ptr[TREE_CONFIG], "debug");

    /* No app_name */
    if (!has_name) {
        kore_log(LOG_ERR, "'app.name' must be set");
        abort();
    }

    /* No app_env */
    if (!has_env) {
        kore_log(LOG_ERR, "'app.env' must be set");
        abort();
    }

    /* No app_key */
    if (!has_key) {
        kore_log(LOG_ERR, "'app.key' must be set");
        abort();
    }

    /* Set debug to false if not found */
    if (!has_debug)
        config_put_int(app, "debug", 0);
}

/*
 * Parse application configuration
 */
void
application_config(app_t *app, const char *configfile)
{
    char logfile[255];

    assert(app != NULL);

    if (ini_parse(configfile, load_config_tree, (void *)app) < 0) {
        kore_log(LOG_ERR, "framework configuration error %s", configfile);
        abort();
    }

    /* Check and default config */
    validate_config(app);

    /* Open file for logging operations */
    snprintf(logfile, 255, "%s/ayahesa.log", app_storage());
    log_fp = fopen(logfile, "a+");
}

/*
 * Fetch config from environment
 */
void
application_env(app_t *app)
{
    int i;
    extern char **environ;

    /* Loop all environment variables */
    char *env = *environ;
    for (i = 1; env; ++i) {
        if (!strncmp("AYA_", env, 4)) {
            char *base = env + 4;
            char *section = strchr(base, '_');
            char *end = strchr(base, '=');
            if (!end) {
                env = *(environ + i);
                continue;
            }

            end[0] = '\0';
            
            char *final_section = "";
            if (section) {
                section[0] = '\0';
                final_section = strtolower(base);
                base = section + 1;
            }

            char *final_name = strtolower(base);
            load_config_tree((void *)app, final_section, final_name, end + 1, i);
        }
        env = *(environ + i);
    }
}

/*
 * Bootstrap additional components
 */
void
application_bootstrap(app_t *app)
{
    int i;

    /* Initialize components */
    database_init();

    /* Call load hooks */
    for (i = 0; strlen(providers[i].module) > 0 ; ++i) {
            if (providers[i].cb_load == NULL)
                continue;

        kore_log(LOG_NOTICE, "boostrap %s", providers[i].module);
        providers[i].cb_load();
    }
}

/*
 * Release application
 */
void
application_release(app_t *app)
{
    int i;

    /* Is already closed */
    if (app == NULL)
        return;

    /* Call unload hooks */
    for (i = 0; strlen(providers[i].module) > 0 ; ++i) {
            if (providers[i].cb_unload == NULL)
                continue;

        kore_log(LOG_NOTICE, "unload %s", providers[i].module);
        providers[i].cb_unload();
    }

    fclose(log_fp);

    /* Traverse tree and free pointers */
    tree_free(app);

    aya_free(app);
    app = NULL;
}

//////////////////////////////////////////////////
// Public functions                             //
//////////////////////////////////////////////////

/*
 * Log action
 */
void
app_log(const char *message, ...)
{
    time_t timer;
    char buffer[32];
    struct tm* tm_info;
    va_list arglist;

    if (log_fp) {
        time(&timer);
        tm_info = localtime(&timer);
        strftime(buffer, 32, "%Y-%m-%d %H:%M:%S", tm_info);

        fputs("[", log_fp);
        fputs(buffer, log_fp);
        fputs("] ", log_fp);

        va_start(arglist, message);
        vfprintf(log_fp, message, arglist);
        va_end(arglist);

        fputs("\n", log_fp);
    }
}

/*
 * Application uptime
 */
char *
app_instance(void)
{
    return root_app->value.str;
}

/*
 * Application uptime
 */
char *
app_uptime(void)
{
    int diff_tm;
    time_t end_tm = 0;
    static char uptime[32];

    time(&end_tm);
    diff_tm = difftime(end_tm, internal_counter.boot);

    int days = diff_tm / 86400;
    int remainder = diff_tm % 86400;
    int hours = remainder / 3600;
    remainder = diff_tm % 3600;
    int minutes = remainder / 60;
    int seconds = remainder % 60;

    sprintf(uptime, "%d days, %02d:%02d:%02d\n", days, hours, minutes, seconds);
    return uptime;
}

/*
 * Application request counter
 */
unsigned int
app_request_count(void)
{
    return internal_counter.req_count;
}

/*
 * Application active connection counter
 */
unsigned int
app_active_conncount(void)
{
    return internal_counter.conn_active;
}

/*
 * Application debug mode check
 */
int
app_isdebug(void)
{
    int debug = 0;
    config_get_int(root_app, "debug", &debug);

    return debug;
}

/*
 * Application name
 */
char *
app_name(void)
{
    char *app_name = NULL;
    config_get_str(root_app, "app.name", &app_name);

    return app_name;
}

/*
 * Application environment
 */
char *
app_environment(void)
{
    char *app_env = NULL;
    config_get_str(root_app, "app.env", &app_env);

    return app_env;
}

/*
 * Application session lifetime in seconds
 */
int
app_session_lifetime(void)
{
    int app_session_lifetime = 0;
    config_get_int(root_app, "app.session", &app_session_lifetime);

    return app_session_lifetime;
}

/*
 * Application key
 */
char *
app_key(void)
{
    char *app_key = NULL;
    config_get_str(root_app, "app.key", &app_key);

    return app_key;
}

/*
 * Application storage location
 */
char *
app_storage(void)
{
    char *storage = NULL;
    config_get_str(root_app, "storage", &storage);

    return storage;
}

/*
 * Application basic authentication username and password
 */
char *
app_basic_auth(void)
{
    char *bais_auth = NULL;
    config_get_str(root_app, "auth", &bais_auth);

    return bais_auth;
}

/*
 * Application domainname
 */
const char *
app_domainname(void)
{
    static char domainname[128];
    char *app_domain = NULL;
    config_get_str(root_app, "app.domain", &app_domain);

    if (app_domain == NULL)
        return NULL;

    kore_strlcpy(domainname, app_instance(), 128);
    if (app_domain[0] != '.')
        strcat(domainname,  ".");
    aya_strlcat(domainname, app_domain, 128);

    return strtolower(domainname);
}
