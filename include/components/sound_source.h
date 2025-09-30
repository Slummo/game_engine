#pragma once

#include "components/icomponent.h"
#include "core/types/id.h"

#include <string>
#include <unordered_map>
#include <cstdint>

#include <AL/al.h>
#include <glm/glm.hpp>

struct SoundSource : public IComponent {
public:
    SoundSource() {
        alGenSources(1, &m_source_id);
    }

    void register_sound(const std::string& name, AssetID sound_id) {
        m_sounds[name] = sound_id;
    }

    void unregister_sound(const std::string& name) {
        m_sounds.erase(name);
    }

    // Plays the current sound
    void play() {
        alSourcePlay(m_source_id);
    }

    // Pauses the current sound (doesn't rewind the sound)
    void pause() {
        alSourcePause(m_source_id);
    }

    // Stop the current sound (rewinds the sound)
    void stop() {
        alSourceStop(m_source_id);
    }

    bool has_sound(const std::string& name) {
        auto it = m_sounds.find(name);
        return it != m_sounds.end();
    }

    AssetID get_sound_id(const std::string& name) {
        return m_sounds.at(name);
    }

    bool is_sound_current(const std::string& name) {
        return m_current_sound_name == name;
    }

    void set_current_sound(const std::string& name, uint32_t buffer_id) {
        m_current_sound_name = std::string(name);
        m_current_buffer_id = buffer_id;
        alSourcei(m_source_id, AL_BUFFER, m_current_buffer_id);
    }

    ~SoundSource() {
        alDeleteSources(1, &m_source_id);
    }

    void set_owner_position(const glm::vec3& owner_position) {
        alSource3f(m_source_id, AL_POSITION, owner_position.x, owner_position.y, owner_position.z);
    }

    void set_owner_velocity(const glm::vec3& owner_velocity) {
        alSource3f(m_source_id, AL_VELOCITY, owner_velocity.x, owner_velocity.y, owner_velocity.z);
        m_has_velocity = true;
    }

    void set_owner_direction(const glm::ivec3& owner_direction) {
        alSource3i(m_source_id, AL_DIRECTION, owner_direction.x, owner_direction.y, owner_direction.z);
    }

    bool has_velocity() const {
        return m_has_velocity;
    }

private:
    uint32_t m_source_id;
    std::unordered_map<std::string, AssetID> m_sounds;
    uint32_t m_current_buffer_id;
    std::string m_current_sound_name;
    bool m_has_velocity = false;
};