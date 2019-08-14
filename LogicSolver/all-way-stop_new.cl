%-----------------Traffic--------------
insideTheIntersection(Vehicle):-
  entersForkAtTime(Vehicle, _, _).

atTheIntersection(Vehicle):-
  arrivesAtForkAtTime(Vehicle, _, _),
  not insideTheIntersection(Vehicle).

isAtFork(Vehicle, Fork):-
  arrivesAtForkAtTime(Vehicle, Fork, Time),
  atTheIntersection(Vehicle).

arrivedEarlierThan(Vehicle1, Vehicle2):-
  arrivesAtForkAtTime(Vehicle1, _, ArrivalTime1),
  arrivesAtForkAtTime(Vehicle2, _, ArrivalTime2),
  ArrivalTime1 < ArrivalTime2.

arrivedSameTime(Vehicle1, Vehicle2):-
  arrivesAtForkAtTime(Vehicle1, _, ArrivalTime),
  arrivesAtForkAtTime(Vehicle2, _, ArrivalTime).

isToTheRightOf(Vehicle1, Vehicle2):-
  isAtFork(Vehicle1, Fork1),
  isAtFork(Vehicle2, Fork2),
  isToTheRightOf(Fork1, Fork2).

leftLane(Vehicle, Lane):-
  leavesLaneAtTime(Vehicle, Lane, _).

% Does not handle re-entries.
isOnLane(Vehicle, Lane):-
  entersLaneAtTime(Vehicle, Lane, _),
  not leftLane(Vehicle, Lane).

laneOnFork(Lane, Fork):-
  laneFromTo(Lane, Fork, _).

%------------------------------------------
%--------------Rules pre-definitions-------
%------------------------------------------
wantsLane(Vehicle, Lane):-
  signalsAtForkAtTime(Vehicle, Signal, Fork, _),
  laneOnFork(Lane, Fork),
  laneCorrectSignal(Lane, Signal).

% vehicle on the planned lane
reservedLane(Vehicle, Lane):-
  isOnLane(Vehicle, Lane),
  wantsLane(Vehicle, Lane).

%---------------- Rules ---------------
% Page 35:
% When there are “STOP” signs at all corners,
%  yield to the vehicle or bicycle that arrives first.
mustYieldToForRule(Vehicle2, Vehicle1, firstInFirstOut):-
  atTheIntersection(Vehicle1),
  atTheIntersection(Vehicle2),
  arrivedEarlierThan(Vehicle1, Vehicle2).

% Page 35:
% When there are “STOP” signs at all corners,
%  yield to the vehicle or bicycle on your right
%  if it reaches the intersection at the same time as you.
mustYieldToForRule(Vehicle1, Vehicle2, yieldToRight):-
  atTheIntersection(Vehicle1),
  atTheIntersection(Vehicle2),
  arrivedSameTime(Vehicle1, Vehicle2),
  isToTheRightOf(Vehicle2, Vehicle1).

mustYieldToForRule(Vehicle1, Vehicle2, yieldToInside):-
  atTheIntersection(Vehicle1),
  wantsLane(Vehicle1, Lane1),
  overlaps(Lane1, Lane2),
  reservedLane(Vehicle2, Lane2),
  not leftLane(Vehicle2, Lane1).

%-------------------------------------------------
mustYield(Vehicle):-
  mustYieldToForRule(Vehicle, _, _).

hasRightOfWay(Vehicle):-
  arrivesAtForkAtTime(Vehicle, _, _),
  not mustYield(Vehicle).

#show mustYieldToForRule/3.
#show hasRightOfWay/1.
