idf_component_register( SRCS 
                            "main.c"
                            "filesystem.c"
                            "config.c"
                            "system.c"
                            "uart.c"
                            "ws_log.c"

                            "networking/http_server.c"
                            "networking/wifi_ap.c"
                            "networking/socket_tcp.c"
                            "networking/ethernet.c"
                            "networking/client_tcp.c"
                           
                        INCLUDE_DIRS 
                            "."
                            "./networking/"
)

