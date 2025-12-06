/* Suppress warnings for extended event types in switch statements */
#ifdef __clang__
#pragma clang diagnostic ignored "-Wswitch"
#pragma clang diagnostic ignored "-Wcast-function-type"
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wswitch"
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif

/**
 * @file example_server_libuv.c
 * @brief High-performance multi-threaded Telnetty server using libuv
 * 
 * This example demonstrates a high-performance Telnetty server that uses libuv
 * for asynchronous I/O and multiple threads for handling many concurrent
 * connections efficiently.
 * 
 * Features demonstrated:
 * - libuv event loop integration
 * - Multi-threaded architecture
 * - Connection pooling
 * - Asynchronous I/O
 * - Performance monitoring
 * - Graceful shutdown
 * 
 * @author Damus <damus@straylightrun.org>
 * @version 1.0.0
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <uv.h>
#include <time.h>
#include <signal.h>

#define TELNETTY_IMPLEMENTATION
#define TELNETTY_EVENTS_IMPLEMENTATION
#define TELNETTY_BUFFER_IMPLEMENTATION
#define TELNETTY_OPTIONS_IMPLEMENTATION
#define TELNETTY_MUD_IMPLEMENTATION
#define TELNETTY_COMPRESSION_IMPLEMENTATION
#define TELNETTY_COLOR_IMPLEMENTATION

#include "telnetty_core.h"
#include "telnetty_events.h"
#include "telnetty_buffer.h"
#include "telnetty_options.h"
#include "telnetty_mud.h"
#include "telnetty_compression.h"
#include "telnetty_color.h"

/* ============================================================================
 * Configuration
 * ============================================================================ */

#define SERVER_PORT 4000
#define DEFAULT_THREAD_COUNT 4
#define MAX_CLIENTS_PER_THREAD 1000
#define BUFFER_SIZE 4096
#define CLIENT_TIMEOUT 300  /* seconds */

/* ============================================================================
 * Thread-Safe Statistics
 * ============================================================================ */

typedef struct {
    uint64_t connections_total;
    uint64_t connections_active;
    uint64_t bytes_received;
    uint64_t bytes_sent;
    uint64_t packets_received;
    uint64_t packets_sent;
    uint64_t errors;
    time_t start_time;
    pthread_mutex_t mutex;
} server_stats_t;

static server_stats_t server_stats = {0};

/* ============================================================================
 * Statistics Management
 * ============================================================================ */

static void stats_init(void) {
    pthread_mutex_init(&server_stats.mutex, NULL);
    server_stats.start_time = time(NULL);
}

static void stats_cleanup(void) {
    pthread_mutex_destroy(&server_stats.mutex);
}

static void stats_add_connection(void) {
    pthread_mutex_lock(&server_stats.mutex);
    server_stats.connections_total++;
    server_stats.connections_active++;
    pthread_mutex_unlock(&server_stats.mutex);
}

static void stats_remove_connection(void) {
    pthread_mutex_lock(&server_stats.mutex);
    server_stats.connections_active--;
    pthread_mutex_unlock(&server_stats.mutex);
}

static void stats_add_bytes_received(size_t bytes) {
    pthread_mutex_lock(&server_stats.mutex);
    server_stats.bytes_received += bytes;
    server_stats.packets_received++;
    pthread_mutex_unlock(&server_stats.mutex);
}

static void stats_add_bytes_sent(size_t bytes) {
    pthread_mutex_lock(&server_stats.mutex);
    server_stats.bytes_sent += bytes;
    server_stats.packets_sent++;
    pthread_mutex_unlock(&server_stats.mutex);
}

static void stats_add_error(void) {
    pthread_mutex_lock(&server_stats.mutex);
    server_stats.errors++;
    pthread_mutex_unlock(&server_stats.mutex);
}

