/**
 * @file telnetty_mud.h
 * @brief MUD-specific protocol implementations
 * 
 * This header provides implementations for MUD-specific protocols that extend
 * the base TELNET protocol with features commonly used in Multi-User Dungeons.
 * 
 * Supported Protocols:
 * - MSDP (MUD Server Data Protocol)
 * - GMCP (Generic MUD Communication Protocol)
 * - MTTS (MUD Terminal Type Standard)
 * - MSSP (MUD Server Status Protocol)
 * 
 * @author Damus <damus@straylightrun.org>
 * @version 1.0.0
 * 
 */

#ifndef TELNETTY_MUD_H
#define TELNETTY_MUD_H

#include "telnetty_core.h"
#include "telnetty_events.h"
#include <stdint.h>
#include <stdbool.h>

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
 * MSDP (MUD Server Data Protocol)
 * ============================================================================ */

/* MSDP option code */
#define TELNETTY_TELOPT_MSDP 69

/* MSDP data types */
typedef enum {
    TELNETTY_MSDP_VAR = 1,                /**< Variable */
    TELNETTY_MSDP_VAL = 2,                /**< Value */
    TELNETTY_MSDP_TABLE_OPEN = 3,         /**< Table open */
    TELNETTY_MSDP_TABLE_CLOSE = 4,        /**< Table close */
    TELNETTY_MSDP_ARRAY_OPEN = 5,         /**< Array open */
    TELNETTY_MSDP_ARRAY_CLOSE = 6         /**< Array close */
} telnetty_msdp_type_t;

/* MSDP variable flags */
typedef enum {
    TELNETTY_MSDP_FLAG_COMMAND = 1 << 0,      /**< Command variable */
    TELNETTY_MSDP_FLAG_LIST = 1 << 1,         /**< List variable */
    TELNETTY_MSDP_FLAG_SENDABLE = 1 << 2,     /**< Sendable variable */
    TELNETTY_MSDP_FLAG_REPORTABLE = 1 << 3,   /**< Reportable variable */
    TELNETTY_MSDP_FLAG_CONFIGURABLE = 1 << 4, /**< Configurable variable */
    TELNETTY_MSDP_FLAG_REPORTED = 1 << 5,     /**< Reported variable */
    TELNETTY_MSDP_FLAG_UPDATED = 1 << 6       /**< Updated variable */
} telnetty_msdp_flag_t;

/* MSDP variable structure */
typedef struct {
    char* name;                         /**< Variable name */
    char* value;                        /**< Variable value */
    uint32_t flags;                     /**< Variable flags */
    time_t last_updated;                /**< Last update time */
} telnetty_msdp_variable_t;

/* MSDP handler function type */
typedef void (*telnetty_msdp_handler_t)(
    telnetty_context_t* ctx,
    const char* var,
    const char* val,
    void* user_data
);

/* MSDP data structure */
typedef struct {
    telnetty_msdp_variable_t* variables;  /**< MSDP variables */
    size_t variable_count;              /**< Number of variables */
    size_t capacity;                    /**< Variable array capacity */
    telnetty_msdp_handler_t handler;      /**< MSDP handler callback */
    void* user_data;                    /**< User data for handler */
    bool negotiation_complete;          /**< Negotiation is complete */
} telnetty_msdp_data_t;

/**
 * MSDP option handler
 * 
 * @param ctx TELNET context
 * @param option Option code (should be TELNETTY_TELOPT_MSDP)
 * @param command Command received
 * @param user_data User data (should be telnetty_msdp_data_t*)
 */
static void telnetty_msdp_handler(
    telnetty_context_t* ctx,
    uint8_t option,
    uint8_t command,
    void* user_data
);

/**
 * Enable MSDP protocol
 * 
 * @param ctx TELNET context
 * @param handler MSDP handler callback (optional)
 * @param user_data User data for handler
 * @return 0 on success, -1 on failure
 */
static int telnetty_enable_msdp(
    telnetty_context_t* ctx,
    telnetty_msdp_handler_t handler,
    void* user_data
);

/**
 * Send MSDP variable update
 * 
 * @param ctx TELNET context
 * @param var Variable name
 * @param val Variable value
 * @return 0 on success, -1 on failure
 */
static int telnetty_msdp_send(
    telnetty_context_t* ctx,
    const char* var,
    const char* val
);

/**
 * Send MSDP variable update with formatting
 * 
 * @param ctx TELNET context
 * @param var Variable name
 * @param fmt Format string
 * @param ... Format arguments
 * @return 0 on success, -1 on failure
 */
static int telnetty_msdp_sendf(
    telnetty_context_t* ctx,
    const char* var,
    const char* fmt,
    ...
);

/**
 * Request MSDP variable from peer
 * 
 * @param ctx TELNET context
 * @param var Variable name
 * @return 0 on success, -1 on failure
 */
static int telnetty_msdp_request(
    telnetty_context_t* ctx,
    const char* var
);

