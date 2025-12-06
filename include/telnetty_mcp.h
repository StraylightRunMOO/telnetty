/**
 * @file telnetty_mcp.h
 * @brief MCP 2.1 (MUD Client Protocol) implementation
 * 
 * This header provides an implementation of the MCP 2.1 protocol as defined
 * at https://www.moo.mud.org/mcp/mcp2.html. MCP allows for structured
 * communication between MUD clients and servers.
 * 
 * Features:
 * - MCP 2.1 negotiation and authentication
 * - Package management
 * - Message routing
 * - Multi-package support
 * 
 * @author Damus <damus@straylightrun.org>
 * @version 1.0.0
 * 
 */

#ifndef TELNETTY_MCP_H
#define TELNETTY_MCP_H

#include "telnetty_core.h"
#include "telnetty_events.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Mark functions as potentially unused (header-only library) */
#ifndef TELNETTY_UNUSED
#ifdef __GNUC__
#define TELNETTY_UNUSED __attribute__((unused))
#else
#define TELNETTY_UNUSED
#endif
#endif


/* ============================================================================
 * MCP 2.1 Constants
 * ============================================================================ */

/* MCP 2.1 option code (uses private option space) */
#define TELNETTY_TELOPT_MCP 200

/* MCP message types */
typedef enum {
    TELNETTY_MCP_MESSAGE_AUTH = 0,        /**< Authentication message */
    TELNETTY_MCP_MESSAGE_NEGOTIATE = 1,   /**< Negotiation message */
    TELNETTY_MCP_MESSAGE_DATA = 2,        /**< Data message */
    TELNETTY_MCP_MESSAGE_ERROR = 3,       /**< Error message */
    TELNETTY_MCP_MESSAGE_CLOSE = 4        /**< Close message */
} telnetty_mcp_message_type_t;

/* MCP authentication states */
typedef enum {
    TELNETTY_MCP_AUTH_NONE = 0,           /**< No authentication */
    TELNETTY_MCP_AUTH_KEY = 1,            /**< Key-based authentication */
    TELNETTY_MCP_AUTH_USER = 2            /**< User/password authentication */
} telnetty_mcp_auth_type_t;

/* MCP negotiation states */
typedef enum {
    TELNETTY_MCP_STATE_DISABLED = 0,      /**< MCP is disabled */
    TELNETTY_MCP_STATE_NEGOTIATING = 1,   /**< Negotiation in progress */
    TELNETTY_MCP_STATE_ENABLED = 2,       /**< MCP is enabled */
    TELNETTY_MCP_STATE_FAILED = 3         /**< Negotiation failed */
} telnetty_mcp_state_t;

/* ============================================================================
 * MCP Package Management
 * ============================================================================ */

/* Forward declaration for MCP package handler */
struct telnetty_mcp_package;
typedef struct telnetty_mcp_package telnetty_mcp_package_t;

/* MCP package handler function type */
typedef void (*telnetty_mcp_package_handler_t)(
    telnetty_context_t* ctx,
    const char* package,
    const char* message,
    const char* data,
    void* user_data
);

/* MCP package structure */
struct telnetty_mcp_package {
    char* name;                         /**< Package name */
    char* version;                      /**< Package version */
    char* description;                  /**< Package description */
    bool active;                        /**< Package is active */
    telnetty_mcp_package_handler_t handler; /**< Package handler */
    void* user_data;                    /**< User data for handler */
    void* handler_data;                 /**< Handler-specific data */
};

/* ============================================================================
 * MCP Data Structures
 * ============================================================================ */

/* MCP authentication data */
typedef struct {
    telnetty_mcp_auth_type_t type;        /**< Authentication type */
    char* key;                          /**< Authentication key */
    char* username;                     /**< Username */
    char* password;                     /**< Password */
    bool authenticated;                 /**< Authentication successful */
} telnetty_mcp_auth_data_t;

/* MCP message structure */
typedef struct {
    char* package;                      /**< Package name */
    char* message;                      /**< Message type */
    char* data;                         /**< Message data (JSON) */
    time_t timestamp;                   /**< Message timestamp */
    int message_id;                     /**< Message ID */
} telnetty_mcp_message_t;

/* MCP main data structure */
typedef struct {
    telnetty_mcp_state_t state;           /**< MCP state */
    telnetty_mcp_auth_data_t auth;        /**< Authentication data */
    telnetty_mcp_package_t** packages;    /**< Registered packages array */
    size_t package_count;               /**< Number of packages */
    size_t package_capacity;            /**< Package array capacity */
    telnetty_mcp_package_handler_t default_handler; /**< Default handler */
    void* default_handler_data;         /**< Default handler data */
    int last_message_id;                /**< Last message ID */
    bool server_mode;                   /**< Server mode */
    char* server_key;                   /**< Server authentication key */
} telnetty_mcp_data_t;

