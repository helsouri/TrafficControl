# TrafficControl
Traffic Control System through threads
## Problem Statement:
School assigment to construct a simple traffic control system through threading and mutexs <br>

## 1 Traffic Lights
We consider a typical intersection with traffic lights as shown in the following picture. It has four
directions (E, W, S, and N). The road on each direction has only one lane.<br>
The intersection has a traffic control system with three different colors (Green, Yellow, and Red).
Here is the pattern for the traffic lights:
The light at each direction follows this pattern: Green for TG seconds, then Yellow for TY
seconds, and Red for TB seconds, then repeating Green, Yellow, and Red, .... We assume
that the lights on E and W are synchronized and the lights on S and N are synchronized.
Moreover, when the lights on E and W directions are in Green or Yellow, the lights on S
and N directions should be in Red; when the lights on S and N directions are in Green or
Yellow, the lights on E and W directions should be in Red. Therefore, we have the following
requirement in terms of the time duration: TG + TY = TR. In this system, all traffic lights
will be modeled together by a single thread, which should define the colors in all directions.
In the following, we describe the traffic control policy for cars. In this system, each car is represented
by one thread, which executes the subroutine Car when it arrives at the intersection:
Car(directions dir) {
ArriveIntersection(dir);
CrossIntersection(dir);
ExitIntersection(dir);
}
Data type directions is defined as
typedef struct _directions {
char dir_original;
char dir_target;
} directions;<br><br>
where dir original and dir target show the original and targeted directions, respectively.<br><br>
*ArriveIntersection* When a car arrives at the intersection, if it sees a car (with the same original
direction) stopping in front, it should wait until the front car starts crossing (it doesn’t need to wait
until the front car finishes crossing). This is called head-of-line blocking. Otherwise, it can either
drive through, or turn left, or turn right (U-turn is not allowed). But it should first check the traffic
light in front.<br><br>
• Green Light: If it attempts to drive through, make sure that no car from the opposite direction
is turing left, or any car is turning right to the same lane; if it attempts to turn left, make sure
that no car is driving through the intersection in the opposite direction, or any car is turning right
to the same lane; if it attempts to turn right, make sure that no car is driving to the same lane.<br>
• Yellow Light: If it attempts to turn right, make sure that no car is driving to the same lane,
otherwise it should stop in front of the intersection.<br><br>
• Red Light: If it attempts to drive through or turn left, it should wait for the Green light and then
follows the rule for Green Light; if it attempts to turn right, make sure that no car is driving to
the same lane.<br><br>
We assume there is no collision for two turning-left cars from the opposite directions. You could use
usleep for the arrival time.<br><br>
*CrossIntersection* We assume that it takes fixed time periods (∆L, ∆S, and ∆R for turning left,
going straight, and turning right, respectively) to cross the intersection and print out a debug message.
You could use the Spin function to simulate the crossing.<br><br>
*ExitIntersection* It is called to indicate that the caller has finished crossing the intersection and
should take steps to let additional cars cross the intersection.<br><br>
