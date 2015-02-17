#include "tests.h"
#include "Geometry.h"
#include "Cylinder.h"
#include "Sphere.h"
#include "Cube.h"
#include "Mesh.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <glm/glm.hpp>

using namespace glm;

void RunRaySphereTests();
void RunRayPolyTests();
void RunRayCubeTests();
void RunRayCylinderTests();
void RunYourTests();
void RunOurTests();

typedef bool (*TestFunc)();

int g_numTests = 0;
int g_numSuccessful = 0;

void ReportTest(std::string name, bool result);

template<typename T>
void RunTest(std::string name, T const& testValue, T const& expectedValue)
{
    ReportTest(name, testValue == expectedValue);
}

template<>
void RunTest<double>(std::string name, double const& testValue, double const& expectedValue)
{
    if (expectedValue == -1) 
	{
        ReportTest(name, testValue < 0);
    } 
	else 
	{
		bool res = ((std::abs(testValue - expectedValue) / std::abs(expectedValue + 1e-5)) < 1e-3);
		if(!res)
		{
			printf("%f %f\n", testValue, expectedValue);
		}
        ReportTest(name, res);
    }
}

void RunTests()
{
    std::cout.sync_with_stdio(true);

//    RunRaySphereTests();
//    RunRayPolyTests();
//    RunRayCubeTests();
//    RunRayCylinderTests();
//    RunYourTests();
    RunOurTests();

    std::cout << g_numSuccessful << " of " << g_numTests << " tests successful. ";
    if (g_numTests == g_numSuccessful) 
	{
        std::cout << "A winner is you!";
    }
    std::cout << std::endl;
}

double Test_RaySphereIntersect(vec3 const& P0, vec3 const& V0,
                               mat4 const& T)
{
	//Test_RaySphereIntersect(vec3(0.5,0,0), vec3(1,0,0), DOUBLE_MATRIX),
    return Sphere().intersect(T, Ray(P0, V0)).t;
}

double Test_RayPolyIntersect(vec3 const& P0, vec3 const& V0,
                             vec3 const& p1, vec3 const& p2, vec3 const& p3,
                             mat4 const& T)
{
    // TODO: Call your triangle intersection function here
	return Mesh().triangle_intersect_basic(T, Ray(P0, V0), p1, p2, p3, false).t;	
}

double Test_RayCubeIntersect(vec3 const& P0, vec3 const& V0,
                             mat4 const& T)
{
    return Cube().intersect(T, Ray(P0, V0)).t;
}

double Test_RayCylinderIntersect(vec3 const& P0, vec3 const& V0,
                                 mat4 const& T)
{
    return Cylinder().intersect(T, Ray(P0, V0)).t;
}

const double SQRT_HALF = 0.70710678;
const double SQRT_TWO = 1.41421356;

const mat4 IDENTITY_MATRIX = mat4();
const mat4 DOUBLE_MATRIX(vec4(2.0f, 0.0f, 0.0f, 0.0f),
                         vec4(0.0f, 2.0f, 0.0f, 0.0f),
                         vec4(0.0f, 0.0f, 2.0f, 0.0f),
                         vec4(0.0f, 0.0f, 0.0f, 1.0f));
const mat4 TALLANDSKINNY_MATRIX(vec4(0.5f, 0.0f, 0.0f, 0.0f),
                                vec4(0.0f, 2.0f, 0.0f, 0.0f),
                                vec4(0.0f, 0.0f, 0.5f, 0.0f),
                                vec4(0.0f, 0.0f, 0.0f, 1.0f));
const mat4 BACK5_MATRIX(vec4(1.0f, 0.0f, 0.0f, 0.0f),
                        vec4(0.0f, 1.0f, 0.0f, 0.0f),
                        vec4(0.0f, 0.0f, 1.0f, 0.0f),
                        vec4(0.0f, 0.0f, -5.0f, 1.0f));
const mat4 BACK5ANDTURN_MATRIX(vec4(SQRT_HALF, 0.0f, -SQRT_HALF, 0.0f),
                               vec4(0.0f, 1.0f, 0.0f, 0.0f),
                               vec4(SQRT_HALF, 0.0f, SQRT_HALF, 0.0f),
                               vec4(0.0f, 0.0f, -5.0f, 1.0f));


