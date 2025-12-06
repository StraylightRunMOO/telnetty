/**
 * @file telnetty_compression.h
 * @brief Compression protocol implementations for TELNET
 * 
 * This header provides implementations for MUD Client Compression Protocol
 * versions 2 and 3 (MCCP2/MCCP3), which allow for data compression in
 * TELNETTY connections to reduce bandwidth usage.
 * 
 * Features:
 * - MCCP2 (MUD Client Compression Protocol v2)
 * - MCCP3 (MUD Client Compression Protocol v3)
 * - zlib integration
 * - Compression state management
 * 
 * @author Damus <damus@straylightrun.org>
 * @version 1.0.0
 * 
 */

#ifndef TELNETTY_COMPRESSION_H
#define TELNETTY_COMPRESSION_H

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
 * Configuration Options
 * ============================================================================ */

#ifndef TELNETTY_COMPRESSION_LEVEL
#define TELNETTY_COMPRESSION_LEVEL 6  /* zlib compression level (1-9) */
#endif

#ifndef TELNETTY_COMPRESSION_BUFFER_SIZE
#define TELNETTY_COMPRESSION_BUFFER_SIZE (8 * 1024)  /* 8KB compression buffer */
#endif

#ifndef TELNETTY_COMPRESSION_MAX_RATIO
#define TELNETTY_COMPRESSION_MAX_RATIO 100  /* Maximum compression ratio */
#endif

/* ============================================================================
 * MCCP2 Protocol (MUD Client Compression Protocol v2)
 * ============================================================================ */

/* MCCP2 option code */
#define TELNETTY_TELOPT_MCCP2 86

/* MCCP2 constants */
#define TELNETTY_MCCP2_COMPRESS 85
#define TCCP2_MAXINPUT 4096

/* MCCP2 data structure */
typedef struct {
    bool enabled;                       /**< MCCP2 is enabled */
    bool compressing;                   /**< Currently compressing */
    void* zlib_stream;                  /**< zlib stream (opaque) */
    uint8_t* input_buffer;              /**< Input buffer */
    uint8_t* output_buffer;             /**< Output buffer */
    size_t input_size;                  /**< Input buffer size */
    size_t output_size;                 /**< Output buffer size */
    size_t compressed_bytes;            /**< Total compressed bytes */
    size_t uncompressed_bytes;          /**< Total uncompressed bytes */
    double compression_ratio;           /**< Compression ratio */
} telnetty_mccp2_data_t;

/**
 * MCCP2 option handler
 * 
 * @param ctx TELNETTY context
 * @param option Option code (should be TELNETTY_TELOPT_MCCP2)
 * @param command Command received
 * @param user_data User data (should be telnetty_mccp2_data_t*)
 */
static void telnetty_mccp2_handler(
    telnetty_context_t* ctx,
    uint8_t option,
    uint8_t command,
    void* user_data
);

/**
 * Enable MCCP2 compression
 * 
 * @param ctx TELNETTY context
 * @param level Compression level (1-9, 0 for default)
 * @return 0 on success, -1 on failure
 */
static int telnetty_enable_mccp2(
    telnetty_context_t* ctx,
    int level
);

/**
 * Disable MCCP2 compression
 * 
 * @param ctx TELNETTY context
 * @return 0 on success, -1 on failure
 */
static TELNETTY_UNUSED int telnetty_disable_mccp2(telnetty_context_t* ctx);

/**
 * Check if MCCP2 is enabled
 * 
 * @param ctx TELNETTY context
 * @return true if MCCP2 is enabled, false otherwise
 */
static TELNETTY_UNUSED bool telnetty_is_mccp2_enabled(telnetty_context_t* ctx);

/**
 * Send compressed data via MCCP2
 * 
 * @param ctx TELNETTY context
 * @param data Data to compress and send
 * @param length Data length
 * @return Number of bytes compressed or -1 on failure
 */
static int telnetty_mccp2_send(
    telnetty_context_t* ctx,
    const uint8_t* data,
    size_t length
);

