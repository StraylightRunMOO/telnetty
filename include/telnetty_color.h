/**
 * @file telnetty_color.h
 * @brief Terminal color support for TELNET
 * 
 * This header provides comprehensive color support for TELNET connections,
 * including ANSI color codes, 256-color support, and true color support.
 * 
 * Features:
 * - ANSI color codes (16 colors)
 * - 256-color support (xterm)
 * - True color support (24-bit RGB)
 * - Color state management
 * - Automatic color capability detection
 * 
 * @author Damus <damus@straylightrun.org>
 * @version 1.0.0
 * 
 */

#ifndef TELNETTY_COLOR_H
#define TELNETTY_COLOR_H

#include "telnetty_core.h"
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
 * Color Configuration
 * ============================================================================ */

#ifndef TELNETTY_COLOR_MAX_CODES
#define TELNETTY_COLOR_MAX_CODES 16  /* Maximum color codes in buffer */
#endif

#ifndef TELNETTY_COLOR_BUFFER_SIZE
#define TELNETTY_COLOR_BUFFER_SIZE 256  /* Color buffer size */
#endif

/* ============================================================================
 * ANSI Color Codes
 * ============================================================================ */

/* Standard ANSI colors */
typedef enum {
    TELNETTY_COLOR_BLACK = 0,             /**< Black */
    TELNETTY_COLOR_RED = 1,               /**< Red */
    TELNETTY_COLOR_GREEN = 2,             /**< Green */
    TELNETTY_COLOR_YELLOW = 3,            /**< Yellow */
    TELNETTY_COLOR_BLUE = 4,              /**< Blue */
    TELNETTY_COLOR_MAGENTA = 5,           /**< Magenta */
    TELNETTY_COLOR_CYAN = 6,              /**< Cyan */
    TELNETTY_COLOR_WHITE = 7,             /**< White */
    TELNETTY_COLOR_BRIGHT_BLACK = 8,      /**< Bright black (gray) */
    TELNETTY_COLOR_BRIGHT_RED = 9,        /**< Bright red */
    TELNETTY_COLOR_BRIGHT_GREEN = 10,     /**< Bright green */
    TELNETTY_COLOR_BRIGHT_YELLOW = 11,    /**< Bright yellow */
    TELNETTY_COLOR_BRIGHT_BLUE = 12,      /**< Bright blue */
    TELNETTY_COLOR_BRIGHT_MAGENTA = 13,   /**< Bright magenta */
    TELNETTY_COLOR_BRIGHT_CYAN = 14,      /**< Bright cyan */
    TELNETTY_COLOR_BRIGHT_WHITE = 15      /**< Bright white */
} telnetty_ansi_color_t;

/* Color attributes */
typedef enum {
    TELNETTY_ATTR_RESET = 0,              /**< Reset all attributes */
    TELNETTY_ATTR_BOLD = 1,               /**< Bold/bright */
    TELNETTY_ATTR_DIM = 2,                /**< Dim */
    TELNETTY_ATTR_ITALIC = 3,             /**< Italic */
    TELNETTY_ATTR_UNDERLINE = 4,          /**< Underline */
    TELNETTY_ATTR_BLINK = 5,              /**< Blink */
    TELNETTY_ATTR_REVERSE = 7,            /**< Reverse video */
    TELNETTY_ATTR_STRIKE = 9,             /**< Strike through */
    TELNETTY_ATTR_NO_BOLD = 22,           /**< Remove bold */
    TELNETTY_ATTR_NO_DIM = 22,            /**< Remove dim */
    TELNETTY_ATTR_NO_ITALIC = 23,         /**< Remove italic */
    TELNETTY_ATTR_NO_UNDERLINE = 24,      /**< Remove underline */
    TELNETTY_ATTR_NO_BLINK = 25,          /**< Remove blink */
    TELNETTY_ATTR_NO_REVERSE = 27,        /**< Remove reverse */
    TELNETTY_ATTR_NO_STRIKE = 29          /**< Remove strike */
} telnetty_color_attr_t;

/* Color capability flags */
typedef enum {
    TELNETTY_COLOR_CAP_NONE = 0,          /**< No color support */
    TELNETTY_COLOR_CAP_ANSI = 1 << 0,     /**< ANSI 16-color support */
    TELNETTY_COLOR_CAP_256 = 1 << 1,      /**< 256-color support */
    TELNETTY_COLOR_CAP_TRUE = 1 << 2,     /**< True color support */
    TELNETTY_COLOR_CAP_RGB = 1 << 3       /**< RGB color support */
} telnetty_color_cap_t;

/* ============================================================================
 * Color Data Structures
 * ============================================================================ */