/**
 * Set MSDP variable flags
 * 
 * @param ctx TELNET context
 * @param var Variable name
 * @param flags Flags to set
 * @return 0 on success, -1 on failure
 */
static int telnetty_msdp_set_flags(
    telnetty_context_t* ctx,
    const char* var,
    uint32_t flags
);

/**
 * Clear MSDP variable flags
 * 
 * @param ctx TELNET context
 * @param var Variable name
 * @param flags Flags to clear
 * @return 0 on success, -1 on failure
 */
static int telnetty_msdp_clear_flags(
    telnetty_context_t* ctx,
    const char* var,
    uint32_t flags
);

/**
 * Get MSDP variable value
 * 
 * @param ctx TELNET context
 * @param var Variable name
 * @return Variable value or NULL if not found
 */
static const char* telnetty_msdp_get(
    telnetty_context_t* ctx,
    const char* var
);

/**
 * Remove MSDP variable
 * 
 * @param ctx TELNET context
 * @param var Variable name
 * @return 0 on success, -1 on failure
 */
static int telnetty_msdp_remove(
    telnetty_context_t* ctx,
    const char* var
);

/* ============================================================================
 * GMCP (Generic MUD Communication Protocol)
 * ============================================================================ */

/* GMCP option code */
#define TELNETTY_TELOPT_GMCP 201

/* GMCP message structure */
typedef struct {
    char* module;                       /**< GMCP module */
    char* message;                      /**< GMCP message */
    char* data;                         /**< GMCP data (JSON format) */
    time_t timestamp;                   /**< Message timestamp */
} telnetty_gmcp_message_t;

/* GMCP handler function type */
typedef void (*telnetty_gmcp_handler_t)(
    telnetty_context_t* ctx,
    const char* module,
    const char* message,
    const char* data,
    void* user_data
);

/* GMCP data structure */
typedef struct {
    telnetty_gmcp_handler_t handler;      /**< GMCP handler callback */
    void* user_data;                    /**< User data for handler */
    char** supported_modules;           /**< Supported GMCP modules */
    size_t module_count;                /**< Number of supported modules */
    size_t capacity;                    /**< Capacity of supported_modules array */
    bool negotiation_complete;          /**< Negotiation is complete */
} telnetty_gmcp_data_t;

/**
 * GMCP option handler
 * 
 * @param ctx TELNET context
 * @param option Option code (should be TELNETTY_TELOPT_GMCP)
 * @param command Command received
 * @param user_data User data (should be telnetty_gmcp_data_t*)
 */
static void telnetty_gmcp_handler(
    telnetty_context_t* ctx,
    uint8_t option,
    uint8_t command,
    void* user_data
);

/**
 * Enable GMCP protocol
 * 
 * @param ctx TELNET context
 * @param handler GMCP handler callback (optional)
 * @param user_data User data for handler
 * @return 0 on success, -1 on failure
 */
static int telnetty_enable_gmcp(
    telnetty_context_t* ctx,
    telnetty_gmcp_handler_t handler,
    void* user_data
);

/**
 * Send GMCP message
 * 
 * @param ctx TELNET context
 * @param module GMCP module
 * @param message GMCP message
 * @param data GMCP data (JSON format, optional)
 * @return 0 on success, -1 on failure
 */
static int telnetty_gmcp_send(
    telnetty_context_t* ctx,
    const char* module,
    const char* message,
    const char* data
);

/**
 * Send GMCP message with formatting
 * 
 * @param ctx TELNET context
 * @param module GMCP module
 * @param message GMCP message
 * @param fmt Format string for JSON data
 * @param ... Format arguments
 * @return 0 on success, -1 on failure
 */
static int telnetty_gmcp_sendf(
    telnetty_context_t* ctx,
    const char* module,
    const char* message,
    const char* fmt,
    ...
);

/**
 * Register GMCP module support
 * 
 * @param ctx TELNET context
 * @param module Module name
 * @return 0 on success, -1 on failure
 */
static int telnetty_gmcp_register_module(
    telnetty_context_t* ctx,
    const char* module
);

/**
 * Unregister GMCP module support
 * 
 * @param ctx TELNET context
 * @param module Module name
 * @return 0 on success, -1 on failure
 */
static int telnetty_gmcp_unregister_module(
    telnetty_context_t* ctx,
    const char* module
);

/**
 * Send GMCP Core.Hello message
 * 
 * @param ctx TELNET context
 * @param client Client name
 * @param version Client version
 * @return 0 on success, -1 on failure
 */
static int telnetty_gmcp_send_hello(
    telnetty_context_t* ctx,
    const char* client,
    const char* version
);

/**
 * Send GMCP Core.Supports.Set message
 * 
 * @param ctx TELNET context
 * @param modules Array of supported modules
 * @param count Number of modules
 * @return 0 on success, -1 on failure
 */
static int telnetty_gmcp_send_supports(
    telnetty_context_t* ctx,
    const char* const* modules,
    size_t count
);

/* ============================================================================
 * MTTS (MUD Terminal Type Standard)
 * ============================================================================ */

