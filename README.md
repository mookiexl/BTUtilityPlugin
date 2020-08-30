## BT Utility Plugin

An extension to the UE4 behavior tree adding support for utility-based selection.

This is a stripped down version of an in-progress engine mod (https://github.com/kamrann/UnrealEngine/tree/bt_utility_49base) which omits some editor modifications, but maintains the core functionality as a plugin which can work with a regular engine installation.

## Notes

In order to keep as much in line as possible with existing behavior tree/decorator behavior, the UtilitySelector node works essentially as a regular Selector node but with dynamically determined ordering of children.
This ordering is calculated each time the UtilitySelector is activated, if using the *Utility Blackboard* decorators that pull utility values from the blackboard, whenever these values change.

For priority selection, on activation the child utility values will be evaluated and then the children ordered by decreasing utility value.
For proportional selection, on activation an ordering is determined such that children with higher utility values are more likely to be earlier in the ordering (The chance of a child being first is directly proportional to its utility value; the process is then repeated for the remaining children with the first one removed from consideration, until all children are in the ordering).

Once the ordering is determined, the node excutes as a regular selector, trying children in order until one succeeds.

Note that custom blueprint based utility functions do not currently expose the usual decorator events such as activation/tick. This is partly due to it requiring either copying a large amount of code from the UBTDecorator_BlueprintBase class, or adding performance overhead by using an interface. However, due to the above way in which utility functions are only evaluated once when the UtilitySelector is activated, it is also not obvious to me that these events are useful for utility functions. If anyone finds they do require them, please get in touch.

### Observer aborts / preempting

The original version of this plugin only evaluated and ordered utilities when the node was first entered, which meant that a long-running task like *Move To* or *Wait* would finish even if it had become a lower priority. If the *Blackboard Utility* decorators are used to read utility values from blackboard keys the latest version of this plugin now re-evaluates the utilities as soon as they change. If you need to avoid this or avoid flapping (a problem with most utility implementations) simply implement some hysterisis into what's driving your utility values on the blackboard: e.g. once a location is selected increase the utility of moving there until you arrive.

### Recommended design

- Add Blackboard Utility decorators to child nodes under a Utility composite node, then point them to individual floats on the blackboard that define utilities.
- Update the blackboard values as you need to, e.g. via events. If you need something closer to per tick, create custom services and run them on the parent Utility node. The utility node is always active when any of its children are running, so putting services here means that even inactive children can be alerted for changes which triggers a reevaluation.

The Blackboard Utility nodes then automatically trigger a recalculation as soon as the value changes, aborting any running tasks to ensure we're always doing the one with the highest utility.

![example 1](Examples/Example1.png)