// Copyright 2015 Cameron Angus. All Rights Reserved.

#include "Decorators/BTDecorator_UtilityFunction.h"
#include "BehaviorTree/BTCompositeNode.h"


UBTDecorator_UtilityFunction::UBTDecorator_UtilityFunction(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "UnknownUtilityFunction";

	bAllowAbortNone = true;
	bAllowAbortLowerPri = true;
	bAllowAbortChildNodes = true;

	bNotifyTick = true;

// 	bNotifyBecomeRelevant = true;
// 	bNotifyCeaseRelevant = true;
// 	bNotifyActivation = true;
// 	bNotifyDeactivation = true;
// 
// 	// all blueprint based nodes must create instances
// 	bCreateNodeInstance = true;
}

float UBTDecorator_UtilityFunction::GetUtility() const
{
	return CurrentUtility;
}

float UBTDecorator_UtilityFunction::CalculateUtilityValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	return 0.0f;
}

void UBTDecorator_UtilityFunction::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	// save last tick's utility for comparing
	PreviousUtility = CurrentUtility;
	// calculate the current utility
	CurrentUtility = WrappedCalculateUtility(OwnerComp, NodeMemory);

	// possible this got ticked due to the decorator being configured as an observer
	if (GetShouldAbort(OwnerComp))
	{
		OwnerComp.RequestExecution(this);
	}
}

float UBTDecorator_UtilityFunction::WrappedCalculateUtility(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBTDecorator_UtilityFunction* NodeOb = bCreateNodeInstance ? (const UBTDecorator_UtilityFunction*)GetNodeInstance(OwnerComp, NodeMemory) : this;
	return NodeOb ? NodeOb->CalculateUtilityValue(OwnerComp, NodeMemory) : 0.0f;
}

bool UBTDecorator_UtilityFunction::GetShouldAbort(UBehaviorTreeComponent& OwnerComp) const
{
	UE_LOG(LogTemp, Log, TEXT("UBTDecorator_UtilityFunction::GetShouldAbort"));
	const bool bIsOnActiveBranch = OwnerComp.IsExecutingBranch(GetMyNode(), GetChildIndex());

	const bool bUtilityHasChanged = CurrentUtility != PreviousUtility;

	bool bShouldAbort = false;
	if (bIsOnActiveBranch)
	{
		bShouldAbort = (FlowAbortMode == EBTFlowAbortMode::Self || FlowAbortMode == EBTFlowAbortMode::Both) && bUtilityHasChanged;
	}
	else
	{
		bShouldAbort = (FlowAbortMode == EBTFlowAbortMode::LowerPriority || FlowAbortMode == EBTFlowAbortMode::Both) && bUtilityHasChanged;
	}

	return bShouldAbort;
}


