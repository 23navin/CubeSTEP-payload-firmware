idf_component_register(
    SRCS main.cpp 
    REQUIRES i2cControl spiffsControl experimentControl pwmControl adcControl telemetryControl
)