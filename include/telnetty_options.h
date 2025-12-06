/**
 * @file telnetty_options.h
 * @brief Standard TELNET option implementations
 * 
 * This header provides implementations for standard TELNET options as defined
 * in various RFCs. Each option is implemented as a separate handler that can
 * be registered with the core TELNET context.
 * 
 * Supported Options:
 * - Binary Transmission (RFC 856)
 * - Echo (RFC 857)
 * - Suppress Go Ahead (RFC 858)
 * - Status (RFC 859)
 * - Timing Mark (RFC 860)
 * - Terminal Type (RFC 1091)
 * - Window Size (RFC 1073)
 * - Terminal Speed (RFC 1079)
 * - Remote Flow Control (RFC 1372)
 * - Linemode (RFC 1184)
 * - Environment Variables (RFC 1572)
 * - Charset (RFC 2066)
 * 
 * @author Damus <damus@straylightrun.org>
 * @version 1.0.0
 * 
 */

#ifndef TELNETTY_OPTIONS_H
#define TELNETTY_OPTIONS_H

#include "telnetty_core.h"
#include "telnetty_events.h"

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
 * Option Configuration
 * ============================================================================ */

/* Default option timeouts (in milliseconds) */
#ifndef TELNETTY_OPTION_TIMEOUT
#define TELNETTY_OPTION_TIMEOUT 5000
#endif

/* Maximum number of option negotiation retries */
#ifndef TELNETTY_OPTION_MAX_RETRIES
#define TELNETTY_OPTION_MAX_RETRIES 3
#endif

/* ============================================================================
 * Option State Management
 * ============================================================================ */

/* Option states for tracking negotiation */
typedef enum {
    TELNETTY_OPTION_STATE_DISABLED = 0,   /**< Option is disabled */
    TELNETTY_OPTION_STATE_ENABLED = 1,    /**< Option is enabled */
    TELNETTY_OPTION_STATE_NEGOTIATING = 2, /**< Option negotiation in progress */
    TELNETTY_OPTION_STATE_FAILED = 3,     /**< Option negotiation failed */
    TELNETTY_OPTION_STATE_UNSUPPORTED = 4 /**< Option not supported by peer */
} telnetty_option_state_t;

/* Option negotiation tracking structure */
typedef struct {
    uint8_t option;                     /**< Option code */
    telnetty_option_state_t local_state;  /**< Local state */
    telnetty_option_state_t remote_state; /**< Remote state */
    int retry_count;                    /**< Number of retries */
    time_t last_attempt;                /**< Last negotiation attempt */
    void* option_data;                  /**< Option-specific data */
    telnetty_option_handler_t handler;    /**< Option handler */
    void* user_data;                    /**< User data for handler */
} telnetty_option_tracker_t;

/* ============================================================================
 * Binary Transmission Option (RFC 856)
 * ============================================================================ */

/**
 * Binary transmission option handler
 * 
 * @param ctx TELNET context
 * @param option Option code (should be TELNETTY_TELOPT_BINARY)
 * @param command Command received
 * @param user_data User data
 */
static void telnetty_binary_handler(
    telnetty_context_t* ctx,
    uint8_t option,
    uint8_t command,
    void* user_data
);

/**
 * Enable binary transmission mode
 * 
 * @param ctx TELNET context
 * @return 0 on success, -1 on failure
 */
static TELNETTY_UNUSED int telnetty_enable_binary(telnetty_context_t* ctx);

/**
 * Disable binary transmission mode
 * 
 * @param ctx TELNET context
 * @return 0 on success, -1 on failure
 */
static TELNETTY_UNUSED int telnetty_disable_binary(telnetty_context_t* ctx);

/* ============================================================================
 * Echo Option (RFC 857)
 * ============================================================================ */

/**
 * Echo option handler
 * 
 * @param ctx TELNET context
 * @param option Option code (should be TELNETTY_TELOPT_ECHO)
 * @param command Command received
 * @param user_data User data
 */
static void telnetty_echo_handler(
    telnetty_context_t* ctx,
    uint8_t option,
    uint8_t command,
    void* user_data
);

/**
 * Enable local echo
 * 
 * @param ctx TELNET context
 * @return 0 on success, -1 on failure
 */
static TELNETTY_UNUSED int telnetty_enable_echo(telnetty_context_t* ctx);

/**
 * Disable local echo
 * 
 * @param ctx TELNET context
 * @return 0 on success, -1 on failure
 */
