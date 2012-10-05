TEMPLATE = app
TARGET   = revisor
DEFINES += NDEBUG
CONFIG  += release # release
QT      += webkit network script
LIBS    += -largtable2
INCLUDEPATH += libjson

HEADERS =  src/http_server.hpp \
           src/application.hpp \
           src/dispatcher.hpp \
           src/session.hpp \
           src/session_tab.hpp \
           src/counting_network_access_manager.hpp \
           src/web_page.hpp \
           src/json.hpp \
           src/main_window.hpp

SOURCES =  libjson/_internal/Source/internalJSONNode.cpp \
           libjson/_internal/Source/JSONAllocator.cpp \
           libjson/_internal/Source/JSONChildren.cpp \
           libjson/_internal/Source/JSONDebug.cpp \
           libjson/_internal/Source/JSONIterators.cpp \
           libjson/_internal/Source/JSONMemory.cpp \
           libjson/_internal/Source/JSONNode.cpp \
           libjson/_internal/Source/JSONNode_Mutex.cpp \
           libjson/_internal/Source/JSONPreparse.cpp \
           libjson/_internal/Source/JSONStream.cpp \
           libjson/_internal/Source/JSONValidator.cpp \
           libjson/_internal/Source/JSONWorker.cpp \
           libjson/_internal/Source/JSONWriter.cpp \
           libjson/_internal/Source/libjson.cpp \
           src/main.cpp \
           src/http_server.cpp \
           src/application.cpp \
           src/dispatcher.cpp \
           src/session.cpp \
           src/session_tab.cpp \
           src/counting_network_access_manager.cpp \
           src/web_page.cpp \
           src/json.cpp \
           src/main_window.cpp