const vec3 ZERO_VECTOR(0.0f, 0.0f, 0.0f);
const vec3 HALFX_VECTOR(0.5f, 0.0f, 0.0f);
const vec3 THIRDX_VECTOR(0.33333333333333333f, 0.0f, 0.0f);
const vec3 NEGX_VECTOR(-1.0f, 0.0f, 0.0f);
const vec3 NEGZ_VECTOR(0.0f, 0.0f, -1.0f);
const vec3 NEGY_VECTOR(0.0f, -1.0f, 0.0f);
const vec3 POSZ_VECTOR(0.0f, 0.0f, 1.0f);
const vec3 POSXPOSZ_VECTOR(1.0f, 0.0f, 1.0f);
const vec3 POSXNEGZ_VECTOR(1.0f, 0.0f, -1.0f);
const vec3 ZNEGTEN_VECTOR(0.0f, 0.0f, -10.0f);
const vec3 ZPOSTEN_VECTOR(0.0f, 0.0f, 10.0f);
const vec3 YPOSTEN_VECTOR(0.0f, 10.0f, 0.0f);
const vec3 XPOSTEN_VECTOR(10.0f, 0.0f, 0.0f);
const vec3 POSXNEGZ_NORM_VECTOR(SQRT_HALF, 0.0f, -SQRT_HALF);
const vec3 NEGFIVEOFIVE_VECTOR(-5.0f, 0.0f, 5.0f);
const vec3 NEGFIVEOFIVE_NORM_VECTOR(-0.707107f, 0.0f, 0.707107f);

const vec3 POINT_N1N10(-1.0f, -1.0f, 0.0f);
const vec3 POINT_1N10(1.0f, -1.0f, 0.0f);
const vec3 POINT_010(0.0f, 1.0f, 0.0f);
const vec3 POINT_N2N10(-2.0f, -1.0f, 0.0f);
const vec3 POINT_2N10(2.0f, -1.0f, 0.0f);

const double TEN_KAZILLION = 1e26; 

void RunRaySphereTests()
{
    RunTest(
        "Easy sphere",
        Test_RaySphereIntersect(ZERO_VECTOR, NEGZ_VECTOR, BACK5_MATRIX),
        4.0);

    RunTest(
        "Offset a bit",
        Test_RaySphereIntersect(HALFX_VECTOR, NEGZ_VECTOR, BACK5_MATRIX),
        4.13397);

    RunTest(
        "What sphere",
        Test_RaySphereIntersect(ZNEGTEN_VECTOR, NEGZ_VECTOR, BACK5_MATRIX),
        -1.0);

    RunTest(
        "TODO Looking back",
        Test_RaySphereIntersect(ZNEGTEN_VECTOR, POSZ_VECTOR, BACK5_MATRIX),
        4.0); // TODO change this to the right number

    RunTest(
        "West pole",
        Test_RaySphereIntersect(ZERO_VECTOR, NEGZ_VECTOR, BACK5ANDTURN_MATRIX),
        4.0);

    RunTest(
        "Another angle",
        Test_RaySphereIntersect(NEGFIVEOFIVE_VECTOR, POSXNEGZ_NORM_VECTOR, IDENTITY_MATRIX),
        (5.0 * SQRT_TWO) - 1);
}

void RunRayPolyTests()
{
    RunTest(
        "Hi, Tri",
        Test_RayPolyIntersect(POSZ_VECTOR, NEGZ_VECTOR, POINT_N1N10, POINT_1N10, POINT_010, IDENTITY_MATRIX),
        1.0);

    RunTest(
        "Bye, Tri",
        Test_RayPolyIntersect(POSXPOSZ_VECTOR, NEGZ_VECTOR, POINT_N1N10, POINT_1N10, POINT_010, IDENTITY_MATRIX),
        -1.0);

    RunTest(
        "Looking back",
        Test_RayPolyIntersect(POSZ_VECTOR, POSZ_VECTOR, POINT_N1N10, POINT_1N10, POINT_010, IDENTITY_MATRIX),
        -1.0);

    RunTest(
        "Flip it good",
        Test_RayPolyIntersect(POSZ_VECTOR, NEGZ_VECTOR, POINT_010, POINT_1N10, POINT_N1N10, IDENTITY_MATRIX),
        1.0);
	
    RunTest(
        "It moves",
        Test_RayPolyIntersect(ZERO_VECTOR, NEGZ_VECTOR, POINT_N1N10, POINT_1N10, POINT_010, BACK5ANDTURN_MATRIX),
        5.0);

    RunTest(
        "TODO And turns",
        Test_RayPolyIntersect(HALFX_VECTOR, NEGZ_VECTOR, POINT_N2N10, POINT_2N10, POINT_010, BACK5ANDTURN_MATRIX),
        5.5); // TODO change this to the right number
}