/* ============================================================================
 * MCP Core Functions
 * ============================================================================ */

/**
 * Enable MCP 2.1 protocol
 * 
 * @param ctx TELNET context
 * @param server_mode true for server mode, false for client mode
 * @param auth_type Authentication type
 * @param key Authentication key (optional)
 * @return 0 on success, -1 on failure
 */
static int telnetty_enable_mcp(
    telnetty_context_t* ctx,
    bool server_mode,
    telnetty_mcp_auth_type_t auth_type,
    const char* key
);

/**
 * Disable MCP 2.1 protocol
 * 
 * @param ctx TELNET context
 * @return 0 on success, -1 on failure
 */
static TELNETTY_UNUSED int telnetty_disable_mcp(telnetty_context_t* ctx);

/**
 * MCP option handler
 * 
 * @param ctx TELNET context
 * @param option Option code (should be TELNETTY_TELOPT_MCP)
 * @param command Command received
 * @param user_data User data (should be telnetty_mcp_data_t*)
 */
static void telnetty_mcp_handler(
    telnetty_context_t* ctx,
    uint8_t option,
    uint8_t command,
    void* user_data
);

/**
 * Register an MCP package
 * 
 * @param ctx TELNET context
 * @param name Package name
 * @param version Package version
 * @param description Package description (optional)
 * @param handler Package handler (optional)
 * @param user_data User data for handler
 * @return 0 on success, -1 on failure
 */
static int telnetty_mcp_register_package(
    telnetty_context_t* ctx,
    const char* name,
    const char* version,
    const char* description,
    telnetty_mcp_package_handler_t handler,
    void* user_data
);

/**
 * Unregister an MCP package
 * 
 * @param ctx TELNET context
 * @param name Package name
 * @return 0 on success, -1 on failure
 */
static int telnetty_mcp_unregister_package(
    telnetty_context_t* ctx,
    const char* name
);

/**
 * Send MCP message
 * 
 * @param ctx TELNET context
 * @param package Package name
 * @param message Message type
 * @param data Message data (JSON format)
 * @return 0 on success, -1 on failure
 */
static int telnetty_mcp_send(
    telnetty_context_t* ctx,
    const char* package,
    const char* message,
    const char* data
);

/**
 * Send MCP message with formatting
 * 
 * @param ctx TELNET context
 * @param package Package name
 * @param message Message type
 * @param fmt Format string for JSON data
 * @param ... Format arguments
 * @return 0 on success, -1 on failure
 */
static int telnetty_mcp_sendf(
    telnetty_context_t* ctx,
    const char* package,
    const char* message,
    const char* fmt,
    ...
);

/**
 * Send MCP authentication request
 * 
 * @param ctx TELNET context
 * @param auth_type Authentication type
 * @param credentials Authentication credentials (JSON format)
 * @return 0 on success, -1 on failure
 */
static int telnetty_mcp_send_auth(
    telnetty_context_t* ctx,
    telnetty_mcp_auth_type_t auth_type,
    const char* credentials
);

/**
 * Send MCP negotiation request
 * 
 * @param ctx TELNET context
 * @param packages Array of supported packages
 * @param package_count Number of packages
 * @return 0 on success, -1 on failure
 */
static int telnetty_mcp_send_negotiate(
    telnetty_context_t* ctx,
    const telnetty_mcp_package_t* packages,
    size_t package_count
);

/**
 * Send MCP error message
 * 
 * @param ctx TELNET context
 * @param package Package that caused error (optional)
 * @param message Error message
 * @param code Error code (optional)
 * @return 0 on success, -1 on failure
 */
static int telnetty_mcp_send_error(
    telnetty_context_t* ctx,
    const char* package,
    const char* message,
    int code
);

/**
 * Send MCP close message
 * 
 * @param ctx TELNET context
 * @param reason Close reason (optional)
 * @return 0 on success, -1 on failure
 */
static int telnetty_mcp_send_close(
    telnetty_context_t* ctx,
    const char* reason
);

/* ============================================================================
 * Package Management Functions
 * ============================================================================ */

/**
 * Get MCP package by name
 * 
 * @param ctx TELNET context
 * @param name Package name
 * @return Package structure or NULL if not found
 */
