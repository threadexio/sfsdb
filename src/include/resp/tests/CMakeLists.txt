set(TEST_INCLUDE_DIR ../../)

set(CATCH_MAIN_SRC main.cpp)

set (TEST_TARGETS
	00-parser
	01-integer
	02-simstr
	03-bulkstr
	04-error
	05-multi-type
)

message(STATUS "resp: Enabled tests")

add_library(RespCatchMain OBJECT ${CATCH_MAIN_SRC})

foreach(target ${TEST_TARGETS})
	add_executable(${target} ${target}.cpp)
	target_include_directories(${target} PRIVATE ${TEST_INCLUDE_DIR})
	target_link_libraries(${target} RespCatchMain main resp)
endforeach()