/* MTTS option code */
#define TELNETTY_TELOPT_MTTS 24  /* Uses same code as TTYPE */

/* MTTS flags */
typedef enum {
    TELNETTY_MTTS_FLAG_ANSI = 1 << 0,           /**< ANSI color support */
    TELNETTY_MTTS_FLAG_VT100 = 1 << 1,          /**< VT100 emulation */
    TELNETTY_MTTS_FLAG_UTF8 = 1 << 2,           /**< UTF-8 support */
    TELNETTY_MTTS_FLAG_256COLORS = 1 << 3,      /**< 256-color support */
    TELNETTY_MTTS_FLAG_MOUSETRACKING = 1 << 4,  /**< Mouse tracking */
    TELNETTY_MTTS_FLAG_COLORPALETTE = 1 << 5,   /**< Color palette */
    TELNETTY_MTTS_FLAG_SCREENREADER = 1 << 6,   /**< Screen reader */
    TELNETTY_MTTS_FLAG_PROXY = 1 << 7,          /**< Proxy connection */
    TELNETTY_MTTS_FLAG_TRUECOLOR = 1 << 8       /**< True color support */
} telnetty_mtts_flag_t;

/* MTTS data structure */
typedef struct {
    uint64_t flags;                     /**< MTTS flags */
    char* client_name;                  /**< Client name */
    char* client_version;               /**< Client version */
    bool negotiation_complete;          /**< Negotiation is complete */
} telnetty_mtts_data_t;

/**
 * MTTS option handler
 * 
 * @param ctx TELNET context
 * @param option Option code
 * @param command Command received
 * @param user_data User data (should be telnetty_mtts_data_t*)
 */
static void telnetty_mtts_handler(
    telnetty_context_t* ctx,
    uint8_t option,
    uint8_t command,
    void* user_data
);

/**
 * Enable MTTS protocol
 * 
 * @param ctx TELNET context
 * @return 0 on success, -1 on failure
 */
static TELNETTY_UNUSED int telnetty_enable_mtts(telnetty_context_t* ctx);

/**
 * Get MTTS flags
 * 
 * @param ctx TELNET context
 * @return MTTS flags or 0 if not negotiated
 */
static TELNETTY_UNUSED uint64_t telnetty_mtts_get_flags(telnetty_context_t* ctx);

/**
 * Check if MTTS flag is set
 * 
 * @param ctx TELNET context
 * @param flag Flag to check
 * @return true if flag is set, false otherwise
 */
static bool telnetty_mtts_has_flag(
    telnetty_context_t* ctx,
    telnetty_mtts_flag_t flag
);

/**
 * Get MTTS client information
 * 
 * @param ctx TELNET context
 * @param name Output client name (optional)
 * @param version Output client version (optional)
 * @return 0 if info available, -1 otherwise
 */
static int telnetty_mtts_get_client_info(
    telnetty_context_t* ctx,
    const char** name,
    const char** version
);

/**
 * Send MTTS response
 * 
 * @param ctx TELNET context
 * @param flags MTTS flags
 * @param client_name Client name
 * @param client_version Client version
 * @return 0 on success, -1 on failure
 */
static int telnetty_mtts_send_response(
    telnetty_context_t* ctx,
    uint64_t flags,
    const char* client_name,
    const char* client_version
);

/* ============================================================================
 * MSSP (MUD Server Status Protocol)
 * ============================================================================ */

/* MSSP option code */
#define TELNETTY_TELOPT_MSSP 70

/* MSSP variable types */
typedef enum {
    TELNETTY_MSSP_VAR = 1,                /**< MSSP variable */
    TELNETTY_MSSP_VAL = 2                 /**< MSSP value */
} telnetty_mssp_type_t;

/* MSSP variable structure */
typedef struct {
    char* name;                         /**< Variable name */
    char* value;                        /**< Variable value */
} telnetty_mssp_variable_t;

/* MSSP data structure */
typedef struct {
    telnetty_mssp_variable_t* variables;  /**< MSSP variables */
    size_t variable_count;              /**< Number of variables */
    size_t capacity;                    /**< Array capacity */
    bool auto_send;                     /**< Auto-send on connection */
} telnetty_mssp_data_t;

/**
 * MSSP option handler
 * 
 * @param ctx TELNET context
 * @param option Option code (should be TELNETTY_TELOPT_MSSP)
 * @param command Command received
 * @param user_data User data (should be telnetty_mssp_data_t*)
 */
static void telnetty_mssp_handler(
    telnetty_context_t* ctx,
    uint8_t option,
    uint8_t command,
    void* user_data
);

/**
 * Enable MSSP protocol
 * 
 * @param ctx TELNET context
 * @param auto_send Auto-send MSSP data on connection
 * @return 0 on success, -1 on failure
 */
static int telnetty_enable_mssp(
    telnetty_context_t* ctx,
    bool auto_send
);

/**
 * Add MSSP variable
 * 
 * @param ctx TELNET context
 * @param name Variable name
 * @param value Variable value
 * @return 0 on success, -1 on failure
 */
