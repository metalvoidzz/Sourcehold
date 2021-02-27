#pragma once

#include <entt/entt.hpp>

namespace Sourcehold {
    namespace ECS {
        const enum class EntityType {
            LORD,
            DEER,

            TREE_APPLE_BUD,
            TREE_APPLE_FLOWER,
            TREE_APPLE_FRUIT,
            TREE_APPLE_EMPTY,
            TREE_APPLE_STUMP,

            TREE_BIRCH_L,
            TREE_CHESTNUT_L,
            TREE_OAK_L,
            TREE_PINE_L,

            TREE_BIRCH_XL,
            TREE_CHESTNUT_XL,
            TREE_OAK_XL,
            TREE_PINE_XL,
            TREE_SHRUB1_GREEN,
            TREE_SHRUB1_RED,
            TREE_SHRUB2
        };

        // TODO - move components to own namespace        
        struct PositionComponent {
            int x;
            int y;
        };
    
        struct EntityTypeComponent {
            EntityType type;
        };

        struct AnimatedComponent {
            bool animate;
            int frameIndex;
        };

        entt::registry initializeECS();
        entt::entity spawn(entt::registry &registry, EntityType type, int x, int y);
        void render(entt::registry &registry);
    }
}
