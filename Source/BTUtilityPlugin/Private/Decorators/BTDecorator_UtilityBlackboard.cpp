// Copyright 2015 Cameron Angus. All Rights Reserved.

#include "Decorators/BTDecorator_UtilityBlackboard.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Int.h"


UBTDecorator_UtilityBlackboard::UBTDecorator_UtilityBlackboard(const FObjectInitializer& ObjectInitializer) :
Super(ObjectInitializer)
{
	NodeName = "Blackboard Utility";

	bAllowAbortNone = true;
	bAllowAbortLowerPri = true;
	bAllowAbortChildNodes = true;

	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;

	NotifyObserver = EBTBlackboardRestart::ResultChange;

	// accept only float and integer keys
	UtilityValueKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_UtilityBlackboard, UtilityValueKey));
	UtilityValueKey.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_UtilityBlackboard, UtilityValueKey));
}

void UBTDecorator_UtilityBlackboard::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	UtilityValueKey.ResolveSelectedKey(*GetBlackboardAsset());
}

float UBTDecorator_UtilityBlackboard::CalculateUtilityValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent();
	float Value = 0.0f;

	if (UtilityValueKey.SelectedKeyType == UBlackboardKeyType_Float::StaticClass())
	{
		Value = MyBlackboard->GetValue< UBlackboardKeyType_Float >(UtilityValueKey.GetSelectedKeyID());
	}
	else if (UtilityValueKey.SelectedKeyType == UBlackboardKeyType_Int::StaticClass())
	{
		Value = (float)MyBlackboard->GetValue< UBlackboardKeyType_Int >(UtilityValueKey.GetSelectedKeyID());
	}

	return FMath::Max(Value, 0.0f);
}

void UBTDecorator_UtilityBlackboard::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UE_LOG(LogTemp, Log, TEXT("UBTDecorator_UtilityBlackboard::OnBecomeRelevant"));
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (BlackboardComp)
	{
		auto KeyID = UtilityValueKey.GetSelectedKeyID();
		BlackboardComp->RegisterObserver(KeyID, this, FOnBlackboardChangeNotification::CreateUObject(this, &UBTDecorator_UtilityBlackboard::OnBlackboardKeyValueChange));
	}
}

void UBTDecorator_UtilityBlackboard::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UE_LOG(LogTemp, Log, TEXT("UBTDecorator_UtilityBlackboard::OnCeaseRelevant"));
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (BlackboardComp)
	{
		BlackboardComp->UnregisterObserversFrom(this);
	}
}

FString UBTDecorator_UtilityBlackboard::GetStaticDescription() const
{
	return FString::Printf(TEXT("Utility Key: %s"), *GetSelectedBlackboardKey().ToString());
}

void UBTDecorator_UtilityBlackboard::DescribeRuntimeValues(const UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTDescriptionVerbosity::Type Verbosity, TArray<FString>& Values) const
{
	Super::DescribeRuntimeValues(OwnerComp, NodeMemory, Verbosity, Values);

	const UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	FString DescKeyValue;

	if (BlackboardComp)
	{
		DescKeyValue = BlackboardComp->DescribeKeyValue(UtilityValueKey.GetSelectedKeyID(), EBlackboardDescription::OnlyValue);
	}

	Values.Add(FString::Printf(TEXT("utility: %s"), *DescKeyValue));
}

EBlackboardNotificationResult UBTDecorator_UtilityBlackboard::OnBlackboardKeyValueChange(const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID)
{
	UE_LOG(LogTemp, Log, TEXT("UBTDecorator_UtilityBlackboard::OnBlackboardKeyValueChange"));
	UBehaviorTreeComponent* BehaviorComp = (UBehaviorTreeComponent*)Blackboard.GetBrainComponent();
	if (BehaviorComp == nullptr)
	{
		return EBlackboardNotificationResult::RemoveObserver;
	}

	if (UtilityValueKey.GetSelectedKeyID() == ChangedKeyID)
	{
		// can't simply use BehaviorComp->RequestExecution(this) here, we need to support condition/value change modes

		const EBTDecoratorAbortRequest RequestMode = (NotifyObserver == EBTBlackboardRestart::ValueChange) ? EBTDecoratorAbortRequest::ConditionPassing : EBTDecoratorAbortRequest::ConditionResultChanged;
		ConditionalFlowAbort(*BehaviorComp, RequestMode);
	}

	return EBlackboardNotificationResult::ContinueObserving;
}

