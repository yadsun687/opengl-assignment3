#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <Animation/animation.h>
#include <Animation/bone.h>

class Animator {
public:
  Animator(Animation *animation) {
    m_CurrentTime = 0.0;
    m_CurrentAnimation = animation;

    m_FinalBoneMatrices.reserve(100);

    for (int i = 0; i < 100; i++)
      m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
  }

  Animation &getCurrentAnimation() { return *m_CurrentAnimation; }

  void UpdateAnimation(float dt, bool blendOnly = false) {
    m_DeltaTime = dt;
    if (!blendOnly) {
      if (m_CurrentAnimation) {
        m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * m_DeltaTime;
        m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());

        if (m_CurrentAnimation2) {
          m_CurrentTime2 +=
              m_CurrentAnimation2->GetTicksPerSecond() * m_DeltaTime;
          m_CurrentTime2 =
              fmod(m_CurrentTime2, m_CurrentAnimation2->GetDuration());
        }
      }

      CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(),
                             glm::mat4(1.0f));
    }
  }

  void PlayAnimation(Animation *pAnimation) {
    m_CurrentAnimation = pAnimation;
    m_CurrentTime = 0.0f;
  }

  void PlayAnimation(Animation *pAnimation, Animation *pAnimation2, float time1,
                     float time2, float blend) {
    m_CurrentAnimation = pAnimation;
    m_CurrentTime = time1;
    m_CurrentAnimation2 = pAnimation2;
    m_CurrentTime2 = time2;
    m_blendAmount = blend;
  }

  void CalculateBoneTransform(const AssimpNodeData *node,
                              glm::mat4 parentTransform) {
    std::string nodeName = node->name;
    glm::mat4 nodeTransform = node->transformation;

    Bone *Bone1 = m_CurrentAnimation->FindBone(nodeName);
    Bone *Bone2 = NULL;
    if (m_CurrentAnimation2) {
      Bone2 = m_CurrentAnimation2->FindBone(nodeName);
    }

    if (Bone1) {
      Bone1->Update(m_CurrentTime);
      nodeTransform = Bone1->GetLocalTransform();

      if (Bone2) {
        nodeTransform = UpdateBlend(Bone1, Bone2);
      }
    }

    glm::mat4 globalTransformation = parentTransform * nodeTransform;

    auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
    if (boneInfoMap.find(nodeName) != boneInfoMap.end()) {
      int index = boneInfoMap[nodeName].id;
      glm::mat4 offset = boneInfoMap[nodeName].offset;
      m_FinalBoneMatrices[index] = globalTransformation * offset;
    }

    for (int i = 0; i < node->childrenCount; i++)
      CalculateBoneTransform(&node->children[i], globalTransformation);
  }

  glm::mat4 UpdateBlend(Bone *Bone1, Bone *Bone2) {
    glm::vec3 bonePos1, bonePos2, finalPos;
    glm::vec3 boneScale1, boneScale2, finalScale;
    glm::quat boneRot1, boneRot2, finalRot;

    Bone1->InterpolatePosition(m_CurrentTime, bonePos1);
    Bone2->InterpolatePosition(m_CurrentTime2, bonePos2);
    Bone1->InterpolateRotation(m_CurrentTime, boneRot1);
    Bone2->InterpolateRotation(m_CurrentTime2, boneRot2);
    Bone1->InterpolateScaling(m_CurrentTime, boneScale1);
    Bone2->InterpolateScaling(m_CurrentTime2, boneScale2);

    finalPos = glm::mix(bonePos1, bonePos2, m_blendAmount);
    finalRot = glm::slerp(boneRot1, boneRot2, m_blendAmount);
    finalRot = glm::normalize(finalRot);
    finalScale = glm::mix(boneScale1, boneScale2, m_blendAmount);

    glm::mat4 translation = glm::translate(glm::mat4(1.0f), finalPos);
    glm::mat4 rotation = glm::toMat4(finalRot);
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), finalScale);

    glm::mat4 TRS = glm::mat4(1.0f);
    TRS = translation * rotation * scale;
    return TRS;
  }

  const std::vector<glm::mat4> &GetFinalBoneMatrices() {
    return m_FinalBoneMatrices;
  }

  inline float getCurrentTime1() { return m_CurrentTime; }
  inline float getCurrentTime2() { return m_CurrentTime2; }

private:
  std::vector<glm::mat4> m_FinalBoneMatrices;
  Animation *m_CurrentAnimation = nullptr;
  Animation *m_CurrentAnimation2 = nullptr;
  float m_CurrentTime;
  float m_CurrentTime2;
  float m_DeltaTime;
  float m_blendAmount;
};