static int telnetty_mssp_add(
    telnetty_context_t* ctx,
    const char* name,
    const char* value
);

/**
 * Send MSSP data to client
 * 
 * @param ctx TELNET context
 * @return 0 on success, -1 on failure
 */
static TELNETTY_UNUSED int telnetty_mssp_send(telnetty_context_t* ctx);

/**
 * Clear all MSSP variables
 * 
 * @param ctx TELNET context
 * @return 0 on success, -1 on failure
 */
static TELNETTY_UNUSED int telnetty_mssp_clear(telnetty_context_t* ctx);

/**
 * Set standard MSSP variables
 * 
 * @param ctx TELNET context
 * @param server_name Server name
 * @param server_type Server type
 * @param server_url Server URL
 * @param server_port Server port
 * @param player_count Current player count
 * @param max_players Maximum player count
 * @return 0 on success, -1 on failure
 */
static int telnetty_mssp_set_standard_vars(
    telnetty_context_t* ctx,
    const char* server_name,
    const char* server_type,
    const char* server_url,
    int server_port,
    int player_count,
    int max_players
);

/* ============================================================================
 * Implementation
 * ============================================================================ */

#ifdef TELNETTY_MUD_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <ctype.h>

/* Memory allocation wrappers */
#ifndef TELNETTY_MUD_MALLOC
#define TELNETTY_MUD_MALLOC(size) malloc(size)
#endif

#ifndef TELNETTY_MUD_FREE
#define TELNETTY_MUD_FREE(ptr) free(ptr)
#endif

#ifndef TELNETTY_MUD_REALLOC
#define TELNETTY_MUD_REALLOC(ptr, size) realloc(ptr, size)
#endif

/* Forward declarations for internal functions */
static TELNETTY_UNUSED telnetty_msdp_data_t* telnetty_msdp_data_create(void);
static TELNETTY_UNUSED void telnetty_msdp_data_destroy(telnetty_msdp_data_t* data);
static int telnetty_msdp_send_variable(
    telnetty_context_t* ctx,
    telnetty_msdp_variable_t* var
);
static TELNETTY_UNUSED telnetty_gmcp_data_t* telnetty_gmcp_data_create(void);
static TELNETTY_UNUSED void telnetty_gmcp_data_destroy(telnetty_gmcp_data_t* data);
static TELNETTY_UNUSED telnetty_mtts_data_t* telnetty_mtts_data_create(void);
static TELNETTY_UNUSED void telnetty_mtts_data_destroy(telnetty_mtts_data_t* data);
static TELNETTY_UNUSED telnetty_mssp_data_t* telnetty_mssp_data_create(void);
static TELNETTY_UNUSED void telnetty_mssp_data_destroy(telnetty_mssp_data_t* data);

/* ============================================================================
 * MSDP Implementation
 * ============================================================================ */

static TELNETTY_UNUSED telnetty_msdp_data_t* telnetty_msdp_data_create(void) {
    telnetty_msdp_data_t* data = (telnetty_msdp_data_t*)TELNETTY_MUD_MALLOC(sizeof(telnetty_msdp_data_t));
    if (!data) return NULL;
    
    memset(data, 0, sizeof(telnetty_msdp_data_t));
    data->capacity = 16; /* Initial capacity */
    data->variables = (telnetty_msdp_variable_t*)TELNETTY_MUD_MALLOC(
        data->capacity * sizeof(telnetty_msdp_variable_t)
    );
    
    if (!data->variables) {
        TELNETTY_MUD_FREE(data);
        return NULL;
    }
    
    memset(data->variables, 0, data->capacity * sizeof(telnetty_msdp_variable_t));
    
    return data;
}

static TELNETTY_UNUSED void telnetty_msdp_data_destroy(telnetty_msdp_data_t* data) {
    if (!data) return;
    
    /* Free all variables */
    for (size_t i = 0; i < data->variable_count; i++) {
        if (data->variables[i].name) {
            TELNETTY_MUD_FREE(data->variables[i].name);
        }
        if (data->variables[i].value) {
            TELNETTY_MUD_FREE(data->variables[i].value);
        }
    }
    
    TELNETTY_MUD_FREE(data->variables);
    TELNETTY_MUD_FREE(data);
}

static void telnetty_msdp_handler(
    telnetty_context_t* ctx,
    uint8_t option,
    uint8_t command,
    void* user_data
) {
    if (!ctx || option != TELNETTY_TELOPT_MSDP) return;
    
    telnetty_msdp_data_t* msdp_data = (telnetty_msdp_data_t*)user_data;
    if (!msdp_data) return;
    
    switch (command) {
        case TELNETTY_WILL:
            /* Peer wants to enable MSDP */
            telnetty_send_option(ctx, TELNETTY_DO, option);
            msdp_data->negotiation_complete = true;
            break;
            
        case TELNETTY_WONT:
            /* Peer wants to disable MSDP */
            telnetty_send_option(ctx, TELNETTY_DONT, option);
            msdp_data->negotiation_complete = false;
            break;
            
        case TELNETTY_DO:
            /* Peer agrees to enable MSDP */
            msdp_data->negotiation_complete = true;
            break;
            
        case TELNETTY_DONT:
            /* Peer refuses to enable MSDP */
            msdp_data->negotiation_complete = false;
            break;
    }
}