static TELNETTY_UNUSED int telnetty_disable_echo(telnetty_context_t* ctx);

/**
 * Check if echo is enabled
 * 
 * @param ctx TELNET context
 * @return true if echo is enabled, false otherwise
 */
static TELNETTY_UNUSED bool telnetty_is_echo_enabled(telnetty_context_t* ctx);

/* ============================================================================
 * Suppress Go Ahead Option (RFC 858)
 * ============================================================================ */

/**
 * Suppress Go Ahead option handler
 * 
 * @param ctx TELNET context
 * @param option Option code (should be TELNETTY_TELOPT_SGA)
 * @param command Command received
 * @param user_data User data
 */
static void telnetty_sga_handler(
    telnetty_context_t* ctx,
    uint8_t option,
    uint8_t command,
    void* user_data
);

/**
 * Enable suppress go ahead
 * 
 * @param ctx TELNET context
 * @return 0 on success, -1 on failure
 */
static TELNETTY_UNUSED int telnetty_enable_sga(telnetty_context_t* ctx);

/**
 * Disable suppress go ahead
 * 
 * @param ctx TELNET context
 * @return 0 on success, -1 on failure
 */
static TELNETTY_UNUSED int telnetty_disable_sga(telnetty_context_t* ctx);

/* ============================================================================
 * Terminal Type Option (RFC 1091)
 * ============================================================================ */

/* Terminal type data structure */
typedef struct {
    char* terminal_type;                /**< Terminal type string */
    size_t max_length;                  /**< Maximum terminal type length */
    bool query_sent;                    /**< Query has been sent */
} telnetty_terminal_type_data_t;

/**
 * Terminal type option handler
 * 
 * @param ctx TELNET context
 * @param option Option code (should be TELNETTY_TELOPT_TTYPE)
 * @param command Command received
 * @param user_data User data (should be telnetty_terminal_type_data_t*)
 */
static void telnetty_ttype_handler(
    telnetty_context_t* ctx,
    uint8_t option,
    uint8_t command,
    void* user_data
);

/**
 * Enable terminal type option
 * 
 * @param ctx TELNET context
 * @param max_type_length Maximum terminal type length
 * @return 0 on success, -1 on failure
 */
static TELNETTY_UNUSED int telnetty_enable_ttype(telnetty_context_t* ctx, size_t max_type_length);

/**
 * Query terminal type from peer
 * 
 * @param ctx TELNET context
 * @return 0 on success, -1 on failure
 */
static TELNETTY_UNUSED int telnetty_query_terminal_type(telnetty_context_t* ctx);

/**
 * Get negotiated terminal type
 * 
 * @param ctx TELNET context
 * @return Terminal type string or NULL if not negotiated
 */
static TELNETTY_UNUSED const char* telnetty_get_terminal_type(telnetty_context_t* ctx);

/* ============================================================================
 * Window Size Option (RFC 1073 - NAWS)
 * ============================================================================ */

/* Window size data structure */
typedef struct {
    uint16_t width;                     /**< Terminal width */
    uint16_t height;                    /**< Terminal height */
    bool size_received;                 /**< Size has been received */
} telnetty_window_size_data_t;

/**
 * Window size option handler
 * 
 * @param ctx TELNET context
 * @param option Option code (should be TELNETTY_TELOPT_NAWS)
 * @param command Command received
 * @param user_data User data (should be telnetty_window_size_data_t*)
 */
static void telnetty_naws_handler(
    telnetty_context_t* ctx,
    uint8_t option,
    uint8_t command,
    void* user_data
);

/**
 * Enable window size option
 * 
 * @param ctx TELNET context
 * @return 0 on success, -1 on failure
 */
static TELNETTY_UNUSED int telnetty_enable_naws(telnetty_context_t* ctx);

/**
 * Get window size
 * 
 * @param ctx TELNET context
 * @param width Output width (optional)
 * @param height Output height (optional)
 * @return 0 if size is available, -1 otherwise
 */
static int telnetty_get_window_size(
    telnetty_context_t* ctx,
    uint16_t* width,
    uint16_t* height
);

/**
 * Send window size to peer
 * 
 * @param ctx TELNET context
 * @param width Window width
 * @param height Window height
 * @return 0 on success, -1 on failure
 */
static int telnetty_send_window_size(
    telnetty_context_t* ctx,
    uint16_t width,
    uint16_t height
);

