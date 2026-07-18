#include "graphics/animation/Animation.hpp"


Animator::Animator(sf::Sprite& sprite) noexcept:
    m_sprite(sprite),
    m_current(nullptr),
    m_rate(0.f),
    m_timer(0.f),
    m_isPlaying(false)
{

}


Animator::~Animator() = default;


void Animator::add(const std::string& name, Frames frames, float rate, bool isLooped) noexcept
{
    auto& anim = m_animations[name];
    anim = { frames, rate, isLooped };
    m_current = &anim;
}


bool Animator::set(const std::string& name) noexcept
{
    if (auto it = m_animations.find(name); it != m_animations.end())
    {
        m_current = &it->second;

        return true;
    }

    return false;
}


void Animator::setRate(float fps) noexcept
{
    m_rate = fabs(fps);
}


void Animator::update(float dt) noexcept
{
    if ((!m_isPlaying) || (!m_current))
        return;

    m_timer += m_rate * dt;
    const uint32_t frame = static_cast<uint32_t>(m_timer);

    if (frame >= m_current->frames.size())
    {
        m_timer = 0;

        if (!m_current->isLooped)
        {
            m_isPlaying = false;

            return;
        }
    }

    m_sprite.setTextureRect(m_current->frames[frame]);
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


bool Animator::isPlaying() const noexcept
{
    return m_isPlaying;
}


const sf::IntRect& Animator::getCurrentFrame() const noexcept
{
    return m_sprite.getTextureRect();
}