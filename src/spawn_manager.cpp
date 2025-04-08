#include "spawn_manager.hpp"
#include "world_init.hpp"
#include "world_system.hpp"
#include <iostream>

SpawnManager::SpawnManager() : wave_timer_ms(0.f),
                               next_wave_ms(WAVE_INTERVAL_MS),
                               zombies_per_wave(INITIAL_ZOMBIES_PER_WAVE),
                               current_wave(0),
                               is_game_started(false)
{
    // Initialize RNG
    rng = std::default_random_engine(std::random_device()());
    uniform_dist = std::uniform_real_distribution<float>(0.f, 1.f);

    initialize_spawn_points();
}

void SpawnManager::initialize_spawn_points()
{
    const float OFFSET = -150.f; // Offset from edges for visibility

    // Create 8 spawn points, 2 on each edge
    // Top edge
    for (int i = 1; i <= 4; i++)
    {
        float x_pos = (MAP_WIDTH_PX * i) / 5.0f;
        spawn_points.push_back({{x_pos, OFFSET}, true});
    }

    // Right edge
    for (int i = 1; i <= 4; i++)
    {
        float y_pos = (MAP_HEIGHT_PX * i) / 5.0f;
        spawn_points.push_back({{MAP_WIDTH_PX - OFFSET, y_pos}, true});
    }

    // Bottom edge
    for (int i = 1; i <= 4; i++)
    {
        float x_pos = (MAP_WIDTH_PX * i) / 5.0f;
        spawn_points.push_back({{x_pos, MAP_HEIGHT_PX - OFFSET}, true});
    }

    // Left edge
    for (int i = 1; i <= 4; i++)
    {
        float y_pos = (MAP_HEIGHT_PX * i) / 5.0f;
        spawn_points.push_back({{OFFSET, y_pos}, true});
    }
}

void SpawnManager::step(float elapsed_ms, RenderSystem *renderer)
{
    // If game isn't started or in test mode, don't do anything
    if (!is_game_started || test_mode || WorldSystem::get_game_screen() == GAME_SCREEN_ID::TUTORIAL)
    {
        return;
    }

    // Update the timer for spawning
    wave_timer_ms += elapsed_ms;

    // Time to spawn an enemy?
    if (wave_timer_ms >= next_wave_ms)
    {
        // Spawn a single enemy
        spawn_enemy(renderer);

        // Reset timer
        wave_timer_ms = 0.f;
    }
}

void SpawnManager::generate_wave(RenderSystem *renderer)
{
    current_wave++;
    std::cout << "=== Wave " << current_wave << " Starting ===" << std::endl;

    // Spawn zombies at random points until we reach the target number
    for (int i = 0; i < zombies_per_wave; i++)
    {
        // Get a random spawn point index
        int random_point = (int)(uniform_dist(rng) * spawn_points.size());
        vec2 spawn_pos = spawn_points[random_point].position;
        if (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) < 0.3f)
        {
            createOrc(renderer, spawn_pos);
        }
        else
        {
            createSkeletonArcher(renderer, spawn_pos);
        }
    }

    if (IS_WAVE_MODE_LINEAR)
    {
        zombies_per_wave = (int)(zombies_per_wave + WAVE_SCALING_FACTOR);
    }
    else
    {
        zombies_per_wave = (int)(zombies_per_wave * WAVE_SCALING_FACTOR);
    }
    std::cout << "Next wave will have " << zombies_per_wave << " zombies" << std::endl;
}

void SpawnManager::reset()
{
    std::cout << "SpawnManager: Reset called!" << std::endl;
    wave_timer_ms = 0.f;
    next_wave_ms = WAVE_INTERVAL_MS;
    zombies_per_wave = INITIAL_ZOMBIES_PER_WAVE;
    current_wave = 0;
    is_game_started = false;
}

void SpawnManager::start_game()
{
    std::cout << "SpawnManager: Game started!" << std::endl;
    is_game_started = true;
    wave_timer_ms = 0.f;
}

void SpawnManager::set_test_mode(bool enabled)
{
    test_mode = enabled;
    std::cout << "Test mode " << (enabled ? "enabled" : "disabled") << std::endl;
}

void SpawnManager::set_day(int day)
{
    if (DAY_MAP.find(day) != DAY_MAP.end())
        this->day = DAY_MAP.at(day);
}

void SpawnManager::spawn_enemy(RenderSystem *renderer)
{
    // Get a random spawn point
    int random_point = (int)(uniform_dist(rng) * spawn_points.size());
    vec2 spawn_pos = spawn_points[random_point].position;

    // Get the current day from WorldSystem
    int current_day = WorldSystem::get_current_day();

    // Spawn enemy based on the current day progression
    spawnEnemyByDay(renderer, spawn_pos, current_day);
}

