/*
	Vending rain by bjotje
*/

#include <stdafx.h>

static void OnStart()
{

}

static void OnStop()
{

}

static void OnTick()
{
	// Thanks to menyoo for the prop names
	static const char* propNames[] = { "prop_vend_snak_01", "prop_vend_fridge01", "prop_vend_water_01" };
	static constexpr int MAX_METEORS = 100;

	static Object meteors[MAX_METEORS] = {};
	static int meteorDespawnTime[MAX_METEORS];
	static int meteorsAmount = 0;

	Vector3 playerPos = GET_ENTITY_COORDS(PLAYER_PED_ID(), false);

	static DWORD64 lastTick = 0;
	DWORD64 curTick = GET_GAME_TIMER();

	if (meteorsAmount <= MAX_METEORS && curTick > lastTick + 200)
	{
		lastTick = curTick;

		Vector3 spawnPos = Vector3::Init(
			playerPos.x + g_Random.GetRandomInt(-50, 50),
			playerPos.y + g_Random.GetRandomInt(-50, 40),
			playerPos.z + g_Random.GetRandomInt(15, 25)
		);
		Hash choosenPropHash = GET_HASH_KEY(propNames[g_Random.GetRandomInt(0,2 )]);
		
		LOG("Loading model hash");
		//Hash choosenPropHash = GET_HASH_KEY("prop_vend_soda_02");
		LoadModel(choosenPropHash);
		LOG("Loaded model hash, creating object");
		Object meteor = CREATE_OBJECT(choosenPropHash, spawnPos.x, spawnPos.y, spawnPos.z, true, false, true);
		LOG("Created object");
		meteorsAmount++;

		for (int i = 0; i < MAX_METEORS; i++)
		{
			Object& prop = meteors[i];
			if (!prop)
			{
				prop = meteor;
				meteorDespawnTime[i] = 5;
				break;
			}
		}

		//SET_OBJECT_PHYSICS_PARAMS(meteor, 100.f, 1.f, 1.f, 0.f, 0.f, .5f, 0.f, 0.f, 0.f, 0.f, 0.f);
		//APPLY_FORCE_TO_ENTITY_CENTER_OF_MASS(meteor, 0, 50.f, 0, -10000.f, true, false, true, true);
		APPLY_FORCE_TO_ENTITY(meteor, 3, 25.f, 10.f, -500.f, 0, 0, 0, 0, true, true, true, false, true);

		SET_MODEL_AS_NO_LONGER_NEEDED(choosenPropHash);
	}

	static DWORD64 lastTick2 = 0;
	for (int i = 0; i < MAX_METEORS; i++)
	{
		Object& prop = meteors[i];
		if (prop)
		{
			if (DOES_ENTITY_EXIST(prop) && meteorDespawnTime[i] > 0)
			{
				Vector3 propPos = GET_ENTITY_COORDS(prop, false);
				if (GET_DISTANCE_BETWEEN_COORDS(playerPos.x, playerPos.y, playerPos.z, propPos.x, propPos.y, propPos.z, true) < 400.f)
				{
					if (HAS_ENTITY_COLLIDED_WITH_ANYTHING(prop))
					{
						if (lastTick2 < curTick - 1000)
						{
							meteorDespawnTime[i]--;
						}
					}
					continue;
				}
			}

			meteorsAmount--;

			SET_OBJECT_AS_NO_LONGER_NEEDED(&prop);

			prop = 0;
		}
	}

	if (lastTick2 < curTick - 1000)
	{
		lastTick2 = curTick;
	}
}

// Any of these functions can be omitted and either replaced with a `nullptr` or completely left out (default parameter values) in the `RegisterEffect` declaration
static RegisterEffect registerEffect(EFFECT_MISC_VENDING_RAIN, OnStart, OnStop, OnTick, EffectInfo
	{
		// These are always required, you may have to add more designators depending on your effect
		.Name = "Vending rain",
		.Id = "misc_vending_rain",
		.IsTimed = true
		
	}
);