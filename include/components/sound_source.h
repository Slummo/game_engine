#pragma once

#include "components/icomponent.h"
#include "managers/asset_manager.h"

#include <string>
#include <unordered_map>

#include <AL/al.h>
#include <glm/glm.hpp>

struct SoundSource : public IComponent {
public:
    SoundSource() {
        alGenSources(1, &source_id);
    }

    uint32_t source_id;
    bool should_play = false;   // Plays the current sound
    bool should_pause = false;  // Pauses the current sound (doesn't rewind the sound)
    bool should_stop = false;   // Stops the current sound (rewinds the sound)

    void add_sound(const std::string& name, AssetID sound_id) {
        m_sounds[name] = sound_id;
    }

    void remove_sound(const std::string& name) {
        m_sounds.erase(name);
    }

    ~SoundSource() {
        alDeleteSources(1, &source_id);
    }

    bool set_sound(const std::string& name) {
        auto it = m_sounds.find(name);
        if (it == m_sounds.end()) {
            return false;
        }

        if (name == m_current_sound_name) {
            // Already the current sound
            return true;
        }

        m_current_sound_name = std::string(name);

        AssetID sound_id = it->second;
        SoundAsset& sound = AssetManager::instance().get_asset<SoundAsset>(sound_id);

        // Set the attached buffer
        m_current_buffer_id = sound.buffer_id();
        alSourcei(source_id, AL_BUFFER, m_current_buffer_id);

        return true;
    }

    void set_owner_position(const glm::vec3& owner_position) {
        alSource3f(source_id, AL_POSITION, owner_position.x, owner_position.y, owner_position.z);
    }

    void set_owner_velocity(const glm::vec3& owner_velocity) {
        alSource3f(source_id, AL_VELOCITY, owner_velocity.x, owner_velocity.y, owner_velocity.z);
        m_has_velocity = true;
    }

    void set_owner_direction(const glm::ivec3& owner_direction) {
        alSource3i(source_id, AL_DIRECTION, owner_direction.x, owner_direction.y, owner_direction.z);
    }

    bool has_velocity() const {
        return m_has_velocity;
    }

private:
    std::unordered_map<std::string, AssetID> m_sounds;
    uint32_t m_current_buffer_id;
    std::string m_current_sound_name;
    bool m_has_velocity = false;
};