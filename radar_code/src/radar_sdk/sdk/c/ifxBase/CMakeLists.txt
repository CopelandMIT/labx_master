set(SDK_BASE_SOURCES
    Complex.c
    Cube.c
    Error.c
    LA.c
    List.cpp
    Log.c
    Math.c
    Matrix.c
    Mda.cpp
    Mem.c
    Util.c
    Uuid.c
    Vector.c
    Version.c
)

set(SDK_BASE_HEADERS
    Base.h
    Complex.h
    Cube.h
    Defines.h
    Error.h
    Exception.hpp
    FunctionWrapper.hpp
    Helper.hpp
    LA.h
    List.cpp
    List.h
    Log.h
    Math.h
    Matrix.h
    Mda.h
    Mem.h
    Types.h
    Uuid.c
    Uuid.h
    Vector.h
    Version.h
    internal/Clamping.hpp
    internal/GuardedHandle.hpp
    internal/List.hpp
    internal/Macros.h
    internal/Mda.hpp
    internal/NonCopyable.hpp
    internal/Simd.h
    internal/Util.h
    Utils.hpp
    )

add_library(sdk_base SHARED ${SDK_BASE_SOURCES} ${SDK_BASE_HEADERS})
target_link_libraries(sdk_base PUBLIC ${RDK_STRATA_LIBRARY})
if(HAS_LIBM)
    target_link_libraries(sdk_base PUBLIC m)
endif()
target_include_directories(sdk_base INTERFACE ..)
