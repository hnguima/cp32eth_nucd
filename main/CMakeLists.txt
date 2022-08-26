idf_component_register( SRCS 
                            "main.c"
                            "system.c"
                            "uart.c"
                            "ws_log.c"
                            
                            "networking/http_server.c"
                            "networking/wifi_ap.c"
                            "networking/socket_tcp.c"
                            "networking/client_tcp.c"
                            
                            "config/config.c"
                            "config/filesystem.c"

                        INCLUDE_DIRS 
                            "."
                            "./networking/"
                            "./config/"
)