void SpawnManager::spawnEnemyByDay(RenderSystem *renderer, vec2 spawn_pos, int current_day)
{
    if (DAY_MAP.find(current_day) != DAY_MAP.end()) {
        if (!day.empty()) {
            ENEMY_ID enemy_id = day[0].first;
            std::cout << "Enemy: " << static_cast<int>(enemy_id) << ", " << "Count: " << day[0].second << std::endl;
            day[0].second--;
            if (day[0].second <= 0) day.erase(day.begin());
            switch (enemy_id) {
            case ENEMY_ID::ORC:
                createOrc(renderer, spawn_pos);
                return;
            case ENEMY_ID::ORC_ELITE:
                createOrcElite(renderer, spawn_pos);
                return;
            case ENEMY_ID::ORC_RIDER:
                createOrcRider(renderer, spawn_pos);
                return;
            case ENEMY_ID::SKELETON:
                createSkeleton(renderer, spawn_pos);
                return;
            case ENEMY_ID::SKELETON_ARCHER:
                createSkeletonArcher(renderer, spawn_pos);
                return;
            case ENEMY_ID::WEREWOLF:
                createWerewolf(renderer, spawn_pos);
                return;
            case ENEMY_ID::WEREBEAR:
                createWerebear(renderer, spawn_pos);
                return;
            case ENEMY_ID::SLIME:
                createSlime(renderer, spawn_pos);
                return;
            default:
                return;
            }
        }
    }

    if (current_day == 5)
    {
        // It's challenge day!
        start_challenge_day(renderer, spawn_pos);
        return;
    }

    // Progressive enemy introduction based on day number
    std::vector<std::function<void()>> available_enemies;
    float elite_chance = 0.0f;

    // Always have Orc as the basic enemy from day 1
    available_enemies.push_back([&]()
        { createOrc(renderer, spawn_pos); });

    // Day 2: Add Skeleton
    if (current_day >= 2)
    {
        available_enemies.push_back([&]()
            { createSkeleton(renderer, spawn_pos); });
    }

    // Day 3: Add OrcElite
    if (current_day >= 3)
    {
        available_enemies.push_back([&]()
            { createOrcElite(renderer, spawn_pos); });
        elite_chance = 0.1f; // 10% chance for elite versions
    }

    // Day 4: Add Werewolf
    if (current_day >= 4)
    {
        available_enemies.push_back([&]()
            { createWerewolf(renderer, spawn_pos); });
    }

    // Day 5: Add SkeletonArcher
    if (current_day >= 5)
    {
        available_enemies.push_back([&]()
            { createSkeletonArcher(renderer, spawn_pos); });
        elite_chance = 0.15f; // 15% chance for elite versions
    }

    // Day 6: Add Werebear
    if (current_day >= 6)
    {
        available_enemies.push_back([&]()
            { createWerebear(renderer, spawn_pos); });
    }

    // Day 7: Add Slime
    if (current_day >= 7)
    {
        available_enemies.push_back([&]()
            { createSlime(renderer, spawn_pos); });
        elite_chance = 0.2f; // 20% chance for elite versions
    }

    // Day 8: Add OrcRider
    if (current_day >= 8)
    {
        available_enemies.push_back([&]()
            { createOrcRider(renderer, spawn_pos); });
        elite_chance = 0.25f; // 25% chance for elite versions
    }

    // Once all enemies are introduced, use a weighted random distribution
    if (current_day >= 9)
    {
        // After day 9, use the original random distribution but with adjusted rates
        float prob = uniform_dist(rng);

        if (prob < 0.25f)
        {
            createSkeletonArcher(renderer, spawn_pos);
            return;
        }
        else if (prob < 0.35f)
        {
            createOrcElite(renderer, spawn_pos);
            return;
        }
        else if (prob < 0.45f)
        {
            createSkeleton(renderer, spawn_pos);
            return;
        }
        else if (prob < 0.55f)
        {
            createWerewolf(renderer, spawn_pos);
            return;
        }
        else if (prob < 0.65f)
        {
            createWerebear(renderer, spawn_pos);
            return;
        }
        else if (prob < 0.75f)
        {
            createSlime(renderer, spawn_pos);
            return;
        }
        else
        {
            createOrc(renderer, spawn_pos);
            return;
        }
    }

    // For days 1-7, select from available enemies list
    int enemy_idx = (int)(uniform_dist(rng) * available_enemies.size());
    available_enemies[enemy_idx]();
}

