# Название исполняемого файла
TARGET = weather

# Имя исходного файла
SOURCE = weather_app.c

# Компилятор
CC = gcc

# Флаги компиляции
CFLAGS = -Wall -Wextra -Wpedantic -std=c23 -O2

# Ссылки на библиотеки
LIBS = -lcurl -lcjson

# Правило для сборки программы
$(TARGET): $(SOURCE)
    @echo "Building $(TARGET)..."
    $(CC) $(CFLAGS) -o $(TARGET) $(SOURCE) $(LIBS)
    @echo "Build complete."

# Правило для очистки временных файлов
clean:
    @echo "Cleaning up..."
    rm -f $(TARGET) *.o
    @echo "Cleanup complete."

# Правило для пересборки при изменении зависимостей
.PHONY: clean