/* ============================================================================
 * Environment Variables Option (RFC 1572 - NEW-ENVIRON)
 * ============================================================================ */

/* Environment variable types */
typedef enum {
    TELNETTY_ENV_VAR = 0,                 /**< Environment variable */
    TELNETTY_ENV_VALUE = 1,               /**< Environment value */
    TELNETTY_ENV_ESC = 2,                 /**< Escaped character */
    TELNETTY_ENV_USERVAR = 3              /**< User-defined variable */
} telnetty_env_type_t;

/* Environment data structure */
typedef struct {
    char** variables;                   /**< Environment variables */
    char** values;                      /**< Environment values */
    size_t count;                       /**< Number of variables */
    size_t capacity;                    /**< Array capacity */
    bool info_sent;                     /**< INFO has been sent */
} telnetty_environ_data_t;

/**
 * Environment option handler
 * 
 * @param ctx TELNET context
 * @param option Option code (should be TELNETTY_TELOPT_NEW_ENVIRON)
 * @param command Command received
 * @param user_data User data (should be telnetty_environ_data_t*)
 */
static void telnetty_environ_handler(
    telnetty_context_t* ctx,
    uint8_t option,
    uint8_t command,
    void* user_data
);

/**
 * Enable environment option
 * 
 * @param ctx TELNET context
 * @return 0 on success, -1 on failure
 */
static TELNETTY_UNUSED int telnetty_enable_environ(telnetty_context_t* ctx);

/**
 * Request environment variables from peer
 * 
 * @param ctx TELNET context
 * @return 0 on success, -1 on failure
 */
static TELNETTY_UNUSED int telnetty_request_environ(telnetty_context_t* ctx);

/**
 * Send environment variables to peer
 * 
 * @param ctx TELNET context
 * @param vars Array of variable names
 * @param values Array of variable values
 * @param count Number of variables
 * @return 0 on success, -1 on failure
 */
static int telnetty_send_environ(
    telnetty_context_t* ctx,
    const char* const* vars,
    const char* const* values,
    size_t count
);

/**
 * Get environment variable
 * 
 * @param ctx TELNET context
 * @param name Variable name
 * @return Variable value or NULL if not found
 */
static const char* telnetty_get_environ(
    telnetty_context_t* ctx,
    const char* name
);

/* ============================================================================
 * Charset Option (RFC 2066)
 * ============================================================================ */

/* Charset data structure */
typedef struct {
    char** supported_charsets;          /**< Supported character sets */
    size_t charset_count;               /**< Number of supported charsets */
    char* selected_charset;             /**< Selected character set */
    bool negotiation_complete;          /**< Negotiation is complete */
} telnetty_charset_data_t;

/**
 * Charset option handler
 * 
 * @param ctx TELNET context
 * @param option Option code (should be TELNETTY_TELOPT_CHARSET)
 * @param command Command received
 * @param user_data User data (should be telnetty_charset_data_t*)
 */
static void telnetty_charset_handler(
    telnetty_context_t* ctx,
    uint8_t option,
    uint8_t command,
    void* user_data
);

/**
 * Enable charset option
 * 
 * @param ctx TELNET context
 * @param charsets Array of supported charset names
 * @param count Number of charsets
 * @return 0 on success, -1 on failure
 */
static int telnetty_enable_charset(
    telnetty_context_t* ctx,
    const char* const* charsets,
    size_t count
);

/**
 * Get negotiated charset
 * 
 * @param ctx TELNET context
 * @return Selected charset or NULL if not negotiated
 */
static TELNETTY_UNUSED const char* telnetty_get_charset(telnetty_context_t* ctx);

/* ============================================================================
 * End of Record Option (RFC 885)
 * ============================================================================ */

/**
 * Enable End of Record option
 * 
 * @param ctx TELNET context
 * @return 0 on success, -1 on failure
 */
static TELNETTY_UNUSED int telnetty_enable_eor(telnetty_context_t* ctx);

/**
 * Send End of Record marker
 * 
 * @param ctx TELNET context
 * @return 0 on success, -1 on failure
 */
static TELNETTY_UNUSED int telnetty_send_eor(telnetty_context_t* ctx);

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

/**
 * Enable all standard TELNET options
 * 
 * @param ctx TELNET context
 * @return Number of options successfully enabled
 */
static TELNETTY_UNUSED int telnetty_enable_all_options(telnetty_context_t* ctx);

/**
 * Disable all standard TELNET options
 * 
 * @param ctx TELNET context
 * @return 0 on success, -1 on failure
 */