void SpawnManager::start_challenge_day(RenderSystem *renderer, vec2 spawn_pos)
{

    if (squad_spawned)
    {
        // Only spawn the squad once per game
        return;
    }

    squad_spawned = true;
    std::cout << "=== SPAWNING DAY 5 SQUAD CHALLENGE ===" << std::endl;

    // Create a squad entity to coordinate the members
    Entity squad_entity = Entity();
    Squad &squad = registry.squads.emplace(squad_entity);
    squad.squad_id = 1; // First squad

    // Get player position for reference
    vec2 player_pos;
    if (!registry.players.entities.empty() && registry.motions.has(registry.players.entities[0]))
    {
        player_pos = registry.motions.get(registry.players.entities[0]).position;
    }
    else
    {
        player_pos = vec2(MAP_WIDTH_PX/2, MAP_HEIGHT_PX/2); // Default to center if no player
    }

    // Choose a side to spawn from (opposite to player's position relative to center)
    vec2 map_center = vec2(MAP_WIDTH_PX/2, MAP_HEIGHT_PX/2);
    vec2 to_player = player_pos - map_center;
    
    // Determine spawn side (opposite to where the player is)
    vec2 spawn_dir;
    if (abs(to_player.x) > abs(to_player.y))
    {
        // Player is more to the left/right, so spawn from opposite horizontal edge
        spawn_dir = vec2(to_player.x < 0 ? 1 : -1, 0);
    }
    else
    {
        // Player is more to the top/bottom, so spawn from opposite vertical edge
        spawn_dir = vec2(0, to_player.y < 0 ? 1 : -1);
    }
    
    // Calculate formation center ahead of the player
    vec2 direction = normalize(player_pos - map_center);
    vec2 formation_center = player_pos + direction * 600.f;
    squad.formation_center = formation_center;
    
    // Determine spawn position (outside the map)
    const float SPAWN_OFFSET = 300.f; // Distance beyond map edges
    vec2 spawn_base;
    
    if (spawn_dir.x > 0) // Spawn from right edge
        spawn_base = vec2(MAP_WIDTH_PX + SPAWN_OFFSET, player_pos.y);
    else if (spawn_dir.x < 0) // Spawn from left edge
        spawn_base = vec2(-SPAWN_OFFSET, player_pos.y);
    else if (spawn_dir.y > 0) // Spawn from bottom edge
        spawn_base = vec2(player_pos.x, MAP_HEIGHT_PX + SPAWN_OFFSET);
    else // Spawn from top edge
        spawn_base = vec2(player_pos.x, -SPAWN_OFFSET);
    
    // Spawn archers outside the map
    for (int i = 0; i < 5; i++)
    {
        // Calculate final position in circle formation
        float angle = (2.0f * 3.14159f * i) / 5.0f;
        vec2 final_pos = formation_center + vec2(cos(angle), sin(angle)) * 200.0f;
        
        // Calculate offset spawn position (all from same side but spread out)
        vec2 offset = vec2(i * 50 - 100, i * 50 - 100);
        // Only apply offset in the direction perpendicular to spawn direction
        if (spawn_dir.x != 0) // Spawning from left/right
            offset.x = 0;
        else // Spawning from top/bottom
            offset.y = 0;
        
        vec2 archer_spawn = spawn_base + offset;
        Entity archer = createSkeletonArcher(renderer, archer_spawn);
        squad.archers.push_back(archer);
        
        // Set initial movement toward final position
        if (registry.motions.has(archer))
        {
            Motion& motion = registry.motions.get(archer);
            vec2 direction = normalize(final_pos - archer_spawn);
            motion.velocity = direction * 120.0f;
            
            // Face movement direction
            if (direction.x != 0)
                motion.scale.x = (direction.x > 0) ? abs(motion.scale.x) : -abs(motion.scale.x);
        }
    }
    
    // Spawn orcs outside the map
    for (int i = 0; i < 5; i++)
    {
        // Calculate final position in orc formation
        float offset = (i - 2) * 80.0f;
        vec2 final_pos = formation_center + vec2(offset, 100.f);
        
        // Calculate offset spawn position
        vec2 offset_vec = vec2(i * 50 - 100, i * 50 - 100);
        // Only apply offset in the direction perpendicular to spawn direction
        if (spawn_dir.x != 0) // Spawning from left/right
            offset_vec.x = 0;
        else // Spawning from top/bottom
            offset_vec.y = 0;
        
        vec2 orc_spawn = spawn_base + offset_vec;
        Entity orc = createOrc(renderer, orc_spawn);
        squad.orcs.push_back(orc);
        
        // Set initial movement toward final position
        if (registry.motions.has(orc))
        {
            Motion& motion = registry.motions.get(orc);
            vec2 direction = normalize(final_pos - orc_spawn);
            motion.velocity = direction * 150.0f;
            
            // Face movement direction
            if (direction.x != 0)
                motion.scale.x = (direction.x > 0) ? abs(motion.scale.x) : -abs(motion.scale.x);
        }
    }
    
    // Spawn knight (OrcRider) outside the map
    vec2 knight_final = formation_center + vec2(-300.f, 0.f);
    vec2 knight_spawn = spawn_base; // Right at the spawn base
    
    Entity knight = createOrcRider(renderer, knight_spawn);
    squad.knights.push_back(knight);
    
    // Set initial movement for knight
    if (registry.motions.has(knight) && registry.orcRiders.has(knight))
    {
        Motion& motion = registry.motions.get(knight);
        OrcRider& rider = registry.orcRiders.get(knight);
        vec2 direction = normalize(knight_final - knight_spawn);
        motion.velocity = direction * rider.walk_speed * 1.2f;
        
        // Face movement direction
        if (direction.x != 0)
            motion.scale.x = (direction.x > 0) ? abs(motion.scale.x) : -abs(motion.scale.x);
    }
    
    // Start with defensive formation
    squad.current_formation = Squad::Formation::DEFENSIVE;
}