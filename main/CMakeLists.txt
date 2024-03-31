idf_component_register(SRCS "app_main.c" "app_gpio.c" "app_ledc.c" "app_light_ctrl.c" "app_sntp.c" "app_wifi.c" "app_web_serv.c"
                       INCLUDE_DIRS "."
					   EMBED_FILES "html/auto.html" "html/man.html" "html/favicon.ico")