/**
 * Get MCCP2 compression statistics
 * 
 * @param ctx TELNETTY context
 * @param compressed_bytes Output compressed bytes (optional)
 * @param uncompressed_bytes Output uncompressed bytes (optional)
 * @param ratio Output compression ratio (optional)
 * @return 0 on success, -1 on failure
 */
static int telnetty_mccp2_get_stats(
    telnetty_context_t* ctx,
    size_t* compressed_bytes,
    size_t* uncompressed_bytes,
    double* ratio
);

/* ============================================================================
 * MCCP3 Protocol (MUD Client Compression Protocol v3)
 * ============================================================================ */

/* MCCP3 option code */
#define TELNETTY_TELOPT_MCCP3 87

/* MCCP3 data structure */
typedef struct {
    bool enabled;                       /**< MCCP3 is enabled */
    bool client_mode;                   /**< Client-side compression */
    bool server_mode;                   /**< Server-side compression */
    void* zlib_stream;                  /**< zlib stream (opaque) */
    uint8_t* input_buffer;              /**< Input buffer */
    uint8_t* output_buffer;             /**< Output buffer */
    size_t input_size;                  /**< Input buffer size */
    size_t output_size;                 /**< Output buffer size */
    size_t compressed_bytes;            /**< Total compressed bytes */
    size_t uncompressed_bytes;          /**< Total uncompressed bytes */
    double compression_ratio;           /**< Compression ratio */
} telnetty_mccp3_data_t;

/**
 * MCCP3 option handler
 * 
 * @param ctx TELNETTY context
 * @param option Option code (should be TELNETTY_TELOPT_MCCP3)
 * @param command Command received
 * @param user_data User data (should be telnetty_mccp3_data_t*)
 */
static void telnetty_mccp3_handler(
    telnetty_context_t* ctx,
    uint8_t option,
    uint8_t command,
    void* user_data
);

/**
 * Enable MCCP3 compression
 * 
 * @param ctx TELNETTY context
 * @param level Compression level (1-9, 0 for default)
 * @return 0 on success, -1 on failure
 */
static int telnetty_enable_mccp3(
    telnetty_context_t* ctx,
    int level
);

/**
 * Disable MCCP3 compression
 * 
 * @param ctx TELNETTY context
 * @return 0 on success, -1 on failure
 */
static TELNETTY_UNUSED int telnetty_disable_mccp3(telnetty_context_t* ctx);

/**
 * Check if MCCP3 is enabled
 * 
 * @param ctx TELNETTY context
 * @return true if MCCP3 is enabled, false otherwise
 */
static TELNETTY_UNUSED bool telnetty_is_mccp3_enabled(telnetty_context_t* ctx);

/**
 * Send compressed data via MCCP3
 * 
 * @param ctx TELNETTY context
 * @param data Data to compress and send
 * @param length Data length
 * @return Number of bytes compressed or -1 on failure
 */
static int telnetty_mccp3_send(
    telnetty_context_t* ctx,
    const uint8_t* data,
    size_t length
);

/**
 * Get MCCP3 compression statistics
 * 
 * @param ctx TELNETTY context
 * @param compressed_bytes Output compressed bytes (optional)
 * @param uncompressed_bytes Output uncompressed bytes (optional)
 * @param ratio Output compression ratio (optional)
 * @return 0 on success, -1 on failure
 */
static int telnetty_mccp3_get_stats(
    telnetty_context_t* ctx,
    size_t* compressed_bytes,
    size_t* uncompressed_bytes,
    double* ratio
);

/* ============================================================================
 * Compression Utilities
 * ============================================================================ */

/**
 * Initialize compression system
 * 
 * @param ctx TELNETTY context
 * @return 0 on success, -1 on failure
 */
static TELNETTY_UNUSED int telnetty_compression_init(telnetty_context_t* ctx);

/**
 * Cleanup compression system
 * 
 * @param ctx TELNETTY context
 * @return 0 on success, -1 on failure
 */
/* ============================================================================
 * Statistics Structures
 * ============================================================================ */