static void stats_print(void) {
    pthread_mutex_lock(&server_stats.mutex);
    
    time_t now = time(NULL);
    time_t uptime = now - server_stats.start_time;
    
    printf("\n");
    printf("==============================================\n");
    printf("Server Statistics\n");
    printf("==============================================\n");
    printf("Uptime: %ld seconds\n", uptime);
    printf("Total connections: %lu\n", server_stats.connections_total);
    printf("Active connections: %lu\n", server_stats.connections_active);
    printf("Bytes received: %lu\n", server_stats.bytes_received);
    printf("Bytes sent: %lu\n", server_stats.bytes_sent);
    printf("Packets received: %lu\n", server_stats.packets_received);
    printf("Packets sent: %lu\n", server_stats.packets_sent);
    printf("Errors: %lu\n", server_stats.errors);
    
    if (uptime > 0) {
        printf("Average bytes/sec: %.2f\n", 
               (double)(server_stats.bytes_received + server_stats.bytes_sent) / uptime);
    }
    
    printf("==============================================\n");
    
    pthread_mutex_unlock(&server_stats.mutex);
}

/* ============================================================================
 * Client Connection Structure
 * ============================================================================ */

typedef struct {
    uv_tcp_t handle;                    /* libuv handle */
    telnetty_context_t* telnetty_ctx;       /* Telnetty context */
    uv_buf_t read_buffer;               /* Read buffer */
    uv_write_t* write_req;              /* Write request */
    char* write_buffer;                 /* Write buffer */
    size_t write_buffer_size;           /* Write buffer size */
    char* client_name;                  /* Client identifier */
    uint16_t window_width;              /* Terminal width */
    uint16_t window_height;             /* Terminal height */
    time_t last_activity;               /* Last activity timestamp */
    struct sockaddr_in addr;            /* Client address */
    bool authenticated;                 /* Client authenticated */
    pthread_mutex_t write_mutex;        /* Write mutex */
    char* buffer;                       /* General purpose buffer */
    size_t buffer_size;                 /* Buffer size */
} client_connection_t;

/* ============================================================================
 * Thread Data Structure
 * ============================================================================ */

typedef struct {
    uv_loop_t* loop;                    /* libuv event loop */
    uv_async_t async_handle;            /* Async handle for inter-thread comm */
    uv_timer_t timer_handle;            /* Timer for periodic tasks */
    pthread_t thread_id;                /* Thread ID */
    int thread_index;                   /* Thread index */
    client_connection_t* clients[MAX_CLIENTS_PER_THREAD];
    size_t client_count;                /* Number of active clients */
    bool running;                       /* Thread running flag */
} thread_data_t;

static thread_data_t* threads = NULL;
static int thread_count = DEFAULT_THREAD_COUNT;
static uv_tcp_t server_handle;
static uv_loop_t* main_loop = NULL;
static bool server_running = true;

/* ============================================================================
 * Connection Pool Management
 * ============================================================================ */

static int find_thread_with_least_clients(void) {
    int min_thread = 0;
    size_t min_clients = threads[0].client_count;
    
    for (int i = 1; i < thread_count; i++) {
        if (threads[i].client_count < min_clients) {
            min_clients = threads[i].client_count;
            min_thread = i;
        }
    }
    
    return min_thread;
}

static int find_free_client_slot(thread_data_t* thread_data) {
    for (int i = 0; i < MAX_CLIENTS_PER_THREAD; i++) {
        if (thread_data->clients[i] == NULL) {
            return i;
        }
    }
    return -1;
}

/* ============================================================================
 * Memory Management
 * ============================================================================ */

static void* safe_malloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    return ptr;
}

static void* safe_realloc(void* ptr, size_t size) {
    void* new_ptr = realloc(ptr, size);
    if (!new_ptr) {
        fprintf(stderr, "Memory reallocation failed\n");
        exit(1);
    }
    return new_ptr;
}

/* ============================================================================
 * TELNET Event Handler
 * ============================================================================ */

