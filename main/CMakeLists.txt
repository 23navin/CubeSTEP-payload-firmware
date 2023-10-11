idf_component_register(
    SRCS main.cpp 
    REQUIRES i2cControl fileControl experimentControl pwmControl adcControl telemetryControl
)