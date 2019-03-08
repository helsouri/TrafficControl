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
##  2 Testing
For the traffic lights, fix all the time periods described above as follows: (TG = 18s, TY = 2s, TR = 20s).
and (∆L = 3s, ∆S = 2s, ∆R = 1s). We assume that the lights on S and N turn in Green and the lights
on E and W turn in Red at time 0 in the beginning of the testing. The simulation will run 30 seconds.
In the main thread, you need to create one thread for each car and one thread to manage traffic
lights. The arrival pattern in the simulator is described as follows: <br><br>
<pre>
cid   arrival_time  dir_original  dir_target<br>
0       1.1             N           N<br>
1       2.0             N           N<br>
2       3.3             N           W<br>
3       3.5             S           S<br>
4       4.2             S           E<br>
5       4.4             N           N<br>
6       5.7             E           N<br>
7       5.9             W           N<br>
</pre>
Associated with every car is a unique id (cid) as the car number, the arrival time (arrival time) for
the given car, and the original dir original and targeted dir target directions. For example, the first
line shows Car 0 arriving at Time 1.1 which originally goes northward and continue to go northward.
Make sure that your simulation outputs information that clearly shows that your solution works. The
message should indicate car number and both original and targeted directions. In particular, messages
should be printed at the following times:<br>
• Whenever a car arrives at an intersection;<br>
• Whenever a car is crossing the intersection;<br>
• Whenever a car exits from the intersection.<br>
## 3 Expected Output
Time 1.1: Car 0 (->N ->N) arriving<br>
Time 1.1: Car 0 (->N ->N) crossing<br>
Time 2.0: Car 1 (->N ->N) arriving<br>
Time 2.0: Car 1 (->N ->N) crossing<br>
Time 3.1: Car 0 (->N ->N) exiting<br>
Time 3.3: Car 2 (->N ->W) arriving<br>
Time 3.3: Car 2 (->N ->W) crossing<br>
Time 3.5: Car 3 (->S ->S) arriving<br>
Time 4.0: Car 1 (->N ->N) exiting<br>
Time 4.2: Car 4 (->S ->E) arriving<br>
Time 4.4: Car 5 (->N ->N) arriving<br>
Time 4.4: Car 5 (->N ->N) crossing<br>
Time 5.7: Car 6 (->E ->N) arriving<br>
Time 5.9: Car 7 (->W ->N) arriving<br>
Time 6.3: Car 2 (->N ->W) exiting<br>
Time 6.3: Car 3 (->S ->S) crossing<br>
Time 6.4: Car 5 (->N ->N) exiting<br>
Time 6.4: Car 4 (->S ->E) crossing<br>
Time 6.4: Car 7 (->W ->N) crossing<br>
Time 7.4: Car 7 (->W ->N) exiting<br>
Time 8.3: Car 3 (->S ->S) exiting<br>
Time 9.4: Car 4 (->S ->E) exiting<br>
Time 20.0: Car 6 (->E ->N) crossing<br>
Time 23.0: Car 6 (->E ->N) exiting<br>