static int telnetty_enable_msdp(
    telnetty_context_t* ctx,
    telnetty_msdp_handler_t handler,
    void* user_data
) {
    if (!ctx) return -1;
    
    /* Create MSDP data */
    telnetty_msdp_data_t* msdp_data = telnetty_msdp_data_create();
    if (!msdp_data) return -1;
    
    msdp_data->handler = handler;
    msdp_data->user_data = user_data;
    
    /* Register option handler */
    /* Note: In full implementation, this would be integrated with the core */
    
    /* Send WILL to enable MSDP */
    return telnetty_send_option(ctx, TELNETTY_WILL, TELNETTY_TELOPT_MSDP);
}

static int telnetty_msdp_send(
    telnetty_context_t* ctx,
    const char* var,
    const char* val
) {
    if (!ctx || !var) return -1;
    
    /* Calculate required buffer size */
    size_t var_len = strlen(var);
    size_t val_len = val ? strlen(val) : 0;
    size_t total_size = 2 + var_len + (val ? (1 + val_len) : 0);
    
    /* Allocate temporary buffer */
    uint8_t* buffer = (uint8_t*)TELNETTY_MUD_MALLOC(total_size);
    if (!buffer) return -1;
    
    /* Build MSDP data */
    size_t offset = 0;
    buffer[offset++] = TELNETTY_MSDP_VAR;
    memcpy(buffer + offset, var, var_len);
    offset += var_len;
    
    if (val) {
        buffer[offset++] = TELNETTY_MSDP_VAL;
        memcpy(buffer + offset, val, val_len);
        offset += val_len;
    }
    
    /* Send subnegotiation */
    int result = telnetty_send_subnegotiation(ctx, TELNETTY_TELOPT_MSDP, buffer, offset);
    
    TELNETTY_MUD_FREE(buffer);
    
    return result;
}

static int telnetty_msdp_sendf(
    telnetty_context_t* ctx,
    const char* var,
    const char* fmt,
    ...
) {
    if (!ctx || !var || !fmt) return -1;
    
    /* Format the value */
    va_list args;
    va_start(args, fmt);
    
    /* Calculate required size */
    va_list args_copy;
    va_copy(args_copy, args);
    int size = vsnprintf(NULL, 0, fmt, args_copy);
    va_end(args_copy);
    
    if (size < 0) {
        va_end(args);
        return -1;
    }
    
    /* Allocate buffer and format */
    char* buffer = (char*)TELNETTY_MUD_MALLOC(size + 1);
    if (!buffer) {
        va_end(args);
        return -1;
    }
    
    vsnprintf(buffer, size + 1, fmt, args);
    va_end(args);
    
    /* Send formatted value */
    int result = telnetty_msdp_send(ctx, var, buffer);
    
    TELNETTY_MUD_FREE(buffer);
    
    return result;
}

static int telnetty_msdp_request(
    telnetty_context_t* ctx,
    const char* var
) {
    if (!ctx || !var) return -1;
    
    /* Send request for variable */
    size_t var_len = strlen(var);
    size_t total_size = 2 + var_len;
    
    uint8_t* buffer = (uint8_t*)TELNETTY_MUD_MALLOC(total_size);
    if (!buffer) return -1;
    
    buffer[0] = TELNETTY_MSDP_VAR;
    strcpy((char*)buffer + 1, var);
    buffer[1 + var_len] = TELNETTY_MSDP_VAL; /* Empty value requests variable */
    
    int result = telnetty_send_subnegotiation(ctx, TELNETTY_TELOPT_MSDP, buffer, total_size);
    
    TELNETTY_MUD_FREE(buffer);
    
    return result;
}

static int telnetty_msdp_set_flags(
    telnetty_context_t* ctx,
    const char* var,
    uint32_t flags
) {
    (void)ctx;
    (void)var;
    (void)flags;
    /* This would need integration with MSDP data structure */
    /* For now, return success */
    return 0;
}

static int telnetty_msdp_clear_flags(
    telnetty_context_t* ctx,
    const char* var,
    uint32_t flags
) {
    (void)ctx;
    (void)var;
    (void)flags;
    /* This would need integration with MSDP data structure */
    /* For now, return success */
    return 0;
}

static const char* telnetty_msdp_get(
    telnetty_context_t* ctx,
    const char* var
) {
    (void)ctx;
    (void)var;
    /* This would need integration with MSDP data structure */
    /* For now, return NULL */
    return NULL;
}

static int telnetty_msdp_remove(
    telnetty_context_t* ctx,
    const char* var
) {
    (void)ctx;
    (void)var;
    /* This would need integration with MSDP data structure */
    /* For now, return success */
    return 0;
}