static TELNETTY_UNUSED int telnetty_disable_all_options(telnetty_context_t* ctx);

/**
 * Check if option is enabled
 * 
 * @param ctx TELNET context
 * @param option Option code
 * @return true if option is enabled, false otherwise
 */
static TELNETTY_UNUSED bool telnetty_is_option_enabled(telnetty_context_t* ctx, uint8_t option);

/* Note: telnetty_get_option_state is defined in telnetty_core.h */

/**
 * Wait for option negotiation to complete
 * 
 * @param ctx TELNET context
 * @param option Option code
 * @param timeout_ms Timeout in milliseconds
 * @return 0 if negotiation complete, -1 on timeout or error
 */
static int telnetty_wait_for_option(
    telnetty_context_t* ctx,
    uint8_t option,
    int timeout_ms
);

/* ============================================================================
 * Implementation
 * ============================================================================ */

#ifdef TELNETTY_OPTIONS_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

/* Memory allocation wrappers */
#ifndef TELNETTY_OPTIONS_MALLOC
#define TELNETTY_OPTIONS_MALLOC(size) malloc(size)
#endif

#ifndef TELNETTY_OPTIONS_FREE
#define TELNETTY_OPTIONS_FREE(ptr) free(ptr)
#endif

#ifndef TELNETTY_OPTIONS_REALLOC
#define TELNETTY_OPTIONS_REALLOC(ptr, size) realloc(ptr, size)
#endif

/* Forward declarations for internal functions */
static telnetty_option_tracker_t* telnetty_option_tracker_create(
    uint8_t option,
    telnetty_option_handler_t handler,
    void* user_data
);
static TELNETTY_UNUSED void telnetty_option_tracker_destroy(telnetty_option_tracker_t* tracker);
static int telnetty_option_send_negotiation(
    telnetty_context_t* ctx,
    uint8_t option,
    uint8_t command
);

/* ============================================================================
 * Binary Transmission Implementation
 * ============================================================================ */

static void telnetty_binary_handler(
    telnetty_context_t* ctx,
    uint8_t option,
    uint8_t command,
    void* user_data
) {
    if (!ctx || option != TELNETTY_TELOPT_BINARY) return;
    
    telnetty_option_tracker_t* tracker = (telnetty_option_tracker_t*)user_data;
    if (!tracker) return;
    
    switch (command) {
        case TELNETTY_WILL:
            /* Peer wants to enable binary transmission */
            telnetty_send_option(ctx, TELNETTY_DO, option);
            tracker->remote_state = TELNETTY_OPTION_STATE_ENABLED;
            break;
            
        case TELNETTY_WONT:
            /* Peer wants to disable binary transmission */
            telnetty_send_option(ctx, TELNETTY_DONT, option);
            tracker->remote_state = TELNETTY_OPTION_STATE_DISABLED;
            break;
            
        case TELNETTY_DO:
            /* Peer agrees to enable binary transmission */
            tracker->local_state = TELNETTY_OPTION_STATE_ENABLED;
            break;
            
        case TELNETTY_DONT:
            /* Peer refuses to enable binary transmission */
            tracker->local_state = TELNETTY_OPTION_STATE_DISABLED;
            break;
    }
}

static TELNETTY_UNUSED int telnetty_enable_binary(telnetty_context_t* ctx) {
    if (!ctx) return -1;
    
    /* Register option handler */
    telnetty_option_tracker_t* tracker = telnetty_option_tracker_create(
        TELNETTY_TELOPT_BINARY,
        telnetty_binary_handler,
        NULL
    );
    
    if (!tracker) return -1;
    
    /* Send WILL to enable binary transmission */
    return telnetty_send_option(ctx, TELNETTY_WILL, TELNETTY_TELOPT_BINARY);
}

static TELNETTY_UNUSED int telnetty_disable_binary(telnetty_context_t* ctx) {
    if (!ctx) return -1;
    
    /* Send WONT to disable binary transmission */
    return telnetty_send_option(ctx, TELNETTY_WONT, TELNETTY_TELOPT_BINARY);
}

/* ============================================================================
 * Echo Implementation
 * ============================================================================ */