void RunRayCubeTests()
{
    RunTest(
        "Behold the cube",
        Test_RayCubeIntersect(ZERO_VECTOR, NEGZ_VECTOR, BACK5_MATRIX),
        4.5);

    RunTest(
        "The cube abides",
        Test_RayCubeIntersect(THIRDX_VECTOR, NEGZ_VECTOR, BACK5_MATRIX),
        4.5);

    RunTest(
        "Cuuuube!",
        Test_RayCubeIntersect(NEGX_VECTOR, NEGZ_VECTOR, BACK5_MATRIX),
        -1.0);

    RunTest(
        "Looking sharp, edge",
        Test_RayCubeIntersect(ZERO_VECTOR, NEGZ_VECTOR, BACK5ANDTURN_MATRIX),
        5.0 - SQRT_HALF);

    RunTest(
        "Big cube",
        Test_RayCubeIntersect(ZPOSTEN_VECTOR, NEGZ_VECTOR, DOUBLE_MATRIX),
        9.0);

    RunTest(
        "TODO Strafing the cube",
        Test_RayCubeIntersect(NEGFIVEOFIVE_VECTOR, POSXNEGZ_NORM_VECTOR, IDENTITY_MATRIX),
        6.363961); // TODO change this to the right number
}

void RunRayCylinderTests()
{
    RunTest(
        "On the can",
        Test_RayCylinderIntersect(ZPOSTEN_VECTOR, NEGZ_VECTOR, IDENTITY_MATRIX),
        9.5);

    RunTest(
        "Same difference",
        Test_RayCylinderIntersect(XPOSTEN_VECTOR, NEGX_VECTOR, IDENTITY_MATRIX),
        9.5);

    RunTest(
        "Can opener",
        Test_RayCylinderIntersect(YPOSTEN_VECTOR, NEGY_VECTOR, TALLANDSKINNY_MATRIX),
        9.0);

    RunTest(
        "Swing and a miss",
        Test_RayCylinderIntersect(ZERO_VECTOR, POSZ_VECTOR, BACK5_MATRIX),
        -1.0);

    RunTest(
        "Plink",
        Test_RayCylinderIntersect(ZERO_VECTOR, NEGZ_VECTOR, BACK5ANDTURN_MATRIX),
        4.5);
  
    RunTest(
        "TODO DIY",
        Test_RayCylinderIntersect(POSXNEGZ_NORM_VECTOR, NEGFIVEOFIVE_NORM_VECTOR, DOUBLE_MATRIX),
        0.0); //TODO change this to the right number
}

void RunYourTests()
{
    // TODO: It can be very useful to put tests of your own here. The unit
    // tests above do NOT test everything!

    // Example things you may want to test:
    // * All of the cases and edge cases of your particular implementations
    // * Various weird transformations: non-uniform scaling, etc.
    // * Unnormalized ray inputs (not necessarily needed, depending on how you do things later on.)

    // In addition to these, you might want to write your own tests to check
    // the surface normals returned by intersection, especially if you have
    // trouble with the debug images that you will generate.
}

