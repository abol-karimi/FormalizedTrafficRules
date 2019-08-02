%-----------------Traffic--------------
assign_external(arrivedFirst(Vehicle), False) :-
  entersIntersection(Vehicle).


%---------------- Rules ---------------
% Page 35:
% When there are “STOP” signs at all corners,
%  yield to the vehicle or bicycle that arrives first.
mustYieldToForRule(Vehicle1, Vehicle2, firstInFirstOut):-
  Vehicle1 != Vehicle2,
  arrivedFirst(Vehicle2),
  not arrivedFirst(Vehicle1).

% Page 35:
% When there are “STOP” signs at all corners,
%  yield to the vehicle or bicycle on your right
%  if it reaches the intersection at the same time as you.
mustYieldToForRule(Vehicle1, Vehicle2, yieldToRight):-
  Vehicle1 != Vehicle2,
  arrivedFirst(Vehicle1),
  arrivedFirst(Vehicle2),
  isToTheRightOf(Vehicle2, Vehicle1).