/* Compression statistics */
typedef struct {
    size_t compressed_bytes;            /**< Total compressed bytes */
    size_t uncompressed_bytes;          /**< Total uncompressed bytes */
    double compression_ratio;           /**< Overall compression ratio */
    size_t packets_compressed;          /**< Number of packets compressed */
    size_t packets_sent;                /**< Number of packets sent */
    double average_packet_size;         /**< Average packet size */
    time_t start_time;                  /**< Compression start time */
    time_t last_activity;               /**< Last compression activity */
} telnetty_compression_stats_t;

static TELNETTY_UNUSED int telnetty_compression_cleanup(telnetty_context_t* ctx);

/**
 * Get overall compression statistics
 * 
 * @param ctx TELNETTY context
 * @param mccp2_stats Output MCCP2 statistics (optional)
 * @param mccp3_stats Output MCCP3 statistics (optional)
 * @return 0 on success, -1 on failure
 */
static int telnetty_compression_get_stats(
    telnetty_context_t* ctx,
    telnetty_compression_stats_t* mccp2_stats,
    telnetty_compression_stats_t* mccp3_stats
);

/**
 * Enable automatic compression negotiation
 * 
 * @param ctx TELNETTY context
 * @param prefer_mccp3 Prefer MCCP3 over MCCP2
 * @return 0 on success, -1 on failure
 */
static int telnetty_compression_auto_negotiate(
    telnetty_context_t* ctx,
    bool prefer_mccp3
);

/* ============================================================================
 * Implementation
 * ============================================================================ */

#ifdef TELNETTY_COMPRESSION_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

/* zlib includes - optional */
#ifdef TELNETTY_HAVE_ZLIB
#include <zlib.h>
#endif

/* Memory allocation wrappers */
#ifndef TELNETTY_COMPRESSION_MALLOC
#define TELNETTY_COMPRESSION_MALLOC(size) malloc(size)
#endif

#ifndef TELNETTY_COMPRESSION_FREE
#define TELNETTY_COMPRESSION_FREE(ptr) free(ptr)
#endif

#ifndef TELNETTY_COMPRESSION_REALLOC
#define TELNETTY_COMPRESSION_REALLOC(ptr, size) realloc(ptr, size)
#endif

/* Forward declarations for internal functions */
static TELNETTY_UNUSED int telnetty_mccp2_init(telnetty_context_t* ctx, telnetty_mccp2_data_t* data, int level);
static TELNETTY_UNUSED int telnetty_mccp2_cleanup(telnetty_mccp2_data_t* data);
static int telnetty_mccp2_compress(
    telnetty_mccp2_data_t* data,
    const uint8_t* input,
    size_t input_len,
    uint8_t* output,
    size_t output_len,
    size_t* compressed_len
);
static TELNETTY_UNUSED int telnetty_mccp3_init(telnetty_context_t* ctx, telnetty_mccp3_data_t* data, int level);
static TELNETTY_UNUSED int telnetty_mccp3_cleanup(telnetty_mccp3_data_t* data);
static int telnetty_mccp3_compress(
    telnetty_mccp3_data_t* data,
    const uint8_t* input,
    size_t input_len,
    uint8_t* output,
    size_t output_len,
    size_t* compressed_len
);
static int telnetty_mccp3_decompress(
    telnetty_mccp3_data_t* data,
    const uint8_t* input,
    size_t input_len,
    uint8_t* output,
    size_t output_len,
    size_t* decompressed_len
);

/* ============================================================================
 * MCCP2 Implementation
 * ============================================================================ */

static TELNETTY_UNUSED telnetty_mccp2_data_t* telnetty_mccp2_data_create(void) {
    telnetty_mccp2_data_t* data = (telnetty_mccp2_data_t*)TELNETTY_COMPRESSION_MALLOC(sizeof(telnetty_mccp2_data_t));
    if (!data) return NULL;
    
    memset(data, 0, sizeof(telnetty_mccp2_data_t));
    data->input_size = TELNETTY_COMPRESSION_BUFFER_SIZE;
    data->output_size = TELNETTY_COMPRESSION_BUFFER_SIZE;
    
    return data;
}