static telnetty_mcp_package_t* telnetty_mcp_get_package(
    telnetty_context_t* ctx,
    const char* name
);

/**
 * Check if package is active
 * 
 * @param ctx TELNET context
 * @param name Package name
 * @return true if package is active, false otherwise
 */
static bool telnetty_mcp_is_package_active(
    telnetty_context_t* ctx,
    const char* name
);

/**
 * Activate MCP package
 * 
 * @param ctx TELNET context
 * @param name Package name
 * @return 0 on success, -1 on failure
 */
static int telnetty_mcp_activate_package(
    telnetty_context_t* ctx,
    const char* name
);

/**
 * Deactivate MCP package
 * 
 * @param ctx TELNET context
 * @param name Package name
 * @return 0 on success, -1 on failure
 */
static int telnetty_mcp_deactivate_package(
    telnetty_context_t* ctx,
    const char* name
);

/**
 * Set default package handler
 * 
 * @param ctx TELNET context
 * @param handler Default handler function
 * @param user_data User data for handler
 * @return 0 on success, -1 on failure
 */
static int telnetty_mcp_set_default_handler(
    telnetty_context_t* ctx,
    telnetty_mcp_package_handler_t handler,
    void* user_data
);

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/**
 * Get MCP state
 * 
 * @param ctx TELNET context
 * @return Current MCP state
 */
static TELNETTY_UNUSED telnetty_mcp_state_t telnetty_mcp_get_state(telnetty_context_t* ctx);

/**
 * Check if MCP is enabled
 * 
 * @param ctx TELNET context
 * @return true if MCP is enabled, false otherwise
 */
static TELNETTY_UNUSED bool telnetty_mcp_is_enabled(telnetty_context_t* ctx);

/**
 * Get number of registered packages
 * 
 * @param ctx TELNET context
 * @return Number of packages
 */
static TELNETTY_UNUSED size_t telnetty_mcp_get_package_count(telnetty_context_t* ctx);

/**
 * Get package list
 * 
 * @param ctx TELNET context
 * @param packages Output array for packages
 * @param max_packages Maximum number of packages to return
 * @return Number of packages returned
 */
static size_t telnetty_mcp_get_packages(
    telnetty_context_t* ctx,
    telnetty_mcp_package_t* packages,
    size_t max_packages
);

/**
 * Generate unique MCP message ID
 * 
 * @param ctx TELNET context
 * @return New message ID
 */
static TELNETTY_UNUSED int telnetty_mcp_generate_message_id(telnetty_context_t* ctx);

/**
 * Parse MCP message
 * 
@brief Parse an MCP message from raw data
 * 
 * @param data Raw message data
 * @param length Data length
 * @param message Output parsed message structure
 * @return 0 on success, -1 on failure
 */
static int telnetty_mcp_parse_message(
    const uint8_t* data,
    size_t length,
    telnetty_mcp_message_t* message
);

/**
 * Build MCP message
 * 
 * @param message Message to build
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @return Number of bytes written or -1 on failure
 */
static int telnetty_mcp_build_message(
    const telnetty_mcp_message_t* message,
    uint8_t* buffer,
    size_t buffer_size
);

/* ============================================================================
 * Built-in MCP Packages
 * ============================================================================ */

/**
 * Register built-in MCP packages
 * 
 * @param ctx TELNET context
 * @return 0 on success, -1 on failure
 */
static TELNETTY_UNUSED int telnetty_mcp_register_builtin_packages(telnetty_context_t* ctx);

/**
 * MCP Core package handler
 * 
 * @param ctx TELNET context
 * @param package Package name
 * @param message Message type
 * @param data Message data
 * @param user_data User data
 */
static void telnetty_mcp_core_handler(
    telnetty_context_t* ctx,
    const char* package,
    const char* message,
    const char* data,
    void* user_data
);

/**
 * MCP SimpleEdit package handler
 * 
 * @param ctx TELNET context
 * @param package Package name
 * @param message Message type
 * @param data Message data
 * @param user_data User data
 */
static void telnetty_mcp_simpleedit_handler(
    telnetty_context_t* ctx,
    const char* package,
    const char* message,
    const char* data,
    void* user_data
);

/**
 * MCP Redirect package handler
 * 
 * @param ctx TELNET context
 * @param package Package name
 * @param message Message type
 * @param data Message data
 * @param user_data User data
 */
static void telnetty_mcp_redirect_handler(
    telnetty_context_t* ctx,
    const char* package,
    const char* message,
    const char* data,
    void* user_data
);