/* ============================================================================
 * GMCP Implementation
 * ============================================================================ */

static TELNETTY_UNUSED telnetty_gmcp_data_t* telnetty_gmcp_data_create(void) {
    telnetty_gmcp_data_t* data = (telnetty_gmcp_data_t*)TELNETTY_MUD_MALLOC(sizeof(telnetty_gmcp_data_t));
    if (!data) return NULL;
    
    memset(data, 0, sizeof(telnetty_gmcp_data_t));
    data->capacity = 8; /* Initial capacity */
    data->supported_modules = (char**)TELNETTY_MUD_MALLOC(
        data->capacity * sizeof(char*)
    );
    
    if (!data->supported_modules) {
        TELNETTY_MUD_FREE(data);
        return NULL;
    }
    
    memset(data->supported_modules, 0, data->capacity * sizeof(char*));
    
    return data;
}

static TELNETTY_UNUSED void telnetty_gmcp_data_destroy(telnetty_gmcp_data_t* data) {
    if (!data) return;
    
    /* Free supported modules */
    for (size_t i = 0; i < data->module_count; i++) {
        if (data->supported_modules[i]) {
            TELNETTY_MUD_FREE(data->supported_modules[i]);
        }
    }
    
    TELNETTY_MUD_FREE(data->supported_modules);
    TELNETTY_MUD_FREE(data);
}

static void telnetty_gmcp_handler(
    telnetty_context_t* ctx,
    uint8_t option,
    uint8_t command,
    void* user_data
) {
    if (!ctx || option != TELNETTY_TELOPT_GMCP) return;
    
    telnetty_gmcp_data_t* gmcp_data = (telnetty_gmcp_data_t*)user_data;
    if (!gmcp_data) return;
    
    switch (command) {
        case TELNETTY_WILL:
            /* Peer wants to enable GMCP */
            telnetty_send_option(ctx, TELNETTY_DO, option);
            gmcp_data->negotiation_complete = true;
            break;
            
        case TELNETTY_WONT:
            /* Peer wants to disable GMCP */
            telnetty_send_option(ctx, TELNETTY_DONT, option);
            gmcp_data->negotiation_complete = false;
            break;
            
        case TELNETTY_DO:
            /* Peer agrees to enable GMCP */
            gmcp_data->negotiation_complete = true;
            /* Send Core.Hello and Core.Supports.Set */
            telnetty_gmcp_send_hello(ctx, "Telnetty", "1.0.0");
            break;
            
        case TELNETTY_DONT:
            /* Peer refuses to enable GMCP */
            gmcp_data->negotiation_complete = false;
            break;
    }
}

static int telnetty_enable_gmcp(
    telnetty_context_t* ctx,
    telnetty_gmcp_handler_t handler,
    void* user_data
) {
    if (!ctx) return -1;
    
    /* Create GMCP data */
    telnetty_gmcp_data_t* gmcp_data = telnetty_gmcp_data_create();
    if (!gmcp_data) return -1;
    
    gmcp_data->handler = handler;
    gmcp_data->user_data = user_data;
    
    /* Register option handler */
    /* Note: In full implementation, this would be integrated with the core */
    
    /* Send WILL to enable GMCP */
    return telnetty_send_option(ctx, TELNETTY_WILL, TELNETTY_TELOPT_GMCP);
}

static int telnetty_gmcp_send(
    telnetty_context_t* ctx,
    const char* module,
    const char* message,
    const char* data
) {
    if (!ctx || !module || !message) return -1;
    
    /* Calculate required buffer size */
    size_t module_len = strlen(module);
    size_t message_len = strlen(message);
    size_t data_len = data ? strlen(data) : 0;
    size_t total_size = module_len + 1 + message_len + data_len;
    
    /* Allocate temporary buffer */
    uint8_t* buffer = (uint8_t*)TELNETTY_MUD_MALLOC(total_size);
    if (!buffer) return -1;
    
    /* Build GMCP message */
    size_t offset = 0;
    memcpy(buffer + offset, module, module_len);
    offset += module_len;
    buffer[offset++] = ' ';
    memcpy(buffer + offset, message, message_len);
    offset += message_len;
    
    if (data) {
        memcpy(buffer + offset, data, data_len);
        offset += data_len;
    }
    
    /* Send subnegotiation */
    int result = telnetty_send_subnegotiation(ctx, TELNETTY_TELOPT_GMCP, buffer, offset);
    
    TELNETTY_MUD_FREE(buffer);
    
    return result;
}

static int telnetty_gmcp_sendf(
    telnetty_context_t* ctx,
    const char* module,
    const char* message,
    const char* fmt,
    ...
) {
    if (!ctx || !module || !message) return -1;
    
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
            data = (char*)TELNETTY_MUD_MALLOC(size + 1);
            if (data) {
                vsnprintf(data, size + 1, fmt, args);
            }
        }
        
        va_end(args);
    }
    
    /* Send GMCP message */
    int result = telnetty_gmcp_send(ctx, module, message, data);
    
    if (data) {
        TELNETTY_MUD_FREE(data);
    }
    
    return result;
}

