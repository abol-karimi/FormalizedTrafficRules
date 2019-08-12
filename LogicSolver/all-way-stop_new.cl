%-----------------Traffic--------------
atTheIntersection(Vehicle):-
  arrivesAtForkAtTime(Vehicle, _, _),
  not entersForkAtTime(Vehicle, _, _).

arrivedEarlierThan(Vehicle1, Vehicle2):-
  arrivesAtForkAtTime(Vehicle1, _, ArrivalTime1),
  arrivesAtForkAtTime(Vehicle2, _, ArrivalTime2),
  ArrivalTime1 < ArrivalTime2.

arrivedSameTime(Vehicle1, Vehicle2):-
  arrivesAtForkAtTime(Vehicle1, _, ArrivalTime),
  arrivesAtForkAtTime(Vehicle2, _, ArrivalTime).
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

#show mustYieldToForRule/3.
