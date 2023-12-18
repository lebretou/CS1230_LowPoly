#include "sceneparser.h"
#include "scenefilereader.h"
#include <glm/gtx/transform.hpp>

#include <chrono>
#include <iostream>



void traverseSceneGraph(const SceneNode* node, const glm::mat4& parentTransform, RenderData& renderData) {
    // return if self is null
    if (node == nullptr) return;

    glm::mat4 ctm = parentTransform;

    // Apply transformations in the order: TRS
//    for (const auto& transformation : node->transformations) {
//        if (transformation->type == TransformationType::TRANSFORMATION_SCALE) {
//            ctm = glm::scale(ctm, transformation->scale);
//        }

//        if (transformation->type == TransformationType::TRANSFORMATION_ROTATE) {
//            ctm = glm::rotate(ctm, transformation->angle, transformation->rotate);
//        }

//        if (transformation->type == TransformationType::TRANSFORMATION_TRANSLATE) {
//            ctm = glm::translate(ctm, transformation->translate);
//        }
//    }


    for (const auto& transformation : node->transformations) {
        if (transformation->type == TransformationType::TRANSFORMATION_TRANSLATE) {
            ctm = glm::translate(ctm, transformation->translate);
        }
    }
    for (const auto& transformation : node->transformations) {
        if (transformation->type == TransformationType::TRANSFORMATION_ROTATE) {
            ctm = glm::rotate(ctm, transformation->angle, transformation->rotate);
        }
    }
    for (const auto& transformation : node->transformations) {
        if (transformation->type == TransformationType::TRANSFORMATION_SCALE) {
            ctm = glm::scale(ctm, transformation->scale);
        }
    }

    // assign primitives and ctm to shapeData
    for (const auto& primitive : node->primitives) {
        RenderShapeData shapeData;
        shapeData.primitive = *primitive;
        shapeData.primitive.material.textureMap.isUsed = false;
        shapeData.ctm = ctm;
        shapeData.inverse_ctm = glm::inverse(ctm);
        shapeData.inverse_transpose_ctm3 = glm::inverse(glm::transpose(glm::mat3(ctm)));
        renderData.shapes.push_back(shapeData);
    }

    // process lights
    for (const auto& light : node->lights) {
        SceneLightData lightData;

        // assign the member variabels of SceneLightData
        lightData.id = light->id;
        lightData.type = light->type;
        lightData.color = light->color;
        lightData.function = light->function;

        // Handle different types of lights
        switch (light->type) {
        case LightType::LIGHT_POINT:
            // assume that the light's initial position is at the origin
            lightData.pos = ctm * glm::vec4(0.0, 0.0, 0.0, 1.0);
            break;
        case LightType::LIGHT_DIRECTIONAL:
            lightData.pos = ctm * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            lightData.dir = glm::normalize(ctm * light->dir);
            break;
        case LightType::LIGHT_SPOT:
            lightData.penumbra = light->penumbra;
            lightData.angle = light->angle;
            lightData.pos = ctm * glm::vec4(0.0, 0.0, 0.0, 1.0);
            lightData.dir = glm::normalize(ctm * light->dir);
            break;
        // for default we just assume directional light for this project
        default:
            break;
        }
        renderData.lights.push_back(lightData);
    }

    // Recur for each child of the current node.
    for (const auto& child : node->children) {
        traverseSceneGraph(child, ctm, renderData);
    }
}

bool SceneParser::parse(std::string filepath, RenderData &renderData) {
    ScenefileReader fileReader = ScenefileReader(filepath);
    bool success = fileReader.readJSON();
    if (!success) {
        return false;
    }

    // TODO: Use your Lab 5 code here
    renderData.globalData = fileReader.getGlobalData();
    renderData.cameraData = fileReader.getCameraData();

    // Task 6: populate renderData's list of primitives and their transforms.
    //         This will involve traversing the scene graph, and we recommend you
    //         create a helper function to do so!
    renderData.shapes.clear();
    renderData.lights.clear();

    glm::mat4 identity = glm::mat4(1.0f); // Identity matrix
    traverseSceneGraph(fileReader.getRootNode(), identity, renderData);

    return true;
}