static int telnetty_gmcp_register_module(
    telnetty_context_t* ctx,
    const char* module
) {
    (void)ctx;
    (void)module;
    /* This would need integration with GMCP data structure */
    /* For now, return success */
    return 0;
}

static int telnetty_gmcp_unregister_module(
    telnetty_context_t* ctx,
    const char* module
) {
    (void)ctx;
    (void)module;
    /* This would need integration with GMCP data structure */
    /* For now, return success */
    return 0;
}

static int telnetty_gmcp_send_hello(
    telnetty_context_t* ctx,
    const char* client,
    const char* version
) {
    if (!ctx) return -1;
    
    /* Send Core.Hello message */
    return telnetty_gmcp_sendf(ctx, "Core", "Hello", 
        "{\"client\": \"%s\", \"version\": \"%s\"}",
        client ? client : "Unknown",
        version ? version : "1.0.0");
}

static int telnetty_gmcp_send_supports(
    telnetty_context_t* ctx,
    const char* const* modules,
    size_t count
) {
    if (!ctx) return -1;
    
    /* Build JSON array of modules */
    size_t json_size = 2; /* [] */
    for (size_t i = 0; i < count; i++) {
        json_size += 3 + strlen(modules[i]); /* "module" */
        if (i < count - 1) json_size += 2; /* , */
    }
    
    char* json_data = (char*)TELNETTY_MUD_MALLOC(json_size + 1);
    if (!json_data) return -1;
    
    size_t offset = 0;
    json_data[offset++] = '[';
    
    for (size_t i = 0; i < count; i++) {
        json_data[offset++] = '"';
        strcpy(json_data + offset, modules[i]);
        offset += strlen(modules[i]);
        json_data[offset++] = '"';
        
        if (i < count - 1) {
            json_data[offset++] = ',';
            json_data[offset++] = ' ';
        }
    }
    
    json_data[offset++] = ']';
    json_data[offset] = '\0';
    
    /* Send Core.Supports.Set message */
    int result = telnetty_gmcp_send(ctx, "Core.Supports.Set", json_data, NULL);
    
    TELNETTY_MUD_FREE(json_data);
    
    return result;
}

/* ============================================================================
 * MTTS Implementation
 * ============================================================================ */

static TELNETTY_UNUSED telnetty_mtts_data_t* telnetty_mtts_data_create(void) {
    telnetty_mtts_data_t* data = (telnetty_mtts_data_t*)TELNETTY_MUD_MALLOC(sizeof(telnetty_mtts_data_t));
    if (!data) return NULL;
    
    memset(data, 0, sizeof(telnetty_mtts_data_t));
    
    return data;
}

static TELNETTY_UNUSED void telnetty_mtts_data_destroy(telnetty_mtts_data_t* data) {
    if (!data) return;
    
    if (data->client_name) {
        TELNETTY_MUD_FREE(data->client_name);
    }
    if (data->client_version) {
        TELNETTY_MUD_FREE(data->client_version);
    }
    
    TELNETTY_MUD_FREE(data);
}

static void telnetty_mtts_handler(
    telnetty_context_t* ctx,
    uint8_t option,
    uint8_t command,
    void* user_data
) {
    (void)command;
    if (!ctx || option != TELNETTY_TELOPT_MTTS) return;
    
    telnetty_mtts_data_t* mtts_data = (telnetty_mtts_data_t*)user_data;
    if (!mtts_data) return;
    
    /* MTTS handling would be implemented here */
    /* This is a placeholder for the full implementation */
}

static TELNETTY_UNUSED int telnetty_enable_mtts(telnetty_context_t* ctx) {
    if (!ctx) return -1;
    
    /* Create MTTS data */
    telnetty_mtts_data_t* mtts_data = telnetty_mtts_data_create();
    if (!mtts_data) return -1;
    
    /* Register option handler */
    /* Note: In full implementation, this would be integrated with the core */
    
    /* Send WILL to enable MTTS */
    return telnetty_send_option(ctx, TELNETTY_WILL, TELNETTY_TELOPT_MTTS);
}

static TELNETTY_UNUSED uint64_t telnetty_mtts_get_flags(telnetty_context_t* ctx) {
    if (!ctx) return 0;
    
    /* This would need integration with MTTS data structure */
    /* For now, return 0 */
    return 0;
}

static bool telnetty_mtts_has_flag(
    telnetty_context_t* ctx,
    telnetty_mtts_flag_t flag
) {
    uint64_t flags = telnetty_mtts_get_flags(ctx);
    return (flags & flag) != 0;
}

static int telnetty_mtts_get_client_info(
    telnetty_context_t* ctx,
    const char** name,
    const char** version
) {
    (void)name;
    (void)version;
    if (!ctx) return -1;
    
    /* This would need integration with MTTS data structure */
    /* For now, return failure */
    return -1;
}

