%-----------------Traffic--------------

% A vehicle reserves each claimed lane (using the turn signal)
% from when they enter the intersection until they leave that lane.
% Note: Each vehicle enters all of its claimed lanes
reservesILaneOnInterval(Vehicle, ClaimedLane, (BeginTime, EndTime)):-
  claimsILane(Vehicle, ClaimedLane),
  entersIntersectionAtTime(Vehicle, BeginTime),
  leavesLaneAtTime(Vehicle, ClaimedLane, EndTime).

% Reserving lanes that overlap a claimed one,
%  and the reserving vehicle will pass through them.
reservesILaneOnInterval(Vehicle, OverlappingLane, (BeginTime, EndTime)):-
  claimsILane(Vehicle, ClaimedLane),
  overlaps(ClaimedLane, OverlappingLane),
  not claimsILane(Vehicle, OverlappingLane),
  entersIntersectionAtTime(Vehicle, BeginTime),
  leavesLaneAtTime(Vehicle, OverlappingLane, EndTime).

% Reserving lanes that overlap a claimed one,
%  but the reserving vehicle will not pass through them.
reservesILaneOnInterval(Vehicle, OverlappingLane, (BeginTime, EndTime)):-
  claimsILane(Vehicle, ClaimedLane),
  overlaps(ClaimedLane, OverlappingLane),
  not claimsILane(Vehicle, OverlappingLane),
  entersIntersectionAtTime(Vehicle, BeginTime),
  not leavesLaneAtTime(Vehicle, OverlappingLane, _),
  leavesLaneAtTime(Vehicle, ClaimedLane, EndTime).


%---------------- Rules ---------------
% Page 36:
% At a four-way stop, the driver reaching the intersection first goes first
% (after coming to a complete stop)...
mustYieldToOnClosedIntervalForRule(Vehicle1, Vehicle2, (BeginTime, EndTime), firstInFirstOut):-
  Vehicle1 != Vehicle2,
  arrivesFromLaneAtTime(Vehicle1, Lane1, BeginTime),
  arrivesFromLaneAtTime(Vehicle2, Lane2, ArrivalTime2),
  BeginTime > ArrivalTime2,
  entersIntersectionAtTime(Vehicle2, EndTime).

% Page 36:
% ...If more than one vehicle arrives at the same time, the vehicle on the right goes first.
mustYieldToOnClosedIntervalForRule(Vehicle1, Vehicle2, (BeginTime, EndTime), yieldToRight):-
  Vehicle1 != Vehicle2,
  arrivesFromLaneAtTime(Vehicle1, Lane1, BeginTime),
  arrivesFromLaneAtTime(Vehicle2, Lane2, BeginTime),
  isToTheRightOf(Lane2, Lane1),
  entersIntersectionAtTime(Vehicle2, EndTime).

 violatesRightOfWayOfAtTimeForRule(Vehicle1, Vehicle2, ViolationTime, Rule) :-
  mustYieldToOnClosedIntervalForRule(Vehicle1, Vehicle2, (BeginTime, EndTime), Rule),
  entersIntersectionAtTime(Vehicle1, ViolationTime),
  BeginTime <= ViolationTime,
  ViolationTime <= EndTime.

% A 'yield' is an action taken by the ego vehicle.
% The ego vehicle knows its own intentions,
% e.g. whether their left turn signal means a U-turn or a left turn.
% However, the ego vehicle may not know the intentions of the other vehicles,
% except the observable variables such as the turn signal.

% Page 46:
% ``You must come to a full and complete stop at a stop
% sign or stop line, if one is present. You must wait until crossing vehicles and pedestrians have cleared. You may
% pull forward only when it is safe to do so."
 mustYieldToOnClosedIntervalForRule(VehicleAtIntersection, VehicleInsideIntersection, (BeginYield, EndReserve), yieldToVehiclesInIntersection):-
  VehicleAtIntersection != VehicleInsideIntersection,
  arrivesFromLaneAtTime(VehicleAtIntersection, _, ArrivalTime),
  entersIntersectionAtTime(VehicleAtIntersection, EntranceTime),
  hasPath(VehicleAtIntersection, PathLane),
  reservesILaneOnInterval(VehicleInsideIntersection, PathLane, (BeginReserve, EndReserve)),
  ArrivalTime <= EndReserve,
  EntranceTime > BeginReserve,
  BeginYield = #max{ArrivalTime:ArrivalTime>=BeginReserve; BeginReserve:ArrivalTime<=BeginReserve}.
