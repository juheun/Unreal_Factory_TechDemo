// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Facility/FactoryFacilityPanelBase.h"
#include "Placement/FactoryPlaceObjectBase.h"

void UFactoryFacilityPanelBase::InitPanel(AFactoryPlaceObjectBase* PlaceObject)
{
	TargetFacility = PlaceObject;
	
	// 자식클래스에서 델리게이트 바인딩
}

void UFactoryFacilityPanelBase::NativeDestruct()
{
	Super::NativeDestruct();
	
	TargetFacility.Reset();
}
