// Copyright 2015 Cameron Angus. All Rights Reserved.
#pragma once

#include "Decorators/BTDecorator_UtilityFunction.h"
#include "BTDecorator_UtilityBlueprintBase.generated.h"

/*
@TODO: Look into what the purpose is of PropertyData in the other blueprint base classes, may need to
duplicate it.
*/

/**
 * This is the base blueprint decorator that allows you to return a custom utility.
 * 
 * Override CalculateUtility in BP and return a value to return the utility for the parent Utility composite node to evaluate with others.
 * 
 * This node now also implements a tick function like the engine BTDecorator_BlueprintBase that allows it to implement Observer Abort:
 * each tick the CalculateUtility output is stored. If it's changed from last tick, the abort functionality is triggered. (the parent, 
 * now only receives the previously cached value).
 * 
 */
UCLASS(Abstract, Blueprintable)
class BTUTILITYPLUGIN_API UBTDecorator_UtilityBlueprintBase : public UBTDecorator_UtilityFunction
{
	GENERATED_UCLASS_BODY()

	virtual FString GetStaticDescription() const override;
	virtual void DescribeRuntimeValues(const UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTDescriptionVerbosity::Type Verbosity, TArray< FString >& Values) const override;

	virtual void SetOwner(AActor* ActorOwner) override;

#if WITH_EDITOR
	virtual bool UsesBlueprint() const override;
#endif

protected:
	/** Cached AIController owner of BehaviorTreeComponent. */
	UPROPERTY(Transient)
	AAIController* AIOwner;

	/** Cached actor owner of BehaviorTreeComponent. */
	UPROPERTY(Transient)
	AActor* ActorOwner;

protected:

	/* UBTDecorator_UtilityFunction interface */
	virtual float CalculateUtilityValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

	UFUNCTION(BlueprintImplementableEvent, Category = Utility)
	float CalculateUtility(AAIController* OwnerController, APawn* ControlledPawn) const;
};