/* RGB color structure */
typedef struct {
    uint8_t r;                          /**< Red component (0-255) */
    uint8_t g;                          /**< Green component (0-255) */
    uint8_t b;                          /**< Blue component (0-255) */
} telnetty_rgb_color_t;

/* Color state structure */
typedef struct {
    telnetty_color_cap_t capabilities;    /**< Color capabilities */
    uint8_t foreground;                 /**< Current foreground color */
    uint8_t background;                 /**< Current background color */
    uint32_t attributes;                /**< Current text attributes */
    bool enabled;                       /**< Color output enabled */
    bool auto_detect;                   /**< Auto-detect capabilities */
    char* buffer;                       /**< Color code buffer */
    size_t buffer_size;                 /**< Buffer size */
    size_t buffer_pos;                  /**< Current buffer position */
} telnetty_color_state_t;

/* ============================================================================
 * Core Color Functions
 * ============================================================================ */

/**
 * Initialize color support
 * 
 * @param ctx TELNET context
 * @param auto_detect Auto-detect color capabilities
 * @return 0 on success, -1 on failure
 */
static int telnetty_color_init(
    telnetty_context_t* ctx,
    bool auto_detect
);

/**
 * Cleanup color support
 * 
 * @param ctx TELNET context
 * @return 0 on success, -1 on failure
 */
static TELNETTY_UNUSED int telnetty_color_cleanup(telnetty_context_t* ctx);

/**
 * Enable color output
 * 
 * @param ctx TELNET context
 * @return 0 on success, -1 on failure
 */
static TELNETTY_UNUSED int telnetty_color_enable(telnetty_context_t* ctx);

/**
 * Disable color output
 * 
 * @param ctx TELNET context
 * @return 0 on success, -1 on failure
 */
static TELNETTY_UNUSED int telnetty_color_disable(telnetty_context_t* ctx);

/**
 * Set color capabilities
 * 
 * @param ctx TELNET context
 * @param capabilities Color capabilities
 * @return 0 on success, -1 on failure
 */
static int telnetty_color_set_capabilities(
    telnetty_context_t* ctx,
    telnetty_color_cap_t capabilities
);

/**
 * Get color capabilities
 * 
 * @param ctx TELNET context
 * @return Current color capabilities
 */
static TELNETTY_UNUSED telnetty_color_cap_t telnetty_color_get_capabilities(telnetty_context_t* ctx);

/**
 * Detect color capabilities from terminal type
 * 
 * @param ctx TELNET context
 * @param terminal_type Terminal type string
 * @return Detected color capabilities
 */
static telnetty_color_cap_t telnetty_color_detect_capabilities(
    telnetty_context_t* ctx,
    const char* terminal_type
);

/* ============================================================================
 * ANSI Color Functions
 * ============================================================================ */

/**
 * Set ANSI foreground color
 * 
 * @param ctx TELNET context
 * @param color ANSI color code
 * @return 0 on success, -1 on failure
 */
static int telnetty_color_set_foreground(
    telnetty_context_t* ctx,
    telnetty_ansi_color_t color
);

/**
 * Set ANSI background color
 * 
 * @param ctx TELNET context
 * @param color ANSI color code
 * @return 0 on success, -1 on failure
 */
static int telnetty_color_set_background(
    telnetty_context_t* ctx,
    telnetty_ansi_color_t color
);

/**
 * Set text attribute
 * 
 * @param ctx TELNET context
 * @param attr Color attribute
 * @return 0 on success, -1 on failure
 */
static int telnetty_color_set_attribute(
    telnetty_context_t* ctx,
    telnetty_color_attr_t attr
);

/**
 * Reset all colors and attributes
 * 
 * @param ctx TELNET context
 * @return 0 on success, -1 on failure
 */
static TELNETTY_UNUSED int telnetty_color_reset(telnetty_context_t* ctx);

/**
 * Reset foreground color
 * 
 * @param ctx TELNET context
 * @return 0 on success, -1 on failure
 */
static TELNETTY_UNUSED int telnetty_color_reset_foreground(telnetty_context_t* ctx);

/**
 * Reset background color
 * 
 * @param ctx TELNET context
 * @return 0 on success, -1 on failure
 */
static TELNETTY_UNUSED int telnetty_color_reset_background(telnetty_context_t* ctx);

/**
 * Clear text attribute
 * 
 * @param ctx TELNET context
 * @param attr Color attribute to clear
 * @return 0 on success, -1 on failure
 */
static int telnetty_color_clear_attribute(
    telnetty_context_t* ctx,
    telnetty_color_attr_t attr
);

/* ============================================================================
 * 256-Color Functions
 * ============================================================================ */

