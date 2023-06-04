
TARGET = main


SOURCES = main.cpp


CXX = g++
CXXFLAGS =-o2 -fPIC -I/home/djoker/Qt/5.15.2/gcc_64/include -I/home/djoker/Qt/5.15.2/gcc_64/include/QtCore -I/home/djoker/Qt/5.15.2/gcc_64/include/QtWidgets -std=c++11 -Wall


QT_LIBS =-L/home/djoker/Qt/5.15.2/gcc_64/lib -lQt5Widgets -lQt5Core


$(TARGET):  $(SOURCES)
		$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(QT_LIBS)
		./$(TARGET)

release: $(TARGET)
		strip $(TARGET)

re: clean $(TARGET)

clean:
		rm -f $(TARGET)