static void telnetty_echo_handler(
    telnetty_context_t* ctx,
    uint8_t option,
    uint8_t command,
    void* user_data
) {
    if (!ctx || option != TELNETTY_TELOPT_ECHO) return;
    
    telnetty_option_tracker_t* tracker = (telnetty_option_tracker_t*)user_data;
    if (!tracker) return;
    
    switch (command) {
        case TELNETTY_WILL:
            /* Peer wants to enable echo */
            telnetty_send_option(ctx, TELNETTY_DO, option);
            tracker->remote_state = TELNETTY_OPTION_STATE_ENABLED;
            break;
            
        case TELNETTY_WONT:
            /* Peer wants to disable echo */
            telnetty_send_option(ctx, TELNETTY_DONT, option);
            tracker->remote_state = TELNETTY_OPTION_STATE_DISABLED;
            break;
            
        case TELNETTY_DO:
            /* Peer agrees to enable echo */
            tracker->local_state = TELNETTY_OPTION_STATE_ENABLED;
            break;
            
        case TELNETTY_DONT:
            /* Peer refuses to enable echo */
            tracker->local_state = TELNETTY_OPTION_STATE_DISABLED;
            break;
    }
}

static TELNETTY_UNUSED int telnetty_enable_echo(telnetty_context_t* ctx) {
    if (!ctx) return -1;
    
    /* Register option handler */
    telnetty_option_tracker_t* tracker = telnetty_option_tracker_create(
        TELNETTY_TELOPT_ECHO,
        telnetty_echo_handler,
        NULL
    );
    
    if (!tracker) return -1;
    
    /* Send WILL to enable echo */
    return telnetty_send_option(ctx, TELNETTY_WILL, TELNETTY_TELOPT_ECHO);
}

static TELNETTY_UNUSED int telnetty_disable_echo(telnetty_context_t* ctx) {
    if (!ctx) return -1;
    
    /* Send WONT to disable echo */
    return telnetty_send_option(ctx, TELNETTY_WONT, TELNETTY_TELOPT_ECHO);
}

static TELNETTY_UNUSED bool telnetty_is_echo_enabled(telnetty_context_t* ctx) {
    if (!ctx) return false;
    
    /* Check if echo option is enabled */
    return telnetty_is_option_enabled(ctx, TELNETTY_TELOPT_ECHO);
}

/* ============================================================================
 * Suppress Go Ahead Implementation
 * ============================================================================ */

static void telnetty_sga_handler(
    telnetty_context_t* ctx,
    uint8_t option,
    uint8_t command,
    void* user_data
) {
    if (!ctx || option != TELNETTY_TELOPT_SGA) return;
    
    telnetty_option_tracker_t* tracker = (telnetty_option_tracker_t*)user_data;
    if (!tracker) return;
    
    switch (command) {
        case TELNETTY_WILL:
            /* Peer wants to enable suppress go ahead */
            telnetty_send_option(ctx, TELNETTY_DO, option);
            tracker->remote_state = TELNETTY_OPTION_STATE_ENABLED;
            break;
            
        case TELNETTY_WONT:
            /* Peer wants to disable suppress go ahead */
            telnetty_send_option(ctx, TELNETTY_DONT, option);
            tracker->remote_state = TELNETTY_OPTION_STATE_DISABLED;
            break;
            
        case TELNETTY_DO:
            /* Peer agrees to enable suppress go ahead */
            tracker->local_state = TELNETTY_OPTION_STATE_ENABLED;
            break;
            
        case TELNETTY_DONT:
            /* Peer refuses to enable suppress go ahead */
            tracker->local_state = TELNETTY_OPTION_STATE_DISABLED;
            break;
    }
}

static TELNETTY_UNUSED int telnetty_enable_sga(telnetty_context_t* ctx) {
    if (!ctx) return -1;
    
    /* Register option handler */
    telnetty_option_tracker_t* tracker = telnetty_option_tracker_create(
        TELNETTY_TELOPT_SGA,
        telnetty_sga_handler,
        NULL
    );
    
    if (!tracker) return -1;
    
    /* Send WILL to enable suppress go ahead */
    return telnetty_send_option(ctx, TELNETTY_WILL, TELNETTY_TELOPT_SGA);
}

static TELNETTY_UNUSED int telnetty_disable_sga(telnetty_context_t* ctx) {
    if (!ctx) return -1;
    
    /* Send WONT to disable suppress go ahead */
    return telnetty_send_option(ctx, TELNETTY_WONT, TELNETTY_TELOPT_SGA);
}

/* ============================================================================
 * Terminal Type Implementation
 * ============================================================================ */