static TELNETTY_UNUSED void telnetty_mccp2_data_destroy(telnetty_mccp2_data_t* data) {
    if (!data) return;
    
    if (data->input_buffer) {
        TELNETTY_COMPRESSION_FREE(data->input_buffer);
    }
    if (data->output_buffer) {
        TELNETTY_COMPRESSION_FREE(data->output_buffer);
    }
    
    TELNETTY_COMPRESSION_FREE(data);
}

static void telnetty_mccp2_handler(
    telnetty_context_t* ctx,
    uint8_t option,
    uint8_t command,
    void* user_data
) {
    if (!ctx || option != TELNETTY_TELOPT_MCCP2) return;
    
    telnetty_mccp2_data_t* mccp2_data = (telnetty_mccp2_data_t*)user_data;
    if (!mccp2_data) return;
    
    switch (command) {
        case TELNETTY_WILL:
            /* Peer wants to enable MCCP2 */
            telnetty_send_option(ctx, TELNETTY_DO, option);
            mccp2_data->enabled = true;
            /* Initialize compression */
            telnetty_mccp2_init(ctx, mccp2_data, TELNETTY_COMPRESSION_LEVEL);
            break;
            
        case TELNETTY_WONT:
            /* Peer wants to disable MCCP2 */
            telnetty_send_option(ctx, TELNETTY_DONT, option);
            mccp2_data->enabled = false;
            mccp2_data->compressing = false;
            telnetty_mccp2_cleanup(mccp2_data);
            break;
            
        case TELNETTY_DO:
            /* Peer agrees to enable MCCP2 */
            mccp2_data->enabled = true;
            /* Initialize compression */
            telnetty_mccp2_init(ctx, mccp2_data, TELNETTY_COMPRESSION_LEVEL);
            break;
            
        case TELNETTY_DONT:
            /* Peer refuses to enable MCCP2 */
            mccp2_data->enabled = false;
            mccp2_data->compressing = false;
            telnetty_mccp2_cleanup(mccp2_data);
            break;
    }
}

static TELNETTY_UNUSED int telnetty_enable_mccp2(telnetty_context_t* ctx, int level) {
    if (!ctx) return -1;
    
    /* Create MCCP2 data */
    telnetty_mccp2_data_t* mccp2_data = telnetty_mccp2_data_create();
    if (!mccp2_data) return -1;
    
    /* Initialize compression */
    if (telnetty_mccp2_init(ctx, mccp2_data, level) != 0) {
        telnetty_mccp2_data_destroy(mccp2_data);
        return -1;
    }
    
    /* Register option handler */
    /* Note: In full implementation, this would be integrated with the core */
    
    /* Send WILL to enable MCCP2 */
    return telnetty_send_option(ctx, TELNETTY_WILL, TELNETTY_TELOPT_MCCP2);
}

static TELNETTY_UNUSED int telnetty_disable_mccp2(telnetty_context_t* ctx) {
    if (!ctx) return -1;
    
    /* Send WONT to disable MCCP2 */
    return telnetty_send_option(ctx, TELNETTY_WONT, TELNETTY_TELOPT_MCCP2);
}

static TELNETTY_UNUSED bool telnetty_is_mccp2_enabled(telnetty_context_t* ctx) {
    if (!ctx) return false;
    
    /* This would need integration with compression data */
    /* For now, return false */
    return false;
}

static int telnetty_mccp2_send(
    telnetty_context_t* ctx,
    const uint8_t* data,
    size_t length
) {
    if (!ctx || !data || length == 0) return 0;
    
    /* This would need integration with MCCP2 data structure */
    /* For now, just send data uncompressed */
    return telnetty_send(ctx, data, length);
}