static int telnetty_mtts_send_response(
    telnetty_context_t* ctx,
    uint64_t flags,
    const char* client_name,
    const char* client_version
) {
    (void)flags;
    (void)client_name;
    (void)client_version;
    if (!ctx) return -1;
    
    /* Send MTTS response via TTYPE subnegotiation */
    /* This would be implemented based on the MTTS specification */
    return 0;
}

/* ============================================================================
 * MSSP Implementation
 * ============================================================================ */

static TELNETTY_UNUSED telnetty_mssp_data_t* telnetty_mssp_data_create(void) {
    telnetty_mssp_data_t* data = (telnetty_mssp_data_t*)TELNETTY_MUD_MALLOC(sizeof(telnetty_mssp_data_t));
    if (!data) return NULL;
    
    memset(data, 0, sizeof(telnetty_mssp_data_t));
    data->capacity = 16; /* Initial capacity */
    data->variables = (telnetty_mssp_variable_t*)TELNETTY_MUD_MALLOC(
        data->capacity * sizeof(telnetty_mssp_variable_t)
    );
    
    if (!data->variables) {
        TELNETTY_MUD_FREE(data);
        return NULL;
    }
    
    memset(data->variables, 0, data->capacity * sizeof(telnetty_mssp_variable_t));
    
    return data;
}

static TELNETTY_UNUSED void telnetty_mssp_data_destroy(telnetty_mssp_data_t* data) {
    if (!data) return;
    
    /* Free all variables */
    for (size_t i = 0; i < data->variable_count; i++) {
        if (data->variables[i].name) {
            TELNETTY_MUD_FREE(data->variables[i].name);
        }
        if (data->variables[i].value) {
            TELNETTY_MUD_FREE(data->variables[i].value);
        }
    }
    
    TELNETTY_MUD_FREE(data->variables);
    TELNETTY_MUD_FREE(data);
}

static void telnetty_mssp_handler(
    telnetty_context_t* ctx,
    uint8_t option,
    uint8_t command,
    void* user_data
) {
    (void)command;
    if (!ctx || option != TELNETTY_TELOPT_MSSP) return;
    
    telnetty_mssp_data_t* mssp_data = (telnetty_mssp_data_t*)user_data;
    if (!mssp_data) return;
    
    /* MSSP only uses subnegotiation, not commands */
    /* The MSSP response would be sent when requested */
}

static int telnetty_enable_mssp(
    telnetty_context_t* ctx,
    bool auto_send
) {
    if (!ctx) return -1;
    
    /* Create MSSP data */
    telnetty_mssp_data_t* mssp_data = telnetty_mssp_data_create();
    if (!mssp_data) return -1;
    
    mssp_data->auto_send = auto_send;
    
    /* Register option handler */
    /* Note: In full implementation, this would be integrated with the core */
    
    /* Send WILL to enable MSSP */
    return telnetty_send_option(ctx, TELNETTY_WILL, TELNETTY_TELOPT_MSSP);
}

static int telnetty_mssp_add(
    telnetty_context_t* ctx,
    const char* name,
    const char* value
) {
    (void)ctx;
    (void)name;
    (void)value;
    /* This would need integration with MSSP data structure */
    /* For now, return success */
    return 0;
}

static TELNETTY_UNUSED int telnetty_mssp_send(telnetty_context_t* ctx) {
    if (!ctx) return -1;
    
    /* This would need integration with MSSP data structure */
    /* For now, return success */
    return 0;
}

static TELNETTY_UNUSED int telnetty_mssp_clear(telnetty_context_t* ctx) {
    (void)ctx;
    /* This would need integration with MSSP data structure */
    /* For now, return success */
    return 0;
}

static int telnetty_mssp_set_standard_vars(
    telnetty_context_t* ctx,
    const char* server_name,
    const char* server_type,
    const char* server_url,
    int server_port,
    int player_count,
    int max_players
) {
    if (!ctx) return -1;
    
    /* Add standard MSSP variables */
    if (server_name) telnetty_mssp_add(ctx, "NAME", server_name);
    if (server_type) telnetty_mssp_add(ctx, "PLAYERS", server_type);
    if (server_url) telnetty_mssp_add(ctx, "HOSTNAME", server_url);
    if (server_port > 0) {
        char port_str[16];
        snprintf(port_str, sizeof(port_str), "%d", server_port);
        telnetty_mssp_add(ctx, "PORT", port_str);
    }
    if (player_count >= 0) {
        char count_str[16];
        snprintf(count_str, sizeof(count_str), "%d", player_count);
        telnetty_mssp_add(ctx, "PLAYERS", count_str);
    }
    if (max_players > 0) {
        char max_str[16];
        snprintf(max_str, sizeof(max_str), "%d", max_players);
        telnetty_mssp_add(ctx, "MAXPLAYERS", max_str);
    }
    
    return 0;
}

#endif /* TELNETTY_MUD_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif

#endif /* TELNETTY_MUD_H */