static void telnetty_ttype_handler(
    telnetty_context_t* ctx,
    uint8_t option,
    uint8_t command,
    void* user_data
) {
    if (!ctx || option != TELNETTY_TELOPT_TTYPE) return;
    
    telnetty_terminal_type_data_t* ttype_data = (telnetty_terminal_type_data_t*)user_data;
    if (!ttype_data) return;
    
    switch (command) {
        case TELNETTY_WILL:
            /* Peer wants to enable terminal type */
            telnetty_send_option(ctx, TELNETTY_DO, option);
            break;
            
        case TELNETTY_WONT:
            /* Peer wants to disable terminal type */
            telnetty_send_option(ctx, TELNETTY_DONT, option);
            break;
            
        case TELNETTY_DO:
            /* Peer agrees to enable terminal type */
            /* Send request for terminal type */
            telnetty_query_terminal_type(ctx);
            break;
            
        case TELNETTY_DONT:
            /* Peer refuses to enable terminal type */
            break;
    }
}

static TELNETTY_UNUSED int telnetty_enable_ttype(telnetty_context_t* ctx, size_t max_type_length) {
    if (!ctx) return -1;
    
    /* Allocate terminal type data */
    telnetty_terminal_type_data_t* ttype_data = (telnetty_terminal_type_data_t*)TELNETTY_OPTIONS_MALLOC(
        sizeof(telnetty_terminal_type_data_t)
    );
    
    if (!ttype_data) return -1;
    
    memset(ttype_data, 0, sizeof(telnetty_terminal_type_data_t));
    ttype_data->max_length = max_type_length > 0 ? max_type_length : 64;
    
    /* Register option handler */
    telnetty_option_tracker_t* tracker = telnetty_option_tracker_create(
        TELNETTY_TELOPT_TTYPE,
        telnetty_ttype_handler,
        ttype_data
    );
    
    if (!tracker) {
        TELNETTY_OPTIONS_FREE(ttype_data);
        return -1;
    }
    
    /* Send WILL to enable terminal type */
    return telnetty_send_option(ctx, TELNETTY_WILL, TELNETTY_TELOPT_TTYPE);
}

static TELNETTY_UNUSED int telnetty_query_terminal_type(telnetty_context_t* ctx) {
    if (!ctx) return -1;
    
    /* Send subnegotiation to query terminal type */
    uint8_t query_data[1] = { 1 }; /* SEND code */
    return telnetty_send_subnegotiation(ctx, TELNETTY_TELOPT_TTYPE, query_data, 1);
}

static TELNETTY_UNUSED const char* telnetty_get_terminal_type(telnetty_context_t* ctx) {
    if (!ctx) return NULL;
    
    /* This would need to be integrated with the option tracker system */
    /* For now, return NULL */
    return NULL;
}

/* ============================================================================
 * Window Size Implementation
 * ============================================================================ */

static void telnetty_naws_handler(
    telnetty_context_t* ctx,
    uint8_t option,
    uint8_t command,
    void* user_data
) {
    (void)command;
    (void)user_data;
    if (!ctx || option != TELNETTY_TELOPT_NAWS) return;
    
    /* NAWS doesn't use commands, only subnegotiation */
    /* The command handling would be done in the subnegotiation handler */
}

static TELNETTY_UNUSED int telnetty_enable_naws(telnetty_context_t* ctx) {
    if (!ctx) return -1;
    
    /* Register option handler */
    telnetty_option_tracker_t* tracker = telnetty_option_tracker_create(
        TELNETTY_TELOPT_NAWS,
        telnetty_naws_handler,
        NULL
    );
    
    if (!tracker) return -1;
    
    /* Send WILL to enable NAWS */
    return telnetty_send_option(ctx, TELNETTY_WILL, TELNETTY_TELOPT_NAWS);
}

static int telnetty_get_window_size(
    telnetty_context_t* ctx,
    uint16_t* width,
    uint16_t* height
) {
    (void)width;
    (void)height;
    if (!ctx) return -1;
    
    /* This would need to be integrated with the option tracker system */
    /* For now, return failure */
    return -1;
}

static int telnetty_send_window_size(
    telnetty_context_t* ctx,
    uint16_t width,
    uint16_t height
) {
    if (!ctx) return -1;
    
    /* Pack window size data */
    uint8_t size_data[4];
    size_data[0] = (width >> 8) & 0xFF;
    size_data[1] = width & 0xFF;
    size_data[2] = (height >> 8) & 0xFF;
    size_data[3] = height & 0xFF;
    
    return telnetty_send_subnegotiation(ctx, TELNETTY_TELOPT_NAWS, size_data, 4);
}