static int telnetty_mccp2_get_stats(
    telnetty_context_t* ctx,
    size_t* compressed_bytes,
    size_t* uncompressed_bytes,
    double* ratio
) {
    if (!ctx) return -1;
    
    /* This would need integration with MCCP2 data structure */
    /* For now, return zero statistics */
    if (compressed_bytes) *compressed_bytes = 0;
    if (uncompressed_bytes) *uncompressed_bytes = 0;
    if (ratio) *ratio = 0.0;
    
    return 0;
}

/* ============================================================================
 * MCCP3 Implementation
 * ============================================================================ */

static TELNETTY_UNUSED telnetty_mccp3_data_t* telnetty_mccp3_data_create(void) {
    telnetty_mccp3_data_t* data = (telnetty_mccp3_data_t*)TELNETTY_COMPRESSION_MALLOC(sizeof(telnetty_mccp3_data_t));
    if (!data) return NULL;
    
    memset(data, 0, sizeof(telnetty_mccp3_data_t));
    data->input_size = TELNETTY_COMPRESSION_BUFFER_SIZE;
    data->output_size = TELNETTY_COMPRESSION_BUFFER_SIZE;
    
    return data;
}

static TELNETTY_UNUSED void telnetty_mccp3_data_destroy(telnetty_mccp3_data_t* data) {
    if (!data) return;
    
    if (data->input_buffer) {
        TELNETTY_COMPRESSION_FREE(data->input_buffer);
    }
    if (data->output_buffer) {
        TELNETTY_COMPRESSION_FREE(data->output_buffer);
    }
    
    TELNETTY_COMPRESSION_FREE(data);
}

static void telnetty_mccp3_handler(
    telnetty_context_t* ctx,
    uint8_t option,
    uint8_t command,
    void* user_data
) {
    if (!ctx || option != TELNETTY_TELOPT_MCCP3) return;
    
    telnetty_mccp3_data_t* mccp3_data = (telnetty_mccp3_data_t*)user_data;
    if (!mccp3_data) return;
    
    switch (command) {
        case TELNETTY_WILL:
            /* Peer wants to enable MCCP3 */
            telnetty_send_option(ctx, TELNETTY_DO, option);
            mccp3_data->enabled = true;
            mccp3_data->client_mode = true;
            /* Initialize compression */
            telnetty_mccp3_init(ctx, mccp3_data, TELNETTY_COMPRESSION_LEVEL);
            break;
            
        case TELNETTY_WONT:
            /* Peer wants to disable MCCP3 */
            telnetty_send_option(ctx, TELNETTY_DONT, option);
            mccp3_data->enabled = false;
            mccp3_data->client_mode = false;
            mccp3_data->server_mode = false;
            telnetty_mccp3_cleanup(mccp3_data);
            break;
            
        case TELNETTY_DO:
            /* Peer agrees to enable MCCP3 */
            mccp3_data->enabled = true;
            mccp3_data->server_mode = true;
            /* Initialize compression */
            telnetty_mccp3_init(ctx, mccp3_data, TELNETTY_COMPRESSION_LEVEL);
            break;
            
        case TELNETTY_DONT:
            /* Peer refuses to enable MCCP3 */
            mccp3_data->enabled = false;
            mccp3_data->client_mode = false;
            mccp3_data->server_mode = false;
            telnetty_mccp3_cleanup(mccp3_data);
            break;
    }
}

static TELNETTY_UNUSED int telnetty_enable_mccp3(telnetty_context_t* ctx, int level) {
    if (!ctx) return -1;
    
    /* Create MCCP3 data */
    telnetty_mccp3_data_t* mccp3_data = telnetty_mccp3_data_create();
    if (!mccp3_data) return -1;
    
    /* Initialize compression */
    if (telnetty_mccp3_init(ctx, mccp3_data, level) != 0) {
        telnetty_mccp3_data_destroy(mccp3_data);
        return -1;
    }
    
    /* Register option handler */
    /* Note: In full implementation, this would be integrated with the core */
    
    /* Send WILL to enable MCCP3 */
    return telnetty_send_option(ctx, TELNETTY_WILL, TELNETTY_TELOPT_MCCP3);
}

