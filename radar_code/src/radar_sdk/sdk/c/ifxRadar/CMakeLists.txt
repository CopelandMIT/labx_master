set(SDK_RADAR_SOURCES
    AngleCapon.c
    AngleMonopulse.c
    DBF.c
    DBF.h
    DeInterleaver.cpp
    PeakSearch.c
    RangeAngleImage.c
    RangeDopplerMap.c
    RangeSpectrum.c
    SpectrumAxis.cpp
    DopplerSpectrogram.c
)

set(SDK_RADAR_HEADERS
    AngleCapon.h
    AngleMonopulse.h
    DBF.h
    DeInterleaver.hpp
    PeakSearch.h
    Radar.h
    RangeAngleImage.c
    RangeAngleImage.h
    RangeDopplerMap.h
    RangeSpectrum.h
    SpectrumAxis.cpp
    SpectrumAxis.h
    DopplerSpectrogram.h
    internal/DeInterleaver.h
)

add_library(sdk_radar SHARED ${SDK_RADAR_SOURCES} ${SDK_RADAR_HEADERS})
target_link_libraries(sdk_radar PUBLIC sdk_base sdk_algo sdk_avian)