/**
 * MCP DNS package handler
 * 
 * @param ctx TELNET context
 * @param package Package name
 * @param message Message type
 * @param data Message data
 * @param user_data User data
 */
static void telnetty_mcp_dns_handler(
    telnetty_context_t* ctx,
    const char* package,
    const char* message,
    const char* data,
    void* user_data
);

/* ============================================================================
 * Implementation
 * ============================================================================ */

#ifdef TELNETTY_MCP_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <ctype.h>

/* Memory allocation wrappers */
#ifndef TELNETTY_MCP_MALLOC
#define TELNETTY_MCP_MALLOC(size) malloc(size)
#endif

#ifndef TELNETTY_MCP_FREE
#define TELNETTY_MCP_FREE(ptr) free(ptr)
#endif

#ifndef TELNETTY_MCP_REALLOC
#define TELNETTY_MCP_REALLOC(ptr, size) realloc(ptr, size)
#endif

/* Forward declarations for internal functions */
static TELNETTY_UNUSED telnetty_mcp_data_t* telnetty_mcp_data_create(void);
static TELNETTY_UNUSED void telnetty_mcp_data_destroy(telnetty_mcp_data_t* data);
static telnetty_mcp_package_t* telnetty_mcp_package_create(
    const char* name,
    const char* version,
    const char* description,
    telnetty_mcp_package_handler_t handler,
    void* user_data
);
static TELNETTY_UNUSED void telnetty_mcp_package_destroy(telnetty_mcp_package_t* package);
static int telnetty_mcp_send_subnegotiation(
    telnetty_context_t* ctx,
    const uint8_t* data,
    size_t length
);

/* ============================================================================
 * MCP Data Management
 * ============================================================================ */

static TELNETTY_UNUSED telnetty_mcp_data_t* telnetty_mcp_data_create(void) {
    telnetty_mcp_data_t* data = (telnetty_mcp_data_t*)TELNETTY_MCP_MALLOC(sizeof(telnetty_mcp_data_t));
    if (!data) return NULL;
    
    memset(data, 0, sizeof(telnetty_mcp_data_t));
    data->state = TELNETTY_MCP_STATE_DISABLED;
    data->package_capacity = 16; /* Initial capacity */
    data->packages = (telnetty_mcp_package_t**)TELNETTY_MCP_MALLOC(
        data->package_capacity * sizeof(telnetty_mcp_package_t*)
    );
    
    if (!data->packages) {
        TELNETTY_MCP_FREE(data);
        return NULL;
    }
    
    memset(data->packages, 0, data->package_capacity * sizeof(telnetty_mcp_package_t*));
    
    return data;
}

static TELNETTY_UNUSED void telnetty_mcp_data_destroy(telnetty_mcp_data_t* data) {
    if (!data) return;
    
    /* Free all packages */
    for (size_t i = 0; i < data->package_count; i++) {
        if (data->packages[i]) {
            telnetty_mcp_package_destroy(data->packages[i]);
        }
    }
    
    /* Free package array */
    TELNETTY_MCP_FREE(data->packages);
    
    /* Free authentication data */
    if (data->auth.key) {
        TELNETTY_MCP_FREE(data->auth.key);
    }
    if (data->auth.username) {
        TELNETTY_MCP_FREE(data->auth.username);
    }
    if (data->auth.password) {
        TELNETTY_MCP_FREE(data->auth.password);
    }
    if (data->server_key) {
        TELNETTY_MCP_FREE(data->server_key);
    }
    
    TELNETTY_MCP_FREE(data);
}

static telnetty_mcp_package_t* telnetty_mcp_package_create(
    const char* name,
    const char* version,
    const char* description,
    telnetty_mcp_package_handler_t handler,
    void* user_data
) {
    if (!name) return NULL;
    
    telnetty_mcp_package_t* package = (telnetty_mcp_package_t*)TELNETTY_MCP_MALLOC(sizeof(telnetty_mcp_package_t));
    if (!package) return NULL;
    
    memset(package, 0, sizeof(telnetty_mcp_package_t));
    
    /* Copy name */
    package->name = strdup(name);
    if (!package->name) {
        TELNETTY_MCP_FREE(package);
        return NULL;
    }
    
    /* Copy version */
    if (version) {
        package->version = strdup(version);
        if (!package->version) {
            TELNETTY_MCP_FREE(package->name);
            TELNETTY_MCP_FREE(package);
            return NULL;
        }
    }
    
    /* Copy description */
    if (description) {
        package->description = strdup(description);
        if (!package->description) {
            TELNETTY_MCP_FREE(package->name);
            TELNETTY_MCP_FREE(package->version);
            TELNETTY_MCP_FREE(package);
            return NULL;
        }
    }
    
    package->handler = handler;
    package->user_data = user_data;
    
    return package;
}