/**
 * Set 256-color foreground
 * 
 * @param ctx TELNET context
 * @param color Color index (0-255)
 * @return 0 on success, -1 on failure
 */
static int telnetty_color_set_foreground_256(
    telnetty_context_t* ctx,
    uint8_t color
);

/**
 * Set 256-color background
 * 
 * @param ctx TELNET context
 * @param color Color index (0-255)
 * @return 0 on success, -1 on failure
 */
static int telnetty_color_set_background_256(
    telnetty_context_t* ctx,
    uint8_t color
);

/**
 * Convert RGB to 256-color index
 * 
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @return Closest 256-color index
 */
static uint8_t telnetty_color_rgb_to_256(
    uint8_t r,
    uint8_t g,
    uint8_t b
);

/**
 * Get 256-color RGB value
 * 
 * @param index Color index (0-255)
 * @return RGB color structure
 */
static TELNETTY_UNUSED telnetty_rgb_color_t telnetty_color_256_to_rgb(uint8_t index);

/* ============================================================================
 * True Color Functions
 * ============================================================================ */

/**
 * Set true color foreground (24-bit RGB)
 * 
 * @param ctx TELNET context
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @return 0 on success, -1 on failure
 */
static int telnetty_color_set_foreground_rgb(
    telnetty_context_t* ctx,
    uint8_t r,
    uint8_t g,
    uint8_t b
);

/**
 * Set true color background (24-bit RGB)
 * 
 * @param ctx TELNET context
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @return 0 on success, -1 on failure
 */
static int telnetty_color_set_background_rgb(
    telnetty_context_t* ctx,
    uint8_t r,
    uint8_t g,
    uint8_t b
);

/**
 * Set true color foreground from RGB structure
 * 
 * @param ctx TELNET context
 * @param color RGB color structure
 * @return 0 on success, -1 on failure
 */
static int telnetty_color_set_foreground_color(
    telnetty_context_t* ctx,
    const telnetty_rgb_color_t* color
);

/**
 * Set true color background from RGB structure
 * 
 * @param ctx TELNET context
 * @param color RGB color structure
 * @return 0 on success, -1 on failure
 */
static int telnetty_color_set_background_color(
    telnetty_context_t* ctx,
    const telnetty_rgb_color_t* color
);

/* ============================================================================
 * Color String Functions
 * ============================================================================ */

/**
 * Generate ANSI color string
 * 
 * @param ctx TELNET context
 * @param fg Foreground color
 * @param bg Background color
 * @param attrs Attributes (bitmask)
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @return Number of bytes written or -1 on failure
 */
static int telnetty_color_string_ansi(
    telnetty_context_t* ctx,
    telnetty_ansi_color_t fg,
    telnetty_ansi_color_t bg,
    uint32_t attrs,
    char* buffer,
    size_t buffer_size
);

/**
 * Generate 256-color string
 * 
 * @param ctx TELNET context
 * @param fg Foreground color (0-255)
 * @param bg Background color (0-255)
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @return Number of bytes written or -1 on failure
 */
static int telnetty_color_string_256(
    telnetty_context_t* ctx,
    uint8_t fg,
    uint8_t bg,
    char* buffer,
    size_t buffer_size
);

/**
 * Generate true color string
 * 
 * @param ctx TELNET context
 * @param fg Foreground RGB color
 * @param bg Background RGB color
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @return Number of bytes written or -1 on failure
 */
static int telnetty_color_string_true(
    telnetty_context_t* ctx,
    const telnetty_rgb_color_t* fg,
    const telnetty_rgb_color_t* bg,
    char* buffer,
    size_t buffer_size
);

/**
 * Generate reset string
 * 
 * @param ctx TELNET context
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @return Number of bytes written or -1 on failure
 */
static int telnetty_color_string_reset(
    telnetty_context_t* ctx,
    char* buffer,
    size_t buffer_size
);

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/**
 * Send colored text
 * 
 * @param ctx TELNET context
 * @param text Text to send
 * @param fg Foreground color
 * @param bg Background color
 * @param attrs Attributes (bitmask)
 * @return 0 on success, -1 on failure
 */
static int telnetty_color_send(
    telnetty_context_t* ctx,
    const char* text,
    telnetty_ansi_color_t fg,
    telnetty_ansi_color_t bg,
    uint32_t attrs
);

/**
 * Send colored text with formatting
 * 
 * @param ctx TELNET context
 * @param fg Foreground color
 * @param bg Background color
 * @param attrs Attributes (bitmask)
 * @param fmt Format string
 * @param ... Format arguments
 * @return 0 on success, -1 on failure
 */
