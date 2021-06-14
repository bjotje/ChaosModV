/*
	Effect by <Your Name>
*/

#include <stdafx.h>

static const std::vector<Vector3> tpLocations =
{
	{309.3626f, 187.4505f, 103.9077f},
	{286.1262f, 195.0628f, 103.3726f},
	{1161.0009f, -319.9483f, 62.6551f},
	{-47.2072f, -1753.4149f, 28.8710f},
	{-60.0585f, -1749.1339f, 28.7721f},
	{-1099.9294f, -2735.4551f, -8.4101f},
	{-1067.2100f, -2696.4822f, -7.9601f},
	{-832.1050f, -2342.9543f, -12.2827f},
	{-874.6208f, -2295.2253f, -12.2828f},
	{-534.4653f, -1301.6726f, 25.4967f},
	{-333.4690f, -783.1644f, 33.4145f},
	{-335.2301f, -784.5850f, 38.2298f},
	{-333.4855f, -782.8314f, 43.0560f},
	{-335.3286f, -784.5587f, 47.8743f},
	{-325.8154f, -739.0118f, 43.0550f},
	{-328.4636f, -737.9450f, 38.2300f},
	{-325.8595f, -739.1099f, 33.4144f},
	{-1692.5210f, -1088.0599f, 12.6029f},
	{-1695.6250f, -1127.3628f, 12.6023f},
	{-1694.9231f, -1126.5022f, 12.6023f},
	{-2956.1011f, 445.8586f, 14.7377f},
	{-2549.4810f, 2316.8193f, 32.6658f},
	{-2550.5862f, 2316.5940f, 32.6658f},
	{1587.4397f, 6649.4136f, 24.6663f},
	{1725.9178f, 6412.1821f, 34.4506f},
	{2753.5618f, 3478.1731f, 55.1213f},
	{2559.1472f, 2601.9556f, 37.5247f},
	{2560.8643f, 379.0765f, 108.0712f},
	{2560.6401f, 380.1907f, 108.0712f}
};

static void OnStart()
{
	Vector3 v3 = tpLocations.at(g_Random.GetRandomInt(0, tpLocations.size() - 1));	
	
	//Remove player from current vehicle as you can't drink from a vehicle
	Ped playerPed = PLAYER_PED_ID();

	if (IS_PED_IN_ANY_VEHICLE(playerPed, false))
	{
		Vehicle veh = GET_VEHICLE_PED_IS_IN(playerPed, false);
		TASK_LEAVE_VEHICLE(playerPed, veh, 16);
		WAIT(100);
	}	

	CLEAR_PED_TASKS_IMMEDIATELY(playerPed);
	TeleportPlayer(v3);
	WAIT(200);		
	CLEAR_PED_TASKS_IMMEDIATELY(playerPed);

	//Take care of the expenses :)
	for (int i = 0; i < 3; i++)
	{
		char statNameFull[32];
		sprintf_s(statNameFull, "SP%d_TOTAL_CASH", i);

		auto hash = GET_HASH_KEY(statNameFull);

		int money;
		STAT_GET_INT(hash, &money, -1);
		STAT_SET_INT(hash, money + 1, 1);
	}

	_SET_CONTROL_NORMAL(0, 51, 1);

	//static Hash objectHash = GET_HASH_KEY("prop_vend_soda_02");
	////Vector3 playerlocation = GET_ENTITY_COORDS(playerPed, false);
	//// 
	//Vector3 randLoc = getRandomLocation();
	//Object o = GET_CLOSEST_OBJECT_OF_TYPE(randLoc.x, randLoc.y , randLoc.z, 1000, objectHash, true, true, true);
	//GET_ENTITY_ROTATION(o, 0);
	
	
}



static Vector3 getRandomLocation()
{
	Ped playerPed = PLAYER_PED_ID();
	Vector3 playerPos = GET_ENTITY_COORDS(playerPed, false);

	float x, y, z = playerPos.z, _;
	do
	{
		x = g_Random.GetRandomInt(-3747.f, 4500.f);
		y = g_Random.GetRandomInt(-4400.f, 8022.f);

	} 	while (TEST_VERTICAL_PROBE_AGAINST_ALL_WATER(x, y, z, 0, &_));

	float groundZ;
	bool useGroundZ;
	for (int i = 0; i < 100; i++)
	{
		float testZ = (i * 10.f) - 100.f;

		TeleportPlayer(x, y, testZ);
		if (i % 5 == 0)
		{
			WAIT(0);
		}

		useGroundZ = GET_GROUND_Z_FOR_3D_COORD(x, y, testZ, &groundZ, false, false);
		if (useGroundZ)
		{
			break;
		}
	}

	return Vector3(x, y, useGroundZ ? groundZ : z);
}



static void OnStop()
{

}

static void OnTick()
{

}

// Any of these functions can be omitted and either replaced with a `nullptr` or completely left out (default parameter values) in the `RegisterEffect` declaration
static RegisterEffect registerEffect(EFFECT_PLAYER_QUICK_REFRESHMENT, OnStart, OnStop, OnTick, EffectInfo
	{
		// These are always required, you may have to add more designators depending on your effect
		.Name = "Quick Refreshment",
		.Id = "player_quick_refreshment"
	}
);