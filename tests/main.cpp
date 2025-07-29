#include <gtest/gtest.h>
#include <QApplication>

int main(int argc, char** argv) {
    // Инициализация Google Test
    ::testing::InitGoogleTest(&argc, argv);

    QApplication app(argc,argv);

    // Запуск всех тестов
    return RUN_ALL_TESTS();
}
