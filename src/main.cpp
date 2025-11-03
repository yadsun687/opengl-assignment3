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
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

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

// player action
bool isPunch = false;
bool isRoll = false;
bool isRunning = false;

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
  glm::vec3 lightPos(0.0f, 4.0f, 0.0f);
  Model cube(FileSystem::getPath("resources/models/cube.obj").c_str());

  GameManager *gameManager = GameManager::getInstance();
  // float blendFactor = 0.5f;
  // gameManager->getPlayer().m_animator.PlayAnimation(
  //     &gameManager->getPlayer().m_animations[0],
  //     &gameManager->getPlayer().m_animations[1], 0.0f, 0.0f, blendFactor);
  PlayerLockCamera playerLockCamera =
      PlayerLockCamera(gameManager->getPlayer());

  Model terrain(
      FileSystem::getPath("resources/models/misc/terrain_prototype.fbx")
          .c_str());

  Model projectile(
      FileSystem::getPath("resources/models/projectile/fireball.gltf").c_str());

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

  // GUI (imgui)
  // ---------
  ImGuiIO *io;
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  io = &ImGui::GetIO();
  io->ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 330");

  // render loop
  // -----------
  while (!glfwWindowShouldClose(window)) {
    // per-frame time logic
    // --------------------
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // gameManager->update(deltaTime);
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

    shader.use();
    shader.setVec3("playerLightPos", gameManager->getPlayer().m_position);

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

    if (true) {

      switch (gameManager->getPlayer().m_actionState) {
      case IDLE:
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
          gameManager->getPlayer().m_blendAmount = 0.0f;
          gameManager->getPlayer().m_animator.PlayAnimation(
              &gameManager->getPlayer().m_animations[0],
              &gameManager->getPlayer().m_animations[3],
              gameManager->getPlayer().m_animator.getCurrentTime1(), 0.0f,
              gameManager->getPlayer().m_blendAmount);
          gameManager->getPlayer().setActionState(IDLE_RUNNING);
        } else if (isPunch) {
          gameManager->getPlayer().m_blendAmount = 0.0f;
          gameManager->getPlayer().m_animator.PlayAnimation(
              &gameManager->getPlayer().m_animations[0],
              &gameManager->getPlayer().m_animations[1],
              gameManager->getPlayer().m_animator.getCurrentTime1(), 0.0f,
              gameManager->getPlayer().m_blendAmount);
          gameManager->getPlayer().setActionState(IDLE_PUNCH);
        }
        printf("idle \n");
        break;
      case IDLE_RUNNING:
        gameManager->getPlayer().m_blendAmount += ANIM_BLEND_RATE;
        gameManager->getPlayer().m_blendAmount =
            fmod(gameManager->getPlayer().m_blendAmount, 1.0f);
        gameManager->getPlayer().m_animator.PlayAnimation(
            &gameManager->getPlayer().m_animations[0],
            &gameManager->getPlayer().m_animations[3],
            gameManager->getPlayer().m_animator.getCurrentTime1(),
            gameManager->getPlayer().m_animator.getCurrentTime2(),
            gameManager->getPlayer().m_blendAmount);
        if (gameManager->getPlayer().m_blendAmount > 0.9f) {
          gameManager->getPlayer().m_blendAmount = 0.0f;
          float startTime =
              gameManager->getPlayer().m_animator.getCurrentTime2();
          gameManager->getPlayer().m_animator.PlayAnimation(
              &gameManager->getPlayer().m_animations[3], NULL, startTime, 0.0f,
              gameManager->getPlayer().m_blendAmount);
          gameManager->getPlayer().setActionState(RUNNING);
        }
        printf("idle_walk \n");
        break;
      case RUNNING:
        gameManager->getPlayer().m_animator.PlayAnimation(
            &gameManager->getPlayer().m_animations[3], NULL,
            gameManager->getPlayer().m_animator.getCurrentTime1(),
            gameManager->getPlayer().m_animator.getCurrentTime2(),
            gameManager->getPlayer().m_blendAmount);
        if (isRoll) {
          float currentTime =
              gameManager->getPlayer().m_animator.getCurrentTime1();
          gameManager->getPlayer().m_blendAmount = 0.0f;
          gameManager->getPlayer().m_animator.PlayAnimation(
              &gameManager->getPlayer().m_animations[3],
              &gameManager->getPlayer().m_animations[2], currentTime, 0.0f,
              gameManager->getPlayer().m_blendAmount);
          gameManager->getPlayer().setActionState(RUN_ROLLING);
          break;
        }
        if (glfwGetKey(window, GLFW_KEY_W) != GLFW_PRESS &&
            glfwGetKey(window, GLFW_KEY_A) != GLFW_PRESS &&
            glfwGetKey(window, GLFW_KEY_S) != GLFW_PRESS &&
            glfwGetKey(window, GLFW_KEY_D) != GLFW_PRESS) {
          gameManager->getPlayer().setActionState(RUNNING_IDLE);
          ;
        }
        printf("running\n");
        break;
      case RUNNING_IDLE:
        gameManager->getPlayer().m_blendAmount += 0.01;
        gameManager->getPlayer().m_blendAmount =
            fmod(gameManager->getPlayer().m_blendAmount, 1.0f);
        gameManager->getPlayer().m_animator.PlayAnimation(
            &gameManager->getPlayer().m_animations[3],
            &gameManager->getPlayer().m_animations[0],
            gameManager->getPlayer().m_animator.getCurrentTime1(),
            gameManager->getPlayer().m_animator.getCurrentTime2(),
            gameManager->getPlayer().m_blendAmount);
        if (gameManager->getPlayer().m_blendAmount > 0.9f) {
          gameManager->getPlayer().m_blendAmount = 0.0f;
          float startTime =
              gameManager->getPlayer().m_animator.getCurrentTime2();
          gameManager->getPlayer().m_animator.PlayAnimation(
              &gameManager->getPlayer().m_animations[0], NULL, startTime, 0.0f,
              gameManager->getPlayer().m_blendAmount);
          gameManager->getPlayer().setActionState(IDLE);
          gameManager->getPlayer().m_blendAmount = 0.0f;
        }
        printf("walk_idle \n");
        break;
      case RUN_ROLLING: {
        gameManager->getPlayer().m_blendAmount +=
            0.005f; // adjust speed as needed
        gameManager->getPlayer().m_blendAmount =
            fmod(gameManager->getPlayer().m_blendAmount, 1.0f);

        gameManager->getPlayer().m_animator.PlayAnimation(
            &gameManager->getPlayer().m_animations[3],
            &gameManager->getPlayer().m_animations[2],
            gameManager->getPlayer().m_animator.getCurrentTime1(),
            gameManager->getPlayer().m_animator.getCurrentTime2(),
            gameManager->getPlayer().m_blendAmount);

        // transition into full roll quickly
        if (gameManager->getPlayer().m_blendAmount > 0.1f) {
          float startTime =
              gameManager->getPlayer().m_animator.getCurrentTime2();
          gameManager->getPlayer().m_animator.PlayAnimation(
              &gameManager->getPlayer().m_animations[2], NULL, startTime, 0.0f,
              0.0f);
          gameManager->getPlayer().setActionState(ROLLING);
        }
        printf("running_roll\n");
        break;
      }
      case ROLLING:
        // transition to running after roll
        if (gameManager->getPlayer().m_animator.getCurrentTime1() >
            0.9f * gameManager->getPlayer()
                       .m_animator.getCurrentAnimation()
                       .GetDuration()) {
          float currentTime =
              gameManager->getPlayer().m_animator.getCurrentTime1();
          gameManager->getPlayer().m_blendAmount = 0.0f;
          gameManager->getPlayer().m_animator.PlayAnimation(
              &gameManager->getPlayer().m_animations[2],
              &gameManager->getPlayer().m_animations[3], currentTime, 0.0f,
              gameManager->getPlayer().m_blendAmount);
          gameManager->getPlayer().setActionState(ROLLING_RUN);
        }
        break;
      case ROLLING_RUN:
        gameManager->getPlayer().m_blendAmount += 0.005;
        gameManager->getPlayer().m_blendAmount =
            fmod(gameManager->getPlayer().m_blendAmount, 1.0f);
        gameManager->getPlayer().m_animator.PlayAnimation(
            &gameManager->getPlayer().m_animations[2],
            &gameManager->getPlayer().m_animations[3],
            gameManager->getPlayer().m_animator.getCurrentTime1(),
            gameManager->getPlayer().m_animator.getCurrentTime2(),
            gameManager->getPlayer().m_blendAmount);
        if (gameManager->getPlayer().m_blendAmount > 0.9f) {
          gameManager->getPlayer().m_blendAmount = 0.0f;
          float startTime =
              gameManager->getPlayer().m_animator.getCurrentTime2();
          gameManager->getPlayer().m_animator.PlayAnimation(
              &gameManager->getPlayer().m_animations[3], NULL, startTime, 0.0f,
              gameManager->getPlayer().m_blendAmount);
          gameManager->getPlayer().m_blendAmount = 0.0f;
          gameManager->getPlayer().setActionState(RUNNING);
          isRoll = false;
        }
        printf("rolling_run\n");
        break;

      case IDLE_PUNCH:
        //+blendAmount until it's 100% punch
        gameManager->getPlayer().m_blendAmount += 0.005f;
        gameManager->getPlayer().m_blendAmount =
            fmod(gameManager->getPlayer().m_blendAmount, 1.0f);
        gameManager->getPlayer().m_animator.PlayAnimation(
            &gameManager->getPlayer().m_animations[0],
            &gameManager->getPlayer().m_animations[1],
            gameManager->getPlayer().m_animator.getCurrentTime1(),
            gameManager->getPlayer().m_animator.getCurrentTime2(),
            gameManager->getPlayer().m_blendAmount);
        if (gameManager->getPlayer().m_blendAmount > 0.9f) {
          gameManager->getPlayer().m_blendAmount = 0.0f;
          gameManager->getPlayer().m_animator.PlayAnimation(
              &gameManager->getPlayer().m_animations[1], NULL,
              gameManager->getPlayer().m_animator.getCurrentTime2(), 0.0f,
              gameManager->getPlayer().m_blendAmount);
          gameManager->getPlayer().setActionState(PUNCH);
        }
        printf("idle_punch\n");
        break;
      case PUNCH:
        if (gameManager->getPlayer().m_animator.getCurrentTime1() >=
            0.9f * gameManager->getPlayer()
                       .m_animator.getCurrentAnimation()
                       .GetDuration()) {
          gameManager->getPlayer().setActionState(PUNCH_IDLE);
        }
        printf("punch\n");

        break;
      case PUNCH_IDLE:
        gameManager->getPlayer().m_blendAmount += 0.005;
        gameManager->getPlayer().m_blendAmount =
            fmod(gameManager->getPlayer().m_blendAmount, 1.0f);
        gameManager->getPlayer().m_animator.PlayAnimation(
            &gameManager->getPlayer().m_animations[1],
            &gameManager->getPlayer().m_animations[0],
            gameManager->getPlayer().m_animator.getCurrentTime1(),
            gameManager->getPlayer().m_animator.getCurrentTime2(),
            gameManager->getPlayer().m_blendAmount);
        if (gameManager->getPlayer().m_blendAmount > 0.9f) {
          gameManager->getPlayer().m_blendAmount = 0.0f;
          float startTime =
              gameManager->getPlayer().m_animator.getCurrentTime2();
          gameManager->getPlayer().m_animator.PlayAnimation(
              &gameManager->getPlayer().m_animations[0], NULL, startTime, 0.0f,
              gameManager->getPlayer().m_blendAmount);
          gameManager->getPlayer().m_blendAmount = 0.0f;

          gameManager->getPlayer().setActionState(IDLE);
          isPunch = false;
        }
        printf("punch_idle\n");

        break;
      }
    }
    // gameManager->getPlayer().m_animator.PlayAnimation(
    //     &gameManager->getPlayer().m_animations[0],
    //     &gameManager->getPlayer().m_animations[1],
    //     gameManager->getPlayer().m_animator.getCurrentTime1(),
    //     gameManager->getPlayer().m_animator.getCurrentTime2(), blendFactor);
    gameManager->getPlayer().m_animator.UpdateAnimation(deltaTime);

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

    auto &transforms =
        gameManager->getPlayer().m_animator.GetFinalBoneMatrices();
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

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    //===========================================
    ImGui::Begin("Parameters Panel");

    ImGui::Text("Light Position");
    ImGui::SliderFloat("Light X", &lightPos.x, -10.0f, 10.0f);
    ImGui::SliderFloat("Light Y", &lightPos.y, -10.0f, 10.0f);
    ImGui::SliderFloat("Light Z", &lightPos.z, -10.0f, 10.0f);
    // ImGui::SliderFloat("Density", &(solver->DENSITY_0), 1.0f, 1000.0f);
    // ImGui::SliderFloat("Viscosity (Mu)", &(solver->MU), 0.0f, 10.0f);
    // ImGui::Checkbox("Using predicted position",
    // &(solver->USE_PREDICTED));

    // ImGui::Text("Environment");
    // ImGui::SliderFloat("Gravity", &(solver->GRAVITY), 0.0f, 100.0f);
    // ImGui::SliderFloat("Bounding box dampening", &(solver->RESTITUTION),
    // 0.0f,
    //                    1.0f);

    // ImGui::Text("Spawning");
    // ImGui::SliderInt("No. of Particles", &(solver->N_PARTICLES), 1,
    // 100000); ImGui::SliderFloat("Spawning gap", &(solver->SPAWN_GAP),
    // 0.0f, 10.0f);

    // ImGui::SliderFloat3("Spawning position",
    // glm::value_ptr(solver->SPAWN_POS),
    //                     *glm::value_ptr(glm::vec3(-50.0f, -50.0f,
    //                     -50.0f)),
    //                     *glm::value_ptr(glm::vec3(50.0f, 50.0f, 50.0f)));

    // ImGui::SliderFloat3("Box size min", glm::value_ptr(solver->BOX_MIN),
    //                     *glm::value_ptr(glm::vec3(-50.0f, -50.0f,
    //                     -50.0f)), *glm::value_ptr(glm::vec3(0.0f, 0.0f,
    //                     0.0f)));
    // ImGui::SliderFloat3("Box size max", glm::value_ptr(solver->BOX_MAX),
    //                     *glm::value_ptr(glm::vec3(0.0f, 0.0f, 0.0f)),
    //                     *glm::value_ptr(glm::vec3(50.0f, 50.0f, 50.0f)));

    ImGui::End();
    //===========================================

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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
  glm::vec3 dir =
      glm::normalize(glm::vec3(camera->Front.x, 0.0f, camera->Front.z));
  glm::vec right_dir =
      glm::normalize(glm::cross(dir, glm::vec3(0.0f, 1.0f, 0.0f)));

  glm::vec3 move_dir(0.0f);
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    move_dir += dir;
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    move_dir -= dir;
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    move_dir -= right_dir;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    move_dir += right_dir;

  // WASD pressed
  if (glm::length(move_dir) > 0.0f) {
    isRunning = true;
    move_dir = glm::normalize(move_dir);
    player->moveToward(move_dir, deltaTime, isRoll ? 3.0f : MOVEMENT_SPEED);
    player->m_targetModelAngle = glm::atan(move_dir.x, move_dir.z);
    player->facingDirection = glm::vec3(move_dir.x, 0.0f, move_dir.z);
  } else if (isRoll) {
    isRunning = true;
    player->moveToward(player->facingDirection, deltaTime,
                       isRoll ? 3.0f : MOVEMENT_SPEED);
  } else {
    isRunning = false;
  }
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

  // can only roll while running
  if (key == GLFW_KEY_C && action == GLFW_PRESS && !isRoll && isRunning) {
    isRoll = true;
  } else if (key == GLFW_KEY_F && action == GLFW_PRESS && !isRunning) {
    isPunch = true;
  }
};

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