static void handle_telnetty_event(
    telnetty_context_t* ctx,
    telnetty_event_type_t event,
    const telnetty_event_data_union_t* data,
    void* user_data
) {
    client_connection_t* client = (client_connection_t*)user_data;
    
    switch (event) {
        case TELNETTY_EVENT_DATA: {
            /* Process received data */
            const char* text = (const char*)data->data.data;
            size_t length = data->data.length;
            
            /* Update statistics */
            stats_add_bytes_received(length);
            
            /* Echo back with color */
            char response[BUFFER_SIZE];
            int response_len = snprintf(response, sizeof(response), 
                                        "Server received: %.*s\r\n", 
                                        (int)length, text);
            
            if (response_len > 0) {
                /* Queue write */
                pthread_mutex_lock(&client->write_mutex);
                
                /* Allocate write buffer if needed */
                if (!client->write_buffer || (int)client->write_buffer_size < response_len) {
                    client->write_buffer = safe_realloc(client->write_buffer, response_len);
                    client->write_buffer_size = response_len;
                }
                
                memcpy(client->write_buffer, response, response_len);
                
                /* Create write request */
                /* uv_buf_t buf = uv_buf_init(client->write_buffer, response_len); */
                
                /* Note: In a real implementation, we'd use uv_write here */
                /* For this example, we'll just accumulate the data */
                
                pthread_mutex_unlock(&client->write_mutex);
            }
            
            /* Update last activity */
            client->last_activity = time(NULL);
            break;
        }
        
        case TELNETTY_EVENT_WILL: {
            uint8_t option = data->option.option;
            
            switch (option) {
                case TELNETTY_TELOPT_ECHO:
                case TELNETTY_TELOPT_SGA:
                case TELNETTY_TELOPT_TTYPE:
                case TELNETTY_TELOPT_NAWS:
                case TELNETTY_TELOPT_MSDP:
                case TELNETTY_TELOPT_GMCP:
                case TELNETTY_TELOPT_MCCP2:
                case TELNETTY_TELOPT_MCCP3:
                    /* Send DO for supported options */
                    telnetty_send_option(ctx, TELNETTY_DO, option);
                    break;
                    
                default:
                    /* Send DONT for unsupported options */
                    telnetty_send_option(ctx, TELNETTY_DONT, option);
                    break;
            }
            break;
        }
        
        case TELNETTY_EVENT_DO: {
            uint8_t option = data->option.option;
            
            switch (option) {
                case TELNETTY_TELOPT_ECHO:
                case TELNETTY_TELOPT_SGA:
                case TELNETTY_TELOPT_MCCP2:
                case TELNETTY_TELOPT_MCCP3:
                    /* Send WILL for supported options */
                    telnetty_send_option(ctx, TELNETTY_WILL, option);
                    break;
                    
                default:
                    /* Send WONT for unsupported options */
                    telnetty_send_option(ctx, TELNETTY_WONT, option);
                    break;
            }
            break;
        }
        
        case TELNETTY_EVENT_SB: {
            /* Handle subnegotiation */
            uint8_t option = data->sub.option;
            const uint8_t* sb_data = data->sub.data;
            size_t sb_length = data->sub.length;
            
            switch (option) {
                case TELNETTY_TELOPT_TTYPE: {
                    if (sb_length > 1 && sb_data[0] == 0) { /* IS */
                        char* term_type = safe_malloc(sb_length);
                        memcpy(term_type, sb_data + 1, sb_length - 1);
                        term_type[sb_length - 1] = '\0';
                        
                        if (client->client_name) {
                            free(client->client_name);
                        }
                        client->client_name = term_type;
                        
                        /* Detect color capabilities */
                        telnetty_color_cap_t caps = telnetty_color_detect_capabilities(
                            ctx, term_type);
                        telnetty_color_set_capabilities(ctx, caps);
                    }
                    break;
                }
                
                case TELNETTY_TELOPT_NAWS: {
                    if (sb_length >= 4) {
                        client->window_width = (sb_data[0] << 8) | sb_data[1];
                        client->window_height = (sb_data[2] << 8) | sb_data[3];
                    }
                    break;
                }
            }
            break;
        }
        
        case (telnetty_event_type_t)TELNETTY_EVENT_CONNECT: {
            /* Send welcome message */
            const char* welcome = 
                "\r\n"
                "Welcome to the High-Performance Telnetty Server!\r\n"
                "==============================================\r\n"
                "This server uses libuv for maximum performance.\r\n"
                "\r\n";
            
            telnetty_color_send(ctx, welcome, TELNETTY_COLOR_GREEN, TELNETTY_COLOR_BLACK, 0);
            
            /* Enable all options */
            telnetty_enable_all_options(ctx);
            
            break;
        }
        
        case (telnetty_event_type_t)TELNETTY_EVENT_DISCONNECT: {
            /* Connection will be closed by libuv callback */
            break;
        }
    }
}

/* ============================================================================
 * libuv Callbacks
 * ============================================================================ */

static void alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    (void)handle;
    buf->base = (char*)safe_malloc(suggested_size);
    buf->len = suggested_size;
}