static TELNETTY_UNUSED void telnetty_mcp_package_destroy(telnetty_mcp_package_t* package) {
    if (!package) return;
    
    if (package->name) {
        TELNETTY_MCP_FREE(package->name);
    }
    if (package->version) {
        TELNETTY_MCP_FREE(package->version);
    }
    if (package->description) {
        TELNETTY_MCP_FREE(package->description);
    }
    
    TELNETTY_MCP_FREE(package);
}

/* ============================================================================
 * MCP Core Implementation
 * ============================================================================ */

static int telnetty_enable_mcp(
    telnetty_context_t* ctx,
    bool server_mode,
    telnetty_mcp_auth_type_t auth_type,
    const char* key
) {
    if (!ctx) return -1;
    
    /* Create MCP data */
    telnetty_mcp_data_t* mcp_data = telnetty_mcp_data_create();
    if (!mcp_data) return -1;
    
    mcp_data->server_mode = server_mode;
    mcp_data->auth.type = auth_type;
    
    /* Set authentication key if provided */
    if (key) {
        mcp_data->server_key = strdup(key);
        if (!mcp_data->server_key) {
            telnetty_mcp_data_destroy(mcp_data);
            return -1;
        }
    }
    
    /* Register option handler */
    /* Note: In full implementation, this would be integrated with the core */
    
    /* Send WILL to enable MCP */
    int result = telnetty_send_option(ctx, TELNETTY_WILL, TELNETTY_TELOPT_MCP);
    if (result != 0) {
        telnetty_mcp_data_destroy(mcp_data);
        return -1;
    }
    
    /* Register built-in packages */
    telnetty_mcp_register_builtin_packages(ctx);
    
    return 0;
}

static TELNETTY_UNUSED int telnetty_disable_mcp(telnetty_context_t* ctx) {
    if (!ctx) return -1;
    
    /* Send WONT to disable MCP */
    return telnetty_send_option(ctx, TELNETTY_WONT, TELNETTY_TELOPT_MCP);
}

static void telnetty_mcp_handler(
    telnetty_context_t* ctx,
    uint8_t option,
    uint8_t command,
    void* user_data
) {
    if (!ctx || option != TELNETTY_TELOPT_MCP) return;
    
    telnetty_mcp_data_t* mcp_data = (telnetty_mcp_data_t*)user_data;
    if (!mcp_data) return;
    
    switch (command) {
        case TELNETTY_WILL:
            /* Peer wants to enable MCP */
            telnetty_send_option(ctx, TELNETTY_DO, option);
            mcp_data->state = TELNETTY_MCP_STATE_NEGOTIATING;
            
            /* Send authentication if server */
            if (mcp_data->server_mode) {
                /* Wait for client to authenticate */
            } else {
                /* Client sends authentication */
                char auth_data[256];
                snprintf(auth_data, sizeof(auth_data), 
                    "{\"type\": \"%s\"}",
                    mcp_data->auth.type == TELNETTY_MCP_AUTH_KEY ? "key" : "none");
                telnetty_mcp_send_auth(ctx, mcp_data->auth.type, auth_data);
            }
            break;
            
        case TELNETTY_WONT:
            /* Peer wants to disable MCP */
            telnetty_send_option(ctx, TELNETTY_DONT, option);
            mcp_data->state = TELNETTY_MCP_STATE_DISABLED;
            break;
            
        case TELNETTY_DO:
            /* Peer agrees to enable MCP */
            mcp_data->state = TELNETTY_MCP_STATE_NEGOTIATING;
            
            /* Send negotiation if server */
            if (mcp_data->server_mode) {
                /* Send supported packages */
                telnetty_mcp_send_negotiate(ctx, NULL, 0);
            }
            break;
            
        case TELNETTY_DONT:
            /* Peer refuses to enable MCP */
            mcp_data->state = TELNETTY_MCP_STATE_DISABLED;
            break;
    }
}

/* ============================================================================
 * Package Management Implementation
 * ============================================================================ */

static int telnetty_mcp_register_package(
    telnetty_context_t* ctx,
    const char* name,
    const char* version,
    const char* description,
    telnetty_mcp_package_handler_t handler,
    void* user_data
) {
    if (!ctx || !name) return -1;
    
    /* This would need integration with MCP data structure */
    /* For now, return success */
    return 0;
}

