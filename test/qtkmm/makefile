# Variáveis
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra
LDFLAGS = `pkg-config gtkmm-3.0 --cflags --libs`
TARGET = main

# Arquivos-fonte
SRCS = main.cpp

# Arquivos-objeto
OBJS = $(SRCS:.cpp=.o)

# Regra padrão
all: $(TARGET)

# Regra para compilar o programa
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	./$(TARGET)

# Regra para compilar os arquivos-objeto
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(LDFLAGS)

# Regras adicionais
.PHONY: clean

clean:
	rm -f $(OBJS) $(TARGET)
