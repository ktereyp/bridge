SET(TestApplication_SRCS TestApplication.cpp)
SET(TEST_LIBRARIES CharmCore ${QT_QTTEST_LIBRARY} ${QT_LIBRARIES})

SET(SqLiteStorageTests_SRCS SqLiteStorageTests.cpp)
QT4_AUTOMOC(${SqLiteStorageTests_SRCS})
ADD_EXECUTABLE(SqLiteStorageTests ${SqLiteStorageTests_SRCS})
TARGET_LINK_LIBRARIES(SqLiteStorageTests ${TEST_LIBRARIES})
ADD_TEST(NAME SqLiteStorageTests COMMAND SqLiteStorageTests)