static int telnetty_color_sendf(
    telnetty_context_t* ctx,
    telnetty_ansi_color_t fg,
    telnetty_ansi_color_t bg,
    uint32_t attrs,
    const char* fmt,
    ...
);

/**
 * Send 256-color text
 * 
 * @param ctx TELNET context
 * @param text Text to send
 * @param fg Foreground color (0-255)
 * @param bg Background color (0-255)
 * @return 0 on success, -1 on failure
 */
static int telnetty_color_send_256(
    telnetty_context_t* ctx,
    const char* text,
    uint8_t fg,
    uint8_t bg
);

/**
 * Send true color text
 * 
 * @param ctx TELNET context
 * @param text Text to send
 * @param fg Foreground RGB color
 * @param bg Background RGB color
 * @return 0 on success, -1 on failure
 */
static int telnetty_color_send_true(
    telnetty_context_t* ctx,
    const char* text,
    const telnetty_rgb_color_t* fg,
    const telnetty_rgb_color_t* bg
);

/**
 * Get current color state
 * 
 * @param ctx TELNET context
 * @param state Output color state structure
 * @return 0 on success, -1 on failure
 */
static int telnetty_color_get_state(
    telnetty_context_t* ctx,
    telnetty_color_state_t* state
);

/**
 * Set color state
 * 
 * @param ctx TELNET context
 * @param state Color state structure
 * @return 0 on success, -1 on failure
 */
static int telnetty_color_set_state(
    telnetty_context_t* ctx,
    const telnetty_color_state_t* state
);

/**
 * Flush color buffer
 * 
 * @param ctx TELNET context
 * @return 0 on success, -1 on failure
 */
static TELNETTY_UNUSED int telnetty_color_flush(telnetty_context_t* ctx);

/* ============================================================================
 * Implementation
 * ============================================================================ */

#ifdef TELNETTY_COLOR_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <math.h>

/* Memory allocation wrappers */
#ifndef TELNETTY_COLOR_MALLOC
#define TELNETTY_COLOR_MALLOC(size) malloc(size)
#endif

#ifndef TELNETTY_COLOR_FREE
#define TELNETTY_COLOR_FREE(ptr) free(ptr)
#endif

#ifndef TELNETTY_COLOR_REALLOC
#define TELNETTY_COLOR_REALLOC(ptr, size) realloc(ptr, size)
#endif

/* Forward declarations for internal functions */
static TELNETTY_UNUSED telnetty_color_state_t* telnetty_color_state_create(void);
static TELNETTY_UNUSED void telnetty_color_state_destroy(telnetty_color_state_t* state);
static int telnetty_color_append_code(
    telnetty_color_state_t* state,
    const char* code
);
static int telnetty_color_append_number(
    telnetty_color_state_t* state,
    int number
);

/* ============================================================================
 * Color State Management
 * ============================================================================ */

static TELNETTY_UNUSED telnetty_color_state_t* telnetty_color_state_create(void) {
    telnetty_color_state_t* state = (telnetty_color_state_t*)TELNETTY_COLOR_MALLOC(sizeof(telnetty_color_state_t));
    if (!state) return NULL;
    
    memset(state, 0, sizeof(telnetty_color_state_t));
    state->capabilities = TELNETTY_COLOR_CAP_ANSI; /* Default to ANSI */
    state->buffer_size = TELNETTY_COLOR_BUFFER_SIZE;
    state->buffer = (char*)TELNETTY_COLOR_MALLOC(state->buffer_size);
    
    if (!state->buffer) {
        TELNETTY_COLOR_FREE(state);
        return NULL;
    }
    
    state->enabled = true;
    state->auto_detect = true;
    
    return state;
}

static TELNETTY_UNUSED void telnetty_color_state_destroy(telnetty_color_state_t* state) {
    if (!state) return;
    
    if (state->buffer) {
        TELNETTY_COLOR_FREE(state->buffer);
    }
    
    TELNETTY_COLOR_FREE(state);
}

static int telnetty_color_append_code(
    telnetty_color_state_t* state,
    const char* code
) {
    if (!state || !code) return -1;
    
    size_t code_len = strlen(code);
    if (state->buffer_pos + code_len >= state->buffer_size) {
        return -1; /* Buffer overflow */
    }
    
    strcpy(state->buffer + state->buffer_pos, code);
    state->buffer_pos += code_len;
    
    return 0;
}

static int telnetty_color_append_number(
    telnetty_color_state_t* state,
    int number
) {
    if (!state) return -1;
    
    char num_str[16];
    int len = snprintf(num_str, sizeof(num_str), "%d", number);
    
    if (len < 0 || state->buffer_pos + len >= state->buffer_size) {
        return -1;
    }
    
    strcpy(state->buffer + state->buffer_pos, num_str);
    state->buffer_pos += len;
    
    return 0;
}

