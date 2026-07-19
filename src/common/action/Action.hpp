#pragma once

#include <SFML/System/Time.hpp>

#include "common/Defines.hpp"

BEGIN_NAMESPACE_ACTION

size_t construct(void* actionData, sf::Time deltaTime) noexcept;

END_NAMESPACE