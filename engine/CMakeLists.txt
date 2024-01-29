#--------------------------------------------------------------------------
# engine
#--------------------------------------------------------------------------

SET(engine_files 
	camera.h
	camera.cc
	deferred.h
	deferred.cc
	drawCallHandler.h
	drawCallHandler.cc
	graphicsObject.h
	graphicsObject.cc
	material.h
	material.cc
	mesh.h
	mesh.cc
	modularShader.h
	modularShader.cc
	object.h
	object.cc
	shader.h
	shader.cc
	timeHandler.h
	timeHandler.cc
	transform.h
	transform.cc
	voxelData.h
	voxelData.cc
	window.h
	window.cc
)
SOURCE_GROUP("engine" FILES ${engine_files})
ADD_LIBRARY(engine STATIC ${engine_files})
ADD_DEPENDENCIES(engine glew glfw)
TARGET_INCLUDE_DIRECTORIES(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
TARGET_LINK_LIBRARIES(engine PUBLIC exts glew glfw ${OPENGL_LIBS})

