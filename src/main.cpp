#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <stb_image.h>
#include <cmath>

#include <Animation/animator.h>
#include <Animation/animation.h>
#include <Game/Player.h>
#include <Game/GameManager.h>
#include <PickingTexture.h>
#include <Utils/Debug.h>
#include <Utils/Calculation.h>
#include <camera.h>
#include <filesystem.h>
#include <model.h>
#include <playerlockcamera.h>
#include <shader.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void mouse_click_callback(GLFWwindow *window, int button, int action, int Mode);
void key_click_callback(GLFWwindow *window, int key, int scancode, int action,
                        int mods);
void processInput(GLFWwindow *window, Player *player, PlayerLockCamera *camera);
unsigned int loadTexture(const char *path);
void pickingPhase(PickingTexture pickingTexture, Shader shader,
                  PlayerLockCamera *camera, Model *terrain);
int glfwInit(GLFWwindow *&window);
unsigned int initializeCubeMapTexture(std::vector<std::string> &textures_faces);

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// camera
// Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
float xoffset = 0.0f;
float yoffset = 0.0f;
bool firstMouse = true;
bool isCameraLock = true;

// mouse point&click
bool isPressedLeft = false;
bool isPressedRight = false;
int mouseX, mouseY;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main() {

  GLFWwindow *window;
  if (!glfwInit(window)) {
    return -1;
  }

  // build and compile shaders
  // -------------------------
  Shader shader(FileSystem::getPath("/src/shader.vs").c_str(),
                FileSystem::getPath("/src/shader.fs").c_str());

  Shader pickingShader(FileSystem::getPath("/src/picking.vs").c_str(),
                       FileSystem::getPath("/src/picking.fs").c_str());

  Shader lightCubeShader(FileSystem::getPath("/src/lightcube.vs").c_str(),
                         FileSystem::getPath("/src/lightcube.fs").c_str());

  Shader skyboxShader(FileSystem::getPath("/src/skybox.vs").c_str(),
                      FileSystem::getPath("/src/skybox.fs").c_str());

  // lighting info
  // -------------
  glm::vec3 lightPos(0.5f, 10.0f, 0.3f);
  Model cube(FileSystem::getPath("resources/models/cube.obj").c_str());

  GameManager *gameManager = GameManager::getInstance();
  Animation playerAnimIdle(
      FileSystem::getPath("resources/models/player/animationonly_Witch.fbx")
          .c_str(),
      &gameManager->getPlayer().m_model, 4);
  Animation playerAnimRun(
      FileSystem::getPath("resources/models/player/animationonly_Witch.fbx")
          .c_str(),
      &gameManager->getPlayer().m_model, 16);
  Animator animator(&playerAnimIdle);

  PlayerLockCamera playerLockCamera =
      PlayerLockCamera(gameManager->getPlayer());

  // "Low Poly City" (https://skfb.ly/o8q8z) by Alessandro.Diamanti is licensed
  // under Creative Commons Attribution
  // (http://creativecommons.org/licenses/by/4.0/).
  Model terrain(
      FileSystem::getPath("resources/models/City/low_poly_city.fbx").c_str());

  Model projectile(
      FileSystem::getPath("resources/models/projectile/fireball.gltf").c_str());

  // "Ghost" (https://skfb.ly/6r9tx) by BConnolly is licensed under Creative
  // Commons Attribution-NonCommercial
  // (http://creativecommons.org/licenses/by-nc/4.0/).
  Model enemy(
      FileSystem::getPath("resources/models/enemy/final_ghost.fbx").c_str());

  PickingTexture pickingTexture;
  pickingTexture.init(SCR_WIDTH, SCR_HEIGHT);

  // [skybox]
  // ----------------
  float skyboxVertices[] = {
      // positions
      -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
      1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

      -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
      -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

      1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
      1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

      -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
      1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

      -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
      1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

      -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
      1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};
  vector<std::string> textures_faces{
      FileSystem::getPath("resources/textures/skybox/right.jpg"),
      FileSystem::getPath("resources/textures/skybox/left.jpg"),
      FileSystem::getPath("resources/textures/skybox/top.jpg"),
      FileSystem::getPath("resources/textures/skybox/bottom.jpg"),
      FileSystem::getPath("resources/textures/skybox/front.jpg"),
      FileSystem::getPath("resources/textures/skybox/back.jpg")};
  // skybox VAO
  unsigned int skyboxVAO, skyboxVBO;
  glGenVertexArrays(1, &skyboxVAO);
  glGenBuffers(1, &skyboxVBO);
  glBindVertexArray(skyboxVAO);
  glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

  // load cube map texture
  unsigned int cubeMapTextureID = initializeCubeMapTexture(textures_faces);

  shader.use();
  shader.setVec3("playerLightPos", gameManager->getPlayer().m_position);

  // render loop
  // -----------
  while (!glfwWindowShouldClose(window)) {
    // per-frame time logic
    // --------------------
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    gameManager->update(deltaTime);
    // input
    // -----
    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
      //[DEBUG]: free camera rotation when cursor hidden
      playerLockCamera.ProcessMouseMovement(xoffset, yoffset);
      xoffset = 0.0f;
      yoffset = 0.0f;
    }
    processInput(window, &gameManager->getPlayer(), &playerLockCamera);

    // Camera's  view/projection matrices
    glm::mat4 projection =
        glm::perspective(glm::radians(playerLockCamera.Zoom),
                         (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = playerLockCamera.GetViewMatrix();

    // Player shoot
    if (isPressedLeft) {
      pickingPhase(pickingTexture, pickingShader, &playerLockCamera, &terrain);
      glm::vec3 p = pickingTexture.getClickedPosition(
          mouseX, SCR_HEIGHT - mouseY - 1, view, projection);
      glm::vec3 playerPos = gameManager->getPlayer().m_position;
      glm::vec3 direction =
          glm::normalize(glm::vec3(p.x, 0.0f, p.z) -
                         glm::vec3(playerPos.x, 0.0f, playerPos.z));
      gameManager->generateProjectile(playerPos, direction);
      isPressedLeft = false;
    }
    // Player walk to cursor clicked position
    else if (isPressedRight) {
      pickingPhase(pickingTexture, pickingShader, &playerLockCamera, &terrain);
      glm::vec3 p = pickingTexture.getClickedPosition(
          mouseX, SCR_HEIGHT - mouseY - 1, view, projection);
      if (p.y < 0.2) {
        // only click on ground
        std::cout << "WALK TOWARD TO: ";
        Debug::logGLMVector(p);
        std::cout << "\n";
        gameManager->getPlayer().m_targetPosition = glm::vec3(p.x, 0.0f, p.z);
        glm::vec2 horizontalDirection =
            glm::vec2(gameManager->getPlayer().m_targetPosition.x -
                          gameManager->getPlayer().m_position.x,
                      gameManager->getPlayer().m_targetPosition.z -
                          gameManager->getPlayer().m_position.z);
        float angle = std::atan2(horizontalDirection.x, horizontalDirection.y);
        gameManager->getPlayer().m_targetModelAngle = angle;

        // switch animation to run
        if (animator.getCurrentAnimation().GetName() !=
            playerAnimRun.GetName()) {
          animator.PlayAnimation(&playerAnimRun);
        }
        isPressedRight = false;
      }
    }

    // continuously move toward target position
    float moveDistance =
        glm::distance(gameManager->getPlayer().m_position,
                      gameManager->getPlayer().m_targetPosition);
    if (moveDistance > 1e-6) {
      glm::vec3 movedPosition = Calculation::lerp(
          gameManager->getPlayer().m_position,
          gameManager->getPlayer().m_targetPosition,
          (gameManager->getPlayer().m_movement_speed * deltaTime) /
              moveDistance);
      gameManager->getPlayer().moveTo(
          glm::vec3(movedPosition.x, gameManager->getPlayer().m_position.y,
                    movedPosition.z));
      shader.use();
      shader.setVec3("playerLightPos", gameManager->getPlayer().m_position);
    } else {
      if (animator.getCurrentAnimation().GetName() !=
          playerAnimIdle.GetName()) {
        animator.PlayAnimation(&playerAnimIdle);
      }
    }
    // continuously move toward facing direction
    if (gameManager->getPlayer().m_modelAngle !=
        gameManager->getPlayer().m_targetModelAngle) {
      float angleDiff = gameManager->getPlayer().m_targetModelAngle -
                        gameManager->getPlayer().m_modelAngle;
      angleDiff = std::remainder(angleDiff, 2.0f * glm::pi<float>());
      float rotatedAngle = Calculation::lerp(
          gameManager->getPlayer().m_modelAngle,
          gameManager->getPlayer().m_modelAngle + angleDiff,
          (gameManager->getPlayer().m_rotationSpeed * deltaTime) /
              abs(angleDiff));
      gameManager->getPlayer().m_modelAngle = rotatedAngle;
    };

    // camera following player
    if (playerLockCamera.isTrackPlayer) {
      playerLockCamera.updateCameraPosition();
    }

    // render
    // ------
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
    shader.setVec3("viewPos", playerLockCamera.Position);
    shader.setVec3("lightPos", lightPos);

    // render terrain
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(1.0f));
    shader.setMat4("model", model);
    terrain.Draw(shader);

    // render player model
    animator.UpdateAnimation(deltaTime);
    auto &transforms = animator.GetFinalBoneMatrices();
    for (int i = 0; i < transforms.size(); ++i)
      shader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]",
                     transforms[i]);
    model = glm::mat4(1.0f);
    model = glm::translate(model, gameManager->getPlayer().m_position);
    model = glm::rotate(model, gameManager->getPlayer().m_modelAngle,
                        glm::vec3(0.0f, 1.0f, 0.0f));
    // model = glm::scale(model, glm::vec3(1.0f));
    shader.setMat4("model", model);
    shader.setBool("hasAnimation", true);
    gameManager->getPlayer().m_model.Draw(shader);
    shader.setBool("hasAnimation", false);

    // draw player's projectiles
    auto &projectiles = gameManager->getProjectiles();
    for (auto &obj : projectiles) {
      if (obj == nullptr)
        continue;
      model = glm::mat4(1.0f);
      model = glm::translate(model, obj->position);
      model = glm::scale(model, glm::vec3(0.6f));
      model = glm::rotate(model, (float)sin(glfwGetTime()),
                          glm::vec3(0.0f, 1.0f, 0.0f));
      shader.setMat4("model", model);
      projectile.Draw(shader);
    }

    // draw enemies
    auto &enemies = gameManager->getEnemies();
    for (auto &obj : enemies) {
      if (obj == nullptr)
        continue;
      model = glm::mat4(1.0f);
      model = glm::translate(model, obj->position);
      model = glm::scale(model, glm::vec3(0.01f));
      model = glm::rotate(model, obj->rotation, glm::vec3(0.0f, 1.0f, 0.0f));

      shader.setMat4("model", model);
      enemy.Draw(shader);
    }

    // render light source (simple cube)
    model = glm::mat4(1.0f);
    model = glm::translate(model, lightPos);
    model = glm::scale(model, glm::vec3(0.1f));
    lightCubeShader.use();
    lightCubeShader.setMat4("projection", projection);
    lightCubeShader.setMat4("view", view);
    lightCubeShader.setMat4("model", model);
    cube.Draw(lightCubeShader);

    // draw skybox as last
    glDepthFunc(GL_LEQUAL); // change depth function so depth test passes when
                            // values are equal to depth buffer's content
    skyboxShader.use();
    view = glm::mat4(glm::mat3(
        playerLockCamera
            .GetViewMatrix())); // remove translation from the view matrix
    skyboxShader.setMat4("view", view);
    skyboxShader.setMat4("projection", projection);
    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS); // set depth function back to default

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse
    // moved etc.)
    // -------------------------------------------------------------------------------
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released
// this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window, Player *player,
                  PlayerLockCamera *camera) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    player->moveToward(glm::vec3(0.0f, 0.0f, -1.0f), deltaTime);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    player->moveToward(glm::vec3(0.0f, 0.0f, 1.0f), deltaTime);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    player->moveToward(glm::vec3(-1.0f, 0.0f, 0.0f), deltaTime);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    player->moveToward(glm::vec3(1.0f, 0.0f, 0.0f), deltaTime);

  camera->isTrackPlayer =
      (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) || isCameraLock;

  if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS) {
    camera->m_cameraDistance -= 1.0f * deltaTime;
  }
  if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS) {
    camera->m_cameraDistance += 1.0f * deltaTime;
  }
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  // make sure the viewport matches the new window dimensions; note that width
  // and height will be significantly larger than specified on retina
  // displays.
  glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn) {
  float xpos = static_cast<float>(xposIn);
  float ypos = static_cast<float>(yposIn);
  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  xoffset = xpos - lastX;
  yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

  lastX = xpos;
  lastY = ypos;
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {}

void mouse_click_callback(GLFWwindow *window, int button, int action,
                          int Mode) {
  double x, y;
  glfwGetCursorPos(window, &x, &y);
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    isPressedLeft = true;
    mouseX = x;
    mouseY = y;
  }
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    isPressedRight = true;
    mouseX = x;
    mouseY = y;
  }
}
void key_click_callback(GLFWwindow *window, int key, int scancode, int action,
                        int mods) {
  if (key == GLFW_KEY_LEFT_ALT && action == GLFW_PRESS) {
    glfwSetInputMode(window, GLFW_CURSOR,
                     glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL
                         ? GLFW_CURSOR_DISABLED
                         : GLFW_CURSOR_NORMAL);
  }
  if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
    isCameraLock = !isCameraLock;
  }
};

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const *path) {
  unsigned int textureID;
  glGenTextures(1, &textureID);

  int width, height, nrComponents;
  unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
  if (data) {
    GLenum format;
    if (nrComponents == 1)
      format = GL_RED;
    else if (nrComponents == 3)
      format = GL_RGB;
    else if (nrComponents == 4)
      format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(
        GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
        format == GL_RGBA
            ? GL_CLAMP_TO_EDGE
            : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to
                          // prevent semi-transparent borders. Due to
                          // interpolation it takes texels from next repeat
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                    format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  } else {
    std::cout << "Texture failed to load at path: " << path << std::endl;
    stbi_image_free(data);
  }

  return textureID;
}

void pickingPhase(PickingTexture pickingTexture, Shader shader,
                  PlayerLockCamera *camera, Model *terrain) {

  pickingTexture.enableWriting();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  shader.use();

  // draw clickable terrain
  glm::mat4 model = glm::mat4(1.0f);
  model = glm::scale(model, glm::vec3(1.0f));
  glm::mat4 view = camera->GetViewMatrix();
  glm::mat4 projection =
      glm::perspective(glm::radians(camera->Zoom),
                       (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
  shader.setMat4("model", model);
  shader.setMat4("view", view);
  shader.setMat4("projection", projection);
  terrain->Draw(shader);

  pickingTexture.disableWriting();
}

int glfwInit(GLFWwindow *&window) {
  // glfw: initialize and configure
  // ------------------------------
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // glfw window creation
  // --------------------
  window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return false;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetMouseButtonCallback(window, mouse_click_callback);
  glfwSetKeyCallback(window, key_click_callback);

  // glad: load all OpenGL function pointers
  // ---------------------------------------
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return false;
  }

  // configure global opengl state
  // -----------------------------
  glEnable(GL_DEPTH_TEST);

  return true;
}

unsigned int
initializeCubeMapTexture(std::vector<std::string> &textures_faces) {
  unsigned int id;
  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_CUBE_MAP, id);

  int width, height, nrComponents;
  for (size_t i = 0; i < textures_faces.size(); i++) {
    unsigned char *textureFace =
        SOIL_load_image(textures_faces[i].c_str(), &width, &height,
                        &nrComponents, SOIL_LOAD_AUTO);
    if (textureFace) {
      GLenum format;
      if (nrComponents == 1)
        format = GL_RED;
      else if (nrComponents == 3)
        format = GL_RGB;
      else if (nrComponents == 4)
        format = GL_RGBA;
      else
        format = GL_RGBA;

      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height,
                   0, format, GL_UNSIGNED_BYTE, textureFace);
    }
    SOIL_free_image_data(textureFace);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  return id;
}