static int telnetty_mcp_unregister_package(
    telnetty_context_t* ctx,
    const char* name
) {
    if (!ctx || !name) return -1;
    
    /* This would need integration with MCP data structure */
    /* For now, return success */
    return 0;
}

static telnetty_mcp_package_t* telnetty_mcp_get_package(
    telnetty_context_t* ctx,
    const char* name
) {
    if (!ctx || !name) return NULL;
    
    /* This would need integration with MCP data structure */
    /* For now, return NULL */
    return NULL;
}

static bool telnetty_mcp_is_package_active(
    telnetty_context_t* ctx,
    const char* name
) {
    telnetty_mcp_package_t* package = telnetty_mcp_get_package(ctx, name);
    return package ? package->active : false;
}

static int telnetty_mcp_activate_package(
    telnetty_context_t* ctx,
    const char* name
) {
    telnetty_mcp_package_t* package = telnetty_mcp_get_package(ctx, name);
    if (!package) return -1;
    
    package->active = true;
    return 0;
}

static int telnetty_mcp_deactivate_package(
    telnetty_context_t* ctx,
    const char* name
) {
    telnetty_mcp_package_t* package = telnetty_mcp_get_package(ctx, name);
    if (!package) return -1;
    
    package->active = false;
    return 0;
}

static int telnetty_mcp_set_default_handler(
    telnetty_context_t* ctx,
    telnetty_mcp_package_handler_t handler,
    void* user_data
) {
    if (!ctx) return -1;
    
    /* This would need integration with MCP data structure */
    /* For now, return success */
    return 0;
}

/* ============================================================================
 * Message Functions Implementation
 * ============================================================================ */

static int telnetty_mcp_send_subnegotiation(
    telnetty_context_t* ctx,
    const uint8_t* data,
    size_t length
) {
    if (!ctx || !data) return -1;
    
    return telnetty_send_subnegotiation(ctx, TELNETTY_TELOPT_MCP, data, length);
}

static int telnetty_mcp_send(
    telnetty_context_t* ctx,
    const char* package,
    const char* message,
    const char* data
) {
    if (!ctx || !package || !message) return -1;
    
    /* Calculate required buffer size */
    size_t package_len = strlen(package);
    size_t message_len = strlen(message);
    size_t data_len = data ? strlen(data) : 0;
    
    /* MCP message format: #package message data\r\n */
    size_t total_size = 1 + package_len + 1 + message_len + (data_len > 0 ? (1 + data_len) : 0) + 2;
    
    /* Allocate buffer */
    uint8_t* buffer = (uint8_t*)TELNETTY_MCP_MALLOC(total_size);
    if (!buffer) return -1;
    
    /* Build MCP message */
    size_t offset = 0;
    buffer[offset++] = '#';  /* MCP message prefix */
    memcpy(buffer + offset, package, package_len);
    offset += package_len;
    buffer[offset++] = ' ';
    memcpy(buffer + offset, message, message_len);
    offset += message_len;
    
    if (data && data_len > 0) {
        buffer[offset++] = ' ';
        memcpy(buffer + offset, data, data_len);
        offset += data_len;
    }
    
    buffer[offset++] = '\r';
    buffer[offset++] = '\n';
    
    /* Send via subnegotiation */
    int result = telnetty_mcp_send_subnegotiation(ctx, buffer, offset);
    
    TELNETTY_MCP_FREE(buffer);
    
    return result;
}

static int telnetty_mcp_sendf(
    telnetty_context_t* ctx,
    const char* package,
    const char* message,
    const char* fmt,
    ...
) {
    if (!ctx || !package || !message) return -1;
    
    /* Format the data if provided */
    char* data = NULL;
    if (fmt) {
        va_list args;
        va_start(args, fmt);
        
        /* Calculate required size */
        va_list args_copy;
        va_copy(args_copy, args);
        int size = vsnprintf(NULL, 0, fmt, args_copy);
        va_end(args_copy);
        
        if (size > 0) {
            data = (char*)TELNETTY_MCP_MALLOC(size + 1);
            if (data) {
                vsnprintf(data, size + 1, fmt, args);
            }
        }
        
        va_end(args);
    }
    
    /* Send MCP message */
    int result = telnetty_mcp_send(ctx, package, message, data);
    
    if (data) {
        TELNETTY_MCP_FREE(data);
    }
    
    return result;
}

