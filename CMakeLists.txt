cmake_minimum_required(VERSION 3.6)
project(scoreFollowing_v3)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp DynamicProgramming.cpp DynamicProgramming.h findPath.cpp findPath.h matchEvent.cpp matchEvent.h readData.cpp readData.h scoreFollowingEvent.cpp scoreFollowingEvent.h EvaluateSfResult.cpp EvaluateSfResult.h midiToEvent.cpp midiToEvent.h ScoreFollowing.cpp ScoreFollowing.h ProcessNMF.cpp ProcessNMF.h)
add_executable(scoreFollowing_v3 ${SOURCE_FILES})