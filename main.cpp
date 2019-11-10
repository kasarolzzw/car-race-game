#include <glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "shader.h"
#include "camera.h"
#include "model.h"
#include <iostream>

#define CAR_X 15.0f
#define CAR_Y -3.1f
#define CAR_Z 0.0f

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void renderScene(const Shader &shader, Model model, Model model2);
unsigned int loadTexture(const char *path);
unsigned int loadCubemap(vector<std::string> faces);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 15.0f, 45.0f),glm::vec3(0.0f, 1.0f, 0.0f),-90.0f,0.0f);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

float currentFrame=0.0f;
float change_time = 0.0f;

//direction
float theta=90.0f;
float direction_x = 0.0f;
float direction_z = 1.0f;
float car_x = CAR_X;
float car_y = CAR_Y;
float car_z = CAR_Z;
float acceleration=1.0f;
float acc_time=0.0f;
float brake=1.0f;
float car_shift = 1.0f;
float car_moving = 0.0f;

//view
int view_type = 0; //type of view
bool view_set = false; //whether have or have not set the view
bool line = false; //line mode


int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Asphalt", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader shader2("shadow_ferrari.vs", "shadow_ferrari.fs");
    Shader skyboxShader("skybox.vs", "skybox.fs");

    // shader configuration
    // --------------------
    shader2.use();
    shader2.setInt("diffuseTexture", 0);
    shader2.setInt("shadowMap", 1);
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    // lighting info
    // -------------
    glm::vec3 lightPos(50.0f, 6.0f, 0.0f);

    // load models
    // -----------
    Model model1("track/Race_Track.obj");
    Model model2("ferrari/Ferrari LaFerrari.obj");

    float skyboxVertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };

    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // load textures
    // -------------
    vector<std::string> faces
            {
                    "darkcity/ft.tga",
                    "darkcity/bk.tga",
                    "darkcity/up.tga",
                    "darkcity/dn.tga",
                    "darkcity/rt.tga",
                    "darkcity/lf.tga"
            };
    unsigned int cubemapTexture = loadCubemap(faces);

    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
//        cout<<deltaTime<<endl;
        // acceleration time check
        if(acc_time!=0.0f&&currentFrame-acc_time>3.0f){
            acceleration=1.0f;
            acc_time=0.0f;
        }

        // input
        // -----
        processInput(window);

        // draw in wireframe
        if(!line)
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        else if(line)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        // render
        // ------
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // camera view type
        if(view_type == 0 && !view_set){ // free observation
            // tell GLFW to capture our mouse
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            camera.original_view();
            view_set = true;
        }
        else if(view_type == 1) { // behind the car
            camera.car_view(car_x, car_y, car_z, theta, 1);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else if(view_type == 2) { // in the car
            camera.car_view(car_x, car_y, car_z, theta, 2);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader2.use();
        glm::mat4 M_projection2 = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 M_view2 = camera.GetViewMatrix();
        shader2.setMat4("projection", M_projection2);
        shader2.setMat4("view", M_view2);
        // set light uniforms
        shader2.setVec3("viewPos", camera.Position);
        shader2.setVec3("lightPos", lightPos);
//        shader2.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        renderScene(shader2,model1,model2);


        // draw skybox as last
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        M_view2 = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", M_view2);
        skyboxShader.setMat4("projection", M_projection2);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();

    }
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVAO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    // drawing mode
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        line = true;
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
        line = false;

    //  shift
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS &&
        (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS))
        car_shift = 0.7f;
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
        car_shift = 1.0f;

    // Turning right
    if ((glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS &&
        (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)) ||
        (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS &&
        glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)){
        theta+=2.0f*(2.7f-2*car_shift+acceleration*1.0f+0.3f*(1-brake))*brake*(20.0*deltaTime);
        //delta time is used to calibrate fps
        direction_x=cos(glm::radians(theta));
        direction_z=sin(glm::radians(theta));
    }
    // Turning left
    if ((glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS &&
        (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)) ||(
        glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS &&
        glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)){
        theta-=2.0f*(2.7f-2*car_shift+acceleration*1.0f+0.3f*(1-brake))*brake*(20.0*deltaTime);
        direction_x=cos(glm::radians(theta));
        direction_z=sin(glm::radians(theta));
    }
    // Moving forward
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
        car_x+=0.1f*direction_x*acceleration*brake*car_shift*15.0f*(20.0*deltaTime);
        car_z+=0.1f*direction_z*acceleration*brake*car_shift*15.0f*(20.0*deltaTime);
    }
    // Moving backwoard
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
        car_x-=0.1f*direction_x*acceleration*brake*15.0f*(20.0*deltaTime);
        car_z-=0.1f*direction_z*acceleration*brake*15.0f*(20.0*deltaTime);
    }
    // Acceleration
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS){
        acceleration = 2.0f;
        acc_time=currentFrame;
    }
    //brake
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS){
        brake = 0.6f;
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE){
        brake = 1.0f;
    }

    // restart
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS){
        car_x = CAR_X;
        car_y = CAR_Y;
        car_z = CAR_Z;
        acceleration=1.0f;
        acc_time=0.0f;
        brake=1.0f;
        car_shift = 1.0f;
        theta = 90.0f;
        direction_x = 0.0f;
        direction_z = 1.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS){
        if(currentFrame-change_time>=0.2f||change_time==0.0f){
            if(view_type == 0){
                view_type = 1; //behind the car
                view_set = true;
            }
            else if(view_type == 1){
                view_type = 2; //in the car
                view_set = true;
            }
            else if(view_type ==2){
                view_type = 0; //free observation
                view_set = false;
            }
            change_time = currentFrame;
        }

    }

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions
    if(width*3>height*4)  //width is larger
        glViewport(0, 0, 4*height/3, height);
    else
        glViewport(0, 0, width, 0.75*width);

}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}


unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (front)
// -X (back)
// +Y (top)
// -Y (bottom)
// +Z (right)
// -Z (left)
// -------------------------------------------------------
unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void renderScene(const Shader &shader, Model model1, Model model2) {
    glm::mat4 M_model1 = glm::mat4(1.0f);
    M_model1 = glm::translate(M_model1, glm::vec3(0.0f, -5.5f, -10.0f)); // translate it down so it's at the center of the scene
    M_model1 = glm::rotate(M_model1, glm::radians(90.0f), glm::vec3(-1.0f, 0, 0));
    M_model1 = glm::scale(M_model1, glm::vec3(1.0f, 2.0f,0.6f));	// it's a bit too big for our scene, so scale it down
    shader.setMat4("model", M_model1);
    model1.Draw(shader);

    glm::mat4 M_model = glm::mat4(1.0f);
    M_model = glm::translate(M_model, glm::vec3(car_x, car_y, car_z)); //Put the car in the right palce
//        M_model2 = glm::translate(M_model2, glm::vec3(10.0f, -3.1f, 0.0f)); // translate it down so it's at the center of the scene
    M_model = glm::rotate(M_model, glm::radians(90.0f-theta),glm::vec3(0.0f, 1.0f, 0.0f));
    M_model = glm::rotate(M_model, glm::radians(90.0f), glm::vec3(-1.0f, 0, 0));
    M_model = glm::scale(M_model, glm::vec3(0.6f));	// it's a bit too big for our scene, so scale it down
    shader.setMat4("model", M_model);
    model2.Draw(shader);

}
