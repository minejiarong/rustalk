#ifndef RUSTALK_CORE_H
#define RUSTALK_CORE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// System API
int rustalk_add(int a, int b);
int rustalk_init(const char* db_path);
void rustalk_shutdown();
int rustalk_connect(const char* url);
int rustalk_status();

// Auth API
int rustalk_register(const char* username, const char* password, int64_t* out_user_id);
int rustalk_login(const char* username, const char* password, int64_t* out_user_id);
int rustalk_logout();

// Contact API
typedef struct {
    int64_t id;
    char* name;
} ContactFFI;

ContactFFI* rustalk_fetch_contacts(int* out_len);
void rustalk_free_contacts(ContactFFI* ptr, int len);
int rustalk_upsert_contact(int64_t id, const char* name);

// Chat API
typedef struct {
    int64_t id;
    int64_t from;
    int64_t to;
    char* content;
    int64_t timestamp;
} MessageFFI;

int rustalk_send_message(int64_t from, int64_t to, const char* content);
MessageFFI* rustalk_fetch_history(int64_t peer, int limit, int* out_len);
MessageFFI* rustalk_search_history(int64_t peer, const char* keyword, int* out_len);
void rustalk_free_messages(MessageFFI* ptr, int len);
int rustalk_delete_message(int64_t id);

#ifdef __cplusplus
}
#endif

#endif // RUSTALK_CORE_H