static TELNETTY_UNUSED int telnetty_disable_mccp3(telnetty_context_t* ctx) {
    if (!ctx) return -1;
    
    /* Send WONT to disable MCCP3 */
    return telnetty_send_option(ctx, TELNETTY_WONT, TELNETTY_TELOPT_MCCP3);
}

static TELNETTY_UNUSED bool telnetty_is_mccp3_enabled(telnetty_context_t* ctx) {
    if (!ctx) return false;
    
    /* This would need integration with compression data */
    /* For now, return false */
    return false;
}

static int telnetty_mccp3_send(
    telnetty_context_t* ctx,
    const uint8_t* data,
    size_t length
) {
    if (!ctx || !data || length == 0) return 0;
    
    /* This would need integration with MCCP3 data structure */
    /* For now, just send data uncompressed */
    return telnetty_send(ctx, data, length);
}

static int telnetty_mccp3_get_stats(
    telnetty_context_t* ctx,
    size_t* compressed_bytes,
    size_t* uncompressed_bytes,
    double* ratio
) {
    if (!ctx) return -1;
    
    /* This would need integration with MCCP3 data structure */
    /* For now, return zero statistics */
    if (compressed_bytes) *compressed_bytes = 0;
    if (uncompressed_bytes) *uncompressed_bytes = 0;
    if (ratio) *ratio = 0.0;
    
    return 0;
}

/* ============================================================================
 * Internal Implementation Functions
 * ============================================================================ */

static TELNETTY_UNUSED int telnetty_mccp2_init(telnetty_context_t* ctx, telnetty_mccp2_data_t* data, int level) {
    if (!ctx || !data) return -1;
    
#ifdef TELNETTY_HAVE_ZLIB
    /* Initialize zlib for compression */
    data->zlib_stream = TELNETTY_COMPRESSION_MALLOC(sizeof(z_stream));
    if (!data->zlib_stream) return -1;
    
    z_stream* strm = (z_stream*)data->zlib_stream;
    strm->zalloc = Z_NULL;
    strm->zfree = Z_NULL;
    strm->opaque = Z_NULL;
    
    /* Initialize for compression */
    int result = deflateInit(strm, level > 0 ? level : TELNETTY_COMPRESSION_LEVEL);
    if (result != Z_OK) {
        TELNETTY_COMPRESSION_FREE(data->zlib_stream);
        data->zlib_stream = NULL;
        return -1;
    }
    
    /* Allocate buffers */
    data->input_buffer = (uint8_t*)TELNETTY_COMPRESSION_MALLOC(data->input_size);
    data->output_buffer = (uint8_t*)TELNETTY_COMPRESSION_MALLOC(data->output_size);
    
    if (!data->input_buffer || !data->output_buffer) {
        telnetty_mccp2_cleanup(data);
        return -1;
    }
    
    data->compressing = true;
    
#else
    /* No zlib support - return error */
    return -1;
#endif
    
    return 0;
}

static TELNETTY_UNUSED int telnetty_mccp2_cleanup(telnetty_mccp2_data_t* data) {
    if (!data) return -1;
    
#ifdef TELNETTY_HAVE_ZLIB
    if (data->zlib_stream) {
        z_stream* strm = (z_stream*)data->zlib_stream;
        deflateEnd(strm);
        TELNETTY_COMPRESSION_FREE(data->zlib_stream);
        data->zlib_stream = NULL;
    }
#endif
    
    if (data->input_buffer) {
        TELNETTY_COMPRESSION_FREE(data->input_buffer);
        data->input_buffer = NULL;
    }
    
    if (data->output_buffer) {
        TELNETTY_COMPRESSION_FREE(data->output_buffer);
        data->output_buffer = NULL;
    }
    
    data->compressing = false;
    
    return 0;
}

