#--------------------------------------------------------------------------
# game
#--------------------------------------------------------------------------

PROJECT(game)

SET(game_files 
	entry.cc
	application.h
	application.cc
)
SOURCE_GROUP("code" FILES ${game_files})

ADD_EXECUTABLE(game ${game_files})
TARGET_LINK_LIBRARIES(game engine)
ADD_DEPENDENCIES(game engine)

IF(MSVC)
	SET_PROPERTY(TARGET game PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
ENDIF(MSVC)