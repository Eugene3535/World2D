#include "game/animation/Animation.hpp"


Animator::Animator() noexcept:
    m_current(nullptr),
    m_timer(0.f),
    m_isPlaying(false),
    m_isReversed(false)
{

}


Animator::~Animator() = default;


void Animator::add(const std::string& name, std::span<const Sprite2D> frames, float rate, bool isLooped) noexcept
{
    m_animations[name] = { frames, rate, isLooped };
}


void Animator::set(const std::string& name) noexcept
{
    if (auto it = m_animations.find(name); it != m_animations.end())
    {
        m_current = &it->second;
    }
}


void Animator::update(float dt) noexcept
{
    if (!m_isPlaying)
        return;

    if (m_current)
    {
        m_timer += m_current->rate * dt;
        const uint32_t frame = static_cast<uint32_t>(m_timer);

        if (frame == m_current->frames.size())
        {
            m_timer = 0;

            if (!m_current->isLooped)
            {
                m_isPlaying = false;

                return;
            }
        }
    }
}


void Animator::stop() noexcept
{
    m_isPlaying = false;
    m_timer = 0;
}


void Animator::pause() noexcept
{
    m_isPlaying = false; 
}


void Animator::play() noexcept
{
    m_isPlaying = true; 
}


void Animator::reverse(bool b) noexcept
{
    m_isReversed = b; 
}


bool Animator::isPlaying() const noexcept
{
    return m_isPlaying;
}


bool Animator::isReversed() const noexcept
{
    return m_isReversed;
}


const Sprite2D* Animator::getCurrentSprite() const noexcept
{
    return m_current ? m_current->frames.data() + static_cast<ptrdiff_t>(m_timer) : nullptr;
}