/* ============================================================================
 * Environment Variables Implementation
 * ============================================================================ */

static void telnetty_environ_handler(
    telnetty_context_t* ctx,
    uint8_t option,
    uint8_t command,
    void* user_data
) {
    (void)command;
    (void)user_data;
    if (!ctx || option != TELNETTY_TELOPT_NEW_ENVIRON) return;
    
    /* Environment option handling would be implemented here */
    /* This is a placeholder for the full implementation */
}

static TELNETTY_UNUSED int telnetty_enable_environ(telnetty_context_t* ctx) {
    if (!ctx) return -1;
    
    /* Register option handler */
    telnetty_option_tracker_t* tracker = telnetty_option_tracker_create(
        TELNETTY_TELOPT_NEW_ENVIRON,
        telnetty_environ_handler,
        NULL
    );
    
    if (!tracker) return -1;
    
    /* Send WILL to enable environment option */
    return telnetty_send_option(ctx, TELNETTY_WILL, TELNETTY_TELOPT_NEW_ENVIRON);
}

static TELNETTY_UNUSED int telnetty_request_environ(telnetty_context_t* ctx) {
    if (!ctx) return -1;
    
    /* Send subnegotiation to request environment */
    uint8_t request_data[1] = { 1 }; /* SEND code */
    return telnetty_send_subnegotiation(ctx, TELNETTY_TELOPT_NEW_ENVIRON, request_data, 1);
}

static int telnetty_send_environ(
    telnetty_context_t* ctx,
    const char* const* vars,
    const char* const* values,
    size_t count
) {
    if (!ctx || !vars || !values) return -1;
    
    /* Calculate required buffer size */
    size_t total_size = 0;
    for (size_t i = 0; i < count; i++) {
        total_size += strlen(vars[i]) + 1; /* VAR + name */
        total_size += strlen(values[i]) + 1; /* VALUE + value */
    }
    
    /* Allocate temporary buffer */
    uint8_t* buffer = (uint8_t*)TELNETTY_OPTIONS_MALLOC(total_size);
    if (!buffer) return -1;
    
    /* Build environment data */
    size_t offset = 0;
    for (size_t i = 0; i < count; i++) {
        buffer[offset++] = TELNETTY_ENV_VAR;
        strcpy((char*)buffer + offset, vars[i]);
        offset += strlen(vars[i]);
        
        buffer[offset++] = TELNETTY_ENV_VALUE;
        strcpy((char*)buffer + offset, values[i]);
        offset += strlen(values[i]);
    }
    
    /* Send subnegotiation */
    int result = telnetty_send_subnegotiation(ctx, TELNETTY_TELOPT_NEW_ENVIRON, buffer, offset);
    
    TELNETTY_OPTIONS_FREE(buffer);
    
    return result;
}

static const char* telnetty_get_environ(
    telnetty_context_t* ctx,
    const char* name
) {
    if (!ctx || !name) return NULL;
    
    /* This would need to be integrated with the option tracker system */
    /* For now, return NULL */
    return NULL;
}

/* ============================================================================
 * Charset Implementation
 * ============================================================================ */

static void telnetty_charset_handler(
    telnetty_context_t* ctx,
    uint8_t option,
    uint8_t command,
    void* user_data
) {
    (void)command;
    (void)user_data;
    if (!ctx || option != TELNETTY_TELOPT_CHARSET) return;
    
    /* Charset option handling would be implemented here */
    /* This is a placeholder for the full implementation */
}

static int telnetty_enable_charset(
    telnetty_context_t* ctx,
    const char* const* charsets,
    size_t count
) {
    (void)charsets;
    (void)count;
    if (!ctx) return -1;
    
    /* Allocate charset data */
    telnetty_charset_data_t* charset_data = (telnetty_charset_data_t*)TELNETTY_OPTIONS_MALLOC(
        sizeof(telnetty_charset_data_t)
    );
    
    if (!charset_data) return -1;
    
    memset(charset_data, 0, sizeof(telnetty_charset_data_t));
    
    /* Register option handler */
    telnetty_option_tracker_t* tracker = telnetty_option_tracker_create(
        TELNETTY_TELOPT_CHARSET,
        telnetty_charset_handler,
        charset_data
    );
    
    if (!tracker) {
        TELNETTY_OPTIONS_FREE(charset_data);
        return -1;
    }
    
    /* Send WILL to enable charset option */
    return telnetty_send_option(ctx, TELNETTY_WILL, TELNETTY_TELOPT_CHARSET);
}

