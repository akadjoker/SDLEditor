#pragma once
#include <string>
#include <iostream>

extern void setBuildFlags(const std::string &string);
extern void setCompileFlags(const std::string &string);


inline std::string genRayLib()
{
    std::string raylib = "#include \"raylib.h\"\n";
    raylib += "const int screenWidth = 800;\n";
    raylib += "const int screenHeight = 450;\n";
    raylib += "\n\n"; 
    raylib += "int main(void)\n";
    raylib += "{\n";
    raylib += "    InitWindow(screenWidth, screenHeight, \"raylib\");\n";
    raylib += "    while (!WindowShouldClose())\n";
    raylib += "    {\n";
    raylib += "        BeginDrawing();\n";
    raylib += "        ClearBackground(RAYWHITE);\n";
    raylib += "        EndDrawing();\n";
    raylib += "    }\n";
    raylib += "    CloseWindow();\n";   
    raylib += "    return 1;\n";
    raylib += "}\n";
  //  setBuildFlags(" -lraylib -lm ");
    return raylib;
}

inline std::string getSDL()
{
    std::string sdl2 = "#include <SDL2/SDL.h>\n";
    sdl2 += "const int screenWidth = 800;\n";
    sdl2 += "const int screenHeight = 450;\n";
    sdl2 += "int main(void)\n";
    sdl2 += "{\n";
    sdl2 += "    SDL_Init(SDL_INIT_EVERYTHING);\n";
    sdl2 += "    SDL_Window* window = SDL_CreateWindow(\"SDL!\", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);\n";
    sdl2 += "    SDL_DestroyWindow(window);\n";
    sdl2 += "    SDL_Quit();\n";
    sdl2 += "    return 0;\n";
    sdl2 += "}\n";
    return sdl2;
}

inline std::string getSDLRender()
{
    std::string sdl2 = "#include <SDL2/SDL.h>\n";
    sdl2 += "const int screenWidth = 800;\n";
    sdl2 += "const int screenHeight = 450;\n";
    sdl2 += "int main(void)\n";
    sdl2 += "{\n";
    sdl2 += "    SDL_Init(SDL_INIT_EVERYTHING);\n";
    sdl2 += "    SDL_Window* window = SDL_CreateWindow(\"SDL!\", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);\n";
    sdl2 += "    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);\n";
    sdl2 += "    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);\n";
    sdl2 += "    SDL_RenderClear(renderer);\n";
    sdl2 += "    SDL_RenderPresent(renderer);\n";
    sdl2 += "    SDL_DestroyWindow(window);\n";
    sdl2 += "    SDL_Quit();\n";
    sdl2 += "    return 0;\n";
    sdl2 += "}\n";
    return sdl2;
}

inline std::string getSDLOpengGl()
{

    std::string sdl2 = "#include <SDL2/SDL.h>\n";
    sdl2 += "#include <SDL2/SDL_opengl.h>\n";
    sdl2 += "const int screenWidth = 800;\n";
    sdl2 += "const int screenHeight = 450;\n";
    sdl2 += "int main(void)\n";
    sdl2 += "{\n";
    sdl2 += "    SDL_Init(SDL_INIT_EVERYTHING);\n";
    sdl2 += "    SDL_Window* window = SDL_CreateWindow(\"SDL!\", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);\n";
    sdl2 += "    SDL_GLContext glContext = SDL_GL_CreateContext(window);\n";
    sdl2 += "    SDL_GL_MakeCurrent(window, glContext);\n";
    sdl2 += "    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);\n";
    sdl2 += "    glClear(GL_COLOR_BUFFER_BIT);\n";
    sdl2 += "    SDL_GL_SwapWindow(window);\n";
    sdl2 += "    SDL_GL_DeleteContext(glContext);\n";
    sdl2 += "    SDL_DestroyWindow(window);\n";
    sdl2 += "    SDL_Quit();\n";
    sdl2 += "    return 0;\n";
    sdl2 += "}\n";
    return sdl2;

}

inline std::string getGTK3()
{

    std::string gtk3 = "#include <gtk/gtk.h>\n";
    gtk3 += "int main(int argc, char* argv[])\n";
    gtk3 += "{\n";
    gtk3 += "    gtk_init(&argc, &argv);\n";
    gtk3 += "    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);\n";
    gtk3 += "    gtk_window_set_title(GTK_WINDOW(window), \"GTK3\");\n";
    gtk3 += "    gtk_window_set_default_size(GTK_WINDOW(window), 800, 450);\n";
    gtk3 += "    gtk_widget_show(window);\n";
    gtk3 += "    gtk_main();\n";
    gtk3 += "    return 0;\n";
    gtk3 += "}\n";
    return gtk3;
}

inline std::string getGTKmm()
{

    std::string gtkmm = "#include <gtkmm.h>\n";
    gtkmm += "const int screenWidth = 800;\n";
    gtkmm += "const int screenHeight = 450;\n";
    gtkmm +="\n";
    gtkmm += "int main(int argc, char* argv[])\n";
    gtkmm += "{\n";
    gtkmm += "    auto app = Gtk::Application::create(argc, argv, \"org.gtkmm.example\");\n";
    gtkmm += "    Gtk::Window window;\n";
    gtkmm += "    window.set_default_size(screenWidth, screenHeight);\n";
    gtkmm += "    return app->run(window);\n";
    gtkmm += "}\n";
    return gtkmm;
}

