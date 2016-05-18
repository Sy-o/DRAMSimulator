#pragma once

enum FaultType
{
	NONE = 0,
	SAF,
	TF,
	CFin,
	CFid,
	CFst
};

struct Fault
{
	FaultType type;
	int victimValue;
	int agressorValue;
	int agressorAddress;
	int victimAddress;
};