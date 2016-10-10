cmake_minimum_required(VERSION 3.2)
project(osea20_gcc)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES

    template.pb.h
        template.pb.cpp
        wfdb.cpp
        analbeat.cpp
    analbeat.h
        bdac.cpp
    bdac.h

        classify.cpp
        easytest.cpp
    #inputs.h
        match.cpp
    match.h
        noisechk.cpp
        postclas.cpp
    postclas.h

    qrsdet.h
        qrsdet2.cpp
        qrsfilt.cpp
        rythmchk.cpp
    rythmchk.h
        config.h
        config.cpp
        filter.h
        filter.cpp
        inotify_main.cpp
        )


#add_executable(bxbep     bxbep.c ${SOURCE_FILES})
add_executable(easytest inotify_main.cpp   ${SOURCE_FILES} )#easytest.cpp
#add_executable(samptest  samptest.c ${SOURCE_FILES})
#add_executable(osea  osea.c ${SOURCE_FILES})
#target_link_libraries(bxbep wfdb m)
target_link_libraries(easytest  wfdb m pthread boost_system boost_thread glog protobuf)
#target_link_libraries(osea wfdb m)