static void on_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
    client_connection_t* client = (client_connection_t*)stream->data;
    
    if (nread > 0) {
        /* Process Telnetty data */
        int processed = telnetty_process(client->telnetty_ctx, 
                                      (uint8_t*)buf->base, 
                                      nread);
        
        if (processed < 0) {
            stats_add_error();
            fprintf(stderr, "Telnetty processing error\n");
        }
        
        /* Update statistics */
        stats_add_bytes_received(nread);
        
        /* Update last activity */
        client->last_activity = time(NULL);
    } else if (nread < 0) {
        if (nread != UV_EOF) {
            fprintf(stderr, "Read error: %s\n", uv_strerror(nread));
            stats_add_error();
        }
        
        /* Close connection */
        uv_close((uv_handle_t*)&client->handle, NULL);
    }
    
    /* Free read buffer */
    if (buf->base) {
        free(buf->base);
    }
}

static void on_write(uv_write_t* req, int status) {
    /* client_connection_t* client = (client_connection_t*)req->data; */
    
    if (status < 0) {
        fprintf(stderr, "Write error: %s\n", uv_strerror(status));
        stats_add_error();
    } else {
        /* Update statistics */
        stats_add_bytes_sent(req->bufs[0].len);
    }
    
    /* Free write request */
    free(req);
}

static void on_close(uv_handle_t* handle) {
    client_connection_t* client = (client_connection_t*)handle->data;
    
    if (client) {
        /* Update statistics */
        stats_remove_connection();
        
        /* Cleanup client resources */
        if (client->telnetty_ctx) {
            telnetty_color_cleanup(client->telnetty_ctx);
            telnetty_destroy(client->telnetty_ctx);
        }
        
        if (client->client_name) {
            free(client->client_name);
        }
        
        if (client->write_buffer) {
            free(client->write_buffer);
        }
        
        pthread_mutex_destroy(&client->write_mutex);
        free(client);
    }
}

static void on_new_connection(uv_stream_t* server, int status) {
    if (status < 0) {
        fprintf(stderr, "New connection error: %s\n", uv_strerror(status));
        return;
    }
    
    /* Find thread with least clients */
    int thread_index = find_thread_with_least_clients();
    thread_data_t* thread_data = &threads[thread_index];
    
    /* Find free slot */
    int slot = find_free_client_slot(thread_data);
    if (slot < 0) {
        /* Server full - reject connection */
        uv_tcp_t* temp_handle = (uv_tcp_t*)safe_malloc(sizeof(uv_tcp_t));
        uv_tcp_init(thread_data->loop, temp_handle);
        
        if (uv_accept(server, (uv_stream_t*)temp_handle) == 0) {
            /* Send rejection message */
            const char* reject_msg = "Server full, please try again later.\r\n";
            uv_buf_t buf = uv_buf_init((char*)reject_msg, strlen(reject_msg));
            uv_write_t* write_req = (uv_write_t*)safe_malloc(sizeof(uv_write_t));
            uv_write(write_req, (uv_stream_t*)temp_handle, &buf, 1, 
                    (uv_write_cb)uv_close);
        }
        
        uv_close((uv_handle_t*)temp_handle, (uv_close_cb)free);
        return;
    }
    
    /* Create client connection */
    client_connection_t* client = (client_connection_t*)safe_malloc(sizeof(client_connection_t));
    memset(client, 0, sizeof(client_connection_t));
    
    /* Initialize libuv handle */
    uv_tcp_init(thread_data->loop, &client->handle);
    
    /* Accept connection */
    if (uv_accept(server, (uv_stream_t*)&client->handle) == 0) {
        /* Get client address */
        int name_len = sizeof(client->addr);
        uv_tcp_getpeername(&client->handle, (struct sockaddr*)&client->addr, &name_len);
        
        /* Set up client */
        client->handle.data = client;
        client->buffer_size = BUFFER_SIZE;
        client->buffer = (char*)safe_malloc(client->buffer_size);
        client->last_activity = time(NULL);
        pthread_mutex_init(&client->write_mutex, NULL);
        
        /* Create Telnetty context */
        client->telnetty_ctx = telnetty_create(handle_telnetty_event, client);
        if (!client->telnetty_ctx) {
            fprintf(stderr, "Failed to create Telnetty context\n");
            uv_close((uv_handle_t*)&client->handle, on_close);
            return;
        }
        
        /* Initialize color support */
        telnetty_color_init(client->telnetty_ctx, true);
        
        /* Enable compression auto-negotiation */
        telnetty_compression_auto_negotiate(client->telnetty_ctx, true);
        
        /* Set client name */
        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client->addr.sin_addr, ip_str, sizeof(ip_str));
        client->client_name = strdup(ip_str);
        
        /* Add to thread's client list */
        thread_data->clients[slot] = client;
        thread_data->client_count++;
        
        /* Update statistics */
        stats_add_connection();
        
        /* Start reading */
        uv_read_start((uv_stream_t*)&client->handle, alloc_buffer, on_read);
        
        printf("[Thread %d] Accepted connection from %s:%d (slot %d)\n",
               thread_index, ip_str, ntohs(client->addr.sin_port), slot);
        
        /* Fire connection event */
        char full_ip[INET_ADDRSTRLEN + 8];
        snprintf(full_ip, sizeof(full_ip), "%s:%d", 
                ip_str, ntohs(client->addr.sin_port));
        
        telnetty_event_data_union_t event_data;
        /* Store connection info in extended data area */
        telnetty_event_connect_t* conn = (telnetty_event_connect_t*)&event_data.ext.data;
        conn->address = full_ip;
        conn->port = ntohs(client->addr.sin_port);
        conn->timestamp = time(NULL);
        
        telnetty_fire_event_ex(client->telnetty_ctx, (telnetty_event_type_t)TELNETTY_EVENT_CONNECT, 
                            TELNETTY_PRIORITY_NORMAL, &event_data);
        
    } else {
        /* Accept failed */
        uv_close((uv_handle_t*)&client->handle, on_close);
    }
}

