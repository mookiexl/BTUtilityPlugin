// Copyright 2015 Cameron Angus. All Rights Reserved.
#pragma once

#include "BehaviorTree/BTCompositeNode.h"
#include "BTUtilityTypes.h"
#include "BTComposite_Utility.generated.h"


UENUM()
enum class EUtilitySelectionMethod: uint8
{
	// The child with the highest utility value is always chosen
	Priority,

	// Selection probability is proportional to utility value
	Proportional,
};

struct FBTUtilityMemory : public FBTCompositeMemory
{
	/** ordering of child nodes for the current search activation (array is a list of child indexes arranged in execution order) */
	FUtilityExecutionOrdering ExecutionOrdering;
};

class UBTDecorator_UtilityFunction;

/** 
 * Utility selector node.
 * Utility Nodes choose at most one of their children to execute. The choice made is a function of the utility values of the children.
 * A Utility Node will succeed if its chosen child succeeds, and will fail if its chosen child fails or if no child could be executed.
 * The recommended design is to:
 *   - Pair this with Blackboard Utility decorators on child nodes, then point them to floats on the blackboard that define utilities.
 *     This avoids per-tick calculations.
 *   - Then update the blackboard values as you need to, e.g. via events. If you need something closer to per tick, create custom
 *     services and run them on this node, so that utilities are being checked for changes as long as we're running. The Blackboard Utility
 *     nodes automatically trigger a recalculation as soon as the value changes, aborting any running tasks to ensure we're always doing
 *     the one with the highest utility.
 */
UCLASS()
class BTUTILITYPLUGIN_API UBTComposite_Utility: public UBTCompositeNode
{
	GENERATED_UCLASS_BODY()

public:
	// Method used to determine child selection based on utility values
	UPROPERTY(EditAnywhere, Category = "Utility")
	EUtilitySelectionMethod SelectionMethod;

public:
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual FString GetStaticDescription() const override;

protected:
	/**
	Attempt to find a utility function attached to the specified child.
	@return The utility function decorator, or nullptr if none was found.
	*/
	const UBTDecorator_UtilityFunction* FindChildUtilityFunction(int32 ChildIndex) const;

	/**
	Invoke utility function for each child and store all the utility values.
	@return true if at least one utility value is non-zero.
	*/
	bool EvaluateUtilityScores(FBehaviorTreeSearchData& SearchData, TArray< float >& OutScores) const;

	/**
	 * Gets all the scores via EvaluateUtilityScores, then sorts them for selection.
	 */
	virtual void OrderUtilityScores(FBehaviorTreeSearchData& SearchData) const;

	/**
	 * Called when start enters this node.
	 * 
	 * Evaluate utilities,
	 * and set up child BTDecorator_UtilityBlackboards to monitor their blackboard keys so that they can abort and we can reevaluate if they change.
	 */
	virtual void NotifyNodeActivation(FBehaviorTreeSearchData& SearchData) const override;
	
	/**
	 * Called when start leaves this node.
	 * 
	 * Stop monitoring for blackboard key changes.
	 */
	virtual void NotifyNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type& NodeResult) const;

	/**
	 * Set up child BTDecorator_UtilityBlackboard to monitor their blackboard keys so that they can abort and reevaluate if they change.
	 * We have to do this here because we're always active when our children are executing, but non-executing children aren't active when other children are,
	 * so they can't register and unregister in OnBecomeRelevant and OnCeaseRelevant because they're always CeaseRelevant unless they're executing.
	 * Additionally we can't register as the observer (i.e. do the whole thing in this node) as all composite (us) node functions are const, so we can't pass `this`
	 * to the observer function.
	 * 
	 * So: when we're entered, we iterate through our children and  tell the blackboard to alert our child blackboard utility decorators when their value changes.
	 * Then when a value changes the matching decorator requests execution, which triggers our NotifyNodeActivation(), which reevaluates all utilities.
	 */
	virtual void WatchChildBlackboardKeys(FBehaviorTreeSearchData& SearchData) const;

	/**
	 * Stop monitoring the blackboard keys of all our BTDecorator_UtilityBlackboard children.
	 */
	virtual void UnwatchChildBlackboardKeys(FBehaviorTreeSearchData& SearchData) const;

public:
	int32 GetNextChildHandler(struct FBehaviorTreeSearchData& SearchData, int32 PrevChild, EBTNodeResult::Type LastResult) const;

#if WITH_EDITOR
	virtual FName GetNodeIconName() const override;
#endif
};

//////////////////////////////////////////////////////////////////////////
// Inlines



