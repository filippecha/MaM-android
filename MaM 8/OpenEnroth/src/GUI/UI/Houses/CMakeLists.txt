cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(GUI_UI_HOUSES_SOURCES
        AdventurersInn.cpp
        Tavern.cpp
        Temple.cpp
        Training.cpp
        Transport.cpp
        Bank.cpp
        Jail.cpp
        MagicGuild.cpp
        TownHall.cpp
        MercenaryGuild.cpp
        Shops.cpp)

set(GUI_UI_HOUSES_HEADERS
        AdventurersInn.h
        Tavern.h
        Temple.h
        Training.h
        Transport.h
        Bank.h
        Jail.h
        MagicGuild.h
        TownHall.h
        MercenaryGuild.h
        Shops.h)

add_library(gui_ui_houses STATIC ${GUI_UI_HOUSES_SOURCES} ${GUI_UI_HOUSES_HEADERS})
target_check_style(gui_ui_houses)

target_link_libraries(gui_ui_houses PUBLIC engine_graphics utility)