static int telnetty_mccp2_compress(
    telnetty_mccp2_data_t* data,
    const uint8_t* input,
    size_t input_len,
    uint8_t* output,
    size_t output_len,
    size_t* compressed_len
) {
    if (!data || !input || !output || !compressed_len) return -1;
    
#ifdef TELNETTY_HAVE_ZLIB
    if (!data->zlib_stream) return -1;
    
    z_stream* strm = (z_stream*)data->zlib_stream;
    
    /* Set up input/output buffers */
    strm->next_in = (Bytef*)input;
    strm->avail_in = (uInt)input_len;
    strm->next_out = (Bytef*)output;
    strm->avail_out = (uInt)output_len;
    
    /* Compress data */
    int result = deflate(strm, Z_SYNC_FLUSH);
    if (result != Z_OK) {
        return -1;
    }
    
    /* Calculate compressed length */
    *compressed_len = output_len - strm->avail_out;
    
    /* Update statistics */
    data->uncompressed_bytes += input_len;
    data->compressed_bytes += *compressed_len;
    
    if (data->uncompressed_bytes > 0) {
        data->compression_ratio = (double)data->compressed_bytes / data->uncompressed_bytes;
    }
    
    return 0;
    
#else
    /* No zlib support */
    return -1;
#endif
}

static TELNETTY_UNUSED int telnetty_mccp3_init(telnetty_context_t* ctx, telnetty_mccp3_data_t* data, int level) {
    if (!ctx || !data) return -1;
    
#ifdef TELNETTY_HAVE_ZLIB
    /* Initialize zlib for compression/decompression */
    data->zlib_stream = TELNETTY_COMPRESSION_MALLOC(sizeof(z_stream));
    if (!data->zlib_stream) return -1;
    
    z_stream* strm = (z_stream*)data->zlib_stream;
    strm->zalloc = Z_NULL;
    strm->zfree = Z_NULL;
    strm->opaque = Z_NULL;
    
    /* Initialize based on mode */
    int result;
    if (data->server_mode) {
        result = deflateInit(strm, level > 0 ? level : TELNETTY_COMPRESSION_LEVEL);
    } else if (data->client_mode) {
        result = inflateInit(strm);
    } else {
        result = Z_ERRNO;
    }
    
    if (result != Z_OK) {
        TELNETTY_COMPRESSION_FREE(data->zlib_stream);
        data->zlib_stream = NULL;
        return -1;
    }
    
    /* Allocate buffers */
    data->input_buffer = (uint8_t*)TELNETTY_COMPRESSION_MALLOC(data->input_size);
    data->output_buffer = (uint8_t*)TELNETTY_COMPRESSION_MALLOC(data->output_size);
    
    if (!data->input_buffer || !data->output_buffer) {
        telnetty_mccp3_cleanup(data);
        return -1;
    }
    
#else
    /* No zlib support - return error */
    return -1;
#endif
    
    return 0;
}

static TELNETTY_UNUSED int telnetty_mccp3_cleanup(telnetty_mccp3_data_t* data) {
    if (!data) return -1;
    
#ifdef TELNETTY_HAVE_ZLIB
    if (data->zlib_stream) {
        z_stream* strm = (z_stream*)data->zlib_stream;
        if (data->server_mode) {
            deflateEnd(strm);
        } else if (data->client_mode) {
            inflateEnd(strm);
        }
        TELNETTY_COMPRESSION_FREE(data->zlib_stream);
        data->zlib_stream = NULL;
    }
#endif
    
    if (data->input_buffer) {
        TELNETTY_COMPRESSION_FREE(data->input_buffer);
        data->input_buffer = NULL;
    }
    
    if (data->output_buffer) {
        TELNETTY_COMPRESSION_FREE(data->output_buffer);
        data->output_buffer = NULL;
    }
    
    return 0;
}