inline std::string getSFML()
{
    
        std::string sfml = "#include <SFML/Graphics.hpp>\n";
        sfml += "const int screenWidth = 800;\n";
        sfml += "const int screenHeight = 450;\n";
           sfml +="\n";
        sfml += "int main(void)\n";
        sfml += "{\n";
        sfml += "    sf::RenderWindow window(sf::VideoMode(screenWidth, screenHeight), \"SFML\");\n";
        sfml += "    while (window.isOpen())\n";
        sfml += "    {\n";
        sfml += "        sf::Event event;\n";
        sfml += "        while (window.pollEvent(event))\n";
        sfml += "        {\n";
        sfml += "            if (event.type == sf::Event::Closed)\n";
        sfml += "                window.close();\n";
        sfml += "        }\n";
        sfml += "        window.clear(sf::Color::White);\n";
        sfml += "        window.display();\n";
        sfml += "    }\n";
        sfml += "    return 0;\n";
        sfml += "}\n";
        return sfml;
}

inline std::string getOGRE3D()
{

    std::string ogre3d = "#include <Ogre.h>\n";
    ogre3d += "const int screenWidth = 800;\n";
    ogre3d += "const int screenHeight = 450;\n";
    ogre3d +="\n";
    ogre3d += "int main()\n";
    ogre3d += "{\n";
    ogre3d += "    Ogre::Root* root = new Ogre::Root();\n";
    ogre3d += "    root->showConfigDialog();\n";
    ogre3d += "    root->initialise(true, \"Ogre3D\");\n";
    ogre3d += "    Ogre::RenderWindow* window = root->createRenderWindow(\"Ogre3D\", screenWidth, screenHeight, false);\n";
    ogre3d += "    Ogre::SceneManager* sceneManager = root->createSceneManager(Ogre::ST_GENERIC);\n";
    ogre3d += "    Ogre::Camera* camera = sceneManager->createCamera(\"Camera\");\n";
    ogre3d += "    camera->setPosition(Ogre::Vector3(0, 0, 80));\n";
    ogre3d += "    camera->lookAt(Ogre::Vector3(0, 0, -300));\n";
    ogre3d += "    camera->setNearClipDistance(5);\n";
    ogre3d += "    Ogre::Viewport* viewport = window->addViewport(camera);\n";
    ogre3d += "    viewport->setBackgroundColour(Ogre::ColourValue(1.0f, 1.0f, 1.0f, 1.0f));\n";
    ogre3d += "    Ogre::Entity* ogreHead = sceneManager->createEntity(\"Head\", \"ogrehead.mesh\");\n";
    ogre3d += "    Ogre::SceneNode* headNode = sceneManager->getRootSceneNode()->createChildSceneNode();\n";
    ogre3d += "    headNode->attachObject(ogreHead);\n";
    ogre3d += "    Ogre::Light* light = sceneManager->createLight(\"Light\");\n";
    ogre3d += "    light->setPosition(20.0f, 80.0f, 50.0f);\n";
    ogre3d += "    root->startRendering();\n";
    ogre3d += "    root->shutdown();\n";
    ogre3d += "    delete root;\n";
    ogre3d += "    return 0;\n";
    ogre3d += "}\n";
    return ogre3d;
    
}

inline std::string getGLFW()
{

    std::string glfw = "#include <GLFW/glfw3.h>\n";
    glfw += "const int screenWidth = 800;\n";
    glfw += "const int screenHeight = 450;\n";
        glfw +="\n";
    glfw += "int main(int argc, char* argv[])\n";
    glfw += "{\n";
    glfw += "    glfwInit();\n";
    glfw += "    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, \"GLFW\", NULL, NULL);\n";
    glfw += "    glfwMakeContextCurrent(window);\n";
    glfw += "    while (!glfwWindowShouldClose(window))\n";
    glfw += "    {\n";
    glfw += "        glfwSwapBuffers(window);\n";
    glfw += "        glfwPollEvents();\n";
    glfw += "    }\n";
    glfw += "    glfwTerminate();\n";
    glfw += "    return 0;\n";
    glfw += "}\n";
    return glfw;
}

inline std::string getQT5()
{

    std::string qt5 = "#include <QApplication>\n";
    qt5 += "#include <QMainWindow>\n";
    qt5 += "const int screenWidth = 800;\n";
    qt5 += "const int screenHeight = 450;\n";
    qt5 +="\n";
    qt5 += "int main(int argc, char* argv[])\n";
    qt5 += "{\n";
    qt5 += "    QApplication app(argc, argv);\n";
    qt5 += "    QMainWindow window;\n";
    qt5 += "    window.resize(screenWidth, screenHeight);\n";
    qt5 += "    window.show();\n";
    qt5 += "    return app.exec();\n";
    qt5 += "}\n";
    return qt5;
}