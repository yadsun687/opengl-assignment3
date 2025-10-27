#pragma once

#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include <Animation/bone.h>
#include <model.h>

struct AssimpNodeData {
  glm::mat4 transformation;
  std::string name;
  int childrenCount;
  std::vector<AssimpNodeData> children;
};

class Animation {
public:
  Animation() = default;

  Animation(string const &animationPath, Model *model, int index = 0) {
    Assimp::Importer importer;
    const aiScene *scene =
        importer.ReadFile(animationPath, aiProcess_Triangulate);
    assert(scene && scene->mRootNode);
    for (size_t i = 0; i < scene->mNumAnimations; i++) {
      std::cout << "[Animation Loaded]: "
                << scene->mAnimations[i]->mName.C_Str() << "\n";
    }

    // handle single animation for now
    auto animation = scene->mAnimations[index];
    m_Name = animation->mName.C_Str();

    m_Duration = animation->mDuration;
    m_TicksPerSecond = animation->mTicksPerSecond;

    aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
    globalTransformation = globalTransformation.Inverse();
    m_globalTransformation =
        Calculation::convertMatrixToGLMFormat(globalTransformation);
    ReadHierarchyData(m_RootNode, scene->mRootNode);
    ReadMissingBones(animation, *model);
  }

  ~Animation() {}

  Bone *FindBone(const std::string &name) {
    // auto iter =
    //     std::find_if(m_Bones.begin(), m_Bones.end(), [&](const Bone &Bone) {
    //       return Bone.GetBoneName() == name;
    //     });
    // if (iter == m_Bones.end())
    //   return nullptr;
    // else
    //   return &(*iter);
    auto iter = m_Bones.find(name);
    if (iter != m_Bones.end())
      return &(iter->second);
    return nullptr;
  }

  inline float GetTicksPerSecond() { return m_TicksPerSecond; }
  inline float GetDuration() { return m_Duration; }
  inline const std::string &GetName() { return m_Name; }
  inline const AssimpNodeData &GetRootNode() { return m_RootNode; }
  inline const glm::mat4 &GetGlobalInverseTransform() {
    return m_globalTransformation;
  }
  inline const std::unordered_map<std::string, BoneInfo> &GetBoneIDMap() {
    return m_BoneInfoMap;
  }

private:
  void ReadMissingBones(const aiAnimation *animation, Model &model) {
    int size = animation->mNumChannels;

    auto &boneInfoMap =
        model.GetBoneInfoMap(); // getting m_BoneInfoMap from Model class
    int &boneCount =
        model.GetBoneCount(); // getting the m_BoneCounter from Model class

    // reading channels(bones engaged in an animation and their keyframes)
    for (int i = 0; i < size; i++) {
      auto channel = animation->mChannels[i];
      std::string boneName = channel->mNodeName.data;

      if (boneInfoMap.find(boneName) == boneInfoMap.end()) {
        boneInfoMap[boneName].id = boneCount;
        boneCount++;
      }
      m_Bones.insert(
          {boneName, Bone(channel->mNodeName.data,
                          boneInfoMap[channel->mNodeName.data].id, channel)});
      // m_Bones.push_back(Bone(channel->mNodeName.data,
      // boneInfoMap[channel->mNodeName.data].id, channel));
    }

    m_BoneInfoMap = boneInfoMap;
  }

  void ReadHierarchyData(AssimpNodeData &dest, const aiNode *src) {
    assert(src);

    dest.name = src->mName.data;
    dest.transformation =
        Calculation::convertMatrixToGLMFormat(src->mTransformation);
    dest.childrenCount = src->mNumChildren;

    for (int i = 0; i < src->mNumChildren; i++) {
      AssimpNodeData newData;
      ReadHierarchyData(newData, src->mChildren[i]);
      dest.children.push_back(newData);
    }
  }
  float m_Duration;
  int m_TicksPerSecond;
  std::string m_Name;
  // std::vector<Bone> m_Bones;
  std::unordered_map<std::string, Bone> m_Bones;
  AssimpNodeData m_RootNode;
  std::unordered_map<std::string, BoneInfo> m_BoneInfoMap;
  glm::mat4 m_globalTransformation;
};