/* ============================================================================
 * Thread Management
 * ============================================================================ */

static void thread_timer_cb(uv_timer_t* handle) {
    thread_data_t* thread_data = (thread_data_t*)handle->data;
    
    /* Check for timed out clients */
    time_t now = time(NULL);
    
    for (int i = 0; i < MAX_CLIENTS_PER_THREAD; i++) {
        client_connection_t* client = thread_data->clients[i];
        if (client && (now - client->last_activity) > CLIENT_TIMEOUT) {
            printf("[Thread %d] Client %s timed out\n",
                   thread_data->thread_index,
                   client->client_name ? client->client_name : "unknown");
            
            /* Close connection */
            uv_close((uv_handle_t*)&client->handle, on_close);
            thread_data->clients[i] = NULL;
            thread_data->client_count--;
        }
    }
}

static void* thread_worker(void* arg) {
    thread_data_t* thread_data = (thread_data_t*)arg;
    
    printf("[Thread %d] Started\n", thread_data->thread_index);
    
    /* Initialize timer for periodic tasks */
    uv_timer_init(thread_data->loop, &thread_data->timer_handle);
    thread_data->timer_handle.data = thread_data;
    uv_timer_start(&thread_data->timer_handle, thread_timer_cb, 
                   CLIENT_TIMEOUT * 1000, CLIENT_TIMEOUT * 1000);
    
    /* Run event loop */
    uv_run(thread_data->loop, UV_RUN_DEFAULT);
    
    /* Cleanup */
    uv_loop_close(thread_data->loop);
    free(thread_data->loop);
    
    printf("[Thread %d] Stopped\n", thread_data->thread_index);
    
    return NULL;
}

static int create_worker_threads(void) {
    /* Allocate thread data */
    threads = (thread_data_t*)safe_malloc(sizeof(thread_data_t) * thread_count);
    
    /* Create threads */
    for (int i = 0; i < thread_count; i++) {
        thread_data_t* thread_data = &threads[i];
        
        thread_data->thread_index = i;
        thread_data->running = true;
        thread_data->client_count = 0;
        memset(thread_data->clients, 0, sizeof(thread_data->clients));
        
        /* Create event loop for this thread */
        thread_data->loop = (uv_loop_t*)safe_malloc(sizeof(uv_loop_t));
        uv_loop_init(thread_data->loop);
        
        /* Create thread */
        if (pthread_create(&thread_data->thread_id, NULL, thread_worker, thread_data) != 0) {
            fprintf(stderr, "Failed to create thread %d\n", i);
            return -1;
        }
        
        /* Set thread name for debugging */
        char thread_name[16];
        snprintf(thread_name, sizeof(thread_name), "telnet-%d", i);
    }
    
    return 0;
}

/* ============================================================================
 * Server Management
 * ============================================================================ */

static void on_server_close(uv_handle_t* handle) {
    (void)handle;
    printf("Server socket closed\n");
}

static int start_server(uv_loop_t* loop, int port) {
    /* Initialize server handle */
    uv_tcp_init(loop, &server_handle);
    
    /* Bind to port */
    struct sockaddr_in addr;
    uv_ip4_addr("0.0.0.0", port, &addr);
    
    int result = uv_tcp_bind(&server_handle, (const struct sockaddr*)&addr, 0);
    if (result < 0) {
        fprintf(stderr, "Failed to bind to port %d: %s\n", port, uv_strerror(result));
        return -1;
    }
    
    /* Start listening */
    result = uv_listen((uv_stream_t*)&server_handle, 128, on_new_connection);
    if (result < 0) {
        fprintf(stderr, "Failed to listen: %s\n", uv_strerror(result));
        return -1;
    }
    
    printf("Server listening on port %d with %d threads\n", port, thread_count);
    
    return 0;
}

static void signal_handler(uv_signal_t* handle, int signum) {
    (void)handle;
    printf("\nReceived signal %d, shutting down...\n", signum);
    server_running = false;
    
    /* Stop server */
    uv_close((uv_handle_t*)&server_handle, on_server_close);
    
    /* Stop all worker threads */
    for (int i = 0; i < thread_count; i++) {
        if (threads[i].running) {
            threads[i].running = false;
            /* Send stop signal to thread */
            uv_async_send(&threads[i].async_handle);
        }
    }
}

static void print_banner(void) {
    printf("\n");
    printf("==============================================\n");
    printf("High-Performance Telnetty Server (libuv)\n");
    printf("==============================================\n");
    printf("Port: %d\n", SERVER_PORT);
    printf("Threads: %d\n", thread_count);
    printf("Max clients per thread: %d\n", MAX_CLIENTS_PER_THREAD);
    printf("Client timeout: %d seconds\n", CLIENT_TIMEOUT);
    printf("==============================================\n");
    printf("Press Ctrl+C to stop the server\n");
    printf("==============================================\n\n");
}

/* ============================================================================
 * Main Function
 * ============================================================================ */

int main(int argc, char* argv[]) {
    int port = SERVER_PORT;
    
    /* Parse command line arguments */
    if (argc > 1) {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            fprintf(stderr, "Invalid port: %s\n", argv[1]);
            return 1;
        }
    }
    
    if (argc > 2) {
        thread_count = atoi(argv[2]);
        if (thread_count <= 0 || thread_count > 64) {
            fprintf(stderr, "Invalid thread count: %s (must be 1-64)\n", argv[2]);
            return 1;
        }
    }
    
    /* Initialize statistics */
    stats_init();
    
    /* Print banner */
    print_banner();
    
    /* Create main event loop */
    main_loop = (uv_loop_t*)safe_malloc(sizeof(uv_loop_t));
    uv_loop_init(main_loop);
    
    /* Create worker threads */
    if (create_worker_threads() != 0) {
        fprintf(stderr, "Failed to create worker threads\n");
        free(main_loop);
        stats_cleanup();
        return 1;
    }
    
    /* Start server on main thread */
    if (start_server(main_loop, port) != 0) {
        fprintf(stderr, "Failed to start server\n");
        free(main_loop);
        stats_cleanup();
        return 1;
    }
    
    /* Set up signal handlers */
    uv_signal_t sig_int, sig_term;
    uv_signal_init(main_loop, &sig_int);
    uv_signal_init(main_loop, &sig_term);
    uv_signal_start(&sig_int, signal_handler, SIGINT);
    uv_signal_start(&sig_term, signal_handler, SIGTERM);
    
    /* Main server loop */
    printf("Server running. Press Ctrl+C to stop.\n");
    uv_run(main_loop, UV_RUN_DEFAULT);
    
    /* Wait for worker threads to finish */
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i].thread_id, NULL);
    }
    
    /* Cleanup */
    free(threads);
    uv_loop_close(main_loop);
    free(main_loop);
    
    /* Print final statistics */
    stats_print();
    
    /* Cleanup statistics */
    stats_cleanup();
    
    printf("Server shutdown complete.\n");
    
    return 0;
}