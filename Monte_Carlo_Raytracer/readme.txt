--------------------by Haoyun Qiu-------------------

--------------Test steps:------------------------------------
1 Release mode and /openmp. 

2 Add the command arguments as config file. 

3 When scene is set up, press "p" to render the image.
----------------------------------------------------------------

IMPORTANT FILES:

In /pics folder: 
Each sub-folder contains my render images, corresponding config files and brief readme.
You may check this to test. 

----------------------------------------------------------------
Config File Format change:

I add an input block like this

""
RAYTRACE
MAX_RECUR 5 
AA_SAMPLE 1 
SHADOW_RAY_SAMPLE 0 
MONTE_CARLO_ITER 0 
DIRECT_ILLU_WEIGHTS 0.f 

KDTREE
MAX_DEPTH 12 
""

Comment:

MAX_RECUR  //ray tracing recursive max depth 
AA_SAMPLE  //anti-aliasing pixel sampling rate, 1 means no additional sampling 
SHADOW_RAY_SAMPLE //soft shadow ray sample nums, use for area-light and soft shadow. 0 means turn off area-light and use point-light
MONTE_CARLO_ITER //Monte-Carlo iteration times, 0 means turn off Monte-Carlo raytrace
DIRECT_ILLU_WEIGHTS //Monte-Carlo raytrace, the direct illumination blend weights

KDTREE
MAX_DEPTH 12 //The max-depth which controls the Kd-Tree building. 

----------------------------------------------------------------
Output File Format:

For easier to debug, I automatically rename the .bmp files to this format: 
AA4_SHD1_MC2000_DI0_TIME17723.bmp

Each field of the name means:
AA- AA_SAMPLE
SHD- SHADOW_RAY_SAMPLE
MC- MONTE_CARLO_ITER
DI- DIRECT_ILLU_WEIGHTS
TIME- time used for rendering

so you can easily compare the parameters of different images.
----------------------------------------------------------------


Finished parts:

5.1 OpenMP
Already use OpenMP as default. And I print each thread's progress to the console.

5.2 Anti-aliasing
You can change the config file field: AA_SAMPLE to test.
Value should be >= 1. (1 means no additional sampling) 

5.3 Area lights and soft shadows 
You can change the config file field: SHADOW_RAY_NUM to test.
Value should be >=0. 

Extra Credits:

6.1 Monte Carlo raytracer(70 points + 30 points) 

Indirect illumination:
You'll check this in directory: pics/Monte_Carlo/
(Check sample folder first please)

Direct+indirect illumination:
You'll check this in pics/Monte_Carlo/Direct-Indirect illumination_campare

which shows the difference between only Indirect illumination and Direct/Indirect blend
illumination. Both with 100 Monte-Carlo iterations.
With direct illumination the picture will has much less noise.


6.3.2b Kd-Tree(65 points + 15 points) 

1. build tree: 
My KdTree's MaxDepth default value is 12;

When to stop subdivide the tree node:
--if current depth >= MaxDepth or current triangles' amount <= 10, return;
--when left and right sub-tree overlap each other over 50%, return.
Which can avoid querying too many same triangles when testing ray-mesh intersection. 

2. How to subdivide the node: 
Sort current node's triangles's min point according to the current selected axis and return the middle index element as the divide pivot.
(I also implement two ways of quick-select algorithm which uses for finding the Kth largest element(in this case time-complexity is O(n) averagely), it does faster when depth becomes big). 

I select the longest axis from current AABB and do the partition.
This will averagely make the KdTree more balanced comparing to select x,y,z axis in turn and make the query
more effcient.
And I store the subset of all triangles objs' pointers in each sub-nodes, which may use more RAM but easier for me to debug.

3.  Efficiency compare:
Run cornell_dragon.txt in /pics/KdTree/: 
which contains a dragon.obj and do a one-time point-light ray tracing.

--With Kd-Tree(select longest axis): 
build-tree time: 0.28 seconds
average render time: 3.4 seconds

--With Kd-Tree(select x,y,z axis in turn) 
build-tree time: 0.27 seconds
average render time: 4.82 seconds

--Without Kd-Tree(only with bounding-box): 
average render time: 279.35 seconds