void RunOurTests()
{
    // Leave this function emty; we'll use it for grading.
	// 2012 fall test cases
	const vec3 SPHERE0P0(0,-1,-4);
	const vec3 SPHERE0V0(0,1,0);
	const mat4 SPHERE0TRANS(vec4(2,0,0,0), vec4(0,1,-1.732,0), vec4(0,1.732,1,0), vec4(0,3,-3,1));
	RunTest(
		"GRADING SPHERE 0",
		Test_RaySphereIntersect(SPHERE0P0, SPHERE0V0, SPHERE0TRANS),
		4 - glm::sqrt(3.0));

	const vec3 SPHERE1P0(1,0,3);
	const vec3 SPHERE1V0(0,0,-1);
	const mat4 SPHERE1TRANS(vec4(2,0,0,0), vec4(0,1,0,0), vec4(0,0,1,0), vec4(0,0,0,1));
	RunTest(
		"GRADING SPHERE 1",
		Test_RaySphereIntersect(SPHERE1P0, SPHERE1V0, SPHERE1TRANS),
		3 - glm::sqrt(0.75));

	/*
	const vec3 POLY0P[3] = {
		vec3(0,2,-2),
		vec3(-3,-2,2),
		vec3(3,-2,2)
	};


	const vec3 POLY0P0(1,-1,2);
	const vec3 POLY0V0(0,0,-1);

	RunTest(
		"GRADING POLY 0",
		Test_RayPolyIntersect(POLY0P0, POLY0V0, POLY0P[0], POLY0P[1], POLY0P[2], IDENTITY_MATRIX),
		1.0);

	const mat4 POLY1TRANS(-3,0,0,0,   0,2,0,0,   0,0,-2,0,   0,0,0,1);

	RunTest(
		"GRADING POLY 1",
		Test_RayPolyIntersect(POLY0P0, POLY0V0, POLY0P[0], POLY0P[1], POLY0P[2], POLY1TRANS),
		3.0);


	const vec3 CUBE0P0(1,1,1);
	const vec3 CUBE0V0(-0.5773,-0.5773,-0.5773);

	RunTest(
		"GRADING CUBE 0",
		Test_RayCubeIntersect(CUBE0P0, CUBE0V0, IDENTITY_MATRIX),
		glm::sqrt(3.0) - glm::sqrt(0.75));

	const mat4 CUBE1TRANS(vec4(0.7071,0.4082,0.5774,0), vec4(0,0.8165,-0.5774,0),  vec4(-0.7071,0.4082,0.5774,0), vec4(0,0,0,1));

	const vec3 CUBE1P0(0,0,1);
	const vec3 CUBE1V0(0,0,-1);

	RunTest(
		"GRADING CUBE 1",
		Test_RayCubeIntersect(CUBE1P0, CUBE1V0, CUBE1TRANS),
		1 - glm::sqrt(0.75));
	
	const vec3 CYLINDER0P0(0.25, 0, 1);
	const vec3 CYLINDER0V0(0, 0, -1);

	RunTest(
		"GRADING CYLINDER 0",
		Test_RayCylinderIntersect(CYLINDER0P0, CYLINDER0V0, IDENTITY_MATRIX),
		0.56699);

	const mat4 CYLINDER1TRANS(vec4(4,0,0,0), vec4(0,4,0,0), vec4(0,0,4,0), vec4(0,0,0,1));	
	const vec3 CYLINDER1P0(0, -4.5, -1.0);
	const vec3 CYLINDER1V0(0, 0.7071, 0.7071);

	RunTest(
		"GRADING CYLINDER 1",
		Test_RayCylinderIntersect(CYLINDER1P0, CYLINDER1V0, CYLINDER1TRANS),
		3.5355);
	*/

	// 2013 fall test cases
	const mat4 HALF_MATRIX(vec4(0.5f, 0.0f, 0.0f, 0.0f),
							 vec4(0.0f, 0.5f, 0.0f, 0.0f),
							 vec4(0.0f, 0.0f, 0.5f, 0.0f),
							 vec4(0.0f, 0.0f, 0.0f, 1.0f));

	const mat4 SCALE_MATRIX(vec4(0.5f, 0.0f, 0.0f, 0.0f),
							 vec4(0.0f, 1.0f, 0.0f, 0.0f),
							 vec4(0.0f, 0.0f, 2.0f, 0.0f),
							 vec4(0.0f, 0.0f, 0.0f, 1.0f));

	RunTest(
		"Inside sphere",
		Test_RaySphereIntersect(vec3(0.5,0,0), vec3(1,0,0), DOUBLE_MATRIX),
		1.5);

	RunTest(
		"Nonuniformly scaled sphere",
		Test_RaySphereIntersect(vec3(-0.7,0,0), normalize(vec3(1,2,0)), SCALE_MATRIX),
		0.671);
	/*
	RunTest(
		"Inside cube",
		Test_RayCubeIntersect(vec3(-0.3,-0.1,0), normalize(vec3(1,3,0)), HALF_MATRIX),
		0.158);

	RunTest(
		"Nonuniformly scaled cube",
		Test_RayCubeIntersect(vec3(0.6,1.3,-0.1), normalize(vec3(2,3,1)), SCALE_MATRIX),
		-1.0);

	RunTest(
		"Nonuniformly scaled triangle",
		Test_RayPolyIntersect(vec3(0,0,-1), normalize(vec3(0.1,-0.05,1)), vec3(0.5,0.4,0.7),
		vec3(-1.2,-0.4,0.6), vec3(0.6,-0.5,0.8), SCALE_MATRIX),
		2.52);

	RunTest(
		"Inside cylinder intersect side",
		Test_RayCylinderIntersect(vec3(0.25,0,0), vec3(1,0,0), DOUBLE_MATRIX),
		0.75);

	RunTest(
		"Inside cylinder intersect cap",
		Test_RayCylinderIntersect(vec3(0.25,0,0), vec3(0,-1,0), IDENTITY_MATRIX),
		0.5);

	RunTest(
		"Nonuniformly scaled cylinder",
		Test_RayCylinderIntersect(vec3(0.1,0.2,0.3), normalize(vec3(5,8,3)), SCALE_MATRIX),
		0.26);
	*/
}

void ReportTest(std::string name, bool result)
{
    std::cout << std::setfill('.') << std::setw(50) << std::left << name;
    std::cout << (result ? "SUCCESS" : "**FAILURE**") << std::endl;
    g_numTests++;
    if (result) {
        g_numSuccessful++;
    } else {
        // It can be very useful to put a breakpoint here
        return;
    }
}