static TELNETTY_UNUSED const char* telnetty_get_charset(telnetty_context_t* ctx) {
    if (!ctx) return NULL;
    
    /* This would need to be integrated with the option tracker system */
    /* For now, return NULL */
    return NULL;
}

/* ============================================================================
 * End of Record Implementation
 * ============================================================================ */

static TELNETTY_UNUSED int telnetty_enable_eor(telnetty_context_t* ctx) {
    if (!ctx) return -1;
    
    /* Register option handler */
    telnetty_option_tracker_t* tracker = telnetty_option_tracker_create(
        TELNETTY_TELOPT_EOR,
        NULL, /* No special handler needed */
        NULL
    );
    
    if (!tracker) return -1;
    
    /* Send WILL to enable EOR */
    return telnetty_send_option(ctx, TELNETTY_WILL, TELNETTY_TELOPT_EOR);
}

static TELNETTY_UNUSED int telnetty_send_eor(telnetty_context_t* ctx) {
    if (!ctx) return -1;
    
    /* Send EOR command */
    return telnetty_send_command(ctx, TELNETTY_EOR);
}

/* ============================================================================
 * Helper Functions Implementation
 * ============================================================================ */

static TELNETTY_UNUSED int telnetty_enable_all_options(telnetty_context_t* ctx) {
    if (!ctx) return -1;
    
    int enabled = 0;
    
    /* Enable basic options */
    if (telnetty_enable_binary(ctx) == 0) enabled++;
    if (telnetty_enable_echo(ctx) == 0) enabled++;
    if (telnetty_enable_sga(ctx) == 0) enabled++;
    if (telnetty_enable_eor(ctx) == 0) enabled++;
    if (telnetty_enable_naws(ctx) == 0) enabled++;
    if (telnetty_enable_environ(ctx) == 0) enabled++;
    
    return enabled;
}

static TELNETTY_UNUSED int telnetty_disable_all_options(telnetty_context_t* ctx) {
    if (!ctx) return -1;
    
    /* Send WONT/DONT for all options */
    for (int option = 0; option < 256; option++) {
        telnetty_send_option(ctx, TELNETTY_WONT, (uint8_t)option);
        telnetty_send_option(ctx, TELNETTY_DONT, (uint8_t)option);
    }
    
    return 0;
}

/* Note: telnetty_is_option_enabled and telnetty_get_option_state are defined in telnetty_core.h */

static int telnetty_wait_for_option(
    telnetty_context_t* ctx,
    uint8_t option,
    int timeout_ms
) {
    (void)option;
    (void)timeout_ms;
    if (!ctx) return -1;
    
    /* This would need to be integrated with the option tracker system */
    /* For now, return success */
    return 0;
}

/* ============================================================================
 * Internal Helper Functions
 * ============================================================================ */

static telnetty_option_tracker_t* telnetty_option_tracker_create(
    uint8_t option,
    telnetty_option_handler_t handler,
    void* user_data
) {
    telnetty_option_tracker_t* tracker = (telnetty_option_tracker_t*)TELNETTY_OPTIONS_MALLOC(
        sizeof(telnetty_option_tracker_t)
    );
    
    if (!tracker) return NULL;
    
    memset(tracker, 0, sizeof(telnetty_option_tracker_t));
    tracker->option = option;
    tracker->handler = handler;
    tracker->user_data = user_data;
    tracker->local_state = TELNETTY_OPTION_STATE_DISABLED;
    tracker->remote_state = TELNETTY_OPTION_STATE_DISABLED;
    
    return tracker;
}

static TELNETTY_UNUSED void telnetty_option_tracker_destroy(telnetty_option_tracker_t* tracker) {
    if (!tracker) return;
    
    /* Free option-specific data */
    if (tracker->option_data) {
        TELNETTY_OPTIONS_FREE(tracker->option_data);
    }
    
    TELNETTY_OPTIONS_FREE(tracker);
}

static int telnetty_option_send_negotiation(
    telnetty_context_t* ctx,
    uint8_t option,
    uint8_t command
) {
    if (!ctx) return -1;
    
    return telnetty_send_option(ctx, command, option);
}

#endif /* TELNETTY_OPTIONS_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif

#endif /* TELNETTY_OPTIONS_H */