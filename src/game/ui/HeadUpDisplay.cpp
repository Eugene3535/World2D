#include "cglm/struct/affine-mat.h"
#include "cglm/struct/ivec2.h"

#include "game/ui/HeadUpDisplay.hpp"
#include "game/window/WindowData.hpp"
#include "graphics/camera/View.hpp"
#include "graphics/tilemap/TileMap.hpp"


#define SCREEN_MARGIN   150
#define CAMERA_VELOCITY 600


HeadUpDisplay::HeadUpDisplay(const WindowData& data) noexcept :
    m_windowData(&data)
{
}


HeadUpDisplay::~HeadUpDisplay() = default;


void HeadUpDisplay::update(TileMap& tilemap, float dt) noexcept
{
    const auto& projection = m_windowData->view->getProjectionMatrix();

    const ivec2s viewSize     = m_windowData->view->getSize();
    const ivec2s locationSize = m_windowData->locationSize;

    if ((locationSize.x < viewSize.x) && (locationSize.y < viewSize.y))
    {
        const ivec2s delta            = glms_ivec2_sub(viewSize, locationSize);
        const ivec2s locationPosition = {delta.x >> 1, delta.y >> 1};
        tilemap.setPosition(static_cast<float>(-locationPosition.x), static_cast<float>(-locationPosition.y));

        const auto modelView = tilemap.getMatrix();
        const auto mvp       = glms_mul(projection, modelView);
        m_windowData->view->updateUniformBuffer(mvp.raw);

        return;
    }

    const vec2s cursorPos           = m_windowData->cursor;
    const bool  isNearTheLeftEdge   = (cursorPos.x > 0 && cursorPos.x < SCREEN_MARGIN);
    const bool  isNearTheTopEdge    = (cursorPos.y > 0 && cursorPos.y < SCREEN_MARGIN);
    const bool  isNearTheRightEdge  = (cursorPos.x > (viewSize.x - SCREEN_MARGIN) && cursorPos.x < viewSize.x);
    const bool  isNearTheBottomEdge = (cursorPos.y > (viewSize.y - SCREEN_MARGIN) && cursorPos.y < viewSize.y);

    const float velocity      = dt * CAMERA_VELOCITY;
    vec2s       scenePosition = tilemap.getPosition();

    if (isNearTheLeftEdge)
        scenePosition.x += velocity;

    if (isNearTheTopEdge)
        scenePosition.y += velocity;

    if (isNearTheRightEdge)
        scenePosition.x -= velocity;

    if (isNearTheBottomEdge)
        scenePosition.y -= velocity;

    if (scenePosition.x > 0)
        scenePosition.x = 0;
    if (scenePosition.y > 0)
        scenePosition.y = 0;
    if (scenePosition.x < (viewSize.x - locationSize.x))
        scenePosition.x = viewSize.x - locationSize.x;
    if (scenePosition.y < (viewSize.y - locationSize.y))
        scenePosition.y = viewSize.y - locationSize.y;

    tilemap.setPosition(scenePosition);

    const auto modelView = tilemap.getMatrix();
    const auto mvp       = glms_mul(projection, modelView);
    m_windowData->view->updateUniformBuffer(mvp.raw);
}