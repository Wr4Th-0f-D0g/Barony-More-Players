/*-------------------------------------------------------------------------------

	BARONY
	File: monster_crystalgolem.cpp
	Desc: implements all of the crystal golem monster's code

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "main.hpp"
#include "game.hpp"
#include "stat.hpp"
#include "entity.hpp"
#include "items.hpp"
#include "monster.hpp"
#include "engine/audio/sound.hpp"
#include "net.hpp"
#include "collision.hpp"
#include "player.hpp"
#include "magic/magic.hpp"
#include "prng.hpp"

void initCrystalgolem(Entity* my, Stat* myStats)
{
	node_t* node;

	my->flags[BURNABLE] = false;
	my->initMonster(475);
	my->z = -1.5;

	if ( multiplayer != CLIENT )
	{
		MONSTER_SPOTSND = 273;
		MONSTER_SPOTVAR = 3;
		MONSTER_IDLESND = 266;
		MONSTER_IDLEVAR = 3;
	}
	if ( multiplayer != CLIENT && !MONSTER_INIT )
	{
		if ( myStats != nullptr )
		{
			if ( !myStats->leader_uid )
			{
				myStats->leader_uid = 0;
			}

			// apply random stat increases if set in stat_shared.cpp or editor
			setRandomMonsterStats(myStats);

			// generate 6 items max, less if there are any forced items from boss variants
			int customItemsToGenerate = ITEM_CUSTOM_SLOT_LIMIT;

			// boss variants

			// random effects
			if ( local_rng.rand() % 8 == 0 )
			{
				myStats->EFFECTS[EFF_ASLEEP] = true;
				myStats->EFFECTS_TIMERS[EFF_ASLEEP] = 1800 + local_rng.rand() % 3600;
			}

			// generates equipment and weapons if available from editor
			createMonsterEquipment(myStats);

			// create any custom inventory items from editor if available
			createCustomInventory(myStats, customItemsToGenerate);

			// count if any custom inventory items from editor
			int customItems = countCustomItems(myStats); //max limit of 6 custom items per entity.

														 // count any inventory items set to default in edtior
			int defaultItems = countDefaultItems(myStats);

			my->setHardcoreStats(*myStats);

			// generate the default inventory items for the monster, provided the editor sprite allowed enough default slots
			switch ( defaultItems )
			{
				case 6:
				case 5:
				case 4:
				case 3:
					if ( local_rng.rand() % 10 == 0 ) // 10% for gemstone, rock to obsidian.
					{
						newItem(static_cast<ItemType>(GEM_ROCK + local_rng.rand() % 17), static_cast<Status>(WORN + local_rng.rand() % 3), 0, 1, local_rng.rand(), false, &myStats->inventory);
					}
				case 2:
					if ( local_rng.rand() % 20 == 0 ) // 5% for secondary armor/weapon.
					{
						if ( local_rng.rand() % 2 == 0 ) // 50% armor
						{
							newItem(static_cast<ItemType>(CRYSTAL_BREASTPIECE + local_rng.rand() % 5), static_cast<Status>(DECREPIT + local_rng.rand() % 4), -2 + local_rng.rand() % 5, 1, local_rng.rand(), false, &myStats->inventory);
						}
						else // 50% weapon
						{
							if ( local_rng.rand() % 5 == 0 ) // 1 in 5 is shuriken, 1-3 count.
							{
								newItem(static_cast<ItemType>(CRYSTAL_SHURIKEN), EXCELLENT, -2 + local_rng.rand() % 5, 1 + local_rng.rand() % 3, local_rng.rand(), false, &myStats->inventory);
							}
							else // pick 1 of 4 normal weapons.
							{
								newItem(static_cast<ItemType>(CRYSTAL_SWORD + local_rng.rand() % 4), static_cast<Status>(DECREPIT + local_rng.rand() % 4), -2 + local_rng.rand() % 5, 1, local_rng.rand(), false, &myStats->inventory);
							}
						}
					}
				case 1:
					if ( local_rng.rand() % 2 == 0 ) // 50% armor
					{
						newItem(static_cast<ItemType>(CRYSTAL_BREASTPIECE + local_rng.rand() % 5), static_cast<Status>(DECREPIT + local_rng.rand() % 4), -2 + local_rng.rand() % 5, 1, local_rng.rand(), false, &myStats->inventory);
					}
					else // 50% weapon
					{
						if ( local_rng.rand() % 5 == 0 ) // 1 in 5 is shuriken, 1-3 count.
						{
							newItem(static_cast<ItemType>(CRYSTAL_SHURIKEN), EXCELLENT, -2 + local_rng.rand() % 5, 1 + local_rng.rand() % 3, local_rng.rand(), false, &myStats->inventory);
						}
						else // pick 1 of 4 normal weapons.
						{
							newItem(static_cast<ItemType>(CRYSTAL_SWORD + local_rng.rand() % 4), static_cast<Status>(DECREPIT + local_rng.rand() % 4), -2 + local_rng.rand() % 5, 1, local_rng.rand(), false, &myStats->inventory);
						}
					}
					break;
				default:
					break;
			}
		}
	}

	// torso
	Entity* entity = newEntity(476, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 4;
	entity->sizey = 4;
	entity->skill[2] = my->getUID();
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->focalx = limbs[CRYSTALGOLEM][1][0]; // 0
	entity->focaly = limbs[CRYSTALGOLEM][1][1]; // 0
	entity->focalz = limbs[CRYSTALGOLEM][1][2]; // 0
	entity->behavior = &actCrystalgolemLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);

	// right leg
	entity = newEntity(480, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 4;
	entity->sizey = 4;
	entity->skill[2] = my->getUID();
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->focalx = limbs[CRYSTALGOLEM][2][0]; // 1
	entity->focaly = limbs[CRYSTALGOLEM][2][1]; // 0
	entity->focalz = limbs[CRYSTALGOLEM][2][2]; // 2
	entity->behavior = &actCrystalgolemLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);

	// left leg
	entity = newEntity(479, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 4;
	entity->sizey = 4;
	entity->skill[2] = my->getUID();
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->focalx = limbs[CRYSTALGOLEM][3][0]; // 1
	entity->focaly = limbs[CRYSTALGOLEM][3][1]; // 0
	entity->focalz = limbs[CRYSTALGOLEM][3][2]; // 2
	entity->behavior = &actCrystalgolemLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);

	// right arm
	entity = newEntity(478, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 4;
	entity->sizey = 4;
	entity->skill[2] = my->getUID();
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->focalx = limbs[CRYSTALGOLEM][4][0]; // -.25
	entity->focaly = limbs[CRYSTALGOLEM][4][1]; // 0
	entity->focalz = limbs[CRYSTALGOLEM][4][2]; // 3
	entity->behavior = &actCrystalgolemLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);

	// left arm
	entity = newEntity(477, 1, map.entities, nullptr); //Limb entity.
	entity->sizex = 4;
	entity->sizey = 4;
	entity->skill[2] = my->getUID();
	entity->flags[PASSABLE] = true;
	entity->flags[NOUPDATE] = true;
	entity->flags[USERFLAG2] = my->flags[USERFLAG2];
	entity->focalx = limbs[CRYSTALGOLEM][5][0]; // -.25
	entity->focaly = limbs[CRYSTALGOLEM][5][1]; // 0
	entity->focalz = limbs[CRYSTALGOLEM][5][2]; // 3
	entity->behavior = &actCrystalgolemLimb;
	entity->parent = my->getUID();
	node = list_AddNodeLast(&my->children);
	node->element = entity;
	node->deconstructor = &emptyDeconstructor;
	node->size = sizeof(Entity*);
	my->bodyparts.push_back(entity);
}

void actCrystalgolemLimb(Entity* my)
{
	my->actMonsterLimb();
}

void crystalgolemDie(Entity* my)
{
	my->removeMonsterDeathNodes();

	for ( int c = 0; c < 6; c++ )
	{
		Entity* gib = spawnGib(my);
		if (c < 6) {
		    gib->sprite = 475 + c;
		    gib->skill[5] = 1; // poof
		}
		serverSpawnGibForClient(gib);
	}

	playSoundEntity(my, 269 + local_rng.rand() % 4, 128);

	list_RemoveNode(my->mynode);
	return;
}

#define CRYSTALGOLEMWALKSPEED .12

void crystalgolemMoveBodyparts(Entity* my, Stat* myStats, double dist)
{
	node_t* node;
	Entity* entity = nullptr;
	Entity* leftbody = nullptr;
	Entity* leftarm = nullptr;
	int bodypart;

	// set invisibility //TODO: isInvisible()?
	if ( multiplayer != CLIENT )
	{
		if ( myStats->EFFECTS[EFF_INVISIBLE] == true )
		{
			my->flags[INVISIBLE] = true;
			my->flags[BLOCKSIGHT] = false;
			bodypart = 0;
			for (node = my->children.first; node != nullptr; node = node->next)
			{
				if ( bodypart < 2 )
				{
					bodypart++;
					continue;
				}
				if ( bodypart >= 7 )
				{
					break;
				}
				entity = (Entity*)node->element;
				if ( !entity->flags[INVISIBLE] )
				{
					entity->flags[INVISIBLE] = true;
					serverUpdateEntityBodypart(my, bodypart);
				}
				bodypart++;
			}
		}
		else
		{
			my->flags[INVISIBLE] = false;
			my->flags[BLOCKSIGHT] = true;
			bodypart = 0;
			for (node = my->children.first; node != nullptr; node = node->next)
			{
				if ( bodypart < 2 )
				{
					bodypart++;
					continue;
				}
				if ( bodypart >= 7 )
				{
					break;
				}
				entity = (Entity*)node->element;
				if ( entity->flags[INVISIBLE] )
				{
					entity->flags[INVISIBLE] = false;
					serverUpdateEntityBodypart(my, bodypart);
					serverUpdateEntityFlag(my, INVISIBLE);
				}
				bodypart++;
			}
		}

		// sleeping
		if ( myStats->EFFECTS[EFF_ASLEEP] )
		{
			my->z = 1.5;
		}
		else
		{
			my->z = -1.5;
		}
	}

	//Move bodyparts
	for (bodypart = 0, node = my->children.first; node != nullptr; node = node->next, bodypart++)
	{
		if ( bodypart < 2 )
		{
			// post-swing head animation. client doesn't need to adjust the entity pitch, server will handle.
			if ( bodypart == 1 && multiplayer != CLIENT )
			{
				if ( my->monsterAnimationLimbOvershoot >= ANIMATE_OVERSHOOT_TO_SETPOINT )
				{
					limbAnimateWithOvershoot(my, ANIMATE_PITCH, 0.2, PI / 4, 0.1, 0, ANIMATE_DIR_POSITIVE);
				}
			}
			continue;
		}
		entity = (Entity*)node->element;
		entity->x = my->x;
		entity->y = my->y;
		entity->z = my->z;
		entity->yaw = my->yaw;
		if ( bodypart == 3 || bodypart == 6 )
		{
			// right leg, left arm.
			if ( bodypart == 3 )
			{
				// set leftbody to the left leg.
				leftbody = (Entity*)node->next->element;
			}
			if ( bodypart == 3 || MONSTER_ATTACK == 0 )
			{
				// swing right leg, left arm in sync.
				if ( dist > 0.1 )
				{
					if ( !leftbody->skill[0] )
					{
						entity->pitch -= dist * CRYSTALGOLEMWALKSPEED;
						if ( entity->pitch < -PI / 4.0 )
						{
							entity->pitch = -PI / 4.0;
							if ( bodypart == 3 && entity->skill[0] == 0 )
							{
								playSoundEntityLocal(my, 115, 128);
								entity->skill[0] = 1;
								/*if ( local_rng.rand() % 4 == 0 )
								{
									playSoundEntityLocal(my, 266, 32);
								}*/
							}
						}
					}
					else
					{
						entity->pitch += dist * CRYSTALGOLEMWALKSPEED;
						if ( entity->pitch > PI / 4.0 )
						{
							entity->pitch = PI / 4.0;
							if ( bodypart == 3 && entity->skill[0] == 1 )
							{
								playSoundEntityLocal(my, 115, 128);
								entity->skill[0] = 0;
							}
						}
					}
				}
				else
				{
					// if not moving, reset position of the leg/arm.
					if ( entity->pitch < 0 )
					{
						entity->pitch += 1 / fmax(dist * .1, 10.0);
						if ( entity->pitch > 0 )
						{
							entity->pitch = 0;
						}
					}
					else if ( entity->pitch > 0 )
					{
						entity->pitch -= 1 / fmax(dist * .1, 10.0);
						if ( entity->pitch < 0 )
						{
							entity->pitch = 0;
						}
					}
				}
			}
			else
			{
				// ATTACK!
				// move left arm

				// vertical chop windup
				if ( MONSTER_ATTACK == MONSTER_POSE_MELEE_WINDUP1 )
				{
					if ( MONSTER_ATTACKTIME == 0 )
					{
						// init rotations
						entity->pitch = 0;
						entity->roll = 0;
					}

					limbAnimateToLimit(entity, ANIMATE_PITCH, -0.25, 5 * PI / 4, false, 0);
					entity->skill[0] = 0;

					if ( MONSTER_ATTACKTIME >= ANIMATE_DURATION_WINDUP )
					{
						if ( multiplayer != CLIENT )
						{
							my->attack(1, 0, nullptr);
						}
					}
				}
				// vertical chop attack
				else if ( MONSTER_ATTACK == 1 )
				{
					if ( entity->skill[0] == 0 )
					{
						if ( limbAnimateToLimit(entity, ANIMATE_PITCH, 0.3, PI / 3, false, 0) )
						{
							entity->skill[0] = 1;
						}
					}
					else if ( entity->skill[0] == 1 )
					{
						if ( limbAnimateToLimit(entity, ANIMATE_PITCH, -0.2, PI / 4, false, 0) )
						{
							entity->skill[0] = leftbody->skill[0];
							entity->pitch = leftbody->pitch;
							MONSTER_ATTACK = 0;
						}
					}
				}
				// horizontal chop windup
				else if ( MONSTER_ATTACK == MONSTER_POSE_MELEE_WINDUP2 )
				{
					if ( MONSTER_ATTACKTIME == 0 )
					{
						// init rotations
						entity->pitch = 0;
						entity->roll = 0;
					}
					limbAnimateToLimit(entity, ANIMATE_PITCH, -0.25, PI, false, 0);
					limbAnimateToLimit(entity, ANIMATE_ROLL, -0.25, 7 * PI / 4, false, 0);
					entity->monsterAnimationLimbOvershoot = ANIMATE_OVERSHOOT_TO_SETPOINT;

					if ( MONSTER_ATTACKTIME >= ANIMATE_DURATION_WINDUP )
					{
						if ( multiplayer != CLIENT )
						{
							//serverUpdateEntitySkill(entity, 30); // update overshoot flag
							my->attack(2, 0, nullptr);
						}
					}
				}
				// horizontal chop attack
				else if ( MONSTER_ATTACK == 2 )
				{
					limbAnimateToLimit(entity, ANIMATE_PITCH, 0.3, 0, false, 0.0);
					if ( limbAnimateWithOvershoot(entity, ANIMATE_ROLL, 0.25, PI / 2, 0.1, 0, ANIMATE_DIR_POSITIVE) == ANIMATE_OVERSHOOT_TO_ENDPOINT )
					{
						entity->skill[0] = leftbody->skill[0];
						entity->pitch = leftbody->pitch;
						entity->roll = 0;
						MONSTER_ATTACK = 0;
					}
				}

				// special double vertical chop
				else if ( MONSTER_ATTACK == MONSTER_POSE_MELEE_WINDUP3 )
				{
					if ( MONSTER_ATTACKTIME == 0 )
					{
						// init rotations
						entity->pitch = 0;
						entity->roll = 0;
						createParticleDot(my);
						if ( multiplayer != CLIENT )
						{
							myStats->EFFECTS[EFF_PARALYZED] = true;
							myStats->EFFECTS_TIMERS[EFF_PARALYZED] = 60;
						}
					}
					
					if ( MONSTER_ATTACKTIME < 40 )
					{
						// move the head.
						limbAnimateToLimit(my, ANIMATE_PITCH, -0.1, 11 * PI / 6, true, 0.1);

						// raise left arm and tilt.
						limbAnimateToLimit(entity, ANIMATE_PITCH, -0.1, 9 * PI / 8, true, 0.1);
						limbAnimateToLimit(entity, ANIMATE_ROLL, -0.2, PI / 16, false, 0);
					}
					else if ( MONSTER_ATTACKTIME == 40 )
					{
						playSoundEntityLocal(my, MONSTER_SPOTSND, 128);
					}
					else if ( MONSTER_ATTACKTIME > 50 )
					{
						if ( multiplayer != CLIENT )
						{
						// set overshoot for head animation
							my->monsterAnimationLimbOvershoot = ANIMATE_OVERSHOOT_TO_SETPOINT;
							my->attack(MONSTER_POSE_GOLEM_SMASH, MAXCHARGE, nullptr);
						}
					}
				}
				
				// golem smash after windup3
				else if ( MONSTER_ATTACK == MONSTER_POSE_GOLEM_SMASH )
				{
					if ( limbAnimateToLimit(entity, ANIMATE_PITCH, 0.25, PI / 4, false, 0) )
					{
						entity->skill[0] = leftbody->skill[0];
						entity->pitch = leftbody->pitch;
						entity->roll = 0;
						if ( multiplayer != CLIENT )
						{
							if ( myStats->EFFECTS[EFF_PARALYZED] == true )
							{
								myStats->EFFECTS[EFF_PARALYZED] = false;
								myStats->EFFECTS_TIMERS[EFF_PARALYZED] = 0;
							}
						}
						MONSTER_ATTACK = 0;
					}
					MONSTER_ATTACKTIME++; // manually increment counter
				}
			}
		}
		else if ( bodypart == 4 || bodypart == 5 )
		{
			// right arm
			if ( bodypart == 5 )
			{
				if ( MONSTER_ATTACK > 0 )
				{
					// get leftarm from bodypart 6 element if ready to attack
					leftarm = (Entity*)node->next->element;

					if ( MONSTER_ATTACK == MONSTER_POSE_GOLEM_SMASH || MONSTER_ATTACK == MONSTER_POSE_MELEE_WINDUP3
						|| MONSTER_ATTACK == 1 || MONSTER_ATTACK == MONSTER_POSE_MELEE_WINDUP1 )
					{
						if ( leftarm != nullptr )
						{
							// follow the right arm animation.
							entity->pitch = leftarm->pitch;
							entity->roll = -leftarm->roll;
						}
					}
					else if ( MONSTER_ATTACK == MONSTER_POSE_MELEE_WINDUP2 )
					{
						limbAnimateToLimit(entity, ANIMATE_PITCH, -0.2, PI / 3, false, 0);
						entity->skill[0] = 0;
					}
					else if ( MONSTER_ATTACK == 2 )
					{
						if ( entity->skill[0] == 0 )
						{
							if ( limbAnimateToLimit(entity, ANIMATE_PITCH, 0.25, PI / 4, false, 0) )
							{
								entity->skill[0] = 1;
							}
						}
						else if ( entity->skill[0] == 1 )
						{
							limbAnimateToLimit(entity, ANIMATE_PITCH, -0.25, 0, false, 0);
						}
					}
				}
				else
				{
					entity->skill[0] = leftbody->skill[0];
					entity->pitch = leftbody->pitch;
					entity->roll = 0;
				}
			}

			if ( bodypart != 5 || (MONSTER_ATTACK == 0) )
			{
				// swing right arm/ left leg in sync
				if ( dist > 0.1 )
				{
					if ( entity->skill[0] )
					{
						entity->pitch -= dist * CRYSTALGOLEMWALKSPEED;
						if ( entity->pitch < -PI / 4.0 )
						{
							entity->skill[0] = 0;
							entity->pitch = -PI / 4.0;
						}
					}
					else
					{
						entity->pitch += dist * CRYSTALGOLEMWALKSPEED;
						if ( entity->pitch > PI / 4.0 )
						{
							entity->skill[0] = 1;
							entity->pitch = PI / 4.0;
						}
					}
				}
				else
				{
					// if not moving, reset position of the leg/arm.
					if ( entity->pitch < 0 )
					{
						entity->pitch += 1 / fmax(dist * .1, 10.0);
						if ( entity->pitch > 0 )
						{
							entity->pitch = 0;
						}
					}
					else if ( entity->pitch > 0 )
					{
						entity->pitch -= 1 / fmax(dist * .1, 10.0);
						if ( entity->pitch < 0 )
						{
							entity->pitch = 0;
						}
					}
				}
			}
		}
		switch ( bodypart )
		{
			// torso
			case 2:
				entity->x -= .5 * cos(my->yaw);
				entity->y -= .5 * sin(my->yaw);
				entity->z += 2.25;
				break;
			// right leg
			case 3:
				entity->x += 2 * cos(my->yaw + PI / 2) - 1.25 * cos(my->yaw);
				entity->y += 2 * sin(my->yaw + PI / 2) - 1.25 * sin(my->yaw);
				entity->z += 5;
				if ( my->z >= 1.4 && my->z <= 1.6 )
				{
					entity->yaw += PI / 8;
					entity->pitch = -PI / 2;
				}
				else if ( entity->pitch <= -PI / 3 )
				{
					entity->pitch = 0;
				}
				break;
			// left leg
			case 4:
				entity->x -= 2 * cos(my->yaw + PI / 2) + 1.25 * cos(my->yaw);
				entity->y -= 2 * sin(my->yaw + PI / 2) + 1.25 * sin(my->yaw);
				entity->z += 5;
				if ( my->z >= 1.4 && my->z <= 1.6 )
				{
					entity->yaw -= PI / 8;
					entity->pitch = -PI / 2;
				}
				else if ( entity->pitch <= -PI / 3 )
				{
					entity->pitch = 0;
				}
				break;
			// right arm
			case 5:
				entity->x += 3.5 * cos(my->yaw + PI / 2) - 1 * cos(my->yaw);
				entity->y += 3.5 * sin(my->yaw + PI / 2) - 1 * sin(my->yaw);
				entity->z += .1;
				entity->yaw += MONSTER_WEAPONYAW;
				if ( my->z >= 1.4 && my->z <= 1.6 )
				{
					entity->pitch = 0;
				}
				break;
			// left arm
			case 6:
				entity->x -= 3.5 * cos(my->yaw + PI / 2) + 1 * cos(my->yaw);
				entity->y -= 3.5 * sin(my->yaw + PI / 2) + 1 * sin(my->yaw);
				entity->z += .1;
				if ( my->z >= 1.4 && my->z <= 1.6 )
				{
					entity->pitch = 0;
				}
				break;
		}
	}
	if ( MONSTER_ATTACK > 0 && MONSTER_ATTACK <= MONSTER_POSE_MAGIC_CAST3 )
	{
		MONSTER_ATTACKTIME++;
	}
	else if ( MONSTER_ATTACK == 0 )
	{
		MONSTER_ATTACKTIME = 0;
	}
	else
	{
		// do nothing, don't reset attacktime or increment it.
	}

}
