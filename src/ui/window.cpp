#include <iostream>
#include <GLFW/glfw3.h>
#include <GL/gl.h>

GLFWwindow* create_window(int width , int height , const char *title){

    if(!glfwInit()){
        std::cerr << "Intialized Failed" << std::endl;
    }

    GLFWwindow* main_window = glfwCreateWindow(width , height , title , NULL , NULL);

    glfwMakeContextCurrent(main_window);

    

}