/* ============================================================================
 * Core Color Implementation
 * ============================================================================ */

static TELNETTY_UNUSED int telnetty_color_init(telnetty_context_t* ctx, bool auto_detect) {
    if (!ctx) return -1;
    
    /* Create color state */
    telnetty_color_state_t* state = telnetty_color_state_create();
    if (!state) return -1;
    
    state->auto_detect = auto_detect;
    
    /* This would be stored in the context structure */
    /* For now, just return success */
    
    return 0;
}

static TELNETTY_UNUSED int telnetty_color_cleanup(telnetty_context_t* ctx) {
    if (!ctx) return -1;
    
    /* This would clean up the color state from context */
    /* For now, just return success */
    
    return 0;
}

static TELNETTY_UNUSED int telnetty_color_enable(telnetty_context_t* ctx) {
    if (!ctx) return -1;
    
    /* This would enable color output */
    /* For now, just return success */
    
    return 0;
}

static TELNETTY_UNUSED int telnetty_color_disable(telnetty_context_t* ctx) {
    if (!ctx) return -1;
    
    /* This would disable color output */
    /* For now, just return success */
    
    return 0;
}

static int telnetty_color_set_capabilities(
    telnetty_context_t* ctx,
    telnetty_color_cap_t capabilities
) {
    (void)capabilities;
    if (!ctx) return -1;
    
    /* This would set color capabilities */
    /* For now, just return success */
    
    return 0;
}

static TELNETTY_UNUSED telnetty_color_cap_t telnetty_color_get_capabilities(telnetty_context_t* ctx) {
    if (!ctx) return TELNETTY_COLOR_CAP_NONE;
    
    /* This would get color capabilities from context */
    /* For now, return ANSI as default */
    return TELNETTY_COLOR_CAP_ANSI;
}

static telnetty_color_cap_t telnetty_color_detect_capabilities(
    telnetty_context_t* ctx,
    const char* terminal_type
) {
    if (!ctx || !terminal_type) return TELNETTY_COLOR_CAP_NONE;
    
    /* Detect capabilities based on terminal type */
    if (strstr(terminal_type, "xterm") != NULL ||
        strstr(terminal_type, "rxvt") != NULL ||
        strstr(terminal_type, "gnome") != NULL ||
        strstr(terminal_type, "konsole") != NULL) {
        return TELNETTY_COLOR_CAP_256 | TELNETTY_COLOR_CAP_ANSI;
    } else if (strstr(terminal_type, "screen") != NULL ||
               strstr(terminal_type, "tmux") != NULL) {
        return TELNETTY_COLOR_CAP_256 | TELNETTY_COLOR_CAP_ANSI;
    } else if (strstr(terminal_type, "vt100") != NULL ||
               strstr(terminal_type, "vt102") != NULL) {
        return TELNETTY_COLOR_CAP_ANSI;
    }
    
    /* Default to ANSI */
    return TELNETTY_COLOR_CAP_ANSI;
}

/* ============================================================================
 * ANSI Color Implementation
 * ============================================================================ */

static int telnetty_color_set_foreground(
    telnetty_context_t* ctx,
    telnetty_ansi_color_t color
) {
    (void)color;
    if (!ctx) return -1;
    
    /* This would set the foreground color */
    /* For now, just return success */
    
    return 0;
}

static int telnetty_color_set_background(
    telnetty_context_t* ctx,
    telnetty_ansi_color_t color
) {
    (void)color;
    if (!ctx) return -1;
    
    /* This would set the background color */
    /* For now, just return success */
    
    return 0;
}

static int telnetty_color_set_attribute(
    telnetty_context_t* ctx,
    telnetty_color_attr_t attr
) {
    (void)attr;
    if (!ctx) return -1;
    
    /* This would set a text attribute */
    /* For now, just return success */
    
    return 0;
}

static TELNETTY_UNUSED int telnetty_color_reset(telnetty_context_t* ctx) {
    if (!ctx) return -1;
    
    /* This would reset all colors and attributes */
    /* For now, just return success */
    
    return 0;
}

static TELNETTY_UNUSED int telnetty_color_reset_foreground(telnetty_context_t* ctx) {
    if (!ctx) return -1;
    
    /* This would reset the foreground color */
    /* For now, just return success */
    
    return 0;
}

static TELNETTY_UNUSED int telnetty_color_reset_background(telnetty_context_t* ctx) {
    if (!ctx) return -1;
    
    /* This would reset the background color */
    /* For now, just return success */
    
    return 0;
}

