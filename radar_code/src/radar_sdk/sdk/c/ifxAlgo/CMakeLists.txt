set(SDK_ALGO_SOURCES
    2DMTI.c
    DBSCAN.c
    FFT.c
    MTI.c
    OSCFAR.c
    PreprocessedFFT.c
    Signal.c
    Window.c
)

set(SDK_ALGO_HEADERS
    2DMTI.h
    Algo.h
    DBSCAN.h
    FFT.h
    MTI.h
    OSCFAR.h
    PreprocessedFFT.h
    Signal.h
    Window.h
)

add_library(sdk_algo SHARED ${SDK_ALGO_SOURCES} ${SDK_ALGO_HEADERS})
target_link_libraries(sdk_algo PRIVATE muFFT)
target_link_libraries(sdk_algo PUBLIC sdk_base)
