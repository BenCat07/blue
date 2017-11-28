#pragma once

class IGameSystem {
public:
    // GameSystems are expected to implement these methods.
    virtual char const *name() = 0;

    // Init, shutdown
    // return true on success. false to abort DLL init!
    virtual auto init() -> bool      = 0;
    virtual auto post_init() -> void = 0;
    virtual auto shutdown() -> void  = 0;

    // Level init, shutdown
    virtual auto level_init_pre_entity() -> void = 0;
    // entities are created / spawned / precached here
    virtual auto level_init_post_entity() -> void = 0;

    virtual auto level_shutdown_pre_clear_steam_api_context() -> void = 0;
    virtual auto level_shutdown_pre_entity() -> void                  = 0;
    // Entities are deleted / released here...
    virtual auto level_shutdown_post_entity() -> void = 0;
    // end of level shutdown

    // Called during game save
    virtual auto on_save() -> void = 0;

    // Called during game restore, after the local player has connected and entities have been fully restored
    virtual auto on_restore() -> void = 0;

    // Called every frame. It's safe to remove an igamesystem from within this callback.
    virtual auto safe_remove_if_desired() -> void = 0;

    virtual auto is_per_frame() -> bool = 0;

    // destructor, cleans up automagically....
    virtual ~IGameSystem(){};

    // Client systems can use this to get at the map name
    static auto map_name() -> const char *;

    // These methods are used to add and remove server systems from the
    // main server loop. The systems are invoked in the order in which
    // they are added.
private:
    static auto add(IGameSystem *pSys) -> void;
    static auto remove(IGameSystem *pSys) -> void;
    static auto remove_all() -> void;

    // These methods are used to initialize, shutdown, etc all systems
    static auto init_all_systems() -> bool;
    static auto post_init_all_systems() -> void;
    static auto shutdown_all_systems() -> void;
    static auto level_init_pre_entity_all_systems(char const *pMapName) -> void;
    static auto level_init_post_all_systems() -> void;
    static auto level_shutdown_pre_clear_steam_api_context_all_systems() -> void; // Called prior to steamgameserverapicontext->Clear()
    static auto level_shutdown_pre_entity_all_systems() -> void;
    static auto level_shutdown_post_entity_all_systems() -> void;

    static auto on_save_all_systems() -> void;
    static auto on_restore_all_systems() -> void;

    static auto safe_remove_if_desired_all_systems() -> void;

    static auto pre_render_all_systems() -> void;
    static auto update_all_systems(float frametime) -> void;
    static auto post_render_all_systems() -> void;
};

class IGameSystemPerFrame : public IGameSystem {
public:
    // destructor, cleans up automagically....
    virtual ~IGameSystemPerFrame(){};

    // Called before rendering
    virtual auto pre_render() -> void = 0;

    // Gets called each frame
    virtual auto update(float frametime) -> void = 0;

    // Called after rendering
    virtual auto post_render() -> void = 0;
};

// Quick and dirty server system for users who don't care about precise ordering
// and usually only want to implement a few of the callbacks
class CBaseGameSystemPerFrame : public IGameSystemPerFrame {

public:
    virtual auto name() -> const char * override { return "unnamed"; }

    // Init, shutdown
    // return true on success. false to abort DLL init!
    virtual auto init() -> bool override { return true; }
    virtual auto post_init() -> void override {}
    virtual auto shutdown() -> void override {}

    // Level init, shutdown
    virtual auto level_init_pre_entity() -> void override {}
    virtual auto level_init_post_entity() -> void override {}
    virtual auto level_shutdown_pre_clear_steam_api_context() -> void override {}
    virtual auto level_shutdown_pre_entity() -> void override {}
    virtual auto level_shutdown_post_entity() -> void override {}

    virtual auto on_save() -> void override {}
    virtual auto on_restore() -> void override {}
    virtual auto safe_remove_if_desired() -> void override {}

    virtual auto is_per_frame() -> bool override { return true; }

    // Called before rendering
    virtual auto pre_render() -> void override {}

    // Gets called each frame
    virtual auto update(float) -> void override {}

    // Called after rendering
    virtual auto post_render() -> void override {}
};

class GameSystem : public CBaseGameSystemPerFrame {
    GameSystem *next;

public:
    GameSystem();
    virtual ~GameSystem();

    // add all functions that are in the linked list
    static void add_all();
};