static int telnetty_color_clear_attribute(
    telnetty_context_t* ctx,
    telnetty_color_attr_t attr
) {
    (void)attr;
    if (!ctx) return -1;
    
    /* This would clear a text attribute */
    /* For now, just return success */
    
    return 0;
}

/* ============================================================================
 * 256-Color Implementation
 * ============================================================================ */

static int telnetty_color_set_foreground_256(
    telnetty_context_t* ctx,
    uint8_t color
) {
    (void)color;
    if (!ctx) return -1;
    
    /* This would set 256-color foreground */
    /* For now, just return success */
    
    return 0;
}

static int telnetty_color_set_background_256(
    telnetty_context_t* ctx,
    uint8_t color
) {
    (void)color;
    if (!ctx) return -1;
    
    /* This would set 256-color background */
    /* For now, just return success */
    
    return 0;
}

static TELNETTY_UNUSED uint8_t telnetty_color_rgb_to_256(uint8_t r, uint8_t g, uint8_t b) {
    /* Convert RGB to 256-color index */
    /* This is a simplified implementation */
    
    if (r == g && g == b) {
        /* Grayscale */
        if (r < 8) return 16;
        if (r > 248) return 231;
        return 232 + (r - 8) / 10;
    }
    
    /* Color cube */
    int r_index = (r >= 48) ? ((r - 48) / 40) : 0;
    int g_index = (g >= 48) ? ((g - 48) / 40) : 0;
    int b_index = (b >= 48) ? ((b - 48) / 40) : 0;
    
    return 16 + (r_index * 36) + (g_index * 6) + b_index;
}

static TELNETTY_UNUSED telnetty_rgb_color_t telnetty_color_256_to_rgb(uint8_t index) {
    telnetty_rgb_color_t color = {0, 0, 0};
    
    if (index < 16) {
        /* Standard colors */
        static const uint8_t standard_colors[16][3] = {
            {0, 0, 0},       /* Black */
            {128, 0, 0},     /* Red */
            {0, 128, 0},     /* Green */
            {128, 128, 0},   /* Yellow */
            {0, 0, 128},     /* Blue */
            {128, 0, 128},   /* Magenta */
            {0, 128, 128},   /* Cyan */
            {192, 192, 192}, /* White */
            {128, 128, 128}, /* Bright Black */
            {255, 0, 0},     /* Bright Red */
            {0, 255, 0},     /* Bright Green */
            {255, 255, 0},   /* Bright Yellow */
            {0, 0, 255},     /* Bright Blue */
            {255, 0, 255},   /* Bright Magenta */
            {0, 255, 255},   /* Bright Cyan */
            {255, 255, 255}  /* Bright White */
        };
        
        color.r = standard_colors[index][0];
        color.g = standard_colors[index][1];
        color.b = standard_colors[index][2];
    } else if (index < 232) {
        /* Color cube */
        int color_index = index - 16;
        int r = (color_index / 36) % 6;
        int g = (color_index / 6) % 6;
        int b = color_index % 6;
        
        color.r = r == 0 ? 0 : (r * 40 + 55);
        color.g = g == 0 ? 0 : (g * 40 + 55);
        color.b = b == 0 ? 0 : (b * 40 + 55);
    } else {
        /* Grayscale */
        int gray = (index - 232) * 10 + 8;
        color.r = color.g = color.b = gray;
    }
    
    return color;
}

/* ============================================================================
 * True Color Implementation
 * ============================================================================ */

static int telnetty_color_set_foreground_rgb(
    telnetty_context_t* ctx,
    uint8_t r,
    uint8_t g,
    uint8_t b
) {
    (void)r;
    (void)g;
    (void)b;
    if (!ctx) return -1;
    
    /* This would set true color foreground */
    /* For now, just return success */
    
    return 0;
}

static int telnetty_color_set_background_rgb(
    telnetty_context_t* ctx,
    uint8_t r,
    uint8_t g,
    uint8_t b
) {
    (void)r;
    (void)g;
    (void)b;
    if (!ctx) return -1;
    
    /* This would set true color background */
    /* For now, just return success */
    
    return 0;
}

static int telnetty_color_set_foreground_color(
    telnetty_context_t* ctx,
    const telnetty_rgb_color_t* color
) {
    if (!ctx || !color) return -1;
    
    return telnetty_color_set_foreground_rgb(ctx, color->r, color->g, color->b);
}

static int telnetty_color_set_background_color(
    telnetty_context_t* ctx,
    const telnetty_rgb_color_t* color
) {
    if (!ctx || !color) return -1;
    
    return telnetty_color_set_background_rgb(ctx, color->r, color->g, color->b);
}

/* ============================================================================
 * Color String Implementation
 * ============================================================================ */

