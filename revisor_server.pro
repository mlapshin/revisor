QT      +=  webkit network script
LIBS    +=  -largtable2

HEADERS =   src/http_server.hpp \
            src/application.hpp \
            src/dispatcher.hpp \
            src/session.hpp \
            src/session_tab.hpp \
            src/counting_network_access_manager.hpp

SOURCES =   src/main.cpp \
            src/http_server.cpp \
            src/application.cpp \
            src/dispatcher.cpp \
            src/session.cpp \
            src/session_tab.cpp \
            src/counting_network_access_manager.cpp