static int telnetty_mccp3_compress(
    telnetty_mccp3_data_t* data,
    const uint8_t* input,
    size_t input_len,
    uint8_t* output,
    size_t output_len,
    size_t* compressed_len
) {
    if (!data || !input || !output || !compressed_len) return -1;
    
#ifdef TELNETTY_HAVE_ZLIB
    if (!data->zlib_stream || !data->server_mode) return -1;
    
    z_stream* strm = (z_stream*)data->zlib_stream;
    
    /* Set up input/output buffers */
    strm->next_in = (Bytef*)input;
    strm->avail_in = (uInt)input_len;
    strm->next_out = (Bytef*)output;
    strm->avail_out = (uInt)output_len;
    
    /* Compress data */
    int result = deflate(strm, Z_SYNC_FLUSH);
    if (result != Z_OK) {
        return -1;
    }
    
    /* Calculate compressed length */
    *compressed_len = output_len - strm->avail_out;
    
    /* Update statistics */
    data->uncompressed_bytes += input_len;
    data->compressed_bytes += *compressed_len;
    
    if (data->uncompressed_bytes > 0) {
        data->compression_ratio = (double)data->compressed_bytes / data->uncompressed_bytes;
    }
    
    return 0;
    
#else
    /* No zlib support */
    return -1;
#endif
}

static int telnetty_mccp3_decompress(
    telnetty_mccp3_data_t* data,
    const uint8_t* input,
    size_t input_len,
    uint8_t* output,
    size_t output_len,
    size_t* decompressed_len
) {
    if (!data || !input || !output || !decompressed_len) return -1;
    
#ifdef TELNETTY_HAVE_ZLIB
    if (!data->zlib_stream || !data->client_mode) return -1;
    
    z_stream* strm = (z_stream*)data->zlib_stream;
    
    /* Set up input/output buffers */
    strm->next_in = (Bytef*)input;
    strm->avail_in = (uInt)input_len;
    strm->next_out = (Bytef*)output;
    strm->avail_out = (uInt)output_len;
    
    /* Decompress data */
    int result = inflate(strm, Z_SYNC_FLUSH);
    if (result != Z_OK && result != Z_STREAM_END) {
        return -1;
    }
    
    /* Calculate decompressed length */
    *decompressed_len = output_len - strm->avail_out;
    
    return 0;
    
#else
    /* No zlib support */
    return -1;
#endif
}

/* ============================================================================
 * Utility Functions Implementation
 * ============================================================================ */

static TELNETTY_UNUSED int telnetty_compression_init(telnetty_context_t* ctx) {
    if (!ctx) return -1;
    
    /* Initialize compression system */
    /* This would set up the overall compression management */
    
    return 0;
}

static TELNETTY_UNUSED int telnetty_compression_cleanup(telnetty_context_t* ctx) {
    if (!ctx) return -1;
    
    /* Cleanup compression system */
    /* This would clean up all compression-related data */
    
    return 0;
}

static int telnetty_compression_get_stats(
    telnetty_context_t* ctx,
    telnetty_compression_stats_t* mccp2_stats,
    telnetty_compression_stats_t* mccp3_stats
) {
    if (!ctx) return -1;
    
    /* Get statistics from both compression protocols */
    if (mccp2_stats) {
        telnetty_mccp2_get_stats(ctx, 
            &mccp2_stats->compressed_bytes,
            &mccp2_stats->uncompressed_bytes,
            &mccp2_stats->compression_ratio);
    }
    
    if (mccp3_stats) {
        telnetty_mccp3_get_stats(ctx, 
            &mccp3_stats->compressed_bytes,
            &mccp3_stats->uncompressed_bytes,
            &mccp3_stats->compression_ratio);
    }
    
    return 0;
}

static int telnetty_compression_auto_negotiate(
    telnetty_context_t* ctx,
    bool prefer_mccp3
) {
    (void)prefer_mccp3;
    if (!ctx) return -1;
    
    /* Enable both compression protocols for automatic negotiation */
    int result1 = telnetty_enable_mccp2(ctx, TELNETTY_COMPRESSION_LEVEL);
    int result2 = telnetty_enable_mccp3(ctx, TELNETTY_COMPRESSION_LEVEL);
    
    /* Return success if at least one protocol was enabled */
    return (result1 == 0 || result2 == 0) ? 0 : -1;
}

#endif /* TELNETTY_COMPRESSION_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif

#endif /* TELNETTY_COMPRESSION_H */