static int telnetty_color_string_ansi(
    telnetty_context_t* ctx,
    telnetty_ansi_color_t fg,
    telnetty_ansi_color_t bg,
    uint32_t attrs,
    char* buffer,
    size_t buffer_size
) {
    if (!ctx || !buffer) return -1;
    
    /* Build ANSI color string */
    size_t offset = 0;
    
    /* Start escape sequence */
    offset += snprintf(buffer + offset, buffer_size - offset, "\033[");
    
    /* Add attributes */
    bool first = true;
    if (attrs & (1 << TELNETTY_ATTR_BOLD)) {
        offset += snprintf(buffer + offset, buffer_size - offset, "1");
        first = false;
    }
    if (attrs & (1 << TELNETTY_ATTR_ITALIC)) {
        if (!first) buffer[offset++] = ';';
        offset += snprintf(buffer + offset, buffer_size - offset, "3");
        first = false;
    }
    if (attrs & (1 << TELNETTY_ATTR_UNDERLINE)) {
        if (!first) buffer[offset++] = ';';
        offset += snprintf(buffer + offset, buffer_size - offset, "4");
        first = false;
    }
    
    /* Add foreground color */
    if (fg <= 7) {
        if (!first) buffer[offset++] = ';';
        offset += snprintf(buffer + offset, buffer_size - offset, "%d", 30 + fg);
        first = false;
    } else if (fg <= 15) {
        if (!first) buffer[offset++] = ';';
        offset += snprintf(buffer + offset, buffer_size - offset, "%d", 90 + (fg - 8));
        first = false;
    }
    
    /* Add background color */
    if (bg <= 7) {
        if (!first) buffer[offset++] = ';';
        offset += snprintf(buffer + offset, buffer_size - offset, "%d", 40 + bg);
    } else if (bg <= 15) {
        if (!first) buffer[offset++] = ';';
        offset += snprintf(buffer + offset, buffer_size - offset, "%d", 100 + (bg - 8));
    }
    
    /* End escape sequence */
    offset += snprintf(buffer + offset, buffer_size - offset, "m");
    
    return (int)offset;
}

static int telnetty_color_string_256(
    telnetty_context_t* ctx,
    uint8_t fg,
    uint8_t bg,
    char* buffer,
    size_t buffer_size
) {
    if (!ctx || !buffer) return -1;
    
    /* Build 256-color string */
    size_t offset = 0;
    
    /* Start escape sequence */
    offset += snprintf(buffer + offset, buffer_size - offset, "\033[");
    
    /* Add foreground color */
    offset += snprintf(buffer + offset, buffer_size - offset, "38;5;%d", fg);
    
    /* Add background color */
    if (bg <= 255) {
        offset += snprintf(buffer + offset, buffer_size - offset, ";48;5;%d", bg);
    }
    
    /* End escape sequence */
    offset += snprintf(buffer + offset, buffer_size - offset, "m");
    
    return (int)offset;
}

static int telnetty_color_string_true(
    telnetty_context_t* ctx,
    const telnetty_rgb_color_t* fg,
    const telnetty_rgb_color_t* bg,
    char* buffer,
    size_t buffer_size
) {
    if (!ctx || !buffer) return -1;
    
    /* Build true color string */
    size_t offset = 0;
    
    /* Start escape sequence */
    offset += snprintf(buffer + offset, buffer_size - offset, "\033[");
    
    /* Add foreground color */
    if (fg) {
        offset += snprintf(buffer + offset, buffer_size - offset, 
            "38;2;%d;%d;%d", fg->r, fg->g, fg->b);
    }
    
    /* Add background color */
    if (bg) {
        if (fg) buffer[offset++] = ';';
        offset += snprintf(buffer + offset, buffer_size - offset, 
            "48;2;%d;%d;%d", bg->r, bg->g, bg->b);
    }
    
    /* End escape sequence */
    offset += snprintf(buffer + offset, buffer_size - offset, "m");
    
    return (int)offset;
}

static int telnetty_color_string_reset(
    telnetty_context_t* ctx,
    char* buffer,
    size_t buffer_size
) {
    if (!ctx || !buffer) return -1;
    
    /* Reset all attributes */
    return snprintf(buffer, buffer_size, "\033[0m");
}

/* ============================================================================
 * Utility Functions Implementation
 * ============================================================================ */

static int telnetty_color_send(
    telnetty_context_t* ctx,
    const char* text,
    telnetty_ansi_color_t fg,
    telnetty_ansi_color_t bg,
    uint32_t attrs
) {
    if (!ctx || !text) return -1;
    
    char color_buf[TELNETTY_COLOR_BUFFER_SIZE];
    
    /* Generate color string */
    int color_len = telnetty_color_string_ansi(ctx, fg, bg, attrs, 
        color_buf, sizeof(color_buf));
    
    if (color_len < 0) return -1;
    
    /* Send color code */
    if (telnetty_send(ctx, (uint8_t*)color_buf, color_len) < 0) {
        return -1;
    }
    
    /* Send text */
    if (telnetty_send(ctx, (uint8_t*)text, strlen(text)) < 0) {
        return -1;
    }
    
    /* Send reset */
    int reset_len = telnetty_color_string_reset(ctx, color_buf, sizeof(color_buf));
    if (reset_len > 0) {
        telnetty_send(ctx, (uint8_t*)color_buf, reset_len);
    }
    
    return 0;
}

static int telnetty_color_sendf(
    telnetty_context_t* ctx,
    telnetty_ansi_color_t fg,
    telnetty_ansi_color_t bg,
    uint32_t attrs,
    const char* fmt,
    ...
) {
    if (!ctx || !fmt) return -1;
    
    /* Format the text */
    va_list args;
    va_start(args, fmt);
    
    va_list args_copy;
    va_copy(args_copy, args);
    int size = vsnprintf(NULL, 0, fmt, args_copy);
    va_end(args_copy);
    
    if (size < 0) {
        va_end(args);
        return -1;
    }
    
    char* text = (char*)TELNETTY_COLOR_MALLOC(size + 1);
    if (!text) {
        va_end(args);
        return -1;
    }
    
    vsnprintf(text, size + 1, fmt, args);
    va_end(args);
    
    /* Send colored text */
    int result = telnetty_color_send(ctx, text, fg, bg, attrs);
    
    TELNETTY_COLOR_FREE(text);
    
    return result;
}

static int telnetty_color_send_256(
    telnetty_context_t* ctx,
    const char* text,
    uint8_t fg,
    uint8_t bg
) {
    if (!ctx || !text) return -1;
    
    char color_buf[TELNETTY_COLOR_BUFFER_SIZE];
    
    /* Generate color string */
    int color_len = telnetty_color_string_256(ctx, fg, bg, 
        color_buf, sizeof(color_buf));
    
    if (color_len < 0) return -1;
    
    /* Send color code */
    if (telnetty_send(ctx, (uint8_t*)color_buf, color_len) < 0) {
        return -1;
    }
    
    /* Send text */
    if (telnetty_send(ctx, (uint8_t*)text, strlen(text)) < 0) {
        return -1;
    }
    
    /* Send reset */
    int reset_len = telnetty_color_string_reset(ctx, color_buf, sizeof(color_buf));
    if (reset_len > 0) {
        telnetty_send(ctx, (uint8_t*)color_buf, reset_len);
    }
    
    return 0;
}

static int telnetty_color_send_true(
    telnetty_context_t* ctx,
    const char* text,
    const telnetty_rgb_color_t* fg,
    const telnetty_rgb_color_t* bg
) {
    if (!ctx || !text) return -1;
    
    char color_buf[TELNETTY_COLOR_BUFFER_SIZE];
    
    /* Generate color string */
    int color_len = telnetty_color_string_true(ctx, fg, bg, 
        color_buf, sizeof(color_buf));
    
    if (color_len < 0) return -1;
    
    /* Send color code */
    if (telnetty_send(ctx, (uint8_t*)color_buf, color_len) < 0) {
        return -1;
    }
    
    /* Send text */
    if (telnetty_send(ctx, (uint8_t*)text, strlen(text)) < 0) {
        return -1;
    }
    
    /* Send reset */
    int reset_len = telnetty_color_string_reset(ctx, color_buf, sizeof(color_buf));
    if (reset_len > 0) {
        telnetty_send(ctx, (uint8_t*)color_buf, reset_len);
    }
    
    return 0;
}

static int telnetty_color_get_state(
    telnetty_context_t* ctx,
    telnetty_color_state_t* state
) {
    if (!ctx || !state) return -1;
    
    /* This would get color state from context */
    /* For now, return default state */
    memset(state, 0, sizeof(telnetty_color_state_t));
    
    return 0;
}

static int telnetty_color_set_state(
    telnetty_context_t* ctx,
    const telnetty_color_state_t* state
) {
    if (!ctx || !state) return -1;
    
    /* This would set color state in context */
    /* For now, just return success */
    
    return 0;
}

static TELNETTY_UNUSED int telnetty_color_flush(telnetty_context_t* ctx) {
    if (!ctx) return -1;
    
    /* This would flush the color buffer */
    /* For now, just return success */
    
    return 0;
}

#endif /* TELNETTY_COLOR_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif

#endif /* TELNETTY_COLOR_H */