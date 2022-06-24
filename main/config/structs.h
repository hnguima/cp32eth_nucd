#ifndef __STRUCTS__H
#define __STRUCTS__H

#define S8_(first, ...) \
    first;              \
    S7_(__VA_ARGS__)
#define S7_(first, ...) \
    first;              \
    S6_(__VA_ARGS__)
#define S6_(first, ...) \
    first;              \
    S5_(__VA_ARGS__)
#define S5_(first, ...) \
    first;              \
    S4_(__VA_ARGS__)
#define S4_(first, ...) \
    first;              \
    S3_(__VA_ARGS__)
#define S3_(first, ...) \
    first;              \
    S2_(__VA_ARGS__)
#define S2_(first, ...) \
    first;              \
    S1_(__VA_ARGS__)
#define S1_(first) first;

#define PASTE3(a, b, c) PASTE3_(a, b, c)
#define PASTE3(a, b, c) a##b##c

#define MY_STRUCT(name, ...)             \
    typedef struct                       \
    {                                    \
        PASTE3(S, NARGS(__VA_ARGS__), _) \
        (__VA_ARGS__)                    \
    } name;

MY_STRUCT(s3, const char *s, int t[17], double sum);

typedef struct
{
    char *description;
    char *hw_version;
    char *fw_version;

    uint16_t resets;
    uint16_t config_updates;
    uint16_t if_updates;
    uint16_t fw_updates;

    uint32_t up_time;
    uint32_t init_time;
    uint32_t install_time;

    uint32_t available_mem;

} info_data_t;

typedef struct
{
    char *username;
    char *password;

} login_data_t;

typedef struct
{
    int32_t ip;
    int32_t mask;
    int32_t gateway;

} ip_data_t;

typedef struct
{

    uint8_t enabled;

    char *ssid;
    char *password;

} wifi_data_t;

typedef struct
{
    int32_t ip;
    uint32_t port;

} client_tcp_data_t;

typedef struct
{

    info_data_t *info;
    login_data_t *login;
    login_data_t *root;

    ip_data_t *ip;
    wifi_data_t *wifi;
    client_tcp_data_t *socket;

} cp32eth_data_t;

#endif //<-- __STRUCTS__H -->