set(app_src keypad.c)
set(pri_req )
idf_component_register(SRCS ${app_src}
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES "driver"
                    REQUIRES ${pri_req})



# idf_component_register(SRCS "${component_srcs}"
#     INCLUDE_DIRS "."
#     PRIV_INCLUDE_DIRS ""
#     PRIV_REQUIRES "driver"
#     REQUIRES  )