static int telnetty_mcp_send_auth(
    telnetty_context_t* ctx,
    telnetty_mcp_auth_type_t auth_type,
    const char* credentials
) {
    if (!ctx) return -1;
    
    const char* auth_type_str = NULL;
    switch (auth_type) {
        case TELNETTY_MCP_AUTH_NONE:
            auth_type_str = "none";
            break;
        case TELNETTY_MCP_AUTH_KEY:
            auth_type_str = "key";
            break;
        case TELNETTY_MCP_AUTH_USER:
            auth_type_str = "user";
            break;
        default:
            return -1;
    }
    
    return telnetty_mcp_sendf(ctx, "mcp", "auth", 
        "{\"type\": \"%s\"%s%s}",
        auth_type_str,
        credentials ? ", \"credentials\": " : "",
        credentials ? credentials : "");
}

static int telnetty_mcp_send_negotiate(
    telnetty_context_t* ctx,
    const telnetty_mcp_package_t* packages,
    size_t package_count
) {
    if (!ctx) return -1;
    
    /* Build package list JSON */
    /* For simplicity, just send empty negotiation for now */
    return telnetty_mcp_send(ctx, "mcp", "negotiate", "{}");
}

static int telnetty_mcp_send_error(
    telnetty_context_t* ctx,
    const char* package,
    const char* message,
    int code
) {
    if (!ctx || !message) return -1;
    
    const char* pkg_str = package ? package : "";
    const char* code_str = code >= 0 ? ", \"code\": " : "";
    char code_buf[32];
    
    if (code >= 0) {
        snprintf(code_buf, sizeof(code_buf), "%d", code);
    }
    
    return telnetty_mcp_sendf(ctx, "mcp", "error",
        "{\"message\": \"%s\"%s%s%s%s}",
        message,
        package ? ", \"package\": \"" : "",
        package ? package : "",
        package ? "\"" : "",
        code >= 0 ? code_buf : "");
}

static int telnetty_mcp_send_close(
    telnetty_context_t* ctx,
    const char* reason
) {
    if (!ctx) return -1;
    
    const char* reason_str = reason ? reason : "";
    
    return telnetty_mcp_sendf(ctx, "mcp", "close",
        reason ? "{\"reason\": \"%s\"}" : "{}",
        reason_str);
}

/* ============================================================================
 * Utility Functions Implementation
 * ============================================================================ */

static TELNETTY_UNUSED telnetty_mcp_state_t telnetty_mcp_get_state(telnetty_context_t* ctx) {
    if (!ctx) return TELNETTY_MCP_STATE_DISABLED;
    
    /* This would need integration with MCP data structure */
    /* For now, return disabled */
    return TELNETTY_MCP_STATE_DISABLED;
}

static TELNETTY_UNUSED bool telnetty_mcp_is_enabled(telnetty_context_t* ctx) {
    return telnetty_mcp_get_state(ctx) == TELNETTY_MCP_STATE_ENABLED;
}

static TELNETTY_UNUSED size_t telnetty_mcp_get_package_count(telnetty_context_t* ctx) {
    if (!ctx) return 0;
    
    /* This would need integration with MCP data structure */
    /* For now, return 0 */
    return 0;
}

static size_t telnetty_mcp_get_packages(
    telnetty_context_t* ctx,
    telnetty_mcp_package_t* packages,
    size_t max_packages
) {
    if (!ctx || !packages) return 0;
    
    /* This would need integration with MCP data structure */
    /* For now, return 0 */
    return 0;
}

static TELNETTY_UNUSED int telnetty_mcp_generate_message_id(telnetty_context_t* ctx) {
    if (!ctx) return -1;
    
    /* This would need integration with MCP data structure */
    /* For now, return a simple counter */
    static int message_id = 0;
    return ++message_id;
}

static int telnetty_mcp_parse_message(
    const uint8_t* data,
    size_t length,
    telnetty_mcp_message_t* message
) {
    if (!data || !message || length == 0) return -1;
    
    /* Simple MCP message parsing */
    /* Format: #package message data\r\n */
    
    if (data[0] != '#') return -1;  /* Invalid MCP message */
    
    /* Find message components */
    size_t package_start = 1;
    size_t package_end = 0;
    size_t message_start = 0;
    size_t message_end = 0;
    size_t data_start = 0;
    size_t data_end = length;
    
    /* Find package end */
    for (size_t i = package_start; i < length; i++) {
        if (data[i] == ' ') {
            package_end = i;
            message_start = i + 1;
            break;
        }
    }
    
    /* Find message end */
    if (message_start > 0) {
        for (size_t i = message_start; i < length; i++) {
            if (data[i] == ' ') {
                message_end = i;
                data_start = i + 1;
                break;
            } else if (data[i] == '\r') {
                message_end = i;
                data_end = i;
                break;
            }
        }
    }
    
    /* Extract components */
    if (package_end > package_start) {
        message->package = strndup((const char*)data + package_start, 
                                   package_end - package_start);
    }
    
    if (message_end > message_start) {
        message->message = strndup((const char*)data + message_start, 
                                   message_end - message_start);
    }
    
    if (data_start > 0 && data_end > data_start) {
        message->data = strndup((const char*)data + data_start, 
                                data_end - data_start);
    }
    
    message->timestamp = time(NULL);
    
    return 0;
}

static int telnetty_mcp_build_message(
    const telnetty_mcp_message_t* message,
    uint8_t* buffer,
    size_t buffer_size
) {
    if (!message || !buffer) return -1;
    
    /* Calculate required size */
    size_t required_size = 1; /* # */
    if (message->package) required_size += strlen(message->package);
    required_size += 1; /* space */
    if (message->message) required_size += strlen(message->message);
    if (message->data) {
        required_size += 1; /* space */
        required_size += strlen(message->data);
    }
    required_size += 2; /* \r\n */
    
    if (buffer_size < required_size) return -1;
    
    /* Build message */
    size_t offset = 0;
    buffer[offset++] = '#';
    
    if (message->package) {
        strcpy((char*)buffer + offset, message->package);
        offset += strlen(message->package);
    }
    
    buffer[offset++] = ' ';
    
    if (message->message) {
        strcpy((char*)buffer + offset, message->message);
        offset += strlen(message->message);
    }
    
    if (message->data) {
        buffer[offset++] = ' ';
        strcpy((char*)buffer + offset, message->data);
        offset += strlen(message->data);
    }
    
    buffer[offset++] = '\r';
    buffer[offset++] = '\n';
    
    return (int)offset;
}

/* ============================================================================
 * Built-in Package Implementation
 * ============================================================================ */

static TELNETTY_UNUSED int telnetty_mcp_register_builtin_packages(telnetty_context_t* ctx) {
    if (!ctx) return -1;
    
    /* Register Core package */
    telnetty_mcp_register_package(ctx, "mcp-core", "2.1", 
        "MCP Core Package", 
        telnetty_mcp_core_handler, NULL);
    
    /* Register SimpleEdit package */
    telnetty_mcp_register_package(ctx, "simple-edit", "1.0", 
        "SimpleEdit package", 
        telnetty_mcp_simpleedit_handler, NULL);
    
    /* Register Redirect package */
    telnetty_mcp_register_package(ctx, "redirect", "1.0", 
        "Redirect package", 
        telnetty_mcp_redirect_handler, NULL);
    
    /* Register DNS package */
    telnetty_mcp_register_package(ctx, "dns", "1.0", 
        "DNS package", 
        telnetty_mcp_dns_handler, NULL);
    
    return 0;
}

static void telnetty_mcp_core_handler(
    telnetty_context_t* ctx,
    const char* package,
    const char* message,
    const char* data,
    void* user_data
) {
    if (!ctx || !package || !message) return;
    
    /* Handle core MCP messages */
    if (strcmp(message, "mcp") == 0) {
        /* Initial MCP negotiation */
        /* Would handle version negotiation */
    } else if (strcmp(message, "auth") == 0) {
        /* Authentication handling */
    }
}

static void telnetty_mcp_simpleedit_handler(
    telnetty_context_t* ctx,
    const char* package,
    const char* message,
    const char* data,
    void* user_data
) {
    if (!ctx || !package || !message) return;
    
    /* Handle SimpleEdit messages */
    /* Would implement SimpleEdit protocol handling */
}

static void telnetty_mcp_redirect_handler(
    telnetty_context_t* ctx,
    const char* package,
    const char* message,
    const char* data,
    void* user_data
) {
    if (!ctx || !package || !message) return;
    
    /* Handle Redirect messages */
    /* Would implement redirect protocol handling */
}

static void telnetty_mcp_dns_handler(
    telnetty_context_t* ctx,
    const char* package,
    const char* message,
    const char* data,
    void* user_data
) {
    if (!ctx || !package || !message) return;
    
    /* Handle DNS messages */
    /* Would implement DNS resolution protocol */
}

#endif /* TELNETTY_MCP_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif

#endif /* TELNETTY_MCP_H */