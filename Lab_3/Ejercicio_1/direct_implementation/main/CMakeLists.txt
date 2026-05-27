idf_component_register(
    SRCS "direct_implementation.c" "inference.c"
    INCLUDE_DIRS "."
    REQUIRES esp-dsp esp_timer
)
