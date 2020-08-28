// Copyright 2015 Cameron Angus. All Rights Reserved.
#pragma once

#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_UtilityFunction.generated.h"


/** 
 * Utility functions are responsible for providing a utility value for their associated node whenever the 
 * parent utility selector requests it.
 * 
 * Override CalculateUtilityValue in C++ and return a value to return the utility for the parent Utility composite node to evaluate with others.
 * 
 * This node now also implements a tick function like the engine BTDecorator_BlueprintBase that allows it to implement Observer Abort:
 * each tick the CalculateUtility output is stored. If it's changed from last tick, the abort functionality is triggered. (the parent, 
 * now only receives the previously cached value).
 * 
 */
UCLASS(Abstract, HideCategories = (Condition))
class BTUTILITYPLUGIN_API UBTDecorator_UtilityFunction : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_UtilityFunction(const FObjectInitializer& ObjectInitializer);

	/**
	 * Returns the current utility value which was calculated this tick or last tick.
	 */
	virtual float GetUtility() const;

	/**
	 * return if this decorator should abort in current circumstances.
	 * 
	 * Mirror BTDecorator_BlueprintBase. Abort if utility value has changed since last calculating.
	 * */
	bool GetShouldAbort(UBehaviorTreeComponent& OwnerComp) const;

protected:

	/** wrapper for node instancing: CalculateUtilityValue */
	float WrappedCalculateUtility(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const;


	/** Calculates the utility value of the associated behavior node. */
	virtual float CalculateUtilityValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const;

	/**
	 * Calculate the utility each tick and cache it.
	 * Trigger an abort if specified and the value has changed, so utility can be recalculated.
	 */
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	/**
	 * The utility value that was calculated last tick.
	 */
	float PreviousUtility = 0.f;

	/**
	 * The utility value that was calculated this tick.
	 * Should be calculated at the start of a tick so that everything else can safely use it afterwards.
	 */
	float CurrentUtility = 0.f;
};


//////////////////////////////////////////////////////////////////////